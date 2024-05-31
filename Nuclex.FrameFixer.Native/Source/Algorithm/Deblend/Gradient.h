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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEBLEND_GRADIENT_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEBLEND_GRADIENT_H

#include "Nuclex/FrameFixer/Config.h"

namespace Nuclex::FrameFixer::Algorithm::Deblend {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Gradient of a pixel in six dimensions</summary>
  struct Gradient {

    /// <summary>Horizontal gradient of the red channel</summary>
    public: float RedHorizontal;
    /// <summary>Vertical gradient of the red channel</summary>
    public: float RedVertical;
    /// <summary>Horizontal gradient of the green channel</summary>
    public: float GreenHorizontal;
    /// <summary>Vertical gradient of the green channel</summary>
    public: float GreenVertical;
    /// <summary>Horizontal gradient of the blue channel</summary>
    public: float BlueHorizontal;
    /// <summary>Vertical gradient of the blue channel</summary>
    public: float BlueVertical;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deblend

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEBLEND_GRADIENT_H
