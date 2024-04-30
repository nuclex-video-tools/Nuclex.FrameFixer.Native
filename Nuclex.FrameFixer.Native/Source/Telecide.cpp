#pragma region CPL License
/*
Nuclex CriuEncoder
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

#include "./Telecide.h"

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

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  //Telecide::Telecide() {}

  // ------------------------------------------------------------------------------------------- //

  SwipeSample Telecide::Sample3(
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

  SwipeSample Telecide::Sample5(
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

  std::tuple<double, double> Telecide::CalculateCombiness(const SwipeSample &sample) {
    double horizontal;
    {
      double topBottomDelta = std::abs(
        max3(
          sample.Above.Red - sample.Below.Red,
          sample.Above.Green - sample.Below.Green,
          sample.Above.Blue - sample.Below.Blue
        )
      );
      double centerTopDelta = std::abs(
        max3(
          sample.Center.Red - sample.Above.Red,
          sample.Center.Green - sample.Above.Green,
          sample.Center.Blue - sample.Above.Blue
        )
      );
      double centerBottomDelta = std::abs(
        max3(
          sample.Center.Red - sample.Above.Red,
          sample.Center.Green - sample.Above.Green,
          sample.Center.Blue - sample.Above.Blue
        )
      );
      horizontal = (centerTopDelta + centerBottomDelta) - (topBottomDelta * 2.0);
    }

    double vertical;
    {
      double leftRightDelta = std::abs(
        max3(
          sample.Left.Red - sample.Right.Red,
          sample.Left.Green - sample.Right.Green,
          sample.Left.Blue - sample.Right.Blue
        )
      );
      double centerLeftDelta = std::abs(
        max3(
          sample.Center.Red - sample.Left.Red,
          sample.Center.Green - sample.Left.Green,
          sample.Center.Blue - sample.Left.Blue
        )
      );
      double centerRightDelta = std::abs(
        max3(
          sample.Center.Red - sample.Right.Red,
          sample.Center.Green - sample.Right.Green,
          sample.Center.Blue - sample.Right.Blue
        )
      );
      vertical = (centerLeftDelta + centerRightDelta) - (leftRightDelta * 2.0);
    }

    return std::tuple<double, double>(horizontal, vertical);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
