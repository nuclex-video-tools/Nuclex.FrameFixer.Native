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

#ifndef NUCLEX_TELECIDE_FRAMETYPE_H
#define NUCLEX_TELECIDE_FRAMETYPE_H

#include "Nuclex/Telecide/Config.h"

#include <cstddef> // for std::size_t
#include <string> // for std::string

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Types of frames appearing in a telecined movie</summary>
  enum class FrameType {

    /// <summary>Frame type has not been detected yet</summary>
    Unknown,
    /// <summary>Frame is the first frame in a telecine sequence</summary>
    A,
    /// <summary>Frame is the second frame in a telecine sequence</summary>
    B,
    /// <summary>Second frame overlaid with the fields of the third frame</summary>
    BC,
    /// <summary>Third frame overlaid with the fields of the fourth frame</summary>
    CD,
    /// <summary>Frame is the fourth frame in a telecine sequence</summary>
    D,
    /// <summary>Frame is progressive but unknown if A, B, D or standalone</summary>
    Progressive

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_FRAMETYPE_H
