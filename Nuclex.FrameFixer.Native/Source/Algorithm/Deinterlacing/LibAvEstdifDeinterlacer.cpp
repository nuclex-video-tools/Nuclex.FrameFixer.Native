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

#include "./LibAvEstdifDeinterlacer.h"

#include <Nuclex/Support/Text/LexicalAppend.h>

#include <vector> // for std::vector
#include <stdexcept> // for std::bad_alloc
#include <memory> // for std::unqiue_ptr

#include "../../Platform/LibAvApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  void LibAvEstdifDeinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
    DefaultFilterParameters parameters = MakeFilterParameters(target, mode);
    //std::shared_ptr<::AVFilterGraph> filterGraph = GetOrCreateFilterGraph(parameters);
    std::shared_ptr<::AVFilterGraph> filterGraph = ConstructFilterGraph(parameters);

    // Yadif can use three frames. Re-feeding the same AV frame instance does
    // not work, so we'll construct three independent frames
    std::shared_ptr<::AVFrame> priorFrame = AvFrameFromQImage(target);
    std::shared_ptr<::AVFrame> inputFrame = AvFrameFromQImage(target);

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
    Platform::LibAvApi::PushFrameIntoFilterGraph(filterGraph, priorFrame);
    Platform::LibAvApi::PushFrameIntoFilterGraph(filterGraph, inputFrame);

    // Finally, read a frame out of the filter graph. I'm not sure if
    // the filter graph starts executing as soon as there is work that can
    // be done or if this triggers it, anyway, the processed frame comes out here.
    std::shared_ptr<::AVFrame> outputFrame1 = (
      Platform::LibAvApi::ReadFrameFromFilterGraph(filterGraph)
    );
    std::shared_ptr<::AVFrame> outputFrame2 = ( // From 2 onwards outputs 2 frames...
      Platform::LibAvApi::ReadFrameFromFilterGraph(filterGraph)
    );

    // Finally, put the processed frame back into the QImage
    if(static_cast<bool>(outputFrame2)) {
      CopyAvFrameToQImage(outputFrame2, target);
    } else {
      CopyAvFrameToQImage(outputFrame1, target);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFilterGraph> LibAvEstdifDeinterlacer::ConstructFilterGraph(
    const DefaultFilterParameters &filterParameters
  ) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    std::shared_ptr<::AVFilterGraph> filterGraph = LibAvApi::NewAvFilterGraph();

    // Parameters that will be passed to the "buffer" filter context which
    // will make out input frame available to the NNedi filter.
    std::string inputBufferArguments(u8"video_size=", 11);
    Nuclex::Support::Text::lexical_append(inputBufferArguments, filterParameters.FrameWidth);
    inputBufferArguments.push_back(u8'x');
    Nuclex::Support::Text::lexical_append(inputBufferArguments, filterParameters.FrameHeight);
    inputBufferArguments.append(u8":pix_fmt=", 9);
    Nuclex::Support::Text::lexical_append(inputBufferArguments, filterParameters.LibAvPixelFormat);
    inputBufferArguments.append(u8":time_base=30000/1001", 21);
    inputBufferArguments.append(u8":pixel_aspect=16/9", 18);

    // Parameters for the NNedi filter. We'll try to configure it for maximum
    // quality and force it to process only the field the user desired.
    std::string estdifArguments(u8"deint=all", 9);
    estdifArguments.append(u8":rslope=2", 9);
    estdifArguments.append(u8":interp=6p", 11);
    
    if(filterParameters.Mode == DeinterlaceMode::TopFieldFirst) {
      estdifArguments.append(":mode=frame", 7); // end_frame
      estdifArguments.append(":parity=tff", 11); // assume top field is first
    } else if(filterParameters.Mode == DeinterlaceMode::BottomFieldFirst) {
      estdifArguments.append(":mode=frame", 7); // send_frame
      estdifArguments.append(":parity=bff", 11); // assume bottom field is first
    } else if(filterParameters.Mode == DeinterlaceMode::TopFieldOnly) {
      estdifArguments.append(":mode=field", 7); // send_field
      estdifArguments.append(":parity=tff", 11); // assume top field is first
    } else if(filterParameters.Mode == DeinterlaceMode::TopFieldOnly) {
      estdifArguments.append(":mode=field", 7); // send_field
      estdifArguments.append(":parity=bff", 11); // assume bottom field is first
    }

    // Create the filter contexts that will be linked together
    ::AVFilterContext *inputFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffer"),
      u8"in",
      inputBufferArguments
    );
    ::AVFilterContext *estdifFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"estdif"),
      u8"deinterlace",
      estdifArguments
    );
    ::AVFilterContext *outputFilterContext = LibAvApi::NewAvFilterContext(
      filterGraph,
      ::avfilter_get_by_name(u8"buffersink"),
      u8"out"
    );

    // Now build a pipeline using the three filter contexts by connection
    // their output pads to the input pads of the filter contexts following them
    LibAvApi::LinkAvFilterContexts(inputFilterContext, estdifFilterContext);
    LibAvApi::LinkAvFilterContexts(estdifFilterContext, outputFilterContext);

    // Unclear what this does. I assume it verifies and pre-loads resources.
    LibAvApi::ConfigureAvFilterGraph(filterGraph);

    return filterGraph;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
