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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_FRAMEINTERPOLATOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_FRAMEINTERPOLATOR_H

#include "Nuclex/FrameFixer/Config.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates intermediate frames using frame interpolation techniques</summary>
  class FrameInterpolator {

    /// <summary>Initializes a new frame interpolator</summary>
    public: FrameInterpolator() = default;
    /// <summary>Frees all resources used by the instance</summary>
    public: virtual ~FrameInterpolator() = default;

    /// <summary>Returns a name by which the interpolator can be displayed</summary>
    /// <returns>A short, human-readable name for the interpolator</returns>
    public: virtual std::string GetName() const = 0;

    /// <summary>Called before the interpolator is used by the application</summary>
    /// <remarks>
    ///   This call should be optional. It gives the interpolator a chance to initialize
    ///   any libraries or devices is uses (for example, load a neural network or set
    ///   up a filter graph in a decoder library such as libav).
    /// </remarks>
    public: virtual void WarmUp() {}

    /// <summary>Called when the interpolator is deselected for the time being</summary>
    /// <remarks>
    ///   Rather than load all resources and access all libraries in the constructor,
    ///   interpolator should do so on first use or when <see cref="WarmUp" /> is called.
    ///   Similarly, when the user picks another interpolator in the application, this
    ///   method is called where the interpolator should drop any larger resources it is
    ///   holding onto in order to not waste memory while it is not being used.
    /// </remarks>
    public: virtual void CoolDown() {}

    /// <summary>
    ///   Whether this interpolator can generate a frame that is in the middle between two frames
    /// </summary>
    /// <returns>True if the interpolate can generate a middle intermediate frame</returns>
    /// <remarks>
    ///   For AI-based interpolation, this is the most basic kind where the interpolation
    ///   point in time is exactly 50% between two other frames. Mathematical models and
    ///   some more advanced AI models, too, can also interpolate an arbitrary point in time
    ///   and thus be used to generate any number of intermediate frames or temporally resample
    ///   a movie between frame rates without repeating or dropping frames.
    /// </remarks>
    public: virtual bool CanInterpolateMiddleFrame() const { return false; }

    /// <summary>Interpolates the frame in the middle between the two input frames</summary>
    /// <param name="prior">Frame that lies before the frame to be generated</param>
    /// <param name="after">Frame that comes after the frame to be generated</param>
    /// <returns>The new, interpolated frame</returns>
    public: virtual QImage Interpolate(const QImage &prior, const QImage &after) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Interpolation

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_FRAMEINTERPOLATOR_H
