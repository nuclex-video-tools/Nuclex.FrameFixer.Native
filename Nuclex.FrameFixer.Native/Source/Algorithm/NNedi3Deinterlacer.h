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

#ifndef NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H
#define NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H

#include "Nuclex/Telecide/Config.h"
#include "./Deinterlacer.h"

#include <QImage> // for QImage
#include <memory> // for std::shared_ptr

extern "C" {
  struct AVFilterGraph;
}

namespace Nuclex::Telecide::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Deinterlacer that uses libav's NNedi3 filter to deinterlace</summary>
  class NNedi3Deinterlacer : public Deinterlacer {

    /// <summary>Initializes the NNedi3 via libav deinterlacer</summary>
    public: NNedi3Deinterlacer();
    /// <summary>Frees all resources used by the deinterlacer</summary>
    public: virtual ~NNedi3Deinterlacer() = default;

    /// <summary>Returns a name by which the deinterlacer can be displayed</summary>
    /// <returns>A short, human-readable name for the deinterlacer</returns>
    public: std::string GetName() const override {
      return u8"NNEdi3-libav: Predict missing fields via AI";
    }

    /// <summary>Called before the deinterlacer is used by the application</summary>
    public: void WarmUp() override;

    /// <summary>Called when the deinterlacer is deselect for the time being</summary>
    public: void CoolDown() override;

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

    /// <summary>Creates or recreates the top field only filter graph as needed</summary>
    /// <param name="width">Width of the frames being processed</param>
    /// <param name="height">Height of the frames being processed</param>
    /// <param name="sixtyFourBitsPerPixel">Whether 16 bit color channels are used</param>
    private: void ensureTopFieldOnlyFilterGraphCreated(
      std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
    );
    
    /// <summary>Creates or recreates the bottom field only filter graph as needed</summary>
    /// <param name="width">Width of the frames being processed</param>
    /// <param name="height">Height of the frames being processed</param>
    /// <param name="sixtyFourBitsPerPixel">Whether 16 bit color channels are used</param>
    private: void ensureBottomFieldOnlyFilterGraphCreated(
      std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
    );

    /// <summary>Creates or recreates the top field first filter graph as needed</summary>
    /// <param name="width">Width of the frames being processed</param>
    /// <param name="height">Height of the frames being processed</param>
    /// <param name="sixtyFourBitsPerPixel">Whether 16 bit color channels are used</param>
    private: void ensureTopFieldFirstFilterGraphCreated(
      std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
    );
    
    /// <summary>Creates or recreates the bottom field first filter graph as needed</summary>
    /// <param name="width">Width of the frames being processed</param>
    /// <param name="height">Height of the frames being processed</param>
    /// <param name="sixtyFourBitsPerPixel">Whether 16 bit color channels are used</param>
    private: void ensureBottomFieldFirstFilterGraphCreated(
      std::size_t width, std::size_t height, bool sixtyFourBitsPerPixel
    );

    /// <summary>NNedi3 filter graph using only the top field</summary>
    private: std::shared_ptr<::AVFilterGraph> topFieldOnlyNnediFilterGraph;
    /// <summary>NNedi3 filter graph using only the bottom field</summary>
    private: std::shared_ptr<::AVFilterGraph> bottomFieldOnlyNnediFilterGraph;
    /// <summary>NNedi3 filter graph using the top field over the bottom field</summary>
    private: std::shared_ptr<::AVFilterGraph> topFieldFirstNnediFilterGraph;
    /// <summary>NNedi3 filter graph using the bottom field over the top field</summary>
    private: std::shared_ptr<::AVFilterGraph> bottomFieldFirstNnediFilterGraph;

    /// <summary>Width of the filter graphs that have been created</summary>
    private: std::size_t filterGraphWidth;
    /// <summary>Height of the filter graphs that have been created</summary>
    private: std::size_t filterGraphHeight;
    /// <summary>Whether the created filter graphs use 64 bits per pixel</summary>
    private: bool filterGraphSixtyFourBitsPerPixel;

    /// <summary>The frame preceding the current one</summary>
    private: QImage priorFrame;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide::Algorithm

#endif // NUCLEX_TELECIDE_ALGORITHM_NNEDI3DEINTERLACER_H
