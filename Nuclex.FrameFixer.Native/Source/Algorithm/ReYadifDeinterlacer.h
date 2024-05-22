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

#ifndef NUCLEX_TELECIDE_ALGORITHM_REYADIFDEINTERLACER_H
#define NUCLEX_TELECIDE_ALGORITHM_REYADIFDEINTERLACER_H

#include "Nuclex/Telecide/Config.h"
#include "./Deinterlacer.h"

namespace Nuclex::Telecide::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summaryDeinterlacer that integrates the Yadif algorithm</summary>
  class ReYadifDeinterlacer : public Deinterlacer {

    /// <summary>Whether this deinterlacer needs to know the previous frame</summary>
    /// <returns>True if the deinterlacer needs the previous frame to work with</returns>
    public: bool NeedsPriorFrame() const override { return true; }

    /// <summary>Whether this deinterlacer needs to know the next frame</summary>
    /// <returns>True if the deinterlacer needs the next frame to work with</returns>
    public: bool NeedsNextFrame() const override { return true; }

    /// <summary>Assigns the prior frame to the deinterlacer</summary>
    /// <param name="priorFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the prior frame is available anyway),
    ///   using the <see cref="NeedsPriorFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: void SetPriorFrame(const QImage &priorFrame) override;

    /// <summary>Assigns the next frame to the deinterlacer</summary>
    /// <param name="nextFrame">QImage containing the previous frame</param>
    /// <remarks>
    ///   This can either always be called (if the next frame is available anyway),
    ///   using the <see cref="NeedsNextFrame" /> method, can potentially be omitted
    ///   depending on the actual deinterlacer implementation.
    /// </remarks>
    public: void SetNextFrame(const QImage &nextFrame) override;

    /// <summary>Deinterlaces the specified frame</summary>
    /// <param name="target">Frame that will be deinterlaced</param>
    /// <param name="mode">
    ///   How to deinterlace the frame (indicates if the top field is first or if
    ///   the bottom field is first, or if special measures need to be taken)
    /// </param>
    public: void Deinterlace(QImage &target, DeinterlaceMode mode) override;

    /// <summary>The frame preceding the current one</summary>
    private: QImage priorFrame;
    /// <summary>The frame following the current one</summary>
    private: QImage nextFrame;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm

#endif // NUCLEX_TELECIDE_ALGORITHM_REYADIFDEINTERLACER_H
