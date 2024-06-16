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
