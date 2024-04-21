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

  inline void div3(Nuclex::Pixels::ColorModels::RgbColor &color) {
    color.Red /= 3.0f;
    color.Green /= 3.0f;
    color.Blue /= 3.0f;
  }

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

} // anonymous namespace

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  //Telecide::Telecide() {}

  // ------------------------------------------------------------------------------------------- //

  std::tuple<double, double> Telecide::CalculateCombiness(
    Nuclex::Pixels::ColorModels::RgbPixelIterator &iterator
  ) {
    Nuclex::Pixels::ColorModels::RgbColor center = *iterator;
    Nuclex::Pixels::ColorModels::RgbColor top, bottom, left, right;
    {
      ++iterator;
      right = *iterator;
      
      {
        iterator -= Nuclex::Pixels::Lines(1);
        Nuclex::Pixels::ColorModels::RgbColor topRight = *iterator;
        
        right.Red += topRight.Red;
        right.Green += topRight.Green;
        right.Blue += topRight.Blue;

        --iterator;
        top = *iterator;

        top.Red += topRight.Red;
        top.Green += topRight.Green;
        top.Blue += topRight.Blue;
      }

      {
        --iterator;
        Nuclex::Pixels::ColorModels::RgbColor topLeft = *iterator;

        top.Red += topLeft.Red;
        top.Green += topLeft.Green;
        top.Blue += topLeft.Blue;

        iterator += Nuclex::Pixels::Lines(1);
        left = *iterator;

        left.Red += topLeft.Red;
        left.Green += topLeft.Green;
        left.Blue += topLeft.Blue;
      }

      {
        iterator += Nuclex::Pixels::Lines(1);
        Nuclex::Pixels::ColorModels::RgbColor bottomLeft = *iterator;

        left.Red += bottomLeft.Red;
        left.Green += bottomLeft.Green;
        left.Blue += bottomLeft.Blue;

        ++iterator;
        bottom = *iterator;

        bottom.Red += bottomLeft.Red;
        bottom.Green += bottomLeft.Green;
        bottom.Blue += bottomLeft.Blue;
      }

      {
        ++iterator;
        Nuclex::Pixels::ColorModels::RgbColor bottomRight = *iterator;

        bottom.Red += bottomRight.Red;
        bottom.Green += bottomRight.Green;
        bottom.Blue += bottomRight.Blue;

        iterator -= Nuclex::Pixels::Lines(1);
        --iterator;

        right.Red += bottomRight.Red;
        right.Green += bottomRight.Green;
        right.Blue += bottomRight.Blue;
      }
    }

    div3(left);
    div3(right);
    div3(top);
    div3(bottom);

    double horizontal;
    {
      double topBottomDelta = std::abs(
        max3(top.Red - bottom.Red, top.Green - bottom.Green, top.Blue - bottom.Blue)
      );
      double centerTopDelta = std::abs(
        max3(center.Red - top.Red, center.Green - top.Green, center.Blue - top.Blue)
      );
      double centerBottomDelta = std::abs(
        max3(center.Red - top.Red, center.Green - top.Green, center.Blue - top.Blue)
      );
      horizontal = (centerTopDelta + centerBottomDelta) - (topBottomDelta * 2.0);
    }

    double vertical;
    {
      double leftRightDelta = std::abs(
        max3(left.Red - right.Red, left.Green - right.Green, left.Blue - right.Blue)
      );
      double centerLeftDelta = std::abs(
        max3(center.Red - left.Red, center.Green - left.Green, center.Blue - left.Blue)
      );
      double centerRightDelta = std::abs(
        max3(center.Red - right.Red, center.Green - right.Green, center.Blue - right.Blue)
      );
      vertical = (centerLeftDelta + centerRightDelta) - (leftRightDelta * 2.0);
    }

    return std::tuple<double, double>(horizontal, vertical);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
