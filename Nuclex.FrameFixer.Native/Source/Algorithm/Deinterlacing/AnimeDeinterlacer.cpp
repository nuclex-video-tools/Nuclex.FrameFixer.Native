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
    (void)previousImage;
    (void)currentImage;
    (void)nextImage;
    (void)targetImage;
    (void)topField;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
