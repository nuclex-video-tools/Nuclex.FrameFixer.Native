#pragma region CPL License
/*
Nuclex Telecide
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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_TELECIDE_SOURCE 1

#include "./AnimeDeinterlacer.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  void AnimeDeinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
