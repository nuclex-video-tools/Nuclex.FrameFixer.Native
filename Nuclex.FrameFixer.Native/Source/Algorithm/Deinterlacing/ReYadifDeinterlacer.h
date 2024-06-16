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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_REYADIFDEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_REYADIFDEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"
#include "./Deinterlacer.h"

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  //
  //     ###      I'm unsure about this implementation for anything but greyscale.
  //    ## ##     
  //   ## | ##    It uses 'step1' as pixel size in bytes, but then steps in bytes (moving
  //  ##  '  ##   through color channels). Perhaps this is okay for YUV, or perhaps it should
  // ###########  be called separately per color plane?
  //

  /// <summaryDeinterlacer that integrates the Yadif algorithm</summary>
  class ReYadifDeinterlacer : public Deinterlacer {

    /// <summary>Frees all resources used by the instance</summary>
    public: ~ReYadifDeinterlacer() = default;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: std::string GetName() const override {
      return u8"ReYadif: Broken Yadif implementation";
    }

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: bool NeedsPriorFrame() const override { return true; }

    /// <summary>Whether this deinterlacer needs to know the next frame</summary>
    /// <returns>True if the deinterlacer needs the next frame to work with</returns>
    public: bool NeedsNextFrame() const override { return true; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the prior frame is available anyway),
    ///   using the <see cref="NeedsPriorFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: void SetPriorFrame(const QImage &priorFrame) override;

    /// <summary>Assigns the next frame to the deinterlacer</summary>
    /// <param name="nextFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the next frame is available anyway),
    ///   using the <see cref="NeedsNextFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: void SetNextFrame(const QImage &nextFrame) override;

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: void Deinterlace(QImage &target, DeinterlaceMode mode) override;

    /// <summary>The frame preceding the current one</summary>
    private: QImage priorFrame;
    /// <summary>The frame following the current one</summary>
    private: QImage nextFrame;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_REYADIFDEINTERLACER_H
