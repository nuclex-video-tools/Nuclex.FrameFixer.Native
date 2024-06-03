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
