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

#ifndef NUCLEX_TELECIDE_FRAME_H
#define NUCLEX_TELECIDE_FRAME_H

#include "Nuclex/Telecide/Config.h"
#include "./FrameType.h"

#include <cstddef> // for std::size_t
#include <string> // for std::string
#include <optional> // for std::optional

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations about a single frame in a movie clip</summary>
  class Frame {

    /// <summary>Initializes a new frame</summary>
    /// <param name="filename">File name (without path) of the image storing the frame</param>
    public: Frame(const std::string &filename) :
      Index(std::size_t(-1)),
      Filename(filename),
      Type(FrameType::Unknown),
      Combedness(),
      ProvisionalType(FrameType::Unknown) {}

    /// <summary>Absolute index of the frame from the beginning of the movie</summary>
    public: std::size_t Index;
    /// <summary>Name of the image file to which the frame has been extracted</summary>
    public: std::string Filename;
    /// <summary>Type of this frame in the telecide sequence</summary>
    public: FrameType Type;
    /// <summary>Amount of combing that was detected in the frame</summary>
    public: std::optional<double> Combedness; 
    /// <summary>Extrapolation point between previous and this frame</summary>
    public: std::optional<double> MixFactor; 

    /// <summary>Type according to the telecine pattern</summary>
    public: FrameType ProvisionalType;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_FRAME_H
