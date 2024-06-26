#pragma region Apache License 2.0
/*
Nuclex Frame Fixer
Copyright (C) 2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./Averager.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  void Averager::Average(QImage &image, const QImage &otherImage) {
    std::size_t lineCount = static_cast<std::size_t>(image.height());
    for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
      if(image.bytesPerLine() >= image.width() * 8) {
        QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
        const QRgba64 *otherPixels = reinterpret_cast<const QRgba64 *>(otherImage.scanLine(lineIndex));

        std::size_t imageWidth = static_cast<std::size_t>(image.width());
        for(std::size_t x = 0; x < imageWidth; ++x) {
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

        std::size_t imageWidth = static_cast<std::size_t>(image.width());
        for(std::size_t x = 0; x < imageWidth; ++x) {
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
    
      std::size_t lineCount = static_cast<std::size_t>(image.height());
      for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {

        // Fetch the original image's scanline
        {
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          int componentIndex = 0;

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            scanline[componentIndex++] = pixels[x].red();
            scanline[componentIndex++] = pixels[x].green();
            scanline[componentIndex++] = pixels[x].blue();
            scanline[componentIndex++] = pixels[x].alpha();
          }
        }

        std::size_t otherImageCount = otherImages.size();
        for(std::size_t imageIndex = 0; imageIndex < otherImageCount; ++imageIndex) {
          const QRgba64 *otherPixels = reinterpret_cast<const QRgba64 *>(
            otherImages[imageIndex].scanLine(lineIndex)
          );

          int componentIndex = 0;

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            scanline[componentIndex++] += otherPixels[x].red();
            scanline[componentIndex++] += otherPixels[x].green();
            scanline[componentIndex++] += otherPixels[x].blue();
            scanline[componentIndex++] += otherPixels[x].alpha();
          }
        }

        // Because *this* image is also an image that was summed in the total
        ++otherImageCount;
        {
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          int componentIndex = 0;

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            quint32 red = scanline[componentIndex++];
            quint32 green = scanline[componentIndex++];
            quint32 blue = scanline[componentIndex++];
            quint32 alpha = scanline[componentIndex++];

            pixels[x] = QRgba64::fromRgba64(
              static_cast<quint16>(red / otherImageCount),
              static_cast<quint16>(green / otherImageCount),
              static_cast<quint16>(blue / otherImageCount),
              static_cast<quint16>(alpha / otherImageCount)
            );
          }
        }
      }
    } else {
      std::size_t imageWidth = static_cast<std::size_t>(image.width());
      std::vector<quint16> scanline(imageWidth * 4);
    
      std::size_t lineCount = static_cast<std::size_t>(image.height());
      for(std::size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {

        // Fetch the original image's scanline
        {
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          int componentIndex = 0;

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
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

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            scanline[componentIndex++] += qRed(otherPixels[x]);
            scanline[componentIndex++] += qGreen(otherPixels[x]);
            scanline[componentIndex++] += qBlue(otherPixels[x]);
            scanline[componentIndex++] += qAlpha(otherPixels[x]);
          }
        }

        {
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          int componentIndex = 0;

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
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

} // namespace Nuclex::FrameFixer
