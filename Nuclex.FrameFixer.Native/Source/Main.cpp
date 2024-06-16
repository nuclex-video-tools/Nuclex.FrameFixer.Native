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

#include "Nuclex/FrameFixer/Config.h"

#include "./Services/ServicesRoot.h"
#include "./Services/DeinterlacerRepository.h"
#include "./Services/InterpolatorRepository.h"
#include "./MainWindow.h"

#include <QApplication>
#include <QMessageBox>

// --------------------------------------------------------------------------------------------- //

/// <summary>Entry point for the application</summary>
/// <param name="argc">The number of command line arguments provided</param>
/// <param name="argv">The values of all command line arguments</param>
/// <returns>The exit code the application has terminated with</returns>
int main(int argc, char *argv[]) {
  int exitCode;
  {
    QApplication application(argc, argv);

    // Create the service provider (we use a simple class that ties all the services
    // together instead of a full-blown IoC container to keep things simple).
    std::shared_ptr<Nuclex::FrameFixer::Services::ServicesRoot> servicesRoot;
    try {
      servicesRoot = std::make_shared<Nuclex::FrameFixer::Services::ServicesRoot>();
      servicesRoot->Deinterlacers()->RegisterBuiltInDeinterlacers();
#if defined(NUCLEX_FRAMEFIXER_ENABLE_LIBAV)
      servicesRoot->Deinterlacers()->RegisterLibAvDeinterlacers();
#endif
      servicesRoot->Interpolators()->RegisterBuiltInInterpolators();
#if defined(NUCLEX_FRAMEFIXER_ENABLE_CLI_INTERPOLATORS)
      servicesRoot->Interpolators()->RegisterCliInterpolators();
#endif
      
      //servicesRoot->GetSettings()->LoadOrUseDefaults();

      //std::string snapshotDatabasePath = (
      //  servicesRoot->GetSettings()->GetSnapshotDatabasePath()
      //);
      //if(!snapshotDatabasePath.empty()) {
      //  servicesRoot->GetSnapshotDatabase()->OpenOrCreateDatabase(snapshotDatabasePath);
      //}
    }
    catch(const std::exception &error) {
      std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>();
      messageBox->setText(
        QString(u8"The application failed to launch because of an error\n") +
        QString(error.what())
      );
      messageBox->setStandardButtons(QMessageBox::Ok);
      messageBox->setDefaultButton(QMessageBox::Ok);

      messageBox->exec();
      return -1;
    }

    std::shared_ptr<Nuclex::FrameFixer::MainWindow> mainWindow = (
      std::make_shared<Nuclex::FrameFixer::MainWindow>()
    );
    mainWindow->BindToServicesRoot(servicesRoot);
    mainWindow->show();

    exitCode = application.exec();
  }

  return exitCode;
}

// --------------------------------------------------------------------------------------------- //
