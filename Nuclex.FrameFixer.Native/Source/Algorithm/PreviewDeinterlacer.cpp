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

#include "./PreviewDeinterlacer.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  void PreviewDeinterlacer::Deinterlace(
    QImage *previousImage, QImage &image, bool topField /* = true */
  ) {
    std::size_t lineCount = image.height();
    std::size_t lineIndex = topField ? 2 : 1;

    while(lineIndex < lineCount - 1) {

      // Without a prior frame, interpolate the missing lines
      if(previousImage == nullptr) {
        if(image.bytesPerLine() >= image.width() * 8) {
          QRgba64 *pixelsAbove = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex - 1));
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          QRgba64 *pixelsBelow = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex + 1));

          for(std::size_t x = 0; x < image.width(); ++x) {
            quint16 red = qRed(pixelsAbove[x]) + qRed(pixelsBelow[x]);
            quint16 green = qGreen(pixelsAbove[x]) + qGreen(pixelsBelow[x]);
            quint16 blue = qBlue(pixelsAbove[x]) + qBlue(pixelsBelow[x]);
            quint16 alpha = qAlpha(pixelsAbove[x]) + qAlpha(pixelsBelow[x]);

            red /= 2;
            green /= 2;
            blue /= 2;
            alpha /= 2;

            pixels[x] = QRgba64::fromRgba(red, green, blue, alpha);
          }
        } else {
          QRgb *pixelsAbove = reinterpret_cast<QRgb *>(image.scanLine(lineIndex - 1));
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          QRgb *pixelsBelow = reinterpret_cast<QRgb *>(image.scanLine(lineIndex + 1));

          for(std::size_t x = 0; x < image.width(); ++x) {
            int red = qRed(pixelsAbove[x]) + qRed(pixelsBelow[x]);
            int green = qGreen(pixelsAbove[x]) + qGreen(pixelsBelow[x]);
            int blue = qBlue(pixelsAbove[x]) + qBlue(pixelsBelow[x]);
            //int alpha = qAlpha(pixelsAbove[x]) + qAlpha(pixelsBelow[x]);

            pixels[x] = qRgb(red / 2, green / 2, blue / 2);
          }
        }
      } else { // With a prior image, use the fields from that image if needed
        std::uint8_t *previousImageLine = previousImage->scanLine(lineIndex);
        std::uint8_t *currentImageLine = image.scanLine(lineIndex);
        std::copy_n(previousImageLine, image.bytesPerLine(), currentImageLine);
      }

      lineIndex += 2;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
