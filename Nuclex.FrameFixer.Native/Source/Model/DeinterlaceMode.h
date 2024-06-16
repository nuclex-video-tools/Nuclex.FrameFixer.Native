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
