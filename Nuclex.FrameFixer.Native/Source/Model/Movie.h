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
