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

#ifndef NUCLEX_FRAMEFIXER_PLATFORM_LIBAVAPI_H
#define NUCLEX_FRAMEFIXER_PLATFORM_LIBAVAPI_H

#include "Nuclex/FrameFixer/Config.h"

#if defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)

#include <memory> // for std::shared_ptr
#include <string> // for std::string

extern "C" {
  #include <libavfilter/avfilter.h>
  #include <libavutil/opt.h>
  #include <libavfilter/avfilter.h>
  #include <libavfilter/buffersrc.h>
  #include <libavfilter/buffersink.h>
}

namespace Nuclex { namespace FrameFixer { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the functions from libav, the libraries behind ffmpeg</summary>
  class LibAvApi {

    /// <summary>Creates a new AV filter graph</summary>
    /// <returns>A new, empty AV filter graph</returns>
    public: static std::shared_ptr<::AVFilterGraph> NewAvFilterGraph();

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
    public: static ::AVFilterContext *NewAvFilterContext(
      const std::shared_ptr<::AVFilterGraph> &filterGraph,
      const ::AVFilter *filter,
      const std::string &name = std::string(),
      const std::string &arguments = std::string(),
      void *opaque = nullptr
    );

    /// <summary>
    ///   Links an output to the input pad of onefilter context to the input pad of another
    /// </summary>
    /// <param name="from">AV filter context whose output pad will be linked</param>
    /// <param name="to">AV filter context whose input pad will be linked</param>
    /// <param name="fromOutputPadIndex">Index of the output pad that will be connected</param>
    /// <param name="toInputPadIndex">Index of the input pad that will be connected</param>
    public: static void LinkAvFilterContexts(
      ::AVFilterContext *from, ::AVFilterContext *to,
      std::size_t fromOutputPadIndex = 0, std::size_t toInputPadIndex = 0
    );

    /// <summary>Verifies a completed AV filter graph and prepared it for execution</summary>
    /// <param name="filterGraph">
    ///   Filter graph that will be verified and prepared for execution
    /// </param>
    public: static void ConfigureAvFilterGraph(
      const std::shared_ptr<::AVFilterGraph> &filterGraph
    );

    /// <summary>Creates a new AV frame</summary>
    /// <returns>A new, empty AV frame</returns>
    public: static std::shared_ptr<::AVFrame> NewAvFrame();

    /// <summary>Sets up a buffer in which an AV frame can store its pixels</summary>
    /// <param name="frame">Frame that will have a buffer set up</param>
    public: static void LockAvFrameBuffer(const std::shared_ptr<::AVFrame> &frame);

    /// <summary>Writes a frame into the &quot;in&quot; filter of a filter graph</summary>
    /// <param name="filterGraph">Filter graph the frame will be pushed into</param>
    /// <param name="frame">Frame that will be pushed into the filter graph</param>
    /// <param name="inputFilterContextName">
    ///   Name of the buffer filter context in the filter graph the frame will be pushed into
    /// </param>
    public: static void PushFrameIntoFilterGraph(
      const std::shared_ptr<::AVFilterGraph> &filterGraph,
      const std::shared_ptr<::AVFrame> &frame,
      const std::string &inputFilterContextName = std::string(u8"in")
    );

    /// <summary>Reads a frame from the output of a filter graph</summary>
    /// <param name="filterGraph">Filter graph the frame will be read from</param>
    /// <returns>The frame in the &quot;out&quot; filter of the filter graph</returns>
    /// <param name="inputFilterContextName">
    ///   Name of the buffer sink filter context in the filter graph the frame will be taken from
    /// </param>
    public: static std::shared_ptr<::AVFrame> ReadFrameFromFilterGraph(
      const std::shared_ptr<::AVFilterGraph> &filterGraph,
      const std::string &sinkFilterContextName = std::string(u8"out")
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::FrameFixer::Platform

#endif // defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)

#endif // NUCLEX_FRAMEFIXER_PLATFORM_LIBAVAPI_H
