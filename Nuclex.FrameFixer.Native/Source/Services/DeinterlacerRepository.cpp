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

#include "./DeinterlacerRepository.h"
#include "../Algorithm/Deinterlacing/Deinterlacer.h"

#include "../Algorithm/Deinterlacing/BasicDeinterlacer.h"
#include "../Algorithm/Deinterlacing/LibAvNNedi3Deinterlacer.h"
#include "../Algorithm/Deinterlacing/LibAvYadifDeinterlacer.h"
#include "../Algorithm/Deinterlacing/LibAvEstdifDeinterlacer.h"

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  DeinterlacerRepository::DeinterlacerRepository() {}

  // ------------------------------------------------------------------------------------------- //

  DeinterlacerRepository::~DeinterlacerRepository() {}

  // ------------------------------------------------------------------------------------------- //

  void DeinterlacerRepository::RegisterBuiltInDeinterlacers() {
    this->deinterlacers.push_back(
      std::make_shared<Algorithm::Deinterlacing::BasicDeinterlacer>()
    );
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)
  void DeinterlacerRepository::RegisterLibAvDeinterlacers() {
    this->deinterlacers.push_back(
      std::make_shared<Algorithm::Deinterlacing::LibAvNNedi3Deinterlacer>()
    );
    this->deinterlacers.push_back(
      std::make_shared<Algorithm::Deinterlacing::LibAvYadifDeinterlacer>(false)
    );
    this->deinterlacers.push_back(
      std::make_shared<Algorithm::Deinterlacing::LibAvYadifDeinterlacer>(true)
    );
    this->deinterlacers.push_back(
      std::make_shared<Algorithm::Deinterlacing::LibAvEstdifDeinterlacer>()
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const DeinterlacerRepository::DeinterlacerList &DeinterlacerRepository::GetDeinterlacers() const {
    return this->deinterlacers;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<
    Algorithm::Deinterlacing::Deinterlacer
  > DeinterlacerRepository::GetBasicDeinterlacer() const {
    std::size_t count = this->deinterlacers.size();
    for(std::size_t index = 0; index < count; ++index) {
      Algorithm::Deinterlacing::Deinterlacer *deinterlacer = this->deinterlacers[index].get();
      if(typeid(*deinterlacer) == typeid(Algorithm::Deinterlacing::BasicDeinterlacer)) {
        return this->deinterlacers[index];
      }
    }

    return std::make_shared<Algorithm::Deinterlacing::BasicDeinterlacer>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<
    Algorithm::Deinterlacing::Deinterlacer
  > DeinterlacerRepository::GetDeinterlacer(std::size_t index) const {
    return this->deinterlacers.at(index);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services
