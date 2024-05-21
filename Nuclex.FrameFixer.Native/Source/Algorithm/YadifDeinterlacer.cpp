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

#include "./YadifDeinterlacer.h"

#include "../yadifmod2-0.2.8/common.h"

#include <vector> // for std::vector

// If set, the ReYadif implementation from the cvDeinterlace repository is used
// Otherwise, YadifMod2 from AviSynth is used.
#define CVDEINTERLACE_REYADIF 1

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
#if defined(CVDEINTERLACE_REYADIF)

    // Without a prior frame, interpolate the missing lines
    std::size_t lineCount = currentImage.height();
    for(std::size_t lineIndex = 1; lineIndex < lineCount - 1; ++lineIndex) {
      if(currentImage.bytesPerLine() >= currentImage.width() * 8) {
        std::uint16_t *targetPixels = reinterpret_cast<std::uint16_t *>(
          targetImage.scanLine(lineIndex)
        );
        const std::uint16_t *previousPixels = reinterpret_cast<const std::uint16_t *>(
          previousImage.scanLine(lineIndex)
        );
        const std::uint16_t *currentPixels = reinterpret_cast<const std::uint16_t *>(
          previousImage.scanLine(lineIndex)
        );
        const std::uint16_t *nextPixels = reinterpret_cast<const std::uint16_t *>(
          previousImage.scanLine(lineIndex)
        );

        ::ReYadif1Row(
          0,
          targetPixels,
          previousPixels,
          currentPixels,
          nextPixels,
          currentImage.width() * 4,
          sizeof(QRgba64),
          (lineIndex & 1) ^ (topField ? 0 : 1)
        );
      } else { // 16 bits per color channel / 8 bits per color channel 
        std::uint8_t *targetPixels = reinterpret_cast<std::uint8_t *>(
          targetImage.scanLine(lineIndex)
        );
        const std::uint8_t *previousPixels = reinterpret_cast<const std::uint8_t *>(
          previousImage.scanLine(lineIndex)
        );
        const std::uint8_t *currentPixels = reinterpret_cast<const std::uint8_t *>(
          previousImage.scanLine(lineIndex)
        );
        const std::uint8_t *nextPixels = reinterpret_cast<const std::uint8_t *>(
          previousImage.scanLine(lineIndex)
        );

        ::ReYadif1Row(
          0,
          targetPixels,
          previousPixels,
          currentPixels,
          nextPixels,
          currentImage.width() * 4,
          sizeof(std::uint8_t),
          (lineIndex & 1) ^ (topField ? 0 : 1)
        );
      } // if 8 bits per color channel
    } // while

#else // defined(CVDEINTERLACE_REYADIF) / !defined(CVDEINTERLACE_REYADIF)

    ::proc_filter_t *yadifDeinterlaceProc = ::get_main_proc(
      16, true, false, arch_t::NO_SIMD
    );

    if(currentImage.bytesPerLine() >= currentImage.width() * 8) {

      // CHECK: I blindly assume that QImage::scanLine() will return a pointer into
      //        a larger image that the Yadif method can move around in.
      std::uint8_t *targetPixels = reinterpret_cast<std::uint8_t *>(
        targetImage.scanLine(0)
      );
      const std::uint8_t *previousPixels = reinterpret_cast<const std::uint8_t *>(
        previousImage.scanLine(0)
      );
      const std::uint8_t *currentPixels = reinterpret_cast<const std::uint8_t *>(
        previousImage.scanLine(0)
      );
      const std::uint8_t *nextPixels = reinterpret_cast<const std::uint8_t *>(
        previousImage.scanLine(0)
      );

      // CHECK: I just assume that pitch will be the same for all images.
      int pitch;
      {
        std::uint8_t *nextTargetPixels = reinterpret_cast<std::uint8_t *>(
          targetImage.scanLine(1)
        );
        pitch = nextTargetPixels - targetPixels;
      }

      // https://github.com/Asd-g/yadifmod2/blob/avs16/avisynth/src/yadifmod2.cpp

      // This nasty bit of confused calculations is courtesy of the AviSynth code.
      int startIndex = 2 + (topField ? 1 : 0);
      int lineCount = (currentImage.height() - 4 + (topField ? 1 : 0) - startIndex) / 2 + 1;

      // CHECK: Taken straight out of avisynth because I have no clue what
      // fm_prev and fm_next are supposed to do.
      const std::uint8_t *fm_prev, *fm_next;
      int fm_ppitch, fm_npitch;
      if(!topField) {
        fm_ppitch = pitch * 2;
        fm_npitch = pitch * 2;
        fm_prev = currentPixels + startIndex * pitch;
        fm_next = nextPixels + startIndex * pitch;
      } else {
        fm_ppitch = pitch * 2;
        fm_npitch = pitch * 2;
        fm_prev = previousPixels + startIndex * pitch;
        fm_next = currentPixels + startIndex * pitch;
      }

      yadifDeinterlaceProc(
        currentPixels + (startIndex * pitch),
        previousPixels + (startIndex * pitch),
        nextPixels + (startIndex * pitch),
        fm_prev,
        fm_next,
        nullptr,
        targetPixels + (startIndex * pitch),
        currentImage.width() * 4, // from PVideoFrame::GetRowSize()
        pitch,
        pitch,
        pitch,
        pitch,
        pitch,
        pitch * 2,
        pitch * 2,
        lineCount
      );
    } else { // 16 bits per color channel / 8 bits per color channel 
      throw u8"8 bit not connected yet.";
    } // if 8 bits per color channel

#endif // !defined(CVDEINTERLACE_REYADIF)
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
