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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H

#include "Nuclex/FrameFixer/Config.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Deblending {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tries to de-blend frames that have been alpha-blended together</summary>
  class Deblender {

    /// <summary>Initializes a new de-blender</summary>
    public: Deblender();
    /// <summary>Frees all resources used by the instance</summary>
    public: ~Deblender() = default;

    public: virtual void Deblend(QImage &target, const QImage &imageToRemove) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deblending

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H
