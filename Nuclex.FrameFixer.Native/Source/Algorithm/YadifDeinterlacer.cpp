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
void ReYadif1Row(
  int mode,
  std::uint8_t *dst,
  const std::uint8_t *prev, const std::uint8_t *cur, const std::uint8_t *next,
  int w, int step1, int parity
);
void ReYadif1Row(
  int mode,
  std::uint16_t *dst,
  const std::uint16_t *prev, const std::uint16_t *cur, const std::uint16_t *next,
  int w, int step1, int parity
);

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  void YadifDeinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
    std::size_t lineCount = currentImage.height();

    // Without a prior frame, interpolate the missing lines
    std::size_t lineIndex = topField ? 2 : 1;
    while(lineIndex < lineCount - 1) {

      // CHECK: I blindly assume that QImage::scanLine() will return a pointer into
      //        a larger image that the Yadif method can move around in.

      if(currentImage.bytesPerLine() >= currentImage.width() * 8) {
        QRgba64 *targetPixels = reinterpret_cast<QRgba64 *>(targetImage.scanLine(lineIndex));
        const QRgba64 *previousPixels = reinterpret_cast<const QRgba64 *>(
          previousImage.scanLine(lineIndex)
        );
        const QRgba64 *currentPixels = reinterpret_cast<const QRgba64 *>(
          previousImage.scanLine(lineIndex)
        );
        const QRgba64 *nextPixels = reinterpret_cast<const QRgba64 *>(
          previousImage.scanLine(lineIndex)
        );

        ReYadif1Row(
          0,
          reinterpret_cast<std::uint16_t *>(targetPixels),
          reinterpret_cast<const std::uint16_t *>(previousPixels),
          reinterpret_cast<const std::uint16_t *>(currentPixels),
          reinterpret_cast<const std::uint16_t *>(nextPixels),
          currentImage.width() * 4,
          sizeof(QRgba64),
          ((lineIndex & 1) != 0) ? (topField ? 0 : 1) : (topField ? 1 : 0)
        );
      } else { // 16 bits per color channel / 8 bits per color channel 
        throw u8"8 bit not connected yet.";
      } // if 8 bits per color channel

      lineIndex += 1;
    } // while
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
