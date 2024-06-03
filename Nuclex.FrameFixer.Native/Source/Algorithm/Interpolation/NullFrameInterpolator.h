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
