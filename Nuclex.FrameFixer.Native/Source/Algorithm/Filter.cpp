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

#include "./Filter.h"

#include <Nuclex/Pixels/ColorModels/HslColor.h>
#include <Nuclex/Pixels/ColorModels/ColorModelConverter.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  float lessEdgeDetectionKernel[3][3] = {
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  4.0f, -0.5f },
    { -0.5f, -0.5f, -0.5f }
  };

  float edgeDetectionKernel[3][3] = {
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  8.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f }
  };

  float moreEdgeDetectionKernel[3][3] = {
    { -2.0f, -2.0f, -2.0f },
    { -2.0f, 16.0f, -2.0f },
    { -2.0f, -2.0f, -2.0f }
  };

  float antiEdgeDetectionKernel[3][3] = {
    { 2.0f,   2.0f, 2.0f },
    { 2.0f, -16.0f, 2.0f },
    { 2.0f,   2.0f, 2.0f }
  };

  // ------------------------------------------------------------------------------------------- //

  float applyKernelToLightness(
    std::vector<Nuclex::Pixels::ColorModels::HslColor> *lines[3],
    float kernel[3][3],
    std::size_t middleLineX
  ) {
    float sum = 0.5f;

    for(int y = -1; y < 2; ++y) {
      for(int x = -1; x < 2; ++x) {
        float kernelValue = kernel[y + 1][x + 1];
        float lightness = (*lines[y + 1])[x + middleLineX].Lightness;
        sum += kernelValue * lightness;
      }
    }

    return sum;
  }

  // ------------------------------------------------------------------------------------------- //

  float applyKernelToSaturation(
    std::vector<Nuclex::Pixels::ColorModels::HslColor> *lines[3],
    float kernel[3][3],
    std::size_t middleLineX
  ) {
    float sum = 0.5f;

    for(int y = -1; y < 2; ++y) {
      for(int x = -1; x < 2; ++x) {
        float kernelValue = kernel[y + 1][x + 1];
        float saturation = (*lines[y + 1])[x + middleLineX].Saturation;
        sum += kernelValue * saturation;
      }
    }

    return sum;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  void Filter::HighPass(QImage &target) {
    using Nuclex::Pixels::ColorModels::HslColor;
    using Nuclex::Pixels::ColorModels::RgbColor;
    using Nuclex::Pixels::ColorModels::ColorModelConverter;

    if(target.bytesPerLine() >= target.width() * 8) {
      std::vector<HslColor> line1, line2, line3;
      line1.resize(target.width());
      line2.resize(target.width());
      line3.resize(target.width());

      // Prepare the initial 2 lines for the filter
      {
        QRgba64 *scanLine1 = reinterpret_cast<QRgba64 *>(target.scanLine(0));
        QRgba64 *scanLine2 = reinterpret_cast<QRgba64 *>(target.scanLine(1));
        for(std::size_t x = 0; x < target.width(); ++x) {
          QRgba64 qtColor = scanLine1[x];
          RgbColor nxColor;
          nxColor.Red = static_cast<float>(qtColor.red()) / 65536.0f;
          nxColor.Green = static_cast<float>(qtColor.green()) / 65536.0f;
          nxColor.Blue = static_cast<float>(qtColor.blue()) / 65536.0f;
          nxColor.Alpha = 1.0f;
          line1[x] = ColorModelConverter::HslFromRgb(nxColor);

          qtColor = scanLine2[x];
          nxColor.Red = static_cast<float>(qtColor.red()) / 65536.0f;
          nxColor.Green = static_cast<float>(qtColor.green()) / 65536.0f;
          nxColor.Blue = static_cast<float>(qtColor.blue()) / 65536.0f;
          nxColor.Alpha = 1.0f;
          line2[x] = ColorModelConverter::HslFromRgb(nxColor);
        }
      }

      std::vector<HslColor> *lines[3] = { &line1, &line2, &line3 };

      // Run the filter over all pixels in the image
      for(std::size_t lineIndex = 1; lineIndex < target.height() - 1; ++lineIndex) {

        // Fill the third line (each loop, the lines are switched round robin such that
        // the former lines 2 and 3 take places 1 and 2).
        {
          std::vector<HslColor> &line = *lines[2];
          QRgba64 *scanLine = reinterpret_cast<QRgba64 *>(target.scanLine(lineIndex + 1));
          for(std::size_t x = 0; x < target.width(); ++x) {
            QRgba64 qtColor = scanLine[x];
            RgbColor nxColor;
            nxColor.Red = static_cast<float>(qtColor.red()) / 65536.0f;
            nxColor.Green = static_cast<float>(qtColor.green()) / 65536.0f;
            nxColor.Blue = static_cast<float>(qtColor.blue()) / 65536.0f;
            nxColor.Alpha = 1.0f;
            line[x] = ColorModelConverter::HslFromRgb(nxColor);
          }
        }

        // Filter the middle of the three processed lines
        {
          std::vector<HslColor> &line = *lines[1];
          QRgba64 *scanLine = reinterpret_cast<QRgba64 *>(target.scanLine(lineIndex));
          for(std::size_t x = 1; x < target.width() - 1; ++x) {
            float lightness = applyKernelToLightness(lines, lessEdgeDetectionKernel, x);
            //float saturation = applyKernelToSaturation(lines, lessEdgeDetectionKernel, x);

            HslColor hslColor = line[x];
            hslColor.Lightness = lightness; //(hslColor.Lightness * 3.0f + lightness) / 4.0f;;
            //hslColor.Saturation = (hslColor.Saturation * 3.0f + saturation) / 4.0f;
            RgbColor rgbColor = ColorModelConverter::RgbFromHsl(hslColor);
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
    } // if 16 bits per color channel
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
