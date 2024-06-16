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

#include "./Filter.h"
#include "./RgbColor.h"
#include "./HslColor.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  qreal lessEdgeDetectionKernel[3][3] = {
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  4.0f, -0.5f },
    { -0.5f, -0.5f, -0.5f }
  };

  qreal edgeDetectionKernel[3][3] = {
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  8.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f }
  };

  qreal moreEdgeDetectionKernel[3][3] = {
    { -2.0f, -2.0f, -2.0f },
    { -2.0f, 16.0f, -2.0f },
    { -2.0f, -2.0f, -2.0f }
  };

  qreal antiEdgeDetectionKernel[3][3] = {
    { 2.0f,   2.0f, 2.0f },
    { 2.0f, -16.0f, 2.0f },
    { 2.0f,   2.0f, 2.0f }
  };


  // ------------------------------------------------------------------------------------------- //

  qreal applyKernelToLightness(
    std::vector<Nuclex::FrameFixer::Algorithm::HslColor> *lines[3],
    qreal kernel[3][3],
    std::size_t middleLineX
  ) {
    qreal sum = 0.5f;

    for(int y = -1; y < 2; ++y) {
      for(int x = -1; x < 2; ++x) {
        qreal kernelValue = kernel[y + 1][x + 1];
        qreal lightness = (*lines[y + 1])[x + middleLineX].Lightness;
        sum += kernelValue * lightness;
      }
    }

    return sum;
  }

  // ------------------------------------------------------------------------------------------- //

  qreal applyKernelToSaturation(
    std::vector<Nuclex::FrameFixer::Algorithm::HslColor> *lines[3],
    qreal kernel[3][3],
    std::size_t middleLineX
  ) {
    qreal sum = 0.5f;

    for(int y = -1; y < 2; ++y) {
      for(int x = -1; x < 2; ++x) {
        qreal kernelValue = kernel[y + 1][x + 1];
        qreal saturation = (*lines[y + 1])[x + middleLineX].Saturation;
        sum += kernelValue * saturation;
      }
    }

    return sum;
  }

  // ------------------------------------------------------------------------------------------- //

  qreal clamp(qreal value, qreal min = 0.0f, qreal max = 1.0f) {
    if(value < min) {
      return min;
    } else if(value >= max) {
      return max;
    } else {
      return value;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  void Filter::LuminanceHighPass(QImage &target) {
    using Nuclex::FrameFixer::Algorithm::HslColor;
    using Nuclex::FrameFixer::Algorithm::RgbColor;

    if(target.bytesPerLine() >= target.width() * 8) {
      std::vector<HslColor> line1, line2, line3;
      line1.resize(target.width());
      line2.resize(target.width());
      line3.resize(target.width());

      // Prepare the initial 2 lines for the filter
      {
        QRgba64 *scanLine1 = reinterpret_cast<QRgba64 *>(target.scanLine(0));
        QRgba64 *scanLine2 = reinterpret_cast<QRgba64 *>(target.scanLine(1));

        std::size_t targetWidth = static_cast<std::size_t>(target.width());
        for(std::size_t x = 0; x < targetWidth; ++x) {
          QRgba64 qtColor = scanLine1[x];
          QColor::fromRgbF(
            static_cast<float>(qtColor.red()) / 65536.0f,
            static_cast<float>(qtColor.green()) / 65536.0f,
            static_cast<float>(qtColor.blue()) / 65536.0f,
            1.0f
          ).getHslF(
            &line1[x].Hue,
            &line1[x].Saturation,
            &line1[x].Lightness,
            &line1[x].Alpha
          );

          qtColor = scanLine2[x];
          QColor::fromRgbF(
            static_cast<float>(qtColor.red()) / 65536.0f,
            static_cast<float>(qtColor.green()) / 65536.0f,
            static_cast<float>(qtColor.blue()) / 65536.0f,
            1.0f
          ).getHslF(
            &line2[x].Hue,
            &line2[x].Saturation,
            &line2[x].Lightness,
            &line2[x].Alpha
          );
        }
      }

      std::vector<HslColor> *lines[3] = { &line1, &line2, &line3 };

      // Run the filter over all pixels in the image
      std::size_t targetHeight = static_cast<std::size_t>(target.height());
      for(std::size_t lineIndex = 1; lineIndex < targetHeight - 1; ++lineIndex) {

        // Fill the third line (each loop, the lines are switched round robin such that
        // the former lines 2 and 3 take places 1 and 2).
        {
          std::vector<HslColor> &line = *lines[2];
          QRgba64 *scanLine = reinterpret_cast<QRgba64 *>(target.scanLine(lineIndex + 1));

          std::size_t targetWidth = static_cast<std::size_t>(target.width());
          for(std::size_t x = 0; x < targetWidth; ++x) {
            QRgba64 qtColor = scanLine[x];
            QColor::fromRgbF(
              static_cast<float>(qtColor.red()) / 65536.0f,
              static_cast<float>(qtColor.green()) / 65536.0f,
              static_cast<float>(qtColor.blue()) / 65536.0f,
              1.0f
            ).getHslF(
              &line[x].Hue,
              &line[x].Saturation,
              &line[x].Lightness,
              &line[x].Alpha
            );
          }
        }

        // Filter the middle of the three processed lines
        {
          std::vector<HslColor> &line = *lines[1];
          QRgba64 *scanLine = reinterpret_cast<QRgba64 *>(target.scanLine(lineIndex));

          std::size_t targetWidth = static_cast<std::size_t>(target.width());
          for(std::size_t x = 1; x < targetWidth - 1; ++x) {
            float lightness = applyKernelToLightness(lines, lessEdgeDetectionKernel, x);

            RgbColor rgbColor;
            QColor::fromHslF(
              line[x].Hue,
              line[x].Saturation,
              clamp(applyKernelToLightness(lines, lessEdgeDetectionKernel, x)),
              line[x].Alpha
            ).getRgbF(
              &rgbColor.Red,
              &rgbColor.Green,
              &rgbColor.Blue,
              &rgbColor.Alpha
            );

            scanLine[x] = QRgba64::fromRgba64(
              static_cast<quint16>(rgbColor.Red * 65535.0f),
              static_cast<quint16>(rgbColor.Green * 65535.0f),
              static_cast<quint16>(rgbColor.Blue * 65535.0f),
              65535U
            );
          }
        }

        // Move the lines around like a ringbuffer
        {
          std::vector<HslColor> *first = lines[0];
          lines[0] = lines[1];
          lines[1] = lines[2];
          lines[2] = first;
        }
      } // for each line
    } else { // if 16 bits per color channel / 8 bits per color channel
      throw std::runtime_error(u8"8 bit color depth not implemented yet, use 16 bit color depth");
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
