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

#ifndef NUCLEX_FRAMEFIXER_SERVICES_DEINTERLACERREPOSITORY_H
#define NUCLEX_FRAMEFIXER_SERVICES_DEINTERLACERREPOSITORY_H

#include "Nuclex/FrameFixer/Config.h"

#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages the set of deinterlacers that can be used in the application</summary>
  class DeinterlacerRepository {

    // List of deinterlacers
    public: typedef std::vector<
      std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer>
    > DeinterlacerList;

    /// <summary>Initializes a new deinterlacer repository</summary>
    public: DeinterlacerRepository();
    /// <summary>Frees the resources and drops all deinterlacers</summary>
    public: ~DeinterlacerRepository();

    /// <summary>Registers all deinterlacers that are built into the application</summary>
    public: void RegisterBuiltInDeinterlacers();

#if defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)
    /// <summary>Registers all deinterlacers that rely on ffmpeg's libav being linked</summary>
    public: void RegisterLibAvDeinterlacers();
#endif

    /// <summary>Provides access to the list containing all registered deinterlacers</summary>
    public: const DeinterlacerList &GetDeinterlacers() const;

    /// <summary>Retrieves the default, basic deinterlacer</summary>
    /// <returns>The basic deinterlacer used by default</returns>
    public: std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> GetBasicDeinterlacer() const;

    /// <summary>Retrieves a deinterlacer by its index in the list</summary>
    /// <param name="index">Index of the deinterlacer that will be retrieved</param>
    /// <returns>The deinterlacer with the specified index</returns>
    public: std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> GetDeinterlacer(
      std::size_t index
    ) const;

    /// <summary>Stores all deinterlacers that have been registered</summary>
    private: DeinterlacerList deinterlacers;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services

#endif // NUCLEX_FRAMEFIXER_SERVICES_DEINTERLACERREPOSITORY_H
