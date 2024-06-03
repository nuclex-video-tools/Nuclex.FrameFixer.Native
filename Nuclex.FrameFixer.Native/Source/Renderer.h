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

#ifndef NUCLEX_FRAMEFIXER_RENDERER_H
#define NUCLEX_FRAMEFIXER_RENDERER_H

#include "Nuclex/FrameFixer/Config.h"

#include <memory> // for std;:shared_ptr
#include <cstddef> // for std::size_t
#include <string> // for std::string
#include <optional> // for std::optional

#include <QImage>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  class Movie;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  class FrameInterpolator;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Processes and saves the processed frames of a movie clip to files</summary>
  class Renderer {

    /// <summary>Initializes a new renderer</summary>
    public: Renderer();
    /// <summary>Frees all resources the renderer is using</summary>
    public: ~Renderer();

    /// <summary>Selects the deinterlacer the renderer should use</summary>
    /// <param name="deinterlacer">Deinterlacer the renderer will be using</param>
    public: void SetDeinterlacer(
      const std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> &deinterlacer
    );

    /// <summary>Selects the interpolator the renderer should use</summary>
    /// <param name="interpolator">Interpolator the renderer will be using</param>
    public: void SetInterpolator(
      const std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> &interpolator
    );

    /// <summary>Enables or disables flipping of the top and bottom fields</summary>
    /// <param name="flip">True to flip the top and bottom fields</param>
    public: void FlipTopAndBottomField(bool flip = true);

    /// <summary>
    ///   Limits the frames being rendered to those produced by the specified input frames
    /// </summary>
    /// <param name="startFrameIndex">First input frame that will be considered</param>
    /// <param name="endFrameIndex">Last input frame that will be considered</param>
    /// <remarks>
    ///   This does not influence the numbering of output frames. If rendering frames
    ///   from 0 to 1000 would produce output frames 0 to 500, then rendering frames from
    ///   500 to 100 would regenerate identical output frames numbeed from 250 too 500.
    /// </remarks>
    public: void RestrictRangeOfInputFrames(
      std::size_t startFrameIndex, std::size_t endFrameIndex
    );

    /// <summary>
    ///   Limits the frames being rendered to those with the specified output frame numbers
    /// </summary>
    /// <param name="startFrameIndex">First output frame that will be written</param>
    /// <param name="endFrameIndex">Last output frame that will be written</param>
    public: void RestrictRangeOfOutputFrames(
      std::size_t startFrameIndex, std::size_t endFrameIndex
    );

    /// <summary>Processes and saves a movie's frames into the specified directory</summary>
    /// <param name="movie">Movie that will be processed (deinterlaced) and saved</param>
    /// <param name="directory">Directory in which the processed frames will be saved</param>
    public: void Render(const std::shared_ptr<Movie> &movie, const std::string &directory);

    /// <summary>Generates a preview image of a single frame without saving it</summary>
    /// <param name="movie">Movie of which a preview frame will be rendered</param>
    /// <param name="index">Index of the frame that will be rendered as a preview</param>
    /// <returns>A Qt image with a preview of the frame with the specified index</returns>
    /// <remarks>
    ///   The frame index is of an input frame, so it is possible to request previews of
    ///   frames that have been tagged as 'Discard' and would not be rendered. This is fine
    ///   and expected (as this method is directly used to display frames in the UI),
    ///   a discarded frame will simply be previewed as-is, with no processing applied.
    /// </remarks>
    public: QImage Preview(const std::shared_ptr<Movie> &movie, const std::size_t frameIndex);

    /// <summary>Deinterlacer the renderer is using on the input frames</summary>
    private: std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> deinterlacer;
    /// <summary>Interpolator the renderer is using on the input frames</summary>
    private: std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> interpolator;
    /// <summary>Range of input frames the renderer should process</summary>
    private: std::optional<std::pair<std::size_t, std::size_t>> inputFrameRange;
    /// <summary>Range of resulting output frames the rendere should save</summary>
    private: std::optional<std::pair<std::size_t, std::size_t>> outputFrameRange;
    /// <summary>Whether the top and bottom fields should be flipped</summary>
    private: bool flipFields;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_RENDERER_H
