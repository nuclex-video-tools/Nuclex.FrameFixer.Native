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

#include <vector> // for std::vector
#include <stdexcept> // for std::bad_alloc
#include <memory> // for std::unqiue_ptr

extern "C" {
  #include <libavfilter/avfilter.h>
}

namespace {

  // ------------------------------------------------------------------------------------------- //

  void deleteAvFilterGraph(AVFilterGraph *filterGraph) {
    ::avfilter_graph_free(&filterGraph);
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFilterGraph> newAvFilterGraph() {
    ::AVFilterGraph *newFilterGraph = ::avfilter_graph_alloc();
    if(newFilterGraph == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFilterGraph");
    }

    return std::shared_ptr<::AVFilterGraph>(newFilterGraph, deleteAvFilterGraph);
  }

  // ------------------------------------------------------------------------------------------- //

  ::AVFilterContext *newAvFilterContext(
    const ::AVFilter *filter,
    const char *name,
    const char *arguments,
    void *opaque,
    const std::shared_ptr<::AVFilterGraph> &filterGraph
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
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not create filter context: ", 33);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }

    return filterContext;
  }

  // ------------------------------------------------------------------------------------------- //

  void linkAvFilterContexts(::AVFilterContext *from, ::AVFilterContext *to) {
    int result = ::avfilter_link(from, 0, to, 0);
    if(result != 0) {
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not link AV filter contexts: ", 35);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void configureAvFilterGraph(const std::shared_ptr<::AVFilterGraph> &filterGraph) {
    int result = ::avfilter_graph_config(filterGraph.get(), nullptr);
    if(result != 0) {
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not configure AV filter graph: ", 30);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFilterGraph> setupNnediFilterGraph() {
    std::shared_ptr<::AVFilterGraph> filterGraph = newAvFilterGraph();

    // AV_PIX_FMT_BGR48LE == 58;
    ::AVFilterContext *inputFilterContext = newAvFilterContext(
      ::avfilter_get_by_name(u8"buffer"),
      u8"in",
      u8"video_size=720x480:pix_fmt=58:time_base=1/25:pixel_aspect=1/1",
      nullptr,
      filterGraph
    );

    ::AVFilterContext *outputFilterContext = newAvFilterContext(
      ::avfilter_get_by_name(u8"buffersink"),
      u8"out",
      nullptr,
      nullptr,
      filterGraph
    );

    ::AVFilterContext *nnediFilterContext = newAvFilterContext(
      ::avfilter_get_by_name(u8"nnedi"),
      "nnedi-deinterlace",
      u8"weights='/tmp/nnedi3_weights.bin':nsize=s48x6:nns=n256",
      nullptr,
      filterGraph
    );

    linkAvFilterContexts(inputFilterContext, nnediFilterContext);
    linkAvFilterContexts(nnediFilterContext, outputFilterContext);

    configureAvFilterGraph(filterGraph);

    return filterGraph;
  }

  // ------------------------------------------------------------------------------------------- //

  void deleteAvFrame(::AVFrame *frame) {
    ::av_frame_free(&frame);
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFrame> newAvFrame() {
    ::AVFrame *newFrame = ::av_frame_alloc();
    if(newFrame == nullptr) {
      throw std::runtime_error(u8"Could not create new AVFrame");
    }

    return std::shared_ptr<::AVFrame>(newFrame, deleteAvFrame);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFilterGraph> NNedi3Deinterlacer::nnediFilterGraph;

  // ------------------------------------------------------------------------------------------- //

  void NNedi3Deinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
    if(nnediFilterGraph == nullptr) {
      nnediFilterGraph = setupNnediFilterGraph();
    }

    std::shared_ptr<::AVFrame> inputFrame = newAvFrame();
    inputFrame->format = AV_PIX_FMT_BGR48LE;
    inputFrame->width = currentImage.width();
    inputFrame->height = currentImage.height();

    int result = ::av_frame_get_buffer(inputFrame.get(), 0);


    

/*
    ::avfilter_graph_config(graph, 0);

Finally you can send frames to the filter via

av_buffersrc_write_frame(source, frame);

And get the result with

 av_buffersink_get_frame(sink, frame);

*/
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
