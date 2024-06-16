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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H

#include "Nuclex/FrameFixer/Config.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Deblending {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tries to de-blend frames that have been alpha-blended together</summary>
  class Deblender {

    /// <summary>Initializes a new de-blender</summary>
    public: Deblender();
    /// <summary>Frees all resources used by the instance</summary>
    public: ~Deblender() = default;

    public: virtual void Deblend(QImage &target, const QImage &imageToRemove) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deblending

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEBLENDING_DEBLENDER_H
