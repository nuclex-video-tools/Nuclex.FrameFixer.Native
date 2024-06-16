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

#include "./BasicDeinterlacer.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  void BasicDeinterlacer::SetPriorFrame(const QImage &priorFrame) {
    this->priorFrame = priorFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void BasicDeinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
    if(mode == DeinterlaceMode::TopFieldFirst)  {
      if(this->priorFrame.isNull()) {
        Deinterlace(nullptr, target, true);
      } else {
        Deinterlace(&this->priorFrame, target, true);
      }
    } else if(mode == DeinterlaceMode::BottomFieldFirst) {
      if(this->priorFrame.isNull()) {
        Deinterlace(nullptr, target, false);
      } else {
        Deinterlace(&this->priorFrame, target, false);
      }
    } else if(mode == DeinterlaceMode::TopFieldOnly) {
      Deinterlace(nullptr, target, true);
    } else if(mode == DeinterlaceMode::BottomFieldOnly) {
      Deinterlace(nullptr, target, false);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BasicDeinterlacer::Deinterlace(
    QImage *previousImage, QImage &image, bool topField /* = true */
  ) {
    std::size_t lineCount = static_cast<std::size_t>(image.height());

    // Without a prior frame, interpolate the missing lines
    if(previousImage == nullptr) {
      std::size_t lineIndex = topField ? 1 : 2;
      while(lineIndex < lineCount - 1) {

        if(image.bytesPerLine() >= image.width() * 8) {
          QRgba64 *pixelsAbove = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex - 1));
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          QRgba64 *pixelsBelow = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex + 1));

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            quint32 red = qRed(pixelsAbove[x]) + qRed(pixelsBelow[x]);
            quint32 green = qGreen(pixelsAbove[x]) + qGreen(pixelsBelow[x]);
            quint32 blue = qBlue(pixelsAbove[x]) + qBlue(pixelsBelow[x]);
            quint32 alpha = qAlpha(pixelsAbove[x]) + qAlpha(pixelsBelow[x]);

            red /= 2;
            green /= 2;
            blue /= 2;
            alpha /= 2;

            pixels[x] = QRgba64::fromRgba(
              static_cast<quint16>(red), static_cast<quint16>(green),
              static_cast<quint16>(blue), static_cast<quint16>(alpha)
            );
          }
        } else { // 16 bits per color channel / 8 bits per color channel 
          QRgb *pixelsAbove = reinterpret_cast<QRgb *>(image.scanLine(lineIndex - 1));
          QRgb *pixels = reinterpret_cast<QRgb *>(image.scanLine(lineIndex));
          QRgb *pixelsBelow = reinterpret_cast<QRgb *>(image.scanLine(lineIndex + 1));

          std::size_t imageWidth = static_cast<std::size_t>(image.width());
          for(std::size_t x = 0; x < imageWidth; ++x) {
            quint16 red = qRed(pixelsAbove[x]) + qRed(pixelsBelow[x]);
            quint16 green = qGreen(pixelsAbove[x]) + qGreen(pixelsBelow[x]);
            quint16 blue = qBlue(pixelsAbove[x]) + qBlue(pixelsBelow[x]);

            red /= 2;
            green /= 2;
            blue /= 2;

            pixels[x] = qRgb(
              static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue)
            );
          }
        } // if 8 bits per color channel

        lineIndex += 2;
      } // while

    } else { // with a prior frame / without prior frame

      std::size_t lineIndex = topField ? 1 : 0;
      while(lineIndex < lineCount - 1) {
        std::uint8_t *previousImageLine = previousImage->scanLine(lineIndex);
        std::uint8_t *currentImageLine = image.scanLine(lineIndex);
        std::copy_n(previousImageLine, image.bytesPerLine(), currentImageLine);

        lineIndex += 2;
      } // while

    } // if no prior frame provided
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
