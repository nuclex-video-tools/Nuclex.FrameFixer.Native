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

#ifndef NUCLEX_TELECIDE_TELECIDE_H
#define NUCLEX_TELECIDE_TELECIDE_H

#include "./Config.h"

#include <tuple>

#include <Nuclex/Pixels/Bitmap.h>
#include <Nuclex/Pixels/ColorModels/RgbPixelIterator.h>

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dialog that lets the user configure various application settings</summary>
  class Telecide {

    /// <summary>Calculates the &quot;combiness&quot; horizontally and vertically</summary>
    /// <param name="iterator">
    ///   Iterator placed on a pixel whose &quot;combiness&quot; will be calculated
    /// </param>
    /// <returns>A tuple containing the horizontal and the vertical combiness</returns>
    public: static std::tuple<double, double> CalculateCombiness(
      Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_TELECIDE_H
