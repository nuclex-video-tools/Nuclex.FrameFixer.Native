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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_YADIFMOD2DEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_YADIFMOD2DEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  /// <summaryDeinterlacer that integrates the Yadif algorithm</summary>
  class YadifMod2Deinterlacer {

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

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlace

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_YADIFMOD2DEINTERLACER_H
