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
