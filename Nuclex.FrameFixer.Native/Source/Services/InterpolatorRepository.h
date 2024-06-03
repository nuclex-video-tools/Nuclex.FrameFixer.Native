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

#ifndef NUCLEX_FRAMEFIXER_SERVICES_INTERPOLATORREPOSITORY_H
#define NUCLEX_FRAMEFIXER_SERVICES_INTERPOLATORREPOSITORY_H

#include "Nuclex/FrameFixer/Config.h"

#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  class FrameInterpolator;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages the set of interpolators that can be used in the application</summary>
  class InterpolatorRepository {

    // List of interpolators
    public: typedef std::vector<
      std::shared_ptr<Algorithm::Interpolation::FrameInterpolator>
    > InterpolatorList;

    /// <summary>Initializes a new interpolator repository</summary>
    public: InterpolatorRepository();
    /// <summary>Frees the resources and drops all interpolators</summary>
    public: ~InterpolatorRepository();

    /// <summary>Registers all interpolators that are built into the application</summary>
    public: void RegisterBuiltInInterpolators();

#if defined(NUCLEX_FRAMEFIXER_ENABLE_CLI_INTERPOLATORS)
    /// <summary>Registers all interpolators that rely on external CLI executables</summary>
    public: void RegisterCliInterpolators();
#endif

    /// <summary>Provides access to the list containing all registered interpolators</summary>
    public: const InterpolatorList &GetInterpolators() const;

    /// <summary>Retrieves the default, null interpolator</summary>
    /// <returns>The null interpolator  used by default</returns>
    public: std::shared_ptr<
      Algorithm::Interpolation::FrameInterpolator
    > GetNullInterpolator() const;

    /// <summary>Retrieves a interpolator by its index in the list</summary>
    /// <param name="index">Index of the interpolator that will be retrieved</param>
    /// <returns>The interpolator with the specified index</returns>
    public: std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> GetInterpolator(
      std::size_t index
    ) const;

    /// <summary>Stores all interpolators that have been registered</summary>
    private: InterpolatorList interpolators;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services

#endif // NUCLEX_FRAMEFIXER_SERVICES_INTERPOLATORREPOSITORY_H
