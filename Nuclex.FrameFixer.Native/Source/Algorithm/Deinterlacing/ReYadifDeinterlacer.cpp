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

#include "./ReYadifDeinterlacer.h"
#include "./BasicDeinterlacer.h"

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

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

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
      BasicDeinterlacer::Deinterlace(
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

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
