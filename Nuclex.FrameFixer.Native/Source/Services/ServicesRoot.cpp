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

#include "./ServicesRoot.h"
#include "./DeinterlacerRepository.h"
#include "./InterpolatorRepository.h"

#include <string> // for std::string

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Application name for the directory resolver</summary>
  /// <remarks>
  ///   This decides the name of the settings directory under ~/.config/ and such things.
  /// </remarks>
  const std::string appName(u8"frame-fixer");

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  ServicesRoot::ServicesRoot() :
    deinterlacers(std::make_shared<DeinterlacerRepository>()),
    interpolators(std::make_shared<InterpolatorRepository>()) {}

  // ------------------------------------------------------------------------------------------- //

  ServicesRoot::~ServicesRoot() {}

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services
