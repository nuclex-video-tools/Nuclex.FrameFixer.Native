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

#ifndef NUCLEX_FRAMEFIXER_MOVIE_H
#define NUCLEX_FRAMEFIXER_MOVIE_H

#include "Nuclex/FrameFixer/Config.h"
#include "./Frame.h"

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr

#include <Nuclex/Platform/Tasks/CancellationWatcher.h> // for CancellationWatcher

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores the list of all frames in a movie</summary>
  class Movie {

    typedef Nuclex::Platform::Tasks::CancellationWatcher CancellationWatcher;

    /// <summary>Path to the directory in which the frame images are stored</summary>
    public: std::string FrameDirectory;
    /// <summary>Informations about each frame in the movie</summary>
    public: std::vector<Frame> Frames;

    /// <summarys>Sets up a movie using images stored in a folder</summary>
    /// <param name="path">Path in which the movie's frames are stored</param>
    /// <param name="cancellationWatcher">Allows the scan to be cancelled</param>
    /// <returns>A movie with all frames set up
    public: static std::shared_ptr<Movie> FromImageFolder(
      const std::string &path,
      const std::shared_ptr<const CancellationWatcher> &cancellationWatcher = (
        std::shared_ptr<const CancellationWatcher>()
      )
    );

    /// <summary>Stores the state of the movie in a text file</summary>
    /// <remarks>
    ///   The text file is placed next to the frame directory with a specific extension,
    ///   which is where the <see cref="FromImageFolder" /> method will look for it.
    /// </remarks>
    public: void SaveState() const;

    /// <summary>Reconstitutes the full path to the image file for a specific frame</summary>
    /// <param name="frameIndex">Index of the frame whose path will be returned</param>
    /// <returns>The full path to the image file storing the requested frame</returns>
    public: std::string GetFramePath(std::size_t frameIndex) const;

    private: static std::string getStateFilePath(const std::string &frameDirectoryPath);

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_MOVIE_H
