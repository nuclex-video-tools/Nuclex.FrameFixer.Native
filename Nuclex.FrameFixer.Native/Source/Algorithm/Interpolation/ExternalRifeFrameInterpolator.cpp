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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./ExternalRifeFrameInterpolator.h"
#include <Nuclex/Support/Threading/Process.h> // for Process

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  ExternalRifeFrameInterpolator::ExternalRifeFrameInterpolator() {}

  // ------------------------------------------------------------------------------------------- //

  QImage ExternalRifeFrameInterpolator::Interpolate(const QImage &prior, const QImage &after) {
    std::unique_ptr<Nuclex::Support::Threading::Process> encoderProcess = (
      std::make_unique<Nuclex::Support::Threading::Process>(
        u8"/opt/rife-ncnn-vulkan-2022.10.29/rife-ncnn-vulkan-2"
      )
    );

    prior.save(u8"/tmp/hacky-prior.png", u8"PNG");
    after.save(u8"/tmp/hacky-after.png", u8"PNG");

    encoderProcess->SetWorkingDirectory(u8"/tmp");
    encoderProcess->Start(
      {
        u8"-m", u8"/opt/rife-ncnn-vulkan-2022.10.29/rife-anime",
        u8"-x",
        u8"-z",
        u8"-v",
        u8"-0", u8"/tmp/hacky-prior.png",
        u8"-1", u8"/tmp/hacky-after.png",
        u8"-o", u8"/tmp/hacky-out.png"
      }
    );
    int exitCode = encoderProcess->Join();
    if(exitCode != 0) {
      throw std::runtime_error(u8"rife-ncnn-vulkan failed.");
    }

    return QImage(u8"/tmp/hacky-out.png");
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Interpolation
