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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H

#include "Nuclex/FrameFixer/Config.h"

#include <QColor> // for qreal

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the RGB color model</summary>
  struct RgbColor {

    /// <summary>Normalized (0.0 .. 1.0) amount of red in the color</summary>
    public: qreal Red;
    /// <summary>Normalized (0.0 .. 1.0) amount of green in the color</summary>
    public: qreal Green;
    /// <summary>Normalized (0.0 .. 1.0) amount of blue in the color</summary>
    public: qreal Blue;
    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: qreal Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H
