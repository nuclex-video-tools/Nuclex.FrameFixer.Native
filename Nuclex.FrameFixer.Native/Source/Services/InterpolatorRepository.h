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
