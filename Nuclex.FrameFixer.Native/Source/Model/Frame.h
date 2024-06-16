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

#ifndef NUCLEX_FRAMEFIXER_FRAME_H
#define NUCLEX_FRAMEFIXER_FRAME_H

#include "Nuclex/FrameFixer/Config.h"
#include "./DeinterlaceMode.h"
#include "./FrameAction.h"

#include <cstddef> // for std::size_t
#include <string> // for std::string
#include <map> // for std::pair
#include <optional> // for std::optional

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations about a single frame in a movie clip</summary>
  class Frame {

    /// <summary>Initializes a new frame</summary>
    /// <param name="filename">File name (without path) of the image storing the frame</param>
    public: Frame(const std::string &filename) :
      Index(std::size_t(-1)),
      Filename(filename),
      Mode(),
      Action(FrameAction::Unknown),
      LeftOrReplacementIndex(),
      InterpolationSourceIndices(),
      AlsoInsertInterpolatedAfter(),
      Combedness(),
      MixFactor(),
      ProvisionalMode(DeinterlaceMode::Dont) {}

    /// <summary>Absolute index of the frame from the beginning of the movie</summary>
    public: std::size_t Index;
    /// <summary>Name of the image file to which the frame has been extracted</summary>
    public: std::string Filename;

    /// <summary>How the deinterlacer should treat this frame</summary>
    /// <remarks>
    ///   Deinterlacing, if requested, will always be done first, regardless of whether
    ///   the frame is tagged to be duplicated, averaged or interpolated.
    /// </remarks>
    public: std::optional<DeinterlaceMode> Mode;
    /// <summary>Action that should be taken with this frame when it is rendered</summary>
    public: FrameAction Action;

    // TODO: Combine LeftOrReplacementIndex and InterpolationSourceIndices
    //   1 index = use that frame
    //   2 indices = interpolate between those frames

    /// <summary>Frame with which this one should be replaced</summary>
    public: std::optional<std::size_t> LeftOrReplacementIndex;
    /// <summary>Frames which will be interpolated to form this one</summary>
    public: std::optional<std::pair<std::size_t, std::size_t>> InterpolationSourceIndices;
    /// <summary>Whether to also insert an interpolated frame after this one</summary>
    public: std::optional<bool> AlsoInsertInterpolatedAfter;

    /// <summary>Amount of combing that was detected in the frame</summary>
    public: std::optional<double> Combedness; 
    /// <summary>Extrapolation point between previous and this frame</summary>
    public: std::optional<double> MixFactor; 

    /// <summary>Type according to the telecine pattern</summary>
    public: DeinterlaceMode ProvisionalMode;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FRAME_H
