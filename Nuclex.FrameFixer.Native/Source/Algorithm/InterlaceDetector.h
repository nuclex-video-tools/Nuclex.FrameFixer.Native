#pragma region CPL License
/*
Nuclex CriuEncoder
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

#ifndef NUCLEX_TELECIDE_INTERLACEDETECTOR_H
#define NUCLEX_TELECIDE_INTERLACEDETECTOR_H

#include "Nuclex/Telecide/Config.h"

#include <tuple>

#include <Nuclex/Pixels/Bitmap.h>
#include <Nuclex/Pixels/ColorModels/RgbPixelIterator.h>

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores the colors obtain by swipe-sampling a pixel</summary>
  /// <remarks>
  ///   <para>
  ///     Swipe sampling is a custom term in this application that refers to averaging
  ///     a variable width stripe of pixels in parallel to the center pixel. For example,
  ///     sampling at 10,10 could provide the average of the pixels from 9,9 to 11,9 as
  ///     the 'Above' sample and the average from 9,9 to 9,11 as the 'Left Of" sample.
  ///   </para>
  ///   <para>
  ///     This doesn't do anything useful for scaling or smoothing, but comparing
  ///     how much the (Left, Right) samples differ in contrast to the (Above, Below) samples
  ///     will give a relatively good estimation about whether a pixel sits inside
  ///     a combed area of a frame.
  ///   </para>
  /// </remarks>
  struct SwipeSample {

    /// <summary>The color of the pixel at the exact sampling location</summary>
    public: Nuclex::Pixels::ColorModels::RgbColor Center;
    /// <summary>The color of the pixel(s) above the sampling location)</summary>
    public: Nuclex::Pixels::ColorModels::RgbColor Above;
    /// <summary>The color of the pixel(s) below the sampling location)</summary>
    public: Nuclex::Pixels::ColorModels::RgbColor Below;
    /// <summary>The color of the pixel(s) left of the sampling location)</summary>
    public: Nuclex::Pixels::ColorModels::RgbColor Left;
    /// <summary>The color of the pixel(s) right of the sampling location)</summary>
    public: Nuclex::Pixels::ColorModels::RgbColor Right;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tries to automatically detect whether video frames are interlaced</summary>
  class InterlaceDetector {

    /// <summary>Collects a swipe sample averaging over 3 parallel pixels</summary>
    /// <param name="iterator">
    ///   Iterator placed on the center pixel around which the samples will be taken
    /// </param>
    /// <returns>A swipe sample from which combiness can be calculated</returns>
    public: static SwipeSample Sample3(
      Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
    );

    /// <summary>Collects a swipe sample averaging over 5 parallel pixels</summary>
    /// <param name="iterator">
    ///   Iterator placed on the center pixel around which the samples will be taken
    /// </param>
    /// <returns>A swipe sample from which combiness can be calculated</returns>
    public: static SwipeSample Sample5(
      Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
    );

    /// <summary>Calculates the &quot;combiness&quot; horizontally and vertically</summary>
    /// <param name="sample">Sample of a pixel taken by one of the sampling methods</param>
    /// <returns>A tuple containing the horizontal and the vertical combiness</returns>
    public: static std::tuple<double, double>  CalculateCombedness(const SwipeSample &sample);

    public: static double GetInterlaceProbability(const Nuclex::Pixels::Bitmap &bitmap);

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_INTERLACEDETECTOR_H
