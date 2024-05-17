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

    /// <summary>Frame is unusable and should be thrown away</summary>
    Discard = -1,

    /// <summary>Frame type has not been detected or assigned yet</summary>
    Unknown = 0,

    /// <summary>Frame is the first frame in a telecine sequence</summary>
    /// <remarks>
    ///   A simple progressive frame but tagged for where it is in the telecine rhythm
    /// </remarks>
    A,
    /// <summary>Frame is the second frame in a telecine sequence</summary>
    /// <remarks>
    ///   A simple progressive frame but tagged for where it is in the telecine rhythm
    /// </remarks>
    B,
    /// <summary>Second frame overlaid with the fields of the third frame</summary>
    /// <remarks>
    ///   Takes the bottom field (all odd lines) from this frame and the top field
    ///   (all even lines) from the previous frame, ending up with pure B.
    /// </remarks>
    BC,
    /// <summary>Third frame overlaid with the fields of the fourth frame</summary>
    /// <remarks>
    ///   Takes the top field (all even lines) from this frame and the bottom field
    ///   (all odd lines) from the previous frame, ending up with pure C.
    /// </remarks>
    CD,
    /// <summary>Frame is the fourth frame in a telecine sequence</summary>
    /// <remarks>
    ///   A simple progressive frame but tagged for where it is in the telecine rhythm
    /// </remarks>
    D,

    /// <summary>Overlaid fields of the third frame missing its other half</summary>
    /// <remarks>
    ///   Only takes the top field (all even lines) and interpolates the missing lines
    ///   under the assumption that the predecesor frame is missing or unusable.
    /// </remarks>
    TopC,
    /// <summary>Overlaid fields of the fourth frame missing its other half</summary>
    /// <remarks>
    ///   Only takes the bottom field (all odd lines) and interpolates the missing lines
    ///   under the assumption that the predecesor frame is missing or unusable.
    /// </remarks>
    BottomC,

    /// <summary>Frame is repeating the contents of the prior frame</summary>
    /// <remarks>
    ///   Can be set if the frame identical to the previous frame (disregarding compression
    ///   artifacts introduced during encoding). Both frames will be kept, but will be
    ///   averaged with each other. If more than one frame is tagged with this,
    ///   all successive frames will be involved in the averaging calculation.
    /// </remarks>
    Average,

    /// <summary>Duplicate this frame</summary>
    /// <remarks>
    ///   Does exactly as it says on the tin. The frame will be repeated once on export.
    /// </remarks>
    Duplicate,

    /// <summary>Triplicate this frame</summary>
    /// <remarks>
    ///   Does exactly as it says on the tin. The frame will be repeated twice on export.
    /// </remarks>
    Triplicate,

#if 0 // This can be in addition to interlacing. D'oh! Will be a separate frame attribute.

    /// <summary>Tries to de-blend the second of two unequally composited frames</summary>
    /// <remarks>
    ///   The amount of weird mastering errors on DVDs is staggering. Some producers managed
    ///   to release DVDs containing frame blended together. This tries to resolve the case
    ///   where in the previous frame, the contents of the B frame are a bit stronger and
    ///   where in this frame, the contents of the C frame are a bit stronger.
    /// </remarks>
    BlendedMix,

    /// <summary>Tries to de-blend the second of two vertically blended frames</summary>
    /// <remarks>
    ///   This curiousity happens when, rather than digitally converting between PAL and NTSC,
    ///   someone put an NTSC camera in front of a PAL display. The camera collects pictures
    ///   where the TV has only partially updated new image. Combined with phosphorescence,
    ///   the result is a vertical transition area, randomly position, where the current
    ///   frame blends over the previous frame.
    /// </remarks>
    VerticalMix,

#endif

    /// <summary>Frame is progressive but unknown if A, B, D or standalone</summary>
    Progressive

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_FRAMETYPE_H
