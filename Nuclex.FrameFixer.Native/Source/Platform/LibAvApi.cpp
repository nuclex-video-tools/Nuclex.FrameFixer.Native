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

#include "LibAvApi.h"

#if defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)

#include <stdexcept> // for std::runtime_error

#include <Nuclex/Support/Text/LexicalAppend.h>

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
#if 0
  /// <summary>Frees an AV frame</summary>
  /// <param name="frame">Frame that will be freed</param>
  /// <remarks>
  ///   This is a wrapper method so the shared_ptr can call av_frame_free()
  /// </remarks>
  void deleteAvFrame(::AVFrame *frame) {
    ::av_frame_free(&frame);
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if 0
  /// <summary>Drops the reference to an AV frame</summary>
  /// <param name="frame">Frame that will be unreferenced</param>
  /// <remarks>
  ///   This is a wrapper method so the shared_ptr can call av_frame_unref()
  /// </remarks>
  void unrefAvFrame(::AVFrame *frame) {
    ::av_frame_unref(frame);
  }
#endif
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFilterGraph> LibAvApi::NewAvFilterGraph() {
    ::AVFilterGraph *newFilterGraph = ::avfilter_graph_alloc();
    if(newFilterGraph == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFilterGraph");
    }

    return std::shared_ptr<::AVFilterGraph>(newFilterGraph, deleteAvFilterGraph);
  }

  // ------------------------------------------------------------------------------------------- //

  ::AVFilterContext *LibAvApi::NewAvFilterContext(
    const std::shared_ptr<::AVFilterGraph> &filterGraph,
    const ::AVFilter *filter,
    const std::string &name /* = std::string() */,
    const std::string &arguments /* = std::string() */,
    void *opaque /* = nullptr */
  ) {
    ::AVFilterContext *filterContext = nullptr;

    int result = ::avfilter_graph_create_filter(
      &filterContext,
      filter,
      name.empty() ? nullptr : name.c_str(),
      arguments.empty() ? nullptr : arguments.c_str(),
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

  void LibAvApi::LinkAvFilterContexts(
    ::AVFilterContext *from,
    ::AVFilterContext *to,
    std::size_t fromOutputPadIndex /* = 0 */,
    std::size_t toInputPadIndex /* = 0 */
  ) {
    int result = ::avfilter_link(from, fromOutputPadIndex, to, toInputPadIndex);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not link AV filter contexts: ", 35)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void LibAvApi::ConfigureAvFilterGraph(const std::shared_ptr<::AVFilterGraph> &filterGraph) {
    int result = ::avfilter_graph_config(filterGraph.get(), nullptr);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not configure AV filter graph: ", 37)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFrame> LibAvApi::NewAvFrame() {
    ::AVFrame *newFrame = ::av_frame_alloc();
    if(newFrame == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFrame");
    }

    // CHECK: AV frames have their own reference counter and we're adding an external one
    //        on top of it. May not be ideal (i.e. filter graph still holds onto frame,
    //        but shared_ptr calls av_frame_free(). Should we use av_frame_unref() instead?)
    return std::shared_ptr<::AVFrame>(newFrame, ::av_frame_unref);
  }

  // ------------------------------------------------------------------------------------------- //

  void LibAvApi::LockAvFrameBuffer(const std::shared_ptr<::AVFrame> &frame) {
    int result = ::av_frame_get_buffer(frame.get(), 0);
    if(result != 0) {
      throwExceptionForAvError(
        result, std::string(u8"Could not get memory buffer for AV frame: ", 30)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void LibAvApi::PushFrameIntoFilterGraph(
    const std::shared_ptr<::AVFilterGraph> &filterGraph,
    const std::shared_ptr<::AVFrame> &frame,
    const std::string &inputFilterContextName /* = std::string(u8"in") */
  ) {
    AVFilterContext *bufferFilterContext = ::avfilter_graph_get_filter(
      filterGraph.get(), inputFilterContextName.c_str()
    );
    if(bufferFilterContext == nullptr) {
      std::string message(u8"Could not fetch '", 17);
      message.append(inputFilterContextName);
      message.append("' filter from filter graph", 26);
      throw std::runtime_error(message);
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

  std::shared_ptr<::AVFrame> LibAvApi::ReadFrameFromFilterGraph(
    const std::shared_ptr<::AVFilterGraph> &filterGraph,
    const std::string &sinkFilterContextName /* = std::string(u8"out") */
  ) {
    AVFilterContext *buffersinkFilterContext = ::avfilter_graph_get_filter(
      filterGraph.get(), sinkFilterContextName.c_str()
    );
    if(buffersinkFilterContext == nullptr) {
      std::string message(u8"Could not fetch '", 17);
      message.append(sinkFilterContextName);
      message.append("' filter from filter graph", 26);
      throw std::runtime_error(message);
    }

    // We've got the filter, now create an (empty) frame and ask the buffersink
    // to hand out the frame it should have collected by this time.
    std::shared_ptr<::AVFrame> frame = NewAvFrame();
    {
      int result = ::av_buffersink_get_frame(buffersinkFilterContext, frame.get());
      if(result == -11) {
        return std::shared_ptr<::AVFrame>();
      }
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

} // namespace Nuclex::FrameFixer::Platform

#endif // defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)
