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

#include "./AnimeDeinterlacer.h"

#include <vector> // for std::vector

// PyTorch
// Three nodes to help you compute optical flow between pairs of images
// https://github.com/seanlynch/comfyui-optical-flow

// Real-time Action Recognition with Enhanced Motion Vector CNNs
// "For extracting Motion Vector image"
// https://github.com/zbwglory/MV-release

// Ideas
//
// - Look for whole lines lacking saturation (typical artifacts when movie has
//   been encoded in YUV-422 or YUV-420 with lower resolution chroma),
//   then replace saturation or chroma with interpolated chroma, keep luma.
//
// - Use 1D feature detection (i.e. via gradients) to figure out movement
//   and consider contents of previous frame, use to decide interpolation angle
//   (perhaps full frame motion estimation of half-image works for this, too?)
//  

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  void AnimeDeinterlacer::Deinterlace(
    const QImage &previousImage, const QImage &currentImage, const QImage &nextImage,
    QImage &targetImage, bool topField /* = true */
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
