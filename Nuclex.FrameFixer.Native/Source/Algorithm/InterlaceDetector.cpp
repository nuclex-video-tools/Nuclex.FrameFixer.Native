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

#include "./InterlaceDetector.h"

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Divides all color channels in a color by 3</summary>
  /// <param name="color">Color whose channels will be divided by 3</param>
  inline void div3(Nuclex::Pixels::ColorModels::RgbColor &color) {
    color.Red /= 3.0f;
    color.Green /= 3.0f;
    color.Blue /= 3.0f;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Divides all color channels in a color by 5</summary>
  /// <param name="color">Color whose channels will be divided by 3</param>
  inline void div5(Nuclex::Pixels::ColorModels::RgbColor &color) {
    color.Red /= 5.0f;
    color.Green /= 5.0f;
    color.Blue /= 5.0f;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Adds the color channels of another color to a color</summary>
  /// <param name="color">Color to which another color's channels will be added</param>
  /// <param name="otherColor">Other color that will be added to the first color</param>
  inline void add(
    Nuclex::Pixels::ColorModels::RgbColor &color,
    const Nuclex::Pixels::ColorModels::RgbColor &otherColor
  ) {
    color.Red += otherColor.Red;
    color.Green += otherColor.Green;
    color.Blue += otherColor.Blue;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the maximum value out of 3 values</summary>
  /// <param name="a">First value to consider for being the maximum</param>
  /// <param name="b">Second value to consider for being the maximum</param>
  /// <param name="c">Third value to consider for being the maximum</param>
  /// <returns>The highest of the three values</returns>
  inline double max3(double a, double b, double c) {
    if(a > b) {
      if(a > c) {
        return a;
      } else {
        return c;
      }
    } else {
      if(b > c) {
        return b;
      } else {
        return c;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  SwipeSample InterlaceDetector::Sample3(
    Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
  ) {
    SwipeSample sample;

    sample.Center = *iterator;
    {
      ++iterator;
      sample.Right = *iterator;
      
      {
        iterator -= Nuclex::Pixels::Lines(1);
        Nuclex::Pixels::ColorModels::RgbColor topRight = *iterator;
        add(sample.Right, topRight);
        --iterator;
        sample.Above = *iterator;
        add(sample.Above, topRight);
      }

      {
        --iterator;
        Nuclex::Pixels::ColorModels::RgbColor topLeft = *iterator;
        add(sample.Above, topLeft);
        iterator += Nuclex::Pixels::Lines(1);
        sample.Left = *iterator;
        add(sample.Left, topLeft);
      }

      {
        iterator += Nuclex::Pixels::Lines(1);
        Nuclex::Pixels::ColorModels::RgbColor bottomLeft = *iterator;
        add(sample.Left, bottomLeft);
        ++iterator;
        sample.Below = *iterator;
        add(sample.Below, bottomLeft);
      }

      {
        ++iterator;
        Nuclex::Pixels::ColorModels::RgbColor bottomRight = *iterator;
        add(sample.Below, bottomRight);
        iterator -= Nuclex::Pixels::Lines(1);
        --iterator;
        add(sample.Right, bottomRight);
      }
    }

    div3(sample.Left);
    div3(sample.Right);
    div3(sample.Above);
    div3(sample.Below);

    return sample;
  }

  // ------------------------------------------------------------------------------------------- //

  SwipeSample InterlaceDetector::Sample5(
    Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
  ) {
    SwipeSample sample;

    sample.Center = *iterator;
    {
      ++iterator;
      sample.Right = *iterator;
      
      // From +1,0
      {
        iterator -= Nuclex::Pixels::Lines(1);
        sample.Above = *iterator;
        add(sample.Right, sample.Above);
        ++iterator;
        add(sample.Above, *iterator);
        iterator -= Nuclex::Pixels::Lines(1);
        --iterator;
        add(sample.Right, *iterator);
        iterator += Nuclex::Pixels::Lines(1);
      }

      // From +!,-1
      {
        --iterator;
        add(sample.Above, *iterator);
        --iterator;
        sample.Left = *iterator;
        add(sample.Above, sample.Left);
        iterator -= Nuclex::Pixels::Lines(1);
        add(sample.Left, *iterator);
        iterator += Nuclex::Pixels::Lines(1);
        --iterator;
        add(sample.Above, *iterator);
        ++iterator;
      }

      // From -1,-1
      {
        iterator += Nuclex::Pixels::Lines(1);
        add(sample.Left, *iterator);
        iterator += Nuclex::Pixels::Lines(1);
        sample.Below = *iterator;
        add(sample.Left, sample.Below);
        iterator += Nuclex::Pixels::Lines(1);
        add(sample.Left, *iterator);
        iterator -= Nuclex::Pixels::Lines(1);
        --iterator;
        add(sample.Below, *iterator);
        ++iterator;
      }

      // From -1,+1
      {
        ++iterator;
        add(sample.Below, *iterator);
        ++iterator;
        Nuclex::Pixels::ColorModels::RgbColor bottomRight = *iterator;
        add(sample.Right, bottomRight);
        add(sample.Below, bottomRight);
        ++iterator;
        add(sample.Below, *iterator);
        iterator += Nuclex::Pixels::Lines(1);
        --iterator;
        add(sample.Right, *iterator);
      }
    }

    div5(sample.Left);
    div5(sample.Right);
    div5(sample.Above);
    div5(sample.Below);

    return sample;
  }

  // ------------------------------------------------------------------------------------------- //

  std::tuple<double, double> InterlaceDetector::CalculateCombedness(const SwipeSample &sample) {
    double horizontal;
    {
      double topBottomDelta = max3(
        std::abs(sample.Above.Red - sample.Below.Red),
        std::abs(sample.Above.Green - sample.Below.Green),
        std::abs(sample.Above.Blue - sample.Below.Blue)
      );
      double centerTopDelta = max3(
        std::abs(sample.Center.Red - sample.Above.Red),
        std::abs(sample.Center.Green - sample.Above.Green),
        std::abs(sample.Center.Blue - sample.Above.Blue)
      );
      double centerBottomDelta = max3(
        std::abs(sample.Center.Red - sample.Below.Red),
        std::abs(sample.Center.Green - sample.Below.Green),
        std::abs(sample.Center.Blue - sample.Below.Blue)
      );
      horizontal = (centerTopDelta + centerBottomDelta) - (topBottomDelta * 2.0);
    }

    double vertical;
    {
      double leftRightDelta = max3(
        std::abs(sample.Left.Red - sample.Right.Red),
        std::abs(sample.Left.Green - sample.Right.Green),
        std::abs(sample.Left.Blue - sample.Right.Blue)
      );
      double centerLeftDelta = max3(
        std::abs(sample.Center.Red - sample.Left.Red),
        std::abs(sample.Center.Green - sample.Left.Green),
        std::abs(sample.Center.Blue - sample.Left.Blue)
      );
      double centerRightDelta = max3(
        std::abs(sample.Center.Red - sample.Right.Red),
        std::abs(sample.Center.Green - sample.Right.Green),
        std::abs(sample.Center.Blue - sample.Right.Blue)
      );
      vertical = (centerLeftDelta + centerRightDelta) - (leftRightDelta * 2.0);
    }

    return std::tuple<double, double>(horizontal, vertical);
  }

  // ------------------------------------------------------------------------------------------- //

  double InterlaceDetector::GetInterlaceProbability(
    const Nuclex::Pixels::Bitmap &bitmap, bool five
  ) {
    const Nuclex::Pixels::BitmapMemory &memory = bitmap.Access();

    std::vector<double> previousLine(memory.Width);
    std::vector<double> currentLine(memory.Width);

    Nuclex::Pixels::ColorModels::RgbPixelIterator it(memory);

    // Number of pixels to stay away from the image borders
    int margin = five ? 2 : 1;

    double totalProbability = 0.0;
    for(std::size_t y = margin; y < memory.Height - margin - 1; ++y) {
      if(five) {
        for(std::size_t x = margin; x < memory.Width - margin - 1; ++x) {
          it.MoveTo(x, y);

          Nuclex::FrameFixer::SwipeSample sample = Nuclex::FrameFixer::InterlaceDetector::Sample5(it);
          std::tuple<double, double> combedness = (
            Nuclex::FrameFixer::InterlaceDetector::CalculateCombedness(sample)
          );

          double horizontal = std::get<0>(combedness);
          double vertical = std::get<1>(combedness);
          currentLine[x] = horizontal - vertical;
        }
      } else {
        for(std::size_t x = margin; x < memory.Width - margin - 1; ++x) {
          it.MoveTo(x, y);

          Nuclex::FrameFixer::SwipeSample sample = Nuclex::FrameFixer::InterlaceDetector::Sample3(it);
          std::tuple<double, double> combedness = (
            Nuclex::FrameFixer::InterlaceDetector::CalculateCombedness(sample)
          );

          double horizontal = std::get<0>(combedness);
          double vertical = std::get<1>(combedness);
          currentLine[x] = horizontal - vertical;
        }

      }

      for(std::size_t x = margin + 1; x < memory.Width - margin - 2; ++x) {
        double value = currentLine[x];
        if((value >= 0) && (previousLine[x] >= 0)) {
          value += previousLine[x];
        } else if((value < 0) && (previousLine[x] < 0)) {
          value += previousLine[x];
        }

        if((value >= 0) && (currentLine[x - 1] >= 0)) {
          value += currentLine[x - 1];
        } else if((value < 0) && (currentLine[x - 1] < 0)) {
          value += currentLine[x - 1];
        }

        if((value >= 0) && (currentLine[x + 1] >= 0)) {
          value += currentLine[x + 1];
        } else if((value < 0) && (currentLine[x + 1] < 0)) {
          value += currentLine[x + 1];
        }

        totalProbability += value;
      }

      previousLine = currentLine;
    }

    return totalProbability;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
