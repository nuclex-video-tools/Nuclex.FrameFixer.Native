#pragma region Apache License 2.0
/*
Nuclex Frame Fixer
Copyright (C) 2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./Renderer.h"

#include "./Model/Movie.h"
#include "./Algorithm/Deinterlacing/Deinterlacer.h"
#include "./Algorithm/Interpolation/FrameInterpolator.h"
#include "./Algorithm/Averager.h"

#include <Nuclex/Support/Text/LexicalCast.h>

#include <QPixmap>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the final frame type as which a frame will be processed</summary>
  /// <param name="frame">Frame whose type will be determined</param>
  /// <param name="flip">Whether the flip field order option is turned on</param>
  /// <returns>The frame type of the specified frame</returns>
  Nuclex::FrameFixer::FrameAction getFrameType(
    const Nuclex::FrameFixer::Frame &frame, bool flip = false
  ) {
    using Nuclex::FrameFixer::FrameAction;

    // Use the assigned frame type. If none was assigned, use the determined frame type
    // which is calculated by other parts of the application by either detecting combing
    // patterns or repeating the most recent 5-frame cycle.
    FrameAction frameType = frame.Action;
    if(frameType == FrameAction::Unknown) {
      // TODO: Re-enable once the DeinterlaceMode split is complete
      //frameType = frame.ProvisionalMode;
    }

    // Swap top and bottom field enum values if the field order is set to flipped.
    if(flip) {
      switch(frameType) {
        case FrameAction::TopFieldFirst: { frameType = FrameAction::BottomFieldFirst; break; }
        case FrameAction::BottomFieldFirst: { frameType = FrameAction::TopFieldFirst; break; }
        case FrameAction::TopFieldOnly: { frameType = FrameAction::BottomFieldOnly; break; }
        case FrameAction::BottomFieldOnly: { frameType = FrameAction::TopFieldOnly; break; }
        default: { break; }
      }
    }

    return frameType;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Saves a frame as a PNG in the target directory if conditions are fulfilled
  /// </summary>
  /// <param name="image">Image containing the pixels that will potentially be saved</param>
  /// <param name="directory">Directory in which the frame will be saved as a PNG</param>
  /// <param name="inputFrameIndex">
  ///   Index of the source frame that produced this outputframe
  /// </param>
  /// <param name="outputFrameIndex">Index of the output frame</param>
  /// <param name="inputFrameRange">Optional range of input frames that will be saved</param>
  /// <param name="outputFrameRange">Optional range of output frames that will be saved</param>
  void saveImage(
    QImage &image,
    const std::string &directory,
    std::size_t inputFrameIndex,
    std::size_t outputFrameIndex,
    const std::optional<std::pair<std::size_t, std::size_t>> &inputFrameRange,
    const std::optional<std::pair<std::size_t, std::size_t>> &outputFrameRange
  ) {

    // If the user limited the export by an input frame range,
    // only write the file if the input frame index is within that range
    if(inputFrameRange.has_value()) {
      bool isInRange = (
        (inputFrameIndex >= inputFrameRange.value().first) &&
        (inputFrameIndex < inputFrameRange.value().second)
      );
      if(!isInRange) {
        return;
      }
    }

    // If the user limited the export by an input frame range,
    // only write the file if the input frame index is within that range
    if(outputFrameRange.has_value()) {
      bool isInRange = (
        (outputFrameIndex >= outputFrameRange.value().first) &&
        (outputFrameIndex < outputFrameRange.value().second)
      );
      if(!isInRange) {
        return;
      }
    }

    std::string path;
    {
      std::string::size_type length = directory.length();
      if((length >= 1) && (directory[length - 1] != '/')) {
        path = directory + u8"/";
      } else {
        path = directory;
      }

      std::string filename = Nuclex::Support::Text::lexical_cast<std::string>(outputFrameIndex);
      for(std::size_t index = filename.length(); index < 8; ++index) {
        path.push_back(u8'0');
      }
    
      path.append(filename);
      path.append(u8".png");
    }

    image.save(QString::fromStdString(path), u8"PNG");

  }

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  Renderer::Renderer() :
    deinterlacer(),
    inputFrameRange(),
    outputFrameRange(),
    flipFields(false),
    collapseAverageFrames(false),
    completedFrameCount(0) {}

  // ------------------------------------------------------------------------------------------- //

  Renderer::~Renderer() {}

  // ------------------------------------------------------------------------------------------- //

  void Renderer::SetDeinterlacer(
    const std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> &deinterlacer
  ) {
    this->deinterlacer = deinterlacer;
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::SetInterpolator(
    const std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> &interpolator
  ) {
    this->interpolator = interpolator;
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::FlipTopAndBottomField(bool flip /* = true */) {
    this->flipFields = flip;
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::CollapseAverageFrames(bool collapse /* = true */) {
    this->collapseAverageFrames = collapse;
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::RestrictRangeOfInputFrames(
    std::size_t startFrameIndex, std::size_t endFrameIndex
  ) {
    this->inputFrameRange = std::pair<std::size_t, std::size_t>(
      startFrameIndex, endFrameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::RestrictRangeOfOutputFrames(
    std::size_t startFrameIndex, std::size_t endFrameIndex
  ) {
    this->outputFrameRange = std::pair<std::size_t, std::size_t>(
      startFrameIndex, endFrameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t Renderer::GetTotalFrameCount(
    const std::shared_ptr<Movie> &movie
  ) const {
    std::size_t outputFrameIndex = 1;
    std::size_t totalFrameCount = 0;

    std::size_t frameCount = movie->Frames.size();
    for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
      const Frame &currentFrame = movie->Frames[frameIndex];
      FrameAction currentFrameType = getFrameType(currentFrame, this->flipFields);

      bool render = true;
      if(this->inputFrameRange.has_value()) {
        render &= (
          (frameIndex >= this->inputFrameRange.value().first) &&
          (frameIndex < this->inputFrameRange.value().second)
        );
      }
      if(this->outputFrameRange.has_value()) {
        render &= (
          (outputFrameIndex >= this->outputFrameRange.value().first) &&
          (outputFrameIndex < this->outputFrameRange.value().second)
        );
      }

      switch(currentFrameType) {
        case FrameAction::Discard: { break; }
        case FrameAction::Duplicate: {
          outputFrameIndex += 2;
          if(render) {
            totalFrameCount += 2;
          }
          break;
        }
        case FrameAction::Triplicate: {
          outputFrameIndex += 3;
          if(render) {
            totalFrameCount += 3;
          }
          break;
        }
        default: {
          ++outputFrameIndex;
          if(render) {
            ++totalFrameCount;
          }
          break;
        }
      }
      if(currentFrame.AlsoInsertInterpolatedAfter.has_value()) {
        if(currentFrame.AlsoInsertInterpolatedAfter.value()) {
          ++outputFrameIndex;
          if(render) {
            ++totalFrameCount;
          }
        }
      }
    }

    return totalFrameCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void Renderer::Render(
    const std::shared_ptr<Movie> &movie,
    const std::string &directory,
    const std::shared_ptr<const Nuclex::Platform::Tasks::CancellationWatcher> &canceller /* = (
      std::shared_ptr<Nuclex::Platform::Tasks::CancellationWatcher>()
    ) */
  ) {
    std::size_t outputFrameIndex = 1;
    bool needsNextFrame = this->deinterlacer->NeedsNextFrame();
    bool needsPriorImage = this->deinterlacer->NeedsPriorFrame();

    QImage lastInterpolatedImage;
    std::size_t lastInterpolationPriorIndex = std::size_t(-1);
    std::size_t lastInterpolationAfterIndex = std::size_t(-1);
    
    QImage priorImage, currentImage, nextImage;
    std::vector<QImage> imagesToAverage;
    FrameAction firstImageToAverageType = FrameAction::Unknown;

    std::size_t frameCount = movie->Frames.size();
    for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
      const Frame &currentFrame = movie->Frames[frameIndex];
      FrameAction currentFrameType = getFrameType(currentFrame, this->flipFields);

      // Figure out if we're still far from the export range. If so, do quick skip mode.
      // TODO: this is currently running in danger of entering a long averaging block without
      //       a start frame. Scan for the start frame before or just assume nobody will
      //       find more than 10 exactly equal frames in a row to average?
      bool skip = false;
      if(this->inputFrameRange.has_value()) {
        skip = (frameIndex + 10) < this->inputFrameRange.value().first;
      }
      if(this->outputFrameRange.has_value()) {
        skip |= (outputFrameIndex + 10) < this->outputFrameRange.value().first;
      }
      if(skip) {
        switch(currentFrameType) {
          case FrameAction::Discard: { break; }
          case FrameAction::Duplicate: { outputFrameIndex += 2; break; }
          case FrameAction::Triplicate: { outputFrameIndex += 3; break; }
          default: { ++outputFrameIndex; break; }
        }
        if(currentFrame.AlsoInsertInterpolatedAfter.has_value()) {
          if(currentFrame.AlsoInsertInterpolatedAfter.value()) {
            ++outputFrameIndex;
          }
        }
        continue; // Skip processing and loop here!
      }

      if(this->outputFrameRange.has_value()) {
        if(outputFrameIndex > this->outputFrameRange.value().first) {
          this->completedFrameCount.store(
            outputFrameIndex - this->outputFrameRange.value().first,
            std::memory_order::memory_order_release
          );
        }
      } else {
        this->completedFrameCount.store(
          outputFrameIndex, std::memory_order::memory_order_release
        );
      }
      canceller->ThrowIfCanceled();

      // If the frame type is 'average', queue the image up as an averaging sample
      if(currentFrameType == FrameAction::Average) {
        if(nextImage.isNull()) {
          std::string imagePath = movie->GetFramePath(frameIndex);
          imagesToAverage.emplace_back(QString::fromStdString(imagePath));
        } else {
          nextImage.swap(imagesToAverage.emplace_back());
        }

        bool isLastFrame = ((frameIndex + 1) >= frameCount);
        if(this->inputFrameRange.has_value()) {
          isLastFrame |= ((frameIndex + 1) >= this->inputFrameRange.value().second);
        }

        // Just keep collecting all frames tagged for the averaging block, unless we
        // reach the end of the movie, in which case we need to flush it now.
        if(!isLastFrame) {
          continue;
        }
      }
      if(imagesToAverage.size() >= 1) {
        Averager::Average(currentImage, imagesToAverage);

        // First, save the very first frame prior to the one or more frames tagged
        // for averaging. Save it to multiple outputs if it is tagged for duplication.
        if(firstImageToAverageType == FrameAction::Triplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        } else if(firstImageToAverageType == FrameAction::Duplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        }
        saveImage(
          currentImage, directory, frameIndex, outputFrameIndex++,
          this->inputFrameRange, this->outputFrameRange
        );

        // Each frame that was tagged to be averaged also needs to result in
        // one output image being saved each.
        if(!this->collapseAverageFrames) {
          for(std::size_t index = 0; index < imagesToAverage.size(); ++index) {
            saveImage(
              currentImage, directory, frameIndex, outputFrameIndex++,
              this->inputFrameRange, this->outputFrameRange
            );
          }
        }

        imagesToAverage.clear();

        // Continue processing the current (yet to be loaded) frame. It was not part
        // of theaveraging chain (in fact, it broke it), so it is processed normally.
      }

      // If the deinterlacer requires a prior frame and we have a current frame
      // in the pipeline, let it become the previous came.
      if(needsPriorImage) {
        if(!currentImage.isNull()) {
          currentImage.swap(priorImage);
        }
      }

      // If we have a next image loaded (for whatever reason), make it become
      // the current image. Otherwise, load the file for the current frame.
      if(!nextImage.isNull()) {
        nextImage.swap(currentImage);
      } else if(movie->Frames[frameIndex].LeftOrReplacementIndex.has_value()) {
        std::string imagePath = movie->GetFramePath(
          movie->Frames[frameIndex].LeftOrReplacementIndex.value()
        );
        currentImage.load(QString::fromStdString(imagePath));
      } else {
        std::string imagePath = movie->GetFramePath(frameIndex);
        currentImage.load(QString::fromStdString(imagePath));
      }

      // If the deinterlacer needs a next frame, also load the image that
      // follows the current one
      if(needsNextFrame && ((frameIndex + 1) < frameCount)) {
        if(movie->Frames[frameIndex + 1].LeftOrReplacementIndex.has_value()) {
          std::string imagePath = movie->GetFramePath(
            movie->Frames[frameIndex + 1].LeftOrReplacementIndex.value()
          );
          nextImage.load(QString::fromStdString(imagePath));
        } else {
          std::string imagePath = movie->GetFramePath(frameIndex + 1);
          nextImage.load(QString::fromStdString(imagePath));
        }
      } else if(!nextImage.isNull()) {
        QImage emptyImage;
        nextImage.swap(emptyImage);
      }

      // Now give the deinterlacer the images it needs to work
      if(needsPriorImage) {
        if(priorImage.isNull()) {
          this->deinterlacer->SetPriorFrame(currentImage);
        } else {
          this->deinterlacer->SetPriorFrame(priorImage);
        }
      }
      if(needsNextFrame) {
        if(nextImage.isNull()) {
          this->deinterlacer->SetNextFrame(currentImage);
        } else {
          this->deinterlacer->SetNextFrame(nextImage);
        }
      }

      if(currentFrameType == FrameAction::TopFieldFirst) {
        this->deinterlacer->Deinterlace(
          currentImage, DeinterlaceMode::TopFieldFirst
        );
      } else if(currentFrameType == FrameAction::BottomFieldFirst) {
        this->deinterlacer->Deinterlace(
          currentImage, DeinterlaceMode::BottomFieldFirst
        );
      } else if(currentFrameType == FrameAction::TopFieldOnly) {
        this->deinterlacer->Deinterlace(
          currentImage, DeinterlaceMode::TopFieldOnly
        );
      } else if(currentFrameType == FrameAction::BottomFieldOnly) {
        this->deinterlacer->Deinterlace(
          currentImage, DeinterlaceMode::BottomFieldOnly
        );
      } else if(currentFrameType == FrameAction::Interpolate) {
        if(static_cast<bool>(this->interpolator)) {
          if(this->interpolator->CanInterpolateMiddleFrame()) {
            std::pair<std::size_t, std::size_t> sourceIndices = (
              movie->Frames[frameIndex].InterpolationSourceIndices.value()
            );
            bool alreadyInterpolated = (
              (!lastInterpolatedImage.isNull()) &&
              (sourceIndices.first == lastInterpolationPriorIndex) &&
              (sourceIndices.second == lastInterpolationAfterIndex)
            );
            if(!alreadyInterpolated) {
              std::string imagePath = movie->GetFramePath(sourceIndices.first);
              QImage prior(QString::fromStdString(imagePath));

              imagePath = movie->GetFramePath(sourceIndices.second);
              QImage after(QString::fromStdString(imagePath));
              
              QImage interpolatedImage = this->interpolator->Interpolate(prior, after);
              lastInterpolatedImage.swap(interpolatedImage);
              lastInterpolationPriorIndex = sourceIndices.first;
              lastInterpolationAfterIndex = sourceIndices.second;
            }

            currentImage = lastInterpolatedImage.copy();
          }
        }
      }

      // Figure out if the frame that follows uses averaging
      bool nextImageUsesAveraging = false;
      if((frameIndex + 1) < frameCount) {
        if(movie->Frames[frameIndex + 1].Action == FrameAction::Average) {
          nextImageUsesAveraging = true;
        }
      }
      if(this->inputFrameRange.has_value()) {
        nextImageUsesAveraging &= ((frameIndex + 1) < this->inputFrameRange.value().second);
      }

      // If the next image is tagged for averaging, don't save the image,
      // we'll have to average it first with one or more future frames.
      if(nextImageUsesAveraging) {
        firstImageToAverageType = currentFrameType;
      } else { // next image tagged / not tagged for averaging
        if(currentFrameType == FrameAction::Triplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        } else if(currentFrameType == FrameAction::Duplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        }
        if(currentFrameType != FrameAction::Discard) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        }

        if(movie->Frames[frameIndex].AlsoInsertInterpolatedAfter.has_value()) {
          if(movie->Frames[frameIndex].AlsoInsertInterpolatedAfter.value()) {
            QImage tempNextImage = Preview(movie, frameIndex + 1);

            QImage interpolatedImage = this->interpolator->Interpolate(
              currentImage, tempNextImage
            );
            saveImage(
              interpolatedImage, directory, frameIndex, outputFrameIndex++,
              this->inputFrameRange, this->outputFrameRange
            );
          }
        }

        // Stop if we produced all the requested frames
        if(this->inputFrameRange.has_value()) {
          if(frameIndex >= this->inputFrameRange.value().second) {
            break;
          }
        }
        if(this->outputFrameRange.has_value()) {
          if(outputFrameIndex >= this->outputFrameRange.value().second) {
            break;
          }
        }

      } // next image is not tagged for averaging
    } // for frame index from 0 to frame count
  }

  // ------------------------------------------------------------------------------------------- //

  QImage Renderer::Preview(const std::shared_ptr<Movie> &movie, const std::size_t frameIndex) {
    std::string imagePath = movie->GetFramePath(frameIndex);
    QImage currentImage(QString::fromStdString(imagePath));

    QImage priorImage;
    if(this->deinterlacer->NeedsPriorFrame()) {
      if(frameIndex > 0) {
        std::string imagePath = movie->GetFramePath(frameIndex - 1);
        priorImage.load(QString::fromStdString(imagePath));
        this->deinterlacer->SetPriorFrame(priorImage);
      } else {
        this->deinterlacer->SetPriorFrame(currentImage);
      }
    }

    QImage nextImage;
    if(this->deinterlacer->NeedsNextFrame()) {
      if(frameIndex > 0) {
        std::string imagePath = movie->GetFramePath(frameIndex + 1);
        nextImage.load(QString::fromStdString(imagePath));
        this->deinterlacer->SetNextFrame(nextImage);
      } else {
        this->deinterlacer->SetNextFrame(nextImage);
      }
    }

    FrameAction currentFrameType = getFrameType(movie->Frames[frameIndex], this->flipFields);

    if(currentFrameType == FrameAction::TopFieldFirst) {
      this->deinterlacer->Deinterlace(
        currentImage, DeinterlaceMode::TopFieldFirst
      );
    } else if(currentFrameType == FrameAction::BottomFieldFirst) {
      this->deinterlacer->Deinterlace(
        currentImage, DeinterlaceMode::BottomFieldFirst
      );
    } else if(currentFrameType == FrameAction::TopFieldOnly) {
      this->deinterlacer->Deinterlace(
        currentImage, DeinterlaceMode::TopFieldOnly
      );
    } else if(currentFrameType == FrameAction::BottomFieldOnly) {
      this->deinterlacer->Deinterlace(
        currentImage, DeinterlaceMode::BottomFieldOnly
      );
    } else if(currentFrameType == FrameAction::Replace) {
      imagePath = movie->GetFramePath(movie->Frames[frameIndex].LeftOrReplacementIndex.value());
      QImage replacementImage(QString::fromStdString(imagePath));
      currentImage.swap(replacementImage);
    }

    return currentImage;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
