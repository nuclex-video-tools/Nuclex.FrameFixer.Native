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
