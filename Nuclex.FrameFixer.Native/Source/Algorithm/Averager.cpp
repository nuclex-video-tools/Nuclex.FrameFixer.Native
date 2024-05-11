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

#include "./Averager.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  void Averager::Average(QImage &image, const QImage &otherImage) {
    std::size_t lineCount = image.height();
    for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
      if(image.bytesPerLine() >= image.width() * 8) {
        QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
        const QRgba64 *otherPixels = reinterpret_cast<const QRgba64 *>(otherImage.scanLine(lineIndex));

        for(std::size_t x = 0; x < image.width(); ++x) {
          quint32 red = qRed(pixels[x]) + qRed(otherPixels[x]);
          quint32 green = qGreen(pixels[x]) + qGreen(otherPixels[x]);
          quint32 blue = qBlue(pixels[x]) + qBlue(otherPixels[x]);
          quint32 alpha = qAlpha(pixels[x]) + qAlpha(otherPixels[x]);

          red /= 2;
          green /= 2;
          blue /= 2;
          alpha /= 2;

          pixels[x] = QRgba64::fromRgba(
            static_cast<quint16>(red), static_cast<quint16>(green),
            static_cast<quint16>(blue), static_cast<quint16>(alpha)
          );
        }
      } else {
        QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
        const QRgb *otherPixels = reinterpret_cast<const QRgb *>(otherImage.scanLine(lineIndex));

        for(std::size_t x = 0; x < image.width(); ++x) {
          quint16 red = qRed(pixels[x]) + qRed(otherPixels[x]);
          quint16 green = qGreen(pixels[x]) + qGreen(otherPixels[x]);
          quint16 blue = qBlue(pixels[x]) + qBlue(otherPixels[x]);

          red /= 2;
          green /= 2;
          blue /= 2;

          pixels[x] = qRgb(
            static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue)
          );
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Averager::Average(QImage &image, const std::vector<QImage> &otherImages) {
    if(image.bytesPerLine() >= image.width() * 8) {
      std::vector<quint32> scanline(image.width() * 4);
    
      std::size_t lineCount = image.height();
      for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {

        // Fetch the original image's scanline
        {
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            scanline[componentIndex++] = qRed(pixels[x]);
            scanline[componentIndex++] = qGreen(pixels[x]);
            scanline[componentIndex++] = qBlue(pixels[x]);
            scanline[componentIndex++] = qAlpha(pixels[x]);
          }
        }

        std::size_t otherImageCount = otherImages.size();
        for(std::size_t imageIndex = 0; imageIndex < otherImageCount; ++imageIndex) {
          const QRgba64 *otherPixels = reinterpret_cast<const QRgba64 *>(
            otherImages[imageIndex].scanLine(lineIndex)
          );

          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            scanline[componentIndex++] += qRed(otherPixels[x]);
            scanline[componentIndex++] += qGreen(otherPixels[x]);
            scanline[componentIndex++] += qBlue(otherPixels[x]);
            scanline[componentIndex++] += qAlpha(otherPixels[x]);
          }
        }

        // Because *this* image is also an image that was summed in the total
        ++otherImageCount;
        {
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            quint32 red = scanline[componentIndex++];
            quint32 green = scanline[componentIndex++];
            quint32 blue = scanline[componentIndex++];
            quint32 alpha = scanline[componentIndex++];

            pixels[x] = QRgba64::fromRgba(
              static_cast<quint16>(red / otherImageCount),
              static_cast<quint16>(green / otherImageCount),
              static_cast<quint16>(blue / otherImageCount),
              static_cast<quint16>(alpha / otherImageCount)
            );
          }
        }
      }
    } else {
      std::vector<quint16> scanline(image.width() * 4);
    
      std::size_t lineCount = image.height();
      for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {

        // Fetch the original image's scanline
        {
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            scanline[componentIndex++] = qRed(pixels[x]);
            scanline[componentIndex++] = qGreen(pixels[x]);
            scanline[componentIndex++] = qBlue(pixels[x]);
            scanline[componentIndex++] = qAlpha(pixels[x]);
          }
        }

        std::size_t otherImageCount = otherImages.size();
        for(std::size_t imageIndex = 0; imageIndex < otherImageCount; ++imageIndex) {
          const QRgb *otherPixels = reinterpret_cast<const QRgb *>(
            otherImages[imageIndex].scanLine(lineIndex)
          );

          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            scanline[componentIndex++] += qRed(otherPixels[x]);
            scanline[componentIndex++] += qGreen(otherPixels[x]);
            scanline[componentIndex++] += qBlue(otherPixels[x]);
            scanline[componentIndex++] += qAlpha(otherPixels[x]);
          }
        }

        {
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          int componentIndex = 0;
          for(std::size_t x = 0; x < image.width(); ++x) {
            quint16 red = scanline[componentIndex++];
            quint16 green = scanline[componentIndex++];
            quint16 blue = scanline[componentIndex++];
            quint16 alpha = scanline[componentIndex++];

            pixels[x] = QRgba64::fromRgba(
              static_cast<quint8>(red / otherImageCount),
              static_cast<quint8>(green / otherImageCount),
              static_cast<quint8>(blue / otherImageCount),
              static_cast<quint8>(alpha / otherImageCount)
            );
          }
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
