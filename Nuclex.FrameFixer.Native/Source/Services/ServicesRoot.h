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

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

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

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services

#endif // NUCLEX_FRAMEFIXER_SERVICES_SERVICESROOT_H
