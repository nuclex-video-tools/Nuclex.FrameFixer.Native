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

#ifndef NUCLEX_FRAMEFIXER_DEINTERLACEMODE_H
#define NUCLEX_FRAMEFIXER_DEINTERLACEMODE_H

#include "Nuclex/FrameFixer/Config.h"

#include <cstddef> // for std::size_t
#include <string> // for std::string

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Method by which a frame should be deinterlaced</summary>
  enum class DeinterlaceMode {

    /// <summary>Don't deinterlace at all, i.e. frame is progressive</summary>
    Dont = -1,

    /// <summary>Deinterlace by only using the lines from the top field</summary>
    /// <remarks>
    ///   This should be used if the already interlaced video was cut or otherwise processed
    ///   in a way that left it without a matching bottom field. It may also produce a cleaner
    ///   image when video was encoded in YUV-422 or lower without interlace awareness.
    /// </remarks>
    TopFieldOnly,

    /// <summary>Deinterlace by only using the lines from the bottom field</summary>
    /// <remarks>
    ///   This should be used if the already interlaced video was cut or otherwise processed
    ///   in a way that left it without a matching bottom field. It may also produce a cleaner
    ///   image when video was encoded in YUV-422 or lower without interlace awareness.
    /// </remarks>
    BottomFieldOnly,

    /// <summary>Even lines from current frame, odd lines from preceding frame</summary>
    /// <remarks>
    ///   The normal interlace process for frames with an even index, assuming the content
    ///   is top-field-first and there are no skips/jumps in the interlace rhythm.
    ///   This is where most deinterlacers can shine, i.e. Yadif
    /// </remarks>
    TopFieldFirst,

    /// <summary>Odd lines from current frame, even lines from preceding frame</summary>
    /// <remarks>
    ///   The normal interlace process for frames with an even index, assuming the content
    ///   is top-field-first and there are no skips/jumps in the interlace rhythm.
    /// </remarks>
    BottomFieldFirst

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_DEINTERLACEMODE_H
