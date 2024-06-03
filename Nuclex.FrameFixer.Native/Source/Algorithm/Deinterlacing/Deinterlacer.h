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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"

#include <QImage>

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

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

  /// <summary>Deinterlacer that uses some tricks that mostly work on anime only</summary>
  class Deinterlacer {

    /// <summary>Frees all resources used by the instance</summary>
    public: ~Deinterlacer() = default;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: virtual std::string GetName() const = 0;

    /// <summary>Called before the deinterlacer is used by the application</summary>
    /// <remarks>
    ///   This call should be optional. It gives the deinterlacer a chance to initialize
    ///   any libraries or devices is uses (for example, load a neural network or set
    ///   up a filter graph in a decoder library such as libav).
    /// </remarks>
    public: virtual void WarmUp() {}

    /// <summary>Called when the deinterlacer is deselected for the time being</summary>
    /// <remarks>
    ///   Rather than load all resources and access all libraries in the constructor,
    ///   deinterlacers should do so on first use or when <see cref="WarmUp" /> is called.
    ///   Similarly, when the user picks another deinterlacer in the application, this
    ///   method is called where the deinterlacer should drop any larger resources it is
    ///   holding onto in order to not waste memory will it is not being used.
    /// </remarks>
    public: virtual void CoolDown() {}

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: virtual bool NeedsPriorFrame() const { return false; }

    /// <summary>Whether this deinterlacer needs to know the next frame</summary>
    /// <returns>True if the deinterlacer needs the next frame to work with</returns>
    public: virtual bool NeedsNextFrame() const { return false; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the prior frame is available anyway),
    ///   using the <see cref="NeedsPriorFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: virtual void SetPriorFrame(const QImage &priorFrame) { (void)priorFrame; }

    /// <summary>Assigns the next frame to the deinterlacer</summary>
    /// <param name="nextFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the next frame is available anyway),
    ///   using the <see cref="NeedsNextFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: virtual void SetNextFrame(const QImage &nextFrame) { (void)nextFrame; }

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: virtual void Deinterlace(QImage &target, DeinterlaceMode mode) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_DEINTERLACER_H
