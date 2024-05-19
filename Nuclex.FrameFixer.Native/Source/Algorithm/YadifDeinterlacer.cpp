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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_TELECIDE_SOURCE 1

#include "./YadifDeinterlacer.h"

#include "../yadifmod2-0.2.8/common.h"

#include <vector> // for std::vector

// Declared ni ReYadif8.cpp
typedef unsigned char u8;
void ReYadif1Row(
  int mode,
  u8 *dst,
  const u8 *prev, const u8 *cur, const u8 *next,
  int w, int step1, int parity
);

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  void YadifDeinterlacer::Deinterlace(
    QImage *previousImage, QImage &image, bool topField /* = true */
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
