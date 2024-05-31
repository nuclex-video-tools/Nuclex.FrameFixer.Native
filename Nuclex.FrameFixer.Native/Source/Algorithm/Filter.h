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

#ifndef NUCLEX_FRAMEFIXER_FILTER_H
#define NUCLEX_FRAMEFIXER_FILTER_H

#include "Nuclex/FrameFixer/Config.h"

#include <vector> // for std::vector
#include <QImage>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Applies filters to images</summary>
  class Filter {

    /// <summary>Runs a high pass filter on an image's luma channel</summary>
    /// <param name="target">Target image that will be filtered</param>
    /// <remarks>
    ///   This filter strongly highlights fine edges, making combing / interlacing
    ///   artifacts much more visible.
    /// </remarks>
    public: static void LuminanceHighPass(QImage &target);

    // Should really be generalized, but YAGNI - until more than one filter exists
#if 0
    /// <summary>Applies a custom filter to an image's saturation</summary>
    /// <param name="target">Image to which the filter gets applied</param>
    /// <param name="kernel">Filter kernel that will be applied to the image</param>
    /// <remarks>
    ///   The image is converted to HSL, then the filter is applied to the saturation
    ///   channel, then the image is converted back into RGB.
    /// </remarks>
    public: static void OnSaturation(QImage &target, float kernel[3][3]);

    /// <summary>Applies a custom filter to an image's luminance</summary>
    /// <param name="target">Image to which the filter gets applied</param>
    /// <param name="kernel">Filter kernel that will be applied to the image</param>
    /// <remarks>
    ///   The image is converted to HSL, then the filter is applied to the luma
    ///   channel, then the image is converted back into RGB.
    /// </remarks>
    public: static void OnLuminance(QImage &target, float kernel[3][3]);
#endif

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FILTER_H
