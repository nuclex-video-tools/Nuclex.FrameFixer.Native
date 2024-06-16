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

#include "./YadifMod2Deinterlacer.h"

#include "./yadifmod2-0.2.8/common.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  void YadifMod2Deinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
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

      (void)fm_ppitch;
      (void)fm_npitch;
      (void)nextImage;

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
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
