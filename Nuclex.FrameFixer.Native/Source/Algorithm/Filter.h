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
