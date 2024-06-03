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
