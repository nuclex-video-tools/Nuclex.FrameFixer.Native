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
