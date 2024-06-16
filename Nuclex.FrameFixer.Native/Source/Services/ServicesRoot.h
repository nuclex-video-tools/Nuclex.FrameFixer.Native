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

#ifndef NUCLEX_FRAMEFIXER_SERVICES_SERVICESROOT_H
#define NUCLEX_FRAMEFIXER_SERVICES_SERVICESROOT_H

#include "Nuclex/FrameFixer/Config.h"

#include <memory> // for std::unique_ptr

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  class DeinterlacerRepository;
  class InterpolatorRepository;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Stand-in for a small service locator, aka a bunch of app-global variables
  /// </summary>
  class ServicesRoot {

    /// <summary>Initializes a new service container</summary>
    public: ServicesRoot();
    /// <summary>Frees the resources and drops all provided services</summary>
    public: ~ServicesRoot();

    /// <summary>Accesses the repository of knwon deinterlacers</summary>
    /// <returns>The application's deinterlacer repository</returns>
    public: const std::shared_ptr<DeinterlacerRepository> &Deinterlacers() const {
      return this->deinterlacers;
    }

    /// <summary>Accesses the repository of knwon interpolators</summary>
    /// <returns>The application's interpolator repository</returns>
    public: const std::shared_ptr<InterpolatorRepository> &Interpolators() const {
      return this->interpolators;
    }

    /// <summary>Manages the deinterlacers available for use by the application<?summary>
    private: std::shared_ptr<DeinterlacerRepository> deinterlacers;
    /// <summary>Manages the interpolators available for use by the application</summary>
    private: std::shared_ptr<InterpolatorRepository> interpolators;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services

#endif // NUCLEX_FRAMEFIXER_SERVICES_SERVICESROOT_H
