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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_ANALYSIS_INTERLACEDETECTOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_ANALYSIS_INTERLACEDETECTOR_H

#include "Nuclex/FrameFixer/Config.h"

#include <tuple>

#if 0

#include <Nuclex/Pixels/Bitmap.h>
#include <Nuclex/Pixels/ColorModels/RgbPixelIterator.h>

namespace Nuclex::FrameFixer::Algorithm::Analysis {

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

    /// <summary>Calculates the probability of a frame being interlaced</summary>
    /// <param name="bitmap">Bitmap that will be examined</param>
    /// <param name="five">Whether to use five pixel swipes instead of three pixels</param>
    /// <returns>The probability that the frame is interlaced</returns>
    /// <remarks>
    ///   This gives you an arbitrary number that should ideally be positive for interlaced
    ///   frames and negative for non-interlaced frames. When frames contain sharp elements
    ///   (text for example), this may artifically push the number towards positivity.
    ///   When frames only contain little movement and thus reveal only tiny regions of
    ///   interlacing patterns, this may be too little and the number may remain negative.
    /// </remarks>
    public: static double GetInterlaceProbability(
      const Nuclex::Pixels::Bitmap &bitmap, bool five = false
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Analysis

#endif

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_ANALYSIS_INTERLACEDETECTOR_H
