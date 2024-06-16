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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H

#include "Nuclex/FrameFixer/Config.h"

#include <QColor> // for qreal

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the RGB color model</summary>
  struct RgbColor {

    /// <summary>Normalized (0.0 .. 1.0) amount of red in the color</summary>
    public: qreal Red;
    /// <summary>Normalized (0.0 .. 1.0) amount of green in the color</summary>
    public: qreal Green;
    /// <summary>Normalized (0.0 .. 1.0) amount of blue in the color</summary>
    public: qreal Blue;
    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: qreal Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_RGBCOLOR_H
