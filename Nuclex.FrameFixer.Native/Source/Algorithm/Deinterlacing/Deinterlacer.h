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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"
#include "../../Model/DeinterlaceMode.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Deinterlacer that uses some tricks that mostly work on anime only</summary>
  class Deinterlacer {

    /// <summary>Frees all resources used by the instance</summary>
    public: virtual ~Deinterlacer() = default;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: virtual std::string GetName() const = 0;

    /// <summary>Called before the deinterlacer is used by the application</summary>
    /// <remarks>
    ///   This call should be optional. It gives the deinterlacer a chance to initialize
    ///   any libraries or devices is uses (for example, load a neural network or set
    ///   up a filter graph in a decoder library such as libav).
    /// </remarks>
    public: virtual void WarmUp() {}

    /// <summary>Called when the deinterlacer is deselected for the time being</summary>
    /// <remarks>
    ///   Rather than load all resources and access all libraries in the constructor,
    ///   deinterlacers should do so on first use or when <see cref="WarmUp" /> is called.
    ///   Similarly, when the user picks another deinterlacer in the application, this
    ///   method is called where the deinterlacer should drop any larger resources it is
    ///   holding onto in order to not waste memory while it is not being used.
    /// </remarks>
    public: virtual void CoolDown() {}

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: virtual bool NeedsPriorFrame() const { return false; }

    /// <summary>Whether this deinterlacer needs to know the next frame</summary>
    /// <returns>True if the deinterlacer needs the next frame to work with</returns>
    public: virtual bool NeedsNextFrame() const { return false; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the prior frame is available anyway),
    ///   using the <see cref="NeedsPriorFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: virtual void SetPriorFrame(const QImage &priorFrame) { (void)priorFrame; }

    /// <summary>Assigns the next frame to the deinterlacer</summary>
    /// <param name="nextFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the next frame is available anyway),
    ///   using the <see cref="NeedsNextFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: virtual void SetNextFrame(const QImage &nextFrame) { (void)nextFrame; }

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: virtual void Deinterlace(QImage &target, DeinterlaceMode mode) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H
