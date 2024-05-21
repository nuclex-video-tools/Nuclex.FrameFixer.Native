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

#ifndef NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H
#define NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H

#include "Nuclex/Telecide/Config.h"
#include "./Deinterlacer.h"

#include <QImage> // for QImage
#include <memory> // for std::shared_ptr

extern "C" {
  struct AVFilterGraph;
}

namespace Nuclex::Telecide::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Deinterlacer that uses ffmpeg's NNedi3 filter to deinterlace</summary>
  class NNedi3Deinterlacer {

    /// <summary>Cheaply deinterlaces the specified image</summary>
    /// <param name="previousImage">
    ///   Image that came before the current one. If provided, the missing rows will be
    ///   taken from this image. Otherwise, the missing rows are interpolated.
    /// </param>
    /// <param name="image">Image that will be deinterlaced</param>
    /// <param name="topField">
    ///   If true, the top field (even rows) will be filled in,
    ///   otherwise, the bottom field (odd rows) will be filled in
    /// </param>
    public: static void Deinterlace(
      const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
      QImage &targetImage, bool topField = true
    );

    private: static std::shared_ptr<::AVFilterGraph> nnediFilterGraph;


  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm

#endif // NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H
