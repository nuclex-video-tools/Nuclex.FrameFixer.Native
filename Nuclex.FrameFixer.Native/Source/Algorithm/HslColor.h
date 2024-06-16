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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_HSLCOLOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_HSLCOLOR_H

#include "Nuclex/FrameFixer/Config.h"

#include <QColor> // for qreal

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the HSL color model</summary>
  struct HslColor {

    /// <summary>
    ///   Index into colors of the visible light spectrum as a circle (0.0 .. 2x PI)
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     Maps the colors of the visible light spectrum into an index from 0.0 to 2x PI
    ///     where 2x PI is identical to 0.0 (so ultraviolet and infrared are imposssible to
    ///     specify, instead blue interpolates back into red).
    ///   </para>
    ///   <para>
    ///     0.00     (0°) = red
    ///     1/3 PI  (60°) = yellow
    ///     2/3 PI (120°) = green
    ///     PI     (180°) = cyan
    ///     4/3 PI (240°) = blue
    ///     5/3 PI (300°) = magenta
    ///   </para>
    /// </remarks>
    public: qreal Hue;

    /// <summary>How saturated the color is (0.0 .. 1.0)</summary>
    /// <remarks>
    ///   0.0 is monochrome (either black or white depending on lightness)
    ///   1.0 is full intensity (either black/white or full color depending on lightness)
    /// </remarks>
    public: qreal Saturation;

    /// <summary>How light the color is (0.0 .. 0.5 .. 1.0)</summary>
    /// <remarks>
    ///   0.0 is always black
    ///   0.5 is the maximum saturated color intensity (depending on saturation)
    ///   1.0 is always white
    /// </remarks>
    public: qreal Lightness;

    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: qreal Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_HSLCOLOR_H
