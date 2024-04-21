#pragma region CPL License
/*
Nuclex CriuGui
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
#define NUCLEX_CRIUGUI_SOURCE 1

#include "./Config.h"

#include "./MainWindow.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <algorithm>>
//#include <tinywav.h>
#include <sys/stat.h>
#include <dirent.h>
//#include <AudioFile.h>
//#include <opusfile.h>

#include "./Telecide.h"

#include <Nuclex/Pixels/Storage/BitmapSerializer.h>
#include <Nuclex/Pixels/ColorModels/RgbPixelIterator.h>

#include <QApplication>
#include <QMessageBox>

// --------------------------------------------------------------------------------------------- //

namespace {

  bool fileExists(const std::string &path) {
    struct ::stat buffer;   
    return (stat(path.c_str(), &buffer) == 0);
  }

  bool isFolder(const std::string &path) {
    struct ::stat buffer;   
    if(stat(path.c_str(), &buffer) != 0) {
      return false;
    }

    return S_ISDIR(buffer.st_mode);
  }

  class Averager {

    public: Averager(std::size_t sampleCount) :
      maximumSampleCount(sampleCount),
      samples() {}

    public: void AddSample(double sample) {
      this->samples.push_back(sample);
      if(this->samples.size() > this->maximumSampleCount) {
        this->samples.erase(this->samples.begin());
      }
    }

    public: double GetAverage() const {
      if(this->samples.size() == 0) {
        return 0.0f;
      }

      double total = this->samples[0];
      for(std::size_t index = 1; index < this->samples.size(); ++index) {
        total += this->samples[index];
      }

      return total / static_cast<double>(this->samples.size());
    }

    private: std::size_t maximumSampleCount;
    private: std::vector<double> samples;

  };

}

std::vector<std::string> listFilesInDirectory(const std::string &path);

void runTelecideAlgorithm(
  Nuclex::Pixels::Storage::BitmapSerializer &serializer,
  const std::vector<std::string> &imageFiles, const std::string &outputFolder
);

std::tuple<double, double> calculateCombedness(const Nuclex::Pixels::Bitmap &bitmap);

// --------------------------------------------------------------------------------------------- //

int main(int argc, char *argv[]) {
  int exitCode;
  {
    QApplication application(argc, argv);

    // Create the service provider (we use a simple class that ties all the services
    // together instead of a full-blown IoC container to keep things simple).
    std::shared_ptr<Nuclex::Telecide::Services::ServicesRoot> servicesRoot;
    try {
      //servicesRoot = std::make_shared<Nuclex::CriuGui::Services::ServicesRoot>();
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

    std::shared_ptr<Nuclex::Telecide::MainWindow> mainWindow = (
      std::make_shared<Nuclex::Telecide::MainWindow>()
    );
    mainWindow->BindToServicesRoot(servicesRoot);
    mainWindow->show();

    exitCode = application.exec();
  }

  return exitCode;
}

int cliMain(int argc, char *argv[]) {
  if((argc != 2) && (argc != 3)) {
    ::fprintf(stdout, u8"Syntax: NuclexTelecideNative <telecine-folder> <output-folder>\n");
    ::fprintf(stdout, u8"\n");
    ::fprintf(stdout, u8"Attempts to fix a digitally produced telecine movie where\n");
    ::fprintf(stdout, u8"the telecine pattern is jumbled (but where no half-frames are\n");
    ::fprintf(stdout, u8"missing their counterpart, otherwise this tool will alert you\n");
    ::fprintf(stdout, u8"and you'll have to fix them yourself using scaling or interpolation\n");
    ::fprintf(stdout, u8"\n");
    return -1;
  }

  std::string telecineFolder(argv[1]);
  if(!isFolder(telecineFolder)) {
    ::fprintf(stdout, u8"ERROR: The specified telecine input folder, '");
    ::fprintf(stdout, telecineFolder.c_str());
    ::fprintf(stdout, u8"', does not exist or is not a folder.\n");
    ::fprintf(stdout, u8"\n");
    return -2;
  }

  std::string outputFolder(argv[2]);
  if(!isFolder(outputFolder)) {
    ::fprintf(stdout, u8"ERROR: The specified output folder, '");
    ::fprintf(stdout, outputFolder.c_str());
    ::fprintf(stdout, u8"', does not exist or is not a folder.\n");
    ::fprintf(stdout, u8"\n");
    return -3;
  }

  try {
    std::vector<std::string> filenames = listFilesInDirectory(telecineFolder);
    std::sort(filenames.begin(), filenames.end());

    if(telecineFolder[telecineFolder.length() - 1] != '/') {
      telecineFolder.push_back('/');
    }

    std::size_t imageCount = filenames.size();
    for(std::size_t index = 0; index < imageCount; ++index) {
      filenames[index] = telecineFolder + filenames[index];
    }

    if(outputFolder[telecineFolder.length() - 1] != '/') {
      outputFolder.push_back('/');
    }

    Nuclex::Pixels::Storage::BitmapSerializer serializer;
    runTelecideAlgorithm(serializer, filenames, outputFolder);
  }
  catch(const std::exception &error) {
    ::fprintf(stdout, u8"ERROR: ");
    ::fprintf(stdout, error.what());
    ::fprintf(stdout, u8"\n");
    return -3;
  }

  return 0;
}

std::vector<std::string> listFilesInDirectory(const std::string &path) {
  ::DIR *directory = ::opendir(path.c_str());
  if(directory == nullptr) {
    throw std::runtime_error(u8"Could not open directory for enumeration");
  }

  std::vector<std::string> filenames;
  for(;;) {
    struct ::dirent *directoryEntry = ::readdir(directory);
    if(directoryEntry == nullptr) {
      break;
    }

    if(directoryEntry->d_name[0] != u8'.') {
      filenames.push_back(directoryEntry->d_name);
    }
  }

  ::closedir(directory);

  return filenames;
}

/// <summary>Detected frame type in classic 3:2 pulldown (telecine)</summary>
enum class FrameType {

  /// <summary>Frame type not detected yet</summary>
  Unknown,
  /// <summary>First progressive frame</summary>
  A,
  /// <summary>Second progressive frame</summary>
  B,
  /// <summary>Top field of third frame with remainder of second frame</summary>
  BC,
  /// <summary>Bottom field of third frame with top field of fourth frame</summary>
  CD,
  /// <summary>Fourth progressive frame</summary>
  D

};

void runTelecideAlgorithm(
  Nuclex::Pixels::Storage::BitmapSerializer &serializer,
  const std::vector<std::string> &imageFiles, const std::string &outputFolder
) {
  // Calculate differnce between line above and below (to see if there's anything to measure)
  // Then calculate difference of middle line. If middle line is more different than line
  // above and below, it's an indicator for interlacing.
  //
  // Figure out the rhythm and when it breaks.
  //
  std::vector<FrameType> frameTypes;
  frameTypes.resize(imageFiles.size());
  std::fill(frameTypes.begin(), frameTypes.end(), FrameType::Unknown);

  std::vector<double> combedness;
  combedness.reserve(imageFiles.size());

  std::vector<bool> isCombed;
  combedness.reserve(imageFiles.size());

  Averager progressiveCombedness(20);
  progressiveCombedness.AddSample(0);
  Averager interlacedCombedness(20);
  interlacedCombedness.AddSample(10000);

  FILE *file = ::fopen("combedness.txt", "wt");

  for(std::size_t index = 0; index < imageFiles.size(); ++index) {
    Nuclex::Pixels::Bitmap bitmap = serializer.Load(imageFiles[index]);

    std::tuple<double, double> imageCombedness = calculateCombedness(bitmap);
    combedness.push_back(std::get<0>(imageCombedness));

    ::fprintf(file, "%f, %f\n", std::get<0>(imageCombedness), std::get<1>(imageCombedness));
    ::fflush(file);
  }

  //for(std::size_t index = 0; index < combedness.size(); ++index) {
  //  fprintf(file, "%d\n", combedness[index]);
  //}
  ::fclose(file);
}

std::tuple<double, double> calculateCombedness(const Nuclex::Pixels::Bitmap &bitmap) {
  const Nuclex::Pixels::BitmapMemory &memory = bitmap.Access();

  Nuclex::Pixels::ColorModels::RgbPixelIterator it(memory);

  double totalHorizontal = 0.0;
  double totalVertical = 0.0;
  for(std::size_t y = 1; y < memory.Height - 2; ++y) {
    for(std::size_t x = 1; x < memory.Width - 2; ++x) {
      it.MoveTo(x, y);

      std::tuple<double, double> sample = (
        Nuclex::Telecide::Telecide::CalculateCombiness(it)
      );

      totalHorizontal += std::get<0>(sample);
      totalVertical += std::get<1>(sample);
    }
  }

  return std::tuple<double, double>(totalHorizontal, totalVertical);
}
