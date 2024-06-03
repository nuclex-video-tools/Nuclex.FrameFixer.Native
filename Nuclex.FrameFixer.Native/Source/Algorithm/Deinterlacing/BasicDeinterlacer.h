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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_BASICDEINTERLACER_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_BASICDEINTERLACER_H

#include "Nuclex/FrameFixer/Config.h"
#include "./Deinterlacer.h"

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Cheapest possible deinterlacer that simply interpolates a field</summary>
  class BasicDeinterlacer : public Deinterlacer {

    /// <summary>Initializes the basic deinterlacer</summary>
    public: BasicDeinterlacer() = default;
    /// <summary>Frees all resources used by the instance</summary>
    public: virtual ~BasicDeinterlacer() = default;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: std::string GetName() const override {
      return u8"Basic: copy or interpolate missing fields";
    }

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: bool NeedsPriorFrame() const override { return true; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the prior frame is available anyway),
    ///   using the <see cref="NeedsPriorFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: void SetPriorFrame(const QImage &priorFrame) override;

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: void Deinterlace(QImage &target, DeinterlaceMode mode) override;

    /// <summary>Cheaply deinterlaces the specified image</summary>
    /// <param name="previousImage">
    ///   Image that came before the current one. If provided, the missing rows will be
    ///   taken from this image. Otherwise, the missing rows are interpolated.
    /// </param>
    /// <param name="image">Image that will be deinterlaced</param>
    /// <param name="topField">
    ///   If true, the top field (even rows) will be filled in,
    ///   otherwise, the bottom field (odd rows) will be filled in
    /// </param>
    public: static void Deinterlace(
      QImage *previousImage, QImage &image, bool topField = true
    );

    /// <summary>The frame preceding the current one</summary>
    private: QImage priorFrame;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_DEINTERLACING_BASICDEINTERLACER_H
