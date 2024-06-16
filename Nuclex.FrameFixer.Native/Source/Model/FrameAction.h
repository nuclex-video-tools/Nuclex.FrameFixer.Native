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

#ifndef NUCLEX_FRAMEFIXER_FRAMEACTION_H
#define NUCLEX_FRAMEFIXER_FRAMEACTION_H

#include "Nuclex/FrameFixer/Config.h"

#include <cstddef> // for std::size_t
#include <string> // for std::string

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Types of frames appearing in a telecined movie</summary>
  enum class FrameAction {

    /// <summary>Skip this frame when rendering</summary>
    Discard = -1,

    /// <summary>Keep the frame and emit it normally when rendering</summary>
    Unknown = 0,

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

    /// <summary>Frame is being replaced with another frame from the movie</summary>
    Replace,

    /// <summary>Frame is a mix between other frames (same behavior as progressive)</summary>
    /// <remarks>
    ///   This merely tags the frame as blended. Such frames are the result of nasty
    ///   deinterlacing software and at best can be mathematically deduced from
    ///   the surrounding frames (which this application current can't do, so you can
    ///   at least tag the frames as blended for later fixing)
    /// </remarks>
    Deblend,
#if 0
    /// <summary>Generate an extra frame by interpolating with the predecessor</summary>
    /// <remarks>
    ///   This always uses the current and the predecessor frame to synthesize an additional
    ///   frame and insert it before the current frame. This is the simplest form of frame
    ///   interpolation and can be used, for example, to replace a damaged frame or if
    ///   some anime scroller animates not at 24 but at 18 fps, so instead of pushing
    ///   a stutter every fourth frame, you can instead generate an intermediate frame.
    /// </remarks>
    InsertInterpolatedBefore,
#endif
    /// <summary>Interpolate frame from two neighbouring frames</summary>
    /// <remarks>
    ///   Uses an interpolator to generate this frame using the contents of the frames
    ///   before and after it. This is either an option for restoring broken frames,
    ///   or to selectively increase the frame rate in anime shows where individual
    ///   scenes use an especially low animation frame rate (anime shows are usually
    ///   animated at 12 fps and some parts may even update at 9 fps). 
    /// </remarks>
    Interpolate,

    // REMOVE ALL AFTER HERE -------------------------------------------------------------

    /// <summary>Frame is progressive but unknown if A, B, D or standalone</summary>
    Progressive,

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
    TopFieldFirst,
    /// <summary>Third frame overlaid with the fields of the fourth frame</summary>
    /// <remarks>
    ///   Takes the top field (all even lines) from this frame and the bottom field
    ///   (all odd lines) from the previous frame, ending up with pure C.
    /// </remarks>
    BottomFieldFirst,
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
    TopFieldOnly,
    /// <summary>Overlaid fields of the fourth frame missing its other half</summary>
    /// <remarks>
    ///   Only takes the bottom field (all odd lines) and interpolates the missing lines
    ///   under the assumption that the predecesor frame is missing or unusable.
    /// </remarks>
    BottomFieldOnly

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FRAMEACTION_H
