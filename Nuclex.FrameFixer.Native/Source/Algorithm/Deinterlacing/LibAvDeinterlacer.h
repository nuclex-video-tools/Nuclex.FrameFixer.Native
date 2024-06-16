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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"
#include "./Deinterlacer.h"
#include "../../Platform/LibAvApi.h"

#include <Nuclex/Support/Text/LexicalAppend.h>

#include <QImage> // for QImage

#include <memory> // for std::shared_ptr
#include <map> // for std::map

extern "C" {
  struct AVFilterGraph;
  struct AVFrame;
}

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Parameters that will be passed to the filter construction method</summary>
  struct DefaultFilterParameters {

    /// <summary>Width of a frame in pixels</summary>
    public: std::size_t FrameWidth;
    /// <summary>Height of a frame in pixels</summary>
    public: std::size_t FrameHeight;
    /// <summary>LibAv pixel format of input frames processed by the filter graph</summary>
    public: std::size_t LibAvPixelFormat;
    /// <summary>How the filter graph should deinterlace frames</summary>
    public: DeinterlaceMode Mode;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Intermediate base class so helper methods won't get templated</summary>
  class LibAvDeinterlacerBase : public Deinterlacer {

    /// <summary>Initializes the libav deinterlcer base class</summary>
    public: LibAvDeinterlacerBase() : filterGraphCache() {}

    /// <summary>Frees all resources owned by the libav deinterlacer base class</summary>
    public: ~LibAvDeinterlacerBase() = default;

    /// <summary>Called when the deinterlacer is deselected for the time being</summary>
    public: virtual void CoolDown() override {
      FlushCachedFilterGraphs();
    }

    /// <summary>Creates a new AV frame containing the pixels of a QImage (from Qt)</summary>
    /// <param name="image">Image whose pixels will be copied into a new AV frame</param>
    /// <returns>An AV frame containing the pixels of the input image</returns>
    protected: static std::shared_ptr<::AVFrame> AvFrameFromQImage(const QImage &image);

    /// <summary>Copies the contents of an AV frame into an existing QImage</summary>
    /// <param name="frame">Frame whose contents will be copied into a QImage</param>
    /// <param name="image">Image into which the pixels from the AV frame will be copied</param>
    protected: static void CopyAvFrameToQImage(
      const std::shared_ptr<::AVFrame> &frame, QImage &image
    );

    /// <summary>Drops all cached filter graphs</summary>
    protected: void FlushCachedFilterGraphs() {
      this->filterGraphCache.clear();
    }

    /// <summary>Stores cached filter graphs</summary>
    protected: std::map<std::string, std::shared_ptr<::AVFilterGraph>> filterGraphCache;
    
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Deinterlacer that uses ffmpeg's libav to deinterlace</summary>
  template<typename TFilterParameters = DefaultFilterParameters>
  class LibAvDeinterlacer : public LibAvDeinterlacerBase {

    /// <summary>Initializes the libav-based deinterlacer</summary>
    public: LibAvDeinterlacer() = default;
    /// <summary>Frees all resources used by the deinterlacer</summary>
    public: virtual ~LibAvDeinterlacer() = default;

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: virtual void Deinterlace(QImage &target, DeinterlaceMode mode) override {
      TFilterParameters parameters = MakeFilterParameters(target, mode);
      std::shared_ptr<::AVFilterGraph> filterGraph = GetOrCreateFilterGraph(parameters);

      Platform::LibAvApi::PushFrameIntoFilterGraph(
        filterGraph, AvFrameFromQImage(target)
      );
      std::shared_ptr<::AVFrame> processedFrame = (
        Platform::LibAvApi::ReadFrameFromFilterGraph(filterGraph)
      );
      CopyAvFrameToQImage(processedFrame, target);
    }

    /// <summary>Collects all parameters that need to be passed to a filter graph</summary>
    /// <param name="target">Frame that is to be processed by the filter graph</param>
    /// <param name="mode">What the filter graph ought to do with the frame</param>
    /// <returns>A structure containing all the parameters needed by the filter</returns>
    protected: virtual TFilterParameters MakeFilterParameters(
      QImage &target, DeinterlaceMode mode
    ) {
      TFilterParameters parameters;

      parameters.FrameWidth = target.width();
      parameters.FrameHeight = target.height();
      if(target.bytesPerLine() >= target.width() * 8) {
        parameters.LibAvPixelFormat = AV_PIX_FMT_RGBA64LE;
      } else {
        parameters.LibAvPixelFormat = AV_PIX_FMT_RGBA;
      }
      parameters.Mode = mode;

      return parameters;
    }

    /// <summary>Constructs a new filter graph with the specified parameters</summary>
    /// <param name="filterParameters">Parameters that will be passed to the filter</param>
    /// <returns>The new filter graph</returns>
    protected: virtual std::shared_ptr<::AVFilterGraph> ConstructFilterGraph(
      const TFilterParameters &filterParameters
    ) = 0;

    /// <summary>Creates a new filter graph or fetches a cached one</summary>
    /// <param name="filterParameters">
    ///   Parameters of the filter graph graph will be fetched or constructed
    /// </param>
    protected: std::shared_ptr<::AVFilterGraph> GetOrCreateFilterGraph(
      const TFilterParameters &filterParameters
    ) {
      typedef std::map<std::string, std::shared_ptr<::AVFilterGraph>> CacheKeyToFilterGraphMap;

      std::string cacheKey = GetCacheKey(filterParameters);

      CacheKeyToFilterGraphMap::const_iterator index = this->filterGraphCache.find(cacheKey);
      if(index != this->filterGraphCache.end()) {
        return index->second;
      }

      std::shared_ptr<::AVFilterGraph> filterGraph = ConstructFilterGraph(filterParameters);
      this->filterGraphCache.insert(
        CacheKeyToFilterGraphMap::value_type(cacheKey, filterGraph)
      );

      return filterGraph;
    }

    /// <summary>Builds a key by which constructed filters will be cached</summary>
    /// <param name="filterParameters">Parameters for which a cache key will be formed</param>
    /// <returns>The cache key for the specified filter parameters</returns>
    protected: virtual std::string GetCacheKey(const TFilterParameters &filterParameters) {
      std::string cacheKey(u8"F-", 2);
      Nuclex::Support::Text::lexical_append(cacheKey, filterParameters.FrameWidth);
      cacheKey.push_back(u8'x');
      Nuclex::Support::Text::lexical_append(cacheKey, filterParameters.FrameHeight);
      cacheKey.push_back(u8'@');
      Nuclex::Support::Text::lexical_append(cacheKey, filterParameters.LibAvPixelFormat);

      switch(filterParameters.Mode) {
        case DeinterlaceMode::TopFieldFirst: { cacheKey.append(u8"-tff", 4); break; }
        case DeinterlaceMode::BottomFieldFirst: { cacheKey.append(u8"-bff", 4); break; }
        case DeinterlaceMode::TopFieldOnly: { cacheKey.append(u8"-tf", 3); break; }
        case DeinterlaceMode::BottomFieldOnly: { cacheKey.append(u8"-bf", 3); break; }
        default: { cacheKey.append(u8"-?", 3); break; }
      }

      return cacheKey;
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVEINTERLACER_H
