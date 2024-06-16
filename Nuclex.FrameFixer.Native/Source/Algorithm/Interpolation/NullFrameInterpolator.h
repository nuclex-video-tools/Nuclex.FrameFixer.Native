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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_NULLFRAMEINTERPOLATOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_NULLFRAMEINTERPOLATOR_H

#include "Nuclex/FrameFixer/Config.h"
#include "./FrameInterpolator.h"

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Does not interpolate frames at all</summary>
  class NullFrameInterpolator : public FrameInterpolator {

    /// <summary>Initializes a new null frame interpolator</summary>
    public: NullFrameInterpolator();
    /// <summary>Frees all resources used by the instance</summary>
    public: ~NullFrameInterpolator() = default;

    /// <summary>Returns a name by which the interpolator can be displayed</summary>
    /// <returns>A short, human-readable name for the interpolator</returns>
    public: std::string GetName() const override {
      return u8"Ignore frame interpolation tags";
    }

    /// <summary>Interpolates the frame in the middle between the two input frames</summary>
    /// <param name="prior">Frame that lies before the frame to be generated</param>
    /// <param name="after">Frame that comes after the frame to be generated</param>
    /// <returns>The new, interpolated frame</returns>
    public: QImage Interpolate(const QImage &prior, const QImage &after) override;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Interpolation

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_INTERPOLATION_NULLFRAMEINTERPOLATOR_H
