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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./PreviewDeinterlacer.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlace {

  // ------------------------------------------------------------------------------------------- //

  void PreviewDeinterlacer::SetPriorFrame(const QImage &priorFrame) {
    this->priorFrame = priorFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void PreviewDeinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
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

  void PreviewDeinterlacer::Deinterlace(
    QImage *previousImage, QImage &image, bool topField /* = true */
  ) {
    std::size_t lineCount = image.height();

    // Without a prior frame, interpolate the missing lines
    if(previousImage == nullptr) {
      std::size_t lineIndex = topField ? 1 : 2;
      while(lineIndex < lineCount - 1) {

        if(image.bytesPerLine() >= image.width() * 8) {
          QRgba64 *pixelsAbove = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex - 1));
          QRgba64 *pixels = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          QRgba64 *pixelsBelow = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex + 1));

          for(std::size_t x = 0; x < image.width(); ++x) {
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

          for(std::size_t x = 0; x < image.width(); ++x) {
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

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlace
