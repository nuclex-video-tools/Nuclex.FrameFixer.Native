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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_TELECIDE_SOURCE 1

#include "Movie.h"

#include <QDirIterator>
#include <QFile>
#include <QTextStream>

#include <Nuclex/Support/Text/LexicalCast.h>
#include <Nuclex/Support/Text/LexicalAppend.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compares the sort order relation of two frames using their filenames</summary>
  /// <param name="left">Frame that will be compared on the left side</param>
  /// <param name="left">Frame that will be compared on the right side</param>
  /// <returns>True if the left frame is ordered before the right frame</returns>
  bool compareFramesByFilename(
    const Nuclex::Telecide::Frame &left, const Nuclex::Telecide::Frame &right
  ) {
    return left.Filename < right.Filename;
  }

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<Movie> Movie::FromImageFolder(
    const std::string &path,
    const std::shared_ptr<const CancellationWatcher> &cancellationWatcher /* = (
      std::shared_ptr<const CancellationWatcher>()
    ) */
  ) {
    std::shared_ptr<Movie> movie = std::make_shared<Movie>();
    movie->FrameDirectory = path;
    {
      QStringList acceptedExtensions({"*.png", "*.tif", "*.bmp"});
      QDirIterator directoryEnumerator(
        QString::fromStdString(path),
        acceptedExtensions,
        QDir::Filter::Files,
        QDirIterator::IteratorFlag::NoIteratorFlags
      );

      // Enumerate all image files in the target directory and add them to the frames
      // list of the movie. The files will be enumerated in an undefined order.
      std::size_t frameIndex = 0;
      while(directoryEnumerator.hasNext()) {
        directoryEnumerator.next();
        movie->Frames.emplace_back(directoryEnumerator.fileName().toStdString());
        ++frameIndex;

        if((frameIndex % 100) == 0) {
          if(static_cast<bool>(cancellationWatcher)) {
            cancellationWatcher->ThrowIfCanceled();
          }
        }
      }

      // Now sort the frames by their filename. This assumes frames have been exported
      // with leading zeroes (i.e. what you get when you export images with ffmpeg),
      std::sort(movie->Frames.begin(), movie->Frames.end(), &compareFramesByFilename);

      // Now that the frames are ordered, give them an index to easily address them.
      for(std::size_t index = 0; index < movie->Frames.size(); ++index) {
        movie->Frames[index].Index = index;
      }
    }

    // This application also saves the states of manually marked frames inside a text file
    // using the same name as the frame directory. Restore the states is the file exists.
    std::string stateFilePath = getStateFilePath(path);
    QFile stateFile(QString::fromStdString(stateFilePath));
    if(stateFile.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text)) {
      using Nuclex::Support::Text::lexical_cast;

      QTextStream stateReader(&stateFile);
      while(!stateReader.atEnd()) {
        QString line = stateReader.readLine();
        QStringList tokens = line.split(',', Qt::SplitBehaviorFlags::KeepEmptyParts);
        
        std::size_t frameIndex = lexical_cast<std::size_t>(tokens[0].trimmed().toStdString());
        QString combinessAsString = tokens[1].trimmed();
        if(!combinessAsString.isEmpty()) {
          movie->Frames[frameIndex].Combedness = lexical_cast<double>(
            combinessAsString.toStdString()
          );
        }

        QString typeAsString = tokens[2].trimmed();
        if(!typeAsString.isEmpty()) {
          if(typeAsString == u8"Discard") {
            movie->Frames[frameIndex].Type = FrameType::Discard;
          } else if((typeAsString == u8"BC") || (typeAsString == u8"TopFieldFirst")) {
            movie->Frames[frameIndex].Type = FrameType::TopFieldFirst;
          } else if((typeAsString == u8"CD") || (typeAsString == u8"BottomFieldFirst")) {
            movie->Frames[frameIndex].Type = FrameType::BottomFieldFirst;
          } else if((typeAsString == u8"TopC") || (typeAsString == u8"TopFieldOnly")) {
            movie->Frames[frameIndex].Type = FrameType::TopFieldOnly;
          } else if((typeAsString == u8"BottomC") || (typeAsString == u8"BottomFieldOnly")) {
            movie->Frames[frameIndex].Type = FrameType::BottomFieldOnly;
          } else if(typeAsString == u8"Progressive") {
            movie->Frames[frameIndex].Type = FrameType::Progressive;
          } else if(typeAsString == u8"Average") {
            movie->Frames[frameIndex].Type = FrameType::Average;
          } else if(typeAsString == u8"Duplicate") {
            movie->Frames[frameIndex].Type = FrameType::Duplicate;
          } else if(typeAsString == u8"Triplicate") {
            movie->Frames[frameIndex].Type = FrameType::Triplicate;
          }
        }
      }
    }

    return movie;
  }

  // ------------------------------------------------------------------------------------------- //

  void Movie::SaveState() const {
    std::string stateFilePath = getStateFilePath(this->FrameDirectory);

    QFile stateFile(QString::fromStdString(stateFilePath));
    if(stateFile.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text)) {
      using Nuclex::Support::Text::lexical_cast;
      using Nuclex::Support::Text::lexical_append;

      for(std::size_t index = 0; index < this->Frames.size(); ++index) {
        std::string line;
        lexical_append(line, this->Frames[index].Index);
        line.append(u8", ");
        if(this->Frames[index].Combedness.has_value()) {
          lexical_append(line, this->Frames[index].Combedness.value());
        }
        line.append(u8", ");
        switch(this->Frames[index].Type) {
          case FrameType::Discard: { line.append(u8"Discard"); break; }
          case FrameType::TopFieldFirst: { line.append(u8"TopFieldFirst"); break; }
          case FrameType::BottomFieldFirst: { line.append(u8"BottomFieldFirst"); break; }
          case FrameType::TopFieldOnly: { line.append(u8"TopFieldOnly"); break; }
          case FrameType::BottomFieldOnly: { line.append(u8"BottomFieldOnly"); break; }
          case FrameType::Progressive: { line.append(u8"Progressive"); break; }
          case FrameType::Average: { line.append(u8"Average"); break; }
          case FrameType::Duplicate: { line.append(u8"Duplicate"); break; }
          case FrameType::Triplicate: { line.append(u8"Triplicate"); break; }
          default: { break; } // Others are not manually assignable and not saved
        }
        line.append(u8"\n");

        stateFile.write(line.data(), line.length());
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::string Movie::GetFramePath(std::size_t frameIndex) const {
    std::string path = this->FrameDirectory;
    {
      std::string::size_type length = path.length();
      if(length >= 1) {
        if(path[length - 1] != '/') {
          path.push_back('/');
        }
      }

      path.append(this->Frames.at(frameIndex).Filename);
    }

    return path;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string Movie::getStateFilePath(const std::string &frameDirectoryPath) {
    std::string::size_type length = frameDirectoryPath.length();

    std::string stateFilePath;
    if((length >= 1) && (frameDirectoryPath[length - 1] == '/')) {
      stateFilePath = frameDirectoryPath.substr(0, length - 1) + u8".frames.txt";
    } else {
      stateFilePath = frameDirectoryPath + u8".frames.txt";
    }

    return stateFilePath;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
