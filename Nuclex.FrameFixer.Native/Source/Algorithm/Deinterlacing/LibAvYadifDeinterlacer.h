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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVYADIFDEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVYADIFDEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"
#include "./LibAvDeinterlacer.h"

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Deinterlacer that uses libav's Yadif filter to deinterlace</summary>
  class LibAvYadifDeinterlacer : public LibAvDeinterlacer<DefaultFilterParameters> {

    /// <summary>Initializes the Yadif via libav deinterlacer</summary>
    /// <param name="bwDifMode">Whether to use bwdif instead</param>
    public: LibAvYadifDeinterlacer(bool bwDifMode);
    /// <summary>Frees all resources used by the deinterlacer</summary>
    public: virtual ~LibAvYadifDeinterlacer() = default;

    /// <summary>Called when the deinterlacer is deselected for the time being</summary>
    public: void CoolDown() override;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: std::string GetName() const override {
      if(this->bwDifMode) {
        return u8"BWDif-libav: Yadif with w3dif and cubic interpolation";
      } else {
        return u8"Yadif-libav: Adaptive temporal and spatial interpolation";
      }
    }

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: bool NeedsPriorFrame() const override { return true; }

    /// <summary>Whether this deinterlacer needs to know the next frame</summary>
    /// <returns>True if the deinterlacer needs the next frame to work with</returns>
    public: bool NeedsNextFrame() const override { return true; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    public: void SetPriorFrame(const QImage &priorFrame) override;

    /// <summary>Assigns the next frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the next frame</param>
    public: void SetNextFrame(const QImage &nextFrame) override;

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: void Deinterlace(QImage &target, DeinterlaceMode mode) override;

    /// <summary>Constructs a new filter graph with the specified parameters</summary>
    /// <param name="filterParameters">Parameters that will be passed to the filter</param>
    /// <returns>The new filter graph</returns>
    protected: std::shared_ptr<::AVFilterGraph> ConstructFilterGraph(
      const DefaultFilterParameters &filterParameters
    ) override;

    /// <summary>Whether BWDif is used instead of Yadif</summary>
    private: bool bwDifMode;
    /// <summary>The frame preceding the current one</summary>
    private: QImage priorFrame;
    /// <summary>The frame succeeding the current one</summary>
    private: QImage nextFrame;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_LIBAVYADIFDEINTERLACER_H
