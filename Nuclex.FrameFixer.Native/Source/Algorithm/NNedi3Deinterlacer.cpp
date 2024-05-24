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

#include "./NNedi3Deinterlacer.h"
#include "./PreviewDeinterlacer.h"

#include <Nuclex/Support/Text/LexicalAppend.h>

#include <vector> // for std::vector
#include <stdexcept> // for std::bad_alloc
#include <memory> // for std::unqiue_ptr

#include "./../Platform/LibAvApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception for the specified libav result code</summary>
  /// <param name="libavResult">
  ///   libav result code for which an error message will be provided in the exception
  /// </param>
  /// <param name="message">
  ///   Additional text that will be prefixed to the exception message
  /// </param>
  [[noreturn]] void throwExceptionForAvError(int libAvResult, const std::string &message) {
    char buffer[1024];
    int errorStringResult = ::av_strerror(libAvResult, buffer, sizeof(buffer));

    std::string combinedMessage(message);
    if(errorStringResult == 0) {
      combinedMessage.append(buffer);
    } else {
      combinedMessage.append(u8"unknown error ", 14);
      Nuclex::Support::Text::lexical_append(combinedMessage, libAvResult);
    }

    throw std::runtime_error(combinedMessage);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Sets up a complete AV filter graph that runs the NNedi3 deinterlace filter
  /// </summary>
  /// <param name="frameWidth">Width of the video frames in pixels</param>
  /// <param name="frameHeight">Height of the video frames in pixels</param>
  /// <param name="pixelFormat">Pixel format of the video frames</param>
  /// <param name="topField">
  ///   Whether to keep the top field (otherwise keeps the bottom field)
  /// </param>
  /// <param name="usePriorField">Whether to take data from the preceding field</param>
  /// <returns>
  ///   The new libav filter graph. The input filter context is of type
  ///   &quot;buffer&quot; and named &quot;in&quot;, the output filter context
  ///   is of type &quot;buffersink&qout; and named &quot;out&quot;.
  /// </returns>
  std::shared_ptr<::AVFilterGraph> setupNnediFilterGraph(
    std::size_t frameWidth,
    std::size_t frameHeight,
    std::size_t pixelFormat,
    bool topField = true,
    bool usePriorField = false
  ) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    std::shared_ptr<::AVFilterGraph> filterGraph = LibAvApi::NewAvFilterGraph();

    // Parameters that will be passed to the "buffer" filter context which
    // will make out input frame available to the NNedi filter.
    std::string inputBufferArguments(u8"video_size=", 11);
    Nuclex::Support::Text::lexical_append(inputBufferArguments, frameWidth);
    inputBufferArguments.push_back(u8'x');
    Nuclex::Support::Text::lexical_append(inputBufferArguments, frameHeight);
    inputBufferArguments.append(u8":pix_fmt=", 9);
    Nuclex::Support::Text::lexical_append(inputBufferArguments, pixelFormat);
    inputBufferArguments.append(u8":time_base=30000/1001", 21);
    inputBufferArguments.append(u8":pixel_aspect=16/9", 18);

    // Parameters for the NNedi filter. We'll try to configure it for maximum
    // quality and force it to process only the field the user desired.
    std::string nnediArguments(u8"weights='/home/cygon/nnedi3_weights.bin'", 40);
    nnediArguments.append(":deint=all", 10); // deinterlace regardless of frame state
    nnediArguments.append(":qual=slow", 10); // use highest quality
    nnediArguments.append(":pscrn=none", 11); // disable prescreener (the human is prescreener)
    nnediArguments.append(":nsize=s48x6", 12); // window the predictor network is working on
    nnediArguments.append(":nns=n256", 9); // complexity of the predictor network
    if(topField) {
      //::av_opt_set(nnediFilterContext, u8"field", u8"top", AV_OPT_SEARCH_CHILDREN);
      if(usePriorField) {
        nnediArguments.append(":field=tf", 9);
      } else {
        nnediArguments.append(":field=t", 8);
      }
    } else {
      //::av_opt_set(nnediFilterContext, u8"field", u8"bottom", AV_OPT_SEARCH_CHILDREN);
      if(usePriorField) {
        nnediArguments.append(":field=bf", 8);
      } else {
        nnediArguments.append(":field=b", 8);
      }
    }

    // Create the filter contexts that will be linked together
    ::AVFilterContext *inputFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffer"),
      u8"in",
      inputBufferArguments
    );
    ::AVFilterContext *nnediFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"nnedi"),
      u8"deinterlace",
      nnediArguments
    );
    ::AVFilterContext *outputFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffersink"),
      u8"out"
    );

    // Now build a pipeline using the three filter contexts by connection
    // their output pads to the input pads of the filter contexts following them
    LibAvApi::LinkAvFilterContexts(inputFilterContext, nnediFilterContext);
    LibAvApi::LinkAvFilterContexts(nnediFilterContext, outputFilterContext);

    // Unclear what this does. I assume it verifies and pre-loads resources.
    LibAvApi::ConfigureAvFilterGraph(filterGraph);

    return filterGraph;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV frame</summary>
  /// <param name="image">QImage of which an AV frame will be constructed</param>
  /// <returns>A new AV frame with the same dimensions and contents as the QImage</returns>
  std::shared_ptr<::AVFrame> newAvFrameFromQImage(const QImage &image) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    std::shared_ptr<::AVFrame> frame = LibAvApi::NewAvFrame();

    frame->width = image.width();
    frame->height = image.height();
    
    // TODO: Cheap and insufficient decision between 16 bits per color channel
    //       and 8 bits per color channel. I only have the former kind of images
    //       currently, but this should compare the actual pixel formats!
    if(image.bytesPerLine() >= image.width() * 8) {
      frame->format = AV_PIX_FMT_RGBA64LE; // AV_PIX_FMT_BGR48LE

      LibAvApi::LockAvFrameBuffer(frame);

      std::uint8_t *frameData = frame->data[0];
      for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgba64 (64 bit interleaved) pixels
          image.bytesPerLine(),
          frameData
        );
        frameData += frame->linesize[0];
      }
    } else {
      frame->format = AV_PIX_FMT_RGBA; // AV_PIX_FMT_ABGR;

      LibAvApi::LockAvFrameBuffer(frame);

      std::uint8_t *frameData = frame->data[0];
      for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgb (32 bit interleaved) pixels
          image.bytesPerLine(),
          frameData
        );
        frameData += frame->linesize[0];
      }
    }

    return frame;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV frame</summary>
  /// <param name="image">QImage of which an AV frame will be constructed</param>
  /// <returns>A new AV frame with the same dimensions and contents as the QImage</returns>
  void copyAvFrameToQImage(const std::shared_ptr<::AVFrame> &frame, QImage &image) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    bool dimensionsMatch = (
      (frame->width == image.width()) &&
      (frame->height == image.height())
    );
    if(!dimensionsMatch) {
      throw std::runtime_error(u8"Processed AV frame has different dimensions from QImage");
    }

    // TODO: Cheap and insufficient decision between 16 bits per color channel
    //       and 8 bits per color channel. I only have the former kind of images
    //       currently, but this should compare the actual pixel formats!
    if(image.bytesPerLine() >= image.width() * 8) {
      if(frame->data[0] == nullptr) {
        LibAvApi::LockAvFrameBuffer(frame);
      }
      if(frame->format == AV_PIX_FMT_GBRAP16LE) { // planar output (NNedi does this)
        const std::uint8_t *greenChannel = frame->data[0];
        const std::uint8_t *blueChannel = frame->data[1];
        const std::uint8_t *redChannel = frame->data[2];
        const std::uint8_t *alphaChannel = frame->data[3];

        for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
          QRgba64 *targetScanline = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          for(std::size_t pixelIndex = 0; pixelIndex < frame->width; ++pixelIndex) {
            targetScanline[pixelIndex] = QRgba64::fromRgba64(
              reinterpret_cast<const quint16 *>(redChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(greenChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(blueChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(alphaChannel)[pixelIndex]
            );
          }

          targetScanline += image.bytesPerLine();
          greenChannel += frame->linesize[0];
          blueChannel += frame->linesize[1];
          redChannel += frame->linesize[2];
          alphaChannel += frame->linesize[3];
        }
      } else if(frame->format == AV_PIX_FMT_RGBA64LE) { // AV_PIX_FMT_BGR48LE
        const std::uint8_t *frameData = frame->data[0];
        for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
          std::copy_n(
            frameData,
            frame->linesize[0],
            image.scanLine(lineIndex) // Will be QRgba64 (64 bit interleaved) pixels
          );
          frameData += frame->linesize[0];
        }
      } else {
        throw std::runtime_error(u8"Processed AV frame has different pixel format from QImage");
      }
    } else {
      if(frame->format != AV_PIX_FMT_RGBA) { // AV_PIX_FMT_ABGR;
        throw std::runtime_error(u8"Processed AV frame has different pixel format from QImage");
      }
      if(frame->data[0] == nullptr) {
        LibAvApi::LockAvFrameBuffer(frame);
      }

      const std::uint8_t *frameData = frame->data[0];
      for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
        std::copy_n(
          frameData,
          frame->linesize[0],
          image.scanLine(lineIndex) // Will be QRgb (32 bit interleaved) pixels
        );
        frameData += frame->linesize[0];
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  NNedi3Deinterlacer::NNedi3Deinterlacer() :
    topFieldOnlyNnediFilterGraph(),
    bottomFieldOnlyNnediFilterGraph(),
    topFieldFirstNnediFilterGraph(),
    bottomFieldFirstNnediFilterGraph(),
    filterGraphWidth(std::size_t(-1)),
    filterGraphHeight(std::size_t(-1)),
    filterGraphSixtyFourBitsPerPixel(false) {}

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::WarmUp() {
    // TODO: Check for and download nnedi3_weights.bin if needed?
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::CoolDown() {
    this->topFieldOnlyNnediFilterGraph.reset();
    this->bottomFieldOnlyNnediFilterGraph.reset();
    this->topFieldFirstNnediFilterGraph.reset();
    this->bottomFieldFirstNnediFilterGraph.reset();

    this->filterGraphWidth = std::size_t(-1);
    this->filterGraphHeight = std::size_t(-1);
    this->filterGraphSixtyFourBitsPerPixel = false;
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::SetPriorFrame(const QImage &priorFrame) {
    this->priorFrame = priorFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
    using Platform::LibAvApi;

    if(mode != DeinterlaceMode::Dont) {

      // Make sure the filter graph has been created an grab a reference to it
      std::shared_ptr<::AVFilterGraph> nnediFilterGraph;
      {
        bool isSixtyFourBitsPerPixel = (target.bytesPerLine() >= target.width() * 8);
        if(mode == DeinterlaceMode::TopFieldFirst) {
          ensureTopFieldFirstFilterGraphCreated(
            target.width(), target.height(), isSixtyFourBitsPerPixel
          );
          nnediFilterGraph = this->topFieldFirstNnediFilterGraph;
        } else if(mode == DeinterlaceMode::BottomFieldFirst) {
          ensureBottomFieldFirstFilterGraphCreated(
            target.width(), target.height(), isSixtyFourBitsPerPixel
          );
          nnediFilterGraph = this->bottomFieldFirstNnediFilterGraph;
        } else if(mode == DeinterlaceMode::TopFieldOnly) {
          ensureTopFieldOnlyFilterGraphCreated(
            target.width(), target.height(), isSixtyFourBitsPerPixel
          );
          nnediFilterGraph = this->topFieldOnlyNnediFilterGraph;
        } else if(mode == DeinterlaceMode::BottomFieldOnly) {
          ensureBottomFieldOnlyFilterGraphCreated(
            target.width(), target.height(), isSixtyFourBitsPerPixel
          );
          nnediFilterGraph = this->bottomFieldOnlyNnediFilterGraph;
        }
      }

      // NNedi requires two frames. Re-feeding the same AV frame instance does
      // not work, so we'll construct two independent frames
      std::shared_ptr<::AVFrame> priorFrame;
      if(this->priorFrame.isNull()) {
        priorFrame = newAvFrameFromQImage(this->priorFrame);
      } else {
        priorFrame = newAvFrameFromQImage(target);
      }
      std::shared_ptr<::AVFrame> inputFrame = newAvFrameFromQImage(target);
      priorFrame->interlaced_frame = 1;
      inputFrame->interlaced_frame = 1;
      if(mode == DeinterlaceMode::TopFieldFirst) {
        priorFrame->top_field_first = 0;
        inputFrame->top_field_first = 1;
      } else {
        priorFrame->top_field_first = 1;
        inputFrame->top_field_first = 0;
      }

      // Put both frames into the filter graph's input buffer
      LibAvApi::PushFrameIntoFilterGraph(nnediFilterGraph, priorFrame);
      LibAvApi::PushFrameIntoFilterGraph(nnediFilterGraph, inputFrame);

      // Finally, read a frame out of the filter graph. I'm not sure if
      // the filter graph starts executing as soon as there is work that can
      // be done or if this triggers it, anyway, the processed frame comes out here.
      std::shared_ptr<::AVFrame> outputFrame = LibAvApi::ReadFrameFromFilterGraph(nnediFilterGraph);

      // Finally, put the processed frame back into the QImage
      copyAvFrameToQImage(outputFrame, target);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureTopFieldOnlyFilterGraphCreated(
    std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height) ||
      (this->filterGraphSixtyFourBitsPerPixel != sixtyFourBitsPerPixel)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->topFieldOnlyNnediFilterGraph)) {
      this->topFieldOnlyNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, true, false // AV_PIX_FMT_BGR48LE == 58;
      );
      this->filterGraphWidth = width;
      this->filterGraphHeight = height;
      this->filterGraphSixtyFourBitsPerPixel = sixtyFourBitsPerPixel;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureBottomFieldOnlyFilterGraphCreated(
    std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height) ||
      (this->filterGraphSixtyFourBitsPerPixel != sixtyFourBitsPerPixel)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->bottomFieldOnlyNnediFilterGraph)) {
      this->bottomFieldOnlyNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, false, false // AV_PIX_FMT_BGR48LE == 58;
      );
      this->filterGraphWidth = width;
      this->filterGraphHeight = height;
      this->filterGraphSixtyFourBitsPerPixel = sixtyFourBitsPerPixel;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureTopFieldFirstFilterGraphCreated(
    std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height) ||
      (this->filterGraphSixtyFourBitsPerPixel != sixtyFourBitsPerPixel)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->topFieldFirstNnediFilterGraph)) {
      this->topFieldFirstNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, true, true // AV_PIX_FMT_BGR48LE == 58;
      );
      this->filterGraphWidth = width;
      this->filterGraphHeight = height;
      this->filterGraphSixtyFourBitsPerPixel = sixtyFourBitsPerPixel;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureBottomFieldFirstFilterGraphCreated(
    std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height) ||
      (this->filterGraphSixtyFourBitsPerPixel != sixtyFourBitsPerPixel)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->bottomFieldFirstNnediFilterGraph)) {
      this->bottomFieldFirstNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, false, true // AV_PIX_FMT_BGR48LE == 58;
      );
      this->filterGraphWidth = width;
      this->filterGraphHeight = height;
      this->filterGraphSixtyFourBitsPerPixel = sixtyFourBitsPerPixel;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm
