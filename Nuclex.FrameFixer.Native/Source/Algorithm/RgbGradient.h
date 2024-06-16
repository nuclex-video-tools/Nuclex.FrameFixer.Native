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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_RGBGRADIENT_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_RGBGRADIENT_H

#include "Nuclex/FrameFixer/Config.h"

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RgbGradient of a pixel in six dimensions</summary>
  struct RgbGradient {

    /// <summary>Horizontal gradient of the red channel</summary>
    public: float RedHorizontal;
    /// <summary>Vertical gradient of the red channel</summary>
    public: float RedVertical;
    /// <summary>Horizontal gradient of the green channel</summary>
    public: float GreenHorizontal;
    /// <summary>Vertical gradient of the green channel</summary>
    public: float GreenVertical;
    /// <summary>Horizontal gradient of the blue channel</summary>
    public: float BlueHorizontal;
    /// <summary>Vertical gradient of the blue channel</summary>
    public: float BlueVertical;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_RGBGRADIENT_H
