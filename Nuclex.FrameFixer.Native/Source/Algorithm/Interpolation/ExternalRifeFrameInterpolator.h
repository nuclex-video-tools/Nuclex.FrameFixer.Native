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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_EXTERNALRIFEFRAMEINTERPOLATOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_EXTERNALRIFEFRAMEINTERPOLATOR_H

#include "Nuclex/FrameFixer/Config.h"
#include "./FrameInterpolator.h"

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calls the external rife-nccn-vulkan executable to interpolate frames</summary>
  /// <remarks>
  ///   Yes, this is slow and loads the interpolation model from scratch each time a frame
  ///   is to be interpolated. It's a stop-gap measure until I'm willing to integrate
  ///   the codebases behind rife-ncnn-vulkan, dain-ncnn-vulkan and cain-ncnn-vulkan into
  ///   this application with proper WarmUp() and CoolDown() handling to speed things up.
  /// </remarks>
  class ExternalRifeFrameInterpolator : public FrameInterpolator {

    /// <summary>Initializes a new frame interpolator</summary>
    public: ExternalRifeFrameInterpolator();
    /// <summary>Frees all resources used by the instance</summary>
    public: ~ExternalRifeFrameInterpolator() = default;

    /// <summary>Returns a name by which the interpolator can be displayed</summary>
    /// <returns>A short, human-readable name for the interpolator</returns>
    public: std::string GetName() const override {
      return u8"Interpolate via rife-nccn-vulkan CLI tool (slow)";
    }

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
    public: bool CanInterpolateMiddleFrame() const override { return true; }

    /// <summary>Interpolates the frame in the middle between the two input frames</summary>
    /// <param name="prior">Frame that lies before the frame to be generated</param>
    /// <param name="after">Frame that comes after the frame to be generated</param>
    /// <returns>The new, interpolated frame</returns>
    public: QImage Interpolate(const QImage &prior, const QImage &after) override;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Interpolation

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_EXTERNALRIFEFRAMEINTERPOLATOR_H
