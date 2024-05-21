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

  std::shared_ptr<::AVFilterGraph> setupNnediFilterGraph(
    const std::string &videoSize, const std::string &pixelFormat, bool topField = true
  ) {
    std::shared_ptr<::AVFilterGraph> filterGraph = newAvFilterGraph();

    std::string arguments(u8"video_size=", 11);
    arguments.append(videoSize);
    arguments.append(u8":pix_fmt=", 9);
    arguments.append(pixelFormat);
    arguments.append(":time_base=1/25:pixel_aspect=1/1");
    ::AVFilterContext *inputFilterContext = newAvFilterContext(
      ::avfilter_get_by_name(u8"buffer"),
      u8"in",
      arguments.c_str(),
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

    if(topField) {
      //::av_opt_set(nnediFilterContext, u8"field", u8"top", AV_OPT_SEARCH_CHILDREN);
    } else {
      //::av_opt_set(nnediFilterContext, u8"field", u8"bottom", AV_OPT_SEARCH_CHILDREN);
    }

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

  void lockAvFrameBuffer(const std::shared_ptr<::AVFrame> &frame) {
    int result = ::av_frame_get_buffer(frame.get(), 0);
    if(result != 0) {
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not obtain memory buffer for AV frame: ", 30);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

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

    int result = ::av_buffersrc_write_frame(bufferFilterContext, frame.get());
    if(result != 0) {
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not store AV frame in buffer AV filter context: ", 54);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFrame> readFrameFromFilterGraph(
    const std::shared_ptr<::AVFilterGraph> &filterGraph
  ) {
    AVFilterContext *buffersinkFilterContext = ::avfilter_graph_get_filter(
      filterGraph.get(), u8"out"
    );
    if(buffersinkFilterContext == nullptr) {
      throw std::runtime_error(u8"Could not fetch 'out' filter from filter graph");
    }

    std::shared_ptr<::AVFrame> frame = newAvFrame();

    int result = ::av_buffersink_get_frame(buffersinkFilterContext, frame.get());
    if(result != 0) {
      char buffer[1024];
      int errorStringResult = ::av_strerror(result, buffer, sizeof(buffer));

      std::string message(u8"Could not extract AV frame from buffersink AV filter context: ", 62);
      if(errorStringResult == 0) {
        message.append(buffer);
      } else {
        message.append(u8"unknown error", 13);
      }

      throw std::runtime_error(message);
    }

    return frame;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide::Algorithm {

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
      std::size_t width = target.width();
      std::size_t height = target.height();

      std::shared_ptr<::AVFrame> inputFrame = newAvFrame();
      inputFrame->format = AV_PIX_FMT_BGR48LE;
      inputFrame->width = width;
      inputFrame->height = height;

      lockAvFrameBuffer(inputFrame);

      // TODO: work by scanline to be at least a little faster than this.
      for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
          QRgb pixel = target.pixel(x, y);
          inputFrame->data[0][y * inputFrame->linesize[0] + x * 3] = qRed(pixel);
          inputFrame->data[0][y * inputFrame->linesize[0] + x * 3 + 1] = qGreen(pixel);
          inputFrame->data[0][y * inputFrame->linesize[0] + x * 3 + 2] = qBlue(pixel);
        }
      }

      std::shared_ptr<::AVFrame> frame;
      if(mode == DeinterlaceMode::TopFieldFirst) {
        ensureTopFieldFilterGraphCreated(width, height);
        writeFrameIntoFilterGraph(this->topFieldNnediFilterGraph, inputFrame);
        frame = readFrameFromFilterGraph(this->topFieldNnediFilterGraph);
      } else {
        ensureBottomFieldFilterGraphCreated(width, height);
        writeFrameIntoFilterGraph(this->bottomFieldNnediFilterGraph, inputFrame);
        frame = readFrameFromFilterGraph(this->bottomFieldNnediFilterGraph);
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
      using Nuclex::Support::Text::lexical_append;

      std::string videoSize;
      lexical_append(videoSize, width);
      videoSize.push_back(u8'x');
      lexical_append(videoSize, height);

      // AV_PIX_FMT_BGR48LE == 58;
      this->topFieldNnediFilterGraph = setupNnediFilterGraph(
        videoSize, u8"58", true
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
      using Nuclex::Support::Text::lexical_append;

      std::string videoSize;
      lexical_append(videoSize, width);
      videoSize.push_back(u8'x');
      lexical_append(videoSize, height);
      
      // AV_PIX_FMT_BGR48LE == 58;
      this->bottomFieldNnediFilterGraph = setupNnediFilterGraph(
        videoSize, u8"58", false
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm
