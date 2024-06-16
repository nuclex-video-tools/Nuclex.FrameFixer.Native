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

#include "./InterpolatorRepository.h"

#include "../Algorithm/Interpolation/NullFrameInterpolator.h"
#include "../Algorithm/Interpolation/ExternalRifeFrameInterpolator.h"

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  InterpolatorRepository::InterpolatorRepository() {}

  // ------------------------------------------------------------------------------------------- //

  InterpolatorRepository::~InterpolatorRepository() {}

  // ------------------------------------------------------------------------------------------- //

  void InterpolatorRepository::RegisterBuiltInInterpolators() {
    this->interpolators.push_back(
      std::make_shared<Algorithm::Interpolation::NullFrameInterpolator>()
    );
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_FRAMEFIXER_ENABLE_CLI_INTERPOLATORS)
  void InterpolatorRepository::RegisterCliInterpolators() {
    this->interpolators.push_back(
      std::make_shared<Algorithm::Interpolation::ExternalRifeFrameInterpolator>()
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const InterpolatorRepository::InterpolatorList &InterpolatorRepository::GetInterpolators() const {
    return this->interpolators;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<
    Algorithm::Interpolation::FrameInterpolator
  > InterpolatorRepository::GetNullInterpolator() const {
    std::size_t count = this->interpolators.size();
    for(std::size_t index = 0; index < count; ++index) {
      Algorithm::Interpolation::FrameInterpolator *interpolator = (
        this->interpolators[index].get()
      );
      if(typeid(*interpolator) == typeid(Algorithm::Interpolation::NullFrameInterpolator)) {
        return this->interpolators[index];
      }
    }

    return std::make_shared<Algorithm::Interpolation::NullFrameInterpolator>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<
    Algorithm::Interpolation::FrameInterpolator
  > InterpolatorRepository::GetInterpolator(std::size_t index) const {
    return this->interpolators.at(index);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services
