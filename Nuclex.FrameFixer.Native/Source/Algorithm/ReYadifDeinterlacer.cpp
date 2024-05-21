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

#include "./ReYadifDeinterlacer.h"
#include "./PreviewDeinterlacer.h"

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

namespace Nuclex::Telecide::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  void ReYadifDeinterlacer::SetPriorFrame(const QImage &priorFrame) {
    this->priorFrame = priorFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void ReYadifDeinterlacer::SetNextFrame(const QImage &nextFrame) {
    this->nextFrame = nextFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  void ReYadifDeinterlacer::Deinterlace(QImage &target, DeinterlaceMode mode) {
    if(this->priorFrame.isNull() || this->nextFrame.isNull()) {
      return;
    }

    if((mode == DeinterlaceMode::TopFieldOnly) || (mode == DeinterlaceMode::BottomFieldOnly)) {
      PreviewDeinterlacer::Deinterlace(
        nullptr, target, (mode == DeinterlaceMode::TopFieldOnly)
      );
    } else if(mode != DeinterlaceMode::Dont) {
      QImage currentFrame = target.copy();
      int startField = (mode == DeinterlaceMode::TopFieldFirst);

      target.fill(Qt::GlobalColor::green);

      std::size_t lineCount = target.height();
      for(std::size_t lineIndex = 1; lineIndex < lineCount - 1; ++lineIndex) {
        if(currentFrame.bytesPerLine() >= currentFrame.width() * 8) {
          std::uint16_t *targetPixels = reinterpret_cast<std::uint16_t *>(
            target.scanLine(lineIndex)
          );
          const std::uint16_t *previousPixels = reinterpret_cast<const std::uint16_t *>(
            this->priorFrame.scanLine(lineIndex)
          );
          const std::uint16_t *currentPixels = reinterpret_cast<const std::uint16_t *>(
            currentFrame.scanLine(lineIndex)
          );
          const std::uint16_t *nextPixels = reinterpret_cast<const std::uint16_t *>(
            this->priorFrame.scanLine(lineIndex)
          );

          ::ReYadif1Row(
            0,
            targetPixels,
            previousPixels,
            currentPixels,
            nextPixels,
            currentFrame.width() * 4,
            sizeof(QRgba64),
            startField ^ (lineIndex & 1)
          );
        } else { // 16 bits per color channel / 8 bits per color channel 
          std::uint8_t *targetPixels = reinterpret_cast<std::uint8_t *>(
            target.scanLine(lineIndex)
          );
          const std::uint8_t *previousPixels = reinterpret_cast<const std::uint8_t *>(
            this->priorFrame.scanLine(lineIndex)
          );
          const std::uint8_t *currentPixels = reinterpret_cast<const std::uint8_t *>(
            currentFrame.scanLine(lineIndex)
          );
          const std::uint8_t *nextPixels = reinterpret_cast<const std::uint8_t *>(
            this->nextFrame.scanLine(lineIndex)
          );

          ::ReYadif1Row(
            0,
            targetPixels,
            previousPixels,
            currentPixels,
            nextPixels,
            currentFrame.width() * 4,
            sizeof(std::uint8_t),
            startField ^ (lineIndex & 1)
          );
        } // if 8 bits per color channel
      } // for each line
    } // if top field first or bottom field first mode
  }

  // ------------------------------------------------------------------------------------------- //

  void ReYadifDeinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm
