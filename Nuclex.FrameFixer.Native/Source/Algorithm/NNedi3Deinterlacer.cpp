#pragma region CPL License
/*
Nuclex Telecide
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
#define NUCLEX_TELECIDE_SOURCE 1

#include "./NNedi3Deinterlacer.h"
#include "./PreviewDeinterlacer.h"
#include <Nuclex/Support/Text/LexicalAppend.h>

#include <vector> // for std::vector
#include <stdexcept> // for std::bad_alloc
#include <memory> // for std::unqiue_ptr

extern "C" {
  #include <libavfilter/avfilter.h>
  #include <libavutil/opt.h>
  #include <libavfilter/avfilter.h>
  #include <libavfilter/buffersrc.h>
  #include <libavfilter/buffersink.h>
}

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

  /// <summary>Frees an AV filter graph and the filters in it</summary>
  /// <param name="filterGraph">Filter graph that will be freed</param>
  /// <remarks>
  ///   This is a wrapper method so the shared_ptr can call avfilter_graph_free()
  /// </remarks>
  void deleteAvFilterGraph(AVFilterGraph *filterGraph) {
    ::avfilter_graph_free(&filterGraph);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV filter graph</summary>
  /// <returns>A new, empty AV filter graph</returns>
  std::shared_ptr<::AVFilterGraph> newAvFilterGraph() {
    ::AVFilterGraph *newFilterGraph = ::avfilter_graph_alloc();
    if(newFilterGraph == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFilterGraph");
    }

    return std::shared_ptr<::AVFilterGraph>(newFilterGraph, deleteAvFilterGraph);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV filter context in the specified filter graph</summary>
  /// <param name="filterGraph">Filter graph in which the filter context will be created</param>
  /// <param name="filter">Filter for which a context will be created</param>
  /// <param name="name">
  ///   Name of the filter (displayed in errors and useful for later retrieval)
  /// </param>
  /// <param name="arguments">
  ///   Parameters that will be passed to the filter, identical to the filter line that
  ///   can be specified for filters in ffmpeg
  /// </param>
  /// <param name="opaque">User pointer that will be accessible to the filter</param>
  /// <returns>The filter context created in the specified filter graph</returns>
  /// <remarks>
  ///   The filter graph takes ownership of the filter context, so it will remain alive
  ///   for as long as the filter graph exists.
  /// </remarks>
  ::AVFilterContext *newAvFilterContext(
    const std::shared_ptr<::AVFilterGraph> &filterGraph,
    const ::AVFilter *filter,
    const char *name = nullptr,
    const char *arguments = nullptr,
    void *opaque = nullptr
  ) {
    ::AVFilterContext *filterContext = nullptr;

    int result = ::avfilter_graph_create_filter(
      &filterContext,
      filter,
      name,
      arguments,
      opaque,
      filterGraph.get()
    );
    if((result < 0) || (filterContext == nullptr)) {
      throwExceptionForAvError(
        result, std::string(u8"Could not create filter context: ", 33)
      );
    }

    return filterContext;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Links an output to the input pad of onefilter context to the input pad of another
  /// </summary>
  /// <param name="from">AV filter context whose output pad will be linked</param>
  /// <param name="to">AV filter context whose input pad will be linked</param>
  /// <param name="fromOutputPadIndex">Index of the output pad that will be connected</param>
  /// <param name="toInputPadIndex">Index of the input pad that will be connected</param>
  void linkAvFilterContexts(
    ::AVFilterContext *from, ::AVFilterContext *to,
    std::size_t fromOutputPadIndex = 0, std::size_t toInputPadIndex = 0
  ) {
    int result = ::avfilter_link(from, fromOutputPadIndex, to, toInputPadIndex);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not link AV filter contexts: ", 35)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Verifies a completed AV filter graph and prepared it for execution</summary>
  /// <param name="filterGraph">
  ///   Filter graph that will be verified and prepared for execution
  /// </param>
  void configureAvFilterGraph(const std::shared_ptr<::AVFilterGraph> &filterGraph) {
    int result = ::avfilter_graph_config(filterGraph.get(), nullptr);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not configure AV filter graph: ", 37)
      );
    }
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
  /// <returns>
  ///   The new libav filter graph. The input filter context is of type
  ///   &quot;buffer&quot; and named &quot;in&quot;, the output filter context
  ///   is of type &quot;buffersink&qout; and named &quot;out&quot;.
  /// </returns>
  std::shared_ptr<::AVFilterGraph> setupNnediFilterGraph(
    std::size_t frameWidth,
    std::size_t frameHeight,
    std::size_t pixelFormat,
    bool topField = true
  ) {
    std::shared_ptr<::AVFilterGraph> filterGraph = newAvFilterGraph();

    std::string arguments(u8"video_size=", 11);
    Nuclex::Support::Text::lexical_append(arguments, frameWidth);
    arguments.push_back(u8'x');
    Nuclex::Support::Text::lexical_append(arguments, frameHeight);
    arguments.append(u8":pix_fmt=", 9);
    Nuclex::Support::Text::lexical_append(arguments, pixelFormat);
    arguments.append(":time_base=1/25:pixel_aspect=1/1", 32);

    ::AVFilterContext *inputFilterContext = newAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffer"),
      u8"in",
      arguments.c_str() //u8"size=720x480:pix_fmt=0"
    );

    ::AVFilterContext *nnediFilterContext = newAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"nnedi"),
      "nnedi-deinterlace",
      u8"weights='/tmp/nnedi3_weights.bin':nsize=s48x6:nns=n256"
    );
    if(topField) {
      //::av_opt_set(nnediFilterContext, u8"field", u8"top", AV_OPT_SEARCH_CHILDREN);
    } else {
      //::av_opt_set(nnediFilterContext, u8"field", u8"bottom", AV_OPT_SEARCH_CHILDREN);
    }

    linkAvFilterContexts(inputFilterContext, nnediFilterContext);

    ::AVFilterContext *outputFilterContext = newAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffersink"),
      u8"out"
    );

    linkAvFilterContexts(nnediFilterContext, outputFilterContext);

    configureAvFilterGraph(filterGraph);

    return filterGraph;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Frees an AV frame</summary>
  /// <param name="frame">Frame that will be freed</param>
  /// <remarks>
  ///   This is a wrapper method so the shared_ptr can call av_frame_free()
  /// </remarks>
  void deleteAvFrame(::AVFrame *frame) {
    ::av_frame_free(&frame);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV frame</summary>
  /// <returns>A new, empty AV frame</returns>
  std::shared_ptr<::AVFrame> newAvFrame() {
    ::AVFrame *newFrame = ::av_frame_alloc();
    if(newFrame == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFrame");
    }

    // CHECK: AV frames have their own reference counter and we're adding an external one
    //        on top of it. May not be ideal (i.e. filter graph still holds onto frame,
    //        but shared_ptr calls av_frame_free(). Should we use av_frame_unref() instead?)
    return std::shared_ptr<::AVFrame>(newFrame, deleteAvFrame);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets up a buffer in which an AV frame can store its pixels</summary>
  /// <param name="frame">Frame that will have a buffer set up</param>
  void lockAvFrameBuffer(const std::shared_ptr<::AVFrame> &frame) {
    int result = ::av_frame_get_buffer(frame.get(), 0);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not get memory buffer for AV frame: ", 30)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a new AV frame</summary>
  /// <param name="image">QImage of which an AV frame will be constructed</param>
  /// <returns>A new AV frame with the same dimensions and contents as the QImage</returns>
  std::shared_ptr<::AVFrame> newAvFrameFromQImage(const QImage &image) {
    std::shared_ptr<::AVFrame> frame = newAvFrame();

    frame->width = image.width();
    frame->height = image.height();
    
    // TODO: Cheap and insufficient decision between 16 bits per color channel
    //       and 8 bits per color channel. I only have the former kind of images
    //       currently, but this should compare the actual pixel formats!
    if(image.bytesPerLine() >= image.width() * 8) {
      frame->format = AV_PIX_FMT_RGBA64LE; // AV_PIX_FMT_BGR48LE

      lockAvFrameBuffer(frame);

      for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgba64 (64 bit interleaved) pixels
          image.bytesPerLine(),
          frame->data[0]
        );
      }
    } else {
      frame->format = AV_PIX_FMT_RGBA; // AV_PIX_FMT_ABGR;

      lockAvFrameBuffer(frame);

      for(std::size_t lineIndex = 0; lineIndex < frame->height; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgb (32 bit interleaved) pixels
          image.bytesPerLine(),
          frame->data[0]
        );
      }
    }

    return frame;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes a frame into the &quot;in&quot; filter of a filter graph</summary>
  /// <param name="filterGraph">Filter graph the frame will be pushed into</param>
  /// <param name="frame">Frame that will be pushed into the filter graph</param>
  /// <remarks>
  ///   Assumes that the filter graph has a filter context for a &quot;buffer&quot; filter
  ///   named &quot;in&quot; and writes the frame into it.
  /// </remarks>
  void writeFrameIntoFilterGraph(
    const std::shared_ptr<::AVFilterGraph> &filterGraph,
    const std::shared_ptr<::AVFrame> &frame
  ) {
    AVFilterContext *bufferFilterContext = ::avfilter_graph_get_filter(
      filterGraph.get(), u8"in"
    );
    if(bufferFilterContext == nullptr) {
      throw std::runtime_error(u8"Could not fetch 'in' filter from filter graph");
    }

    // CHECK: Where is the difference between _write_frame and _add_frame()?
    int result = ::av_buffersrc_add_frame(bufferFilterContext, frame.get());
    //int result = ::av_buffersrc_write_frame(bufferFilterContext, frame.get());
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not store AV frame in buffer AV filter context: ", 54)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads a frame from the output of a filter graph</summary>
  /// <param name="filterGraph">Filter graph the frame will be read from</param>
  /// <returns>The frame in the &quot;out&quot; filter of the filter graph</returns>
  /// <remarks>
  ///   Assumes that the filter graph has a filter context for a &quot;buffersink&quot; filter
  ///   named &quot;out&quot; and tries to retrieve a frame from it.
  /// </remarks>
  std::shared_ptr<::AVFrame> readFrameFromFilterGraph(
    const std::shared_ptr<::AVFilterGraph> &filterGraph
  ) {
    AVFilterContext *buffersinkFilterContext = ::avfilter_graph_get_filter(
      filterGraph.get(), u8"out"
    );
    if(buffersinkFilterContext == nullptr) {
      throw std::runtime_error(u8"Could not fetch 'out' filter from filter graph");
    }

    // We've got the filter, now create an (empty) frame and ask the buffersink
    // to hand out the frame it should have collected by this time.
    std::shared_ptr<::AVFrame> frame = newAvFrame();
    {
      int result = ::av_buffersink_get_frame(buffersinkFilterContext, frame.get());
      if(result != 0) {
        throwExceptionForAvError(
          result,
          std::string(u8"Could not extract AV frame from buffersink AV filter context: ", 62)
        );
      }
    }

    return frame;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  NNedi3Deinterlacer::NNedi3Deinterlacer() :
    topFieldNnediFilterGraph(),
    bottomFieldNnediFilterGraph(),
    filterGraphWidth(std::size_t(-1)),
    filterGraphHeight(std::size_t(-1)) {}

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::WarmUp() {
    this->filterGraphWidth = std::size_t(-1);
    this->filterGraphHeight = std::size_t(-1);
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::CoolDown() {
    this->topFieldNnediFilterGraph.reset();
    this->bottomFieldNnediFilterGraph.reset();

    this->filterGraphWidth = std::size_t(-1);
    this->filterGraphHeight = std::size_t(-1);
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::SetPriorFrame(const QImage &priorFrame) {
    this->priorFrame = priorFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::SetNextFrame(const QImage &nextFrame) {
    this->nextFrame = nextFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
    if((mode == DeinterlaceMode::TopFieldOnly) || (mode == DeinterlaceMode::BottomFieldOnly)) {
      PreviewDeinterlacer::Deinterlace(
        nullptr, target, (mode == DeinterlaceMode::TopFieldOnly)
      );
    } else if(mode != DeinterlaceMode::Dont) {
      std::shared_ptr<::AVFrame> inputFrame = newAvFrameFromQImage(target);
      inputFrame->interlaced_frame = 1;

      std::shared_ptr<::AVFrame> outputFrame;
      if(mode == DeinterlaceMode::TopFieldFirst) {
        inputFrame->top_field_first = true;

        ensureTopFieldFilterGraphCreated(target.width(), target.height());
        writeFrameIntoFilterGraph(this->topFieldNnediFilterGraph, inputFrame);
        outputFrame = readFrameFromFilterGraph(this->topFieldNnediFilterGraph);
      } else {
        inputFrame->top_field_first = false;

        ensureBottomFieldFilterGraphCreated(target.width(), target.height());
        writeFrameIntoFilterGraph(this->bottomFieldNnediFilterGraph, inputFrame);
        outputFrame = readFrameFromFilterGraph(this->bottomFieldNnediFilterGraph);
      }


    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureTopFieldFilterGraphCreated(
    std::size_t width, std::size_t height
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->topFieldNnediFilterGraph)) {
      this->topFieldNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, true // AV_PIX_FMT_BGR48LE == 58;
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::ensureBottomFieldFilterGraphCreated(
    std::size_t width, std::size_t height
  ) {
    bool needRebuild = (
      (this->filterGraphWidth != width) ||
      (this->filterGraphHeight != height)
    );
    if(needRebuild) {
      CoolDown();
    }

    if(!static_cast<bool>(this->bottomFieldNnediFilterGraph)) {
      this->bottomFieldNnediFilterGraph = setupNnediFilterGraph(
        width, height, AV_PIX_FMT_RGBA64LE, false // AV_PIX_FMT_BGR48LE == 58;
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm
