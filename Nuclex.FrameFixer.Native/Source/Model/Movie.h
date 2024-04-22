#pragma region CPL License
/*
Nuclex Telecide
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

#ifndef NUCLEX_TELECIDE_MOVIE_H
#define NUCLEX_TELECIDE_MOVIE_H

#include "../Config.h"
#include "./Frame.h"

#include <vector> // for std::vector

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores the list of all frames in a movie</summary>
  class Movie {

    /// <summary>Path to the directory in which the frame images are stored</summary>
    public: std::string FrameDirectory;
    /// <summary>Informations about each frame in the movie</summary>
    public: std::vector<Frame> Frames;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_MOVIE_H
