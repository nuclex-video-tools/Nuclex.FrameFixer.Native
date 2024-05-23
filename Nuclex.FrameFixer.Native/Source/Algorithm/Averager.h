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

#ifndef NUCLEX_FRAMEFIXER_AVERAGER_H
#define NUCLEX_FRAMEFIXER_AVERAGER_H

#include "Nuclex/FrameFixer/Config.h"

#include <vector> // for std::vector
#include <QImage>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Averages images of the same size</summary>
  class Averager {

    /// <summary>Composites another image onto an image at 50% opacity</summary>
    /// <param name="image">Image onto which the second image will be composited</param>
    /// <param name="otherImage">Image that will be composited onto the first image</param>
    public: static void Average(QImage &image, const QImage &otherImage);

    /// <summary>Composites multiple images onto an image</summary>
    /// <param name="image">Image onto which the other images will be composited</param>
    /// <param name="otherImages">Images that will be composited onto the first image</param>
    public: static void Average(QImage &image, const std::vector<QImage> &otherImages);


  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_AVERAGER_H
