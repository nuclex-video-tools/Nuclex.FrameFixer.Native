#pragma region CPL License
/*
Nuclex FrameFixer
Copyright (C) 2024 Nuclex Development Labs

This application is free software; you can redistribute it and/or modify it
under the terms of the IBM Common Public License as published by
the IBM Corporation; either version 1.0 of the License,
or (at your option) any later version.

This application is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the IBM Common Public License
for more details.

You should have received a copy of the IBM Common Public License
along with this library
*/
#pragma endregion // CPL License

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./Exporter.h"
#include "./Model/Movie.h"
#include "./Algorithm/PreviewDeinterlacer.h"
#include "./Algorithm/Averager.h"

#include <Nuclex/Support/Text/LexicalCast.h>

#include <QPixmap>

namespace {

  // ------------------------------------------------------------------------------------------- //

  Nuclex::FrameFixer::FrameType getFrameType(
    const Nuclex::FrameFixer::Frame &frame, bool flip =false
  ) {
    using Nuclex::FrameFixer::FrameType;

    FrameType frameType = frame.Type;
    if(frameType == FrameType::Unknown) {
      frameType = frame.ProvisionalType;
    }

    if(flip) {
      switch(frameType) {
        case FrameType::TopFieldFirst: { frameType = FrameType::BottomFieldFirst; break; }
        case FrameType::BottomFieldFirst: { frameType = FrameType::TopFieldFirst; break; }
        case FrameType::TopFieldOnly: { frameType = FrameType::BottomFieldOnly; break; }
        case FrameType::BottomFieldOnly: { frameType = FrameType::TopFieldOnly; break; }
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

  Exporter::Exporter() :
    deinterlacer(std::make_shared<Algorithm::PreviewDeinterlacer>()),
    inputFrameRange(),
    outputFrameRange(),
    flipFields(false) {}

  // ------------------------------------------------------------------------------------------- //

  Exporter::~Exporter() {}

  // ------------------------------------------------------------------------------------------- //

  void Exporter::SetDeinterlacer(
    const std::shared_ptr<Algorithm::Deinterlacer> &deinterlacer
  ) {
    this->deinterlacer = deinterlacer;
  }

  // ------------------------------------------------------------------------------------------- //

  void Exporter::FlipTopAndBottomField(bool flip /* = true */) {
    this->flipFields = flip;
  }

  // ------------------------------------------------------------------------------------------- //

  void Exporter::RestrictRangeOfInputFrames(
    std::size_t startFrameIndex, std::size_t endFrameIndex
  ) {
    this->inputFrameRange = std::pair<std::size_t, std::size_t>(
      startFrameIndex, endFrameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void Exporter::RestrictRangeOfOutputFrames(
    std::size_t startFrameIndex, std::size_t endFrameIndex
  ) {
    this->inputFrameRange = std::pair<std::size_t, std::size_t>(
      startFrameIndex, endFrameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void Exporter::Export(const std::shared_ptr<Movie> &movie, const std::string &directory) {
    std::size_t outputFrameIndex = 0;

    bool needsNextFrame = this->deinterlacer->NeedsNextFrame();
    bool needsPriorImage = this->deinterlacer->NeedsPriorFrame();
    
    QImage priorImage, currentImage, nextImage;
    std::vector<QImage> imagesToAverage;
    FrameType firstImageToAverageType = FrameType::Unknown;

    std::size_t frameCount = movie->Frames.size();
    for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
      const Frame &currentFrame = movie->Frames[frameIndex];
      FrameType currentFrameType = getFrameType(currentFrame, this->flipFields);

      // Figure out if we're still far from the export range. If so, do quick skip mode.
      bool skip = false;
      if(this->inputFrameRange.has_value()) {
        skip = (frameIndex + 10) < this->inputFrameRange.value().first;
      }
      if(this->outputFrameRange.has_value()) {
        skip |= (outputFrameIndex + 10) < this->outputFrameRange.value().first;
      }
      if(skip) {
        switch(currentFrameType) {
          case FrameType::Discard: { break; }
          case FrameType::Duplicate: { outputFrameIndex += 2; break; }
          case FrameType::Triplicate: { outputFrameIndex += 3; break; }
          default: { ++outputFrameIndex; break; }
        }
        continue; // Skip procesing and loop here!
      }

      // If the frame type is set to 'average', queue the image up as an
      // averaging sample
      if(currentFrameType == FrameType::Average) {
        std::string imagePath = movie->GetFramePath(frameIndex);
        imagesToAverage.emplace_back(QString::fromStdString(imagePath));

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
        if(firstImageToAverageType == FrameType::Triplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        } else if(firstImageToAverageType == FrameType::Duplicate) {
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
        for(std::size_t index = 0; index < imagesToAverage.size(); ++index) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
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
      } else {
        std::string imagePath = movie->GetFramePath(frameIndex);
        currentImage.load(QString::fromStdString(imagePath));
      }
      if(needsNextFrame && (frameIndex < frameCount)) {
        std::string imagePath = movie->GetFramePath(frameIndex + 1);
        nextImage.load(QString::fromStdString(imagePath));
      } else if(!nextImage.isNull()) {
        QImage emptyImage;
        nextImage.swap(emptyImage);
      }

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

      if(currentFrameType == FrameType::TopFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldFirst);
      } else if(currentFrameType == FrameType::BottomFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldFirst);
      } else if(currentFrameType == FrameType::TopFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldOnly);
      } else if(currentFrameType == FrameType::BottomFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldOnly);
      }

      // Figure out if the frame that follows uses averaging
      bool nextImageUsesAveraging = false;
      if((frameIndex + 1) < frameCount) {
        if(movie->Frames[frameIndex + 1].Type == FrameType::Average) {
          nextImageUsesAveraging = true;
        } else if(movie->Frames[frameIndex + 1].Type == FrameType::Unknown) {
          if(movie->Frames[frameIndex + 1].ProvisionalType == FrameType::Average) {
            nextImageUsesAveraging = true;
          }
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
        if(currentFrameType == FrameType::Triplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        } else if(currentFrameType == FrameType::Duplicate) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
        }
        if(currentFrameType != FrameType::Discard) {
          saveImage(
            currentImage, directory, frameIndex, outputFrameIndex++,
            this->inputFrameRange, this->outputFrameRange
          );
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

  QImage Exporter::Preview(const std::shared_ptr<Movie> &movie, const std::size_t frameIndex) {
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

    FrameType currentFrameType = movie->Frames[frameIndex].Type;
    if(currentFrameType == FrameType::Unknown) {
      currentFrameType = movie->Frames[frameIndex].ProvisionalType;
    }

    if(this->flipFields) {
      if(currentFrameType == FrameType::TopFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldFirst);
      } else if(currentFrameType == FrameType::BottomFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldFirst);
      } else if(currentFrameType == FrameType::TopFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldOnly);
      } else if(currentFrameType == FrameType::BottomFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldOnly);
      }
    } else {
      if(currentFrameType == FrameType::TopFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldFirst);
      } else if(currentFrameType == FrameType::BottomFieldFirst) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldFirst);
      } else if(currentFrameType == FrameType::TopFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::TopFieldOnly);
      } else if(currentFrameType == FrameType::BottomFieldOnly) {
        this->deinterlacer->Deinterlace(currentImage, Algorithm::DeinterlaceMode::BottomFieldOnly);
      }
    }

    return currentImage;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
