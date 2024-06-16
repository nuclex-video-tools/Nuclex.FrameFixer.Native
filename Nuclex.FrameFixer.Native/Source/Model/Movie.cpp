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
    const Nuclex::FrameFixer::Frame &left, const Nuclex::FrameFixer::Frame &right
  ) {
    return left.Filename < right.Filename;
  }

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

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
        
        std::size_t frameIndex = lexical_cast<std::size_t>(
          tokens[0].trimmed().toStdString()
        );

        QString typeAsString = tokens[2].trimmed();
        if(!typeAsString.isEmpty()) {
          if(typeAsString == u8"Discard") {
            movie->Frames[frameIndex].Action = FrameAction::Discard;
          } else if((typeAsString == u8"BC") || (typeAsString == u8"TopFieldFirst")) {
            movie->Frames[frameIndex].Action = FrameAction::TopFieldFirst;
          } else if((typeAsString == u8"CD") || (typeAsString == u8"BottomFieldFirst")) {
            movie->Frames[frameIndex].Action = FrameAction::BottomFieldFirst;
          } else if((typeAsString == u8"TopC") || (typeAsString == u8"TopFieldOnly")) {
            movie->Frames[frameIndex].Action = FrameAction::TopFieldOnly;
          } else if((typeAsString == u8"BottomC") || (typeAsString == u8"BottomFieldOnly")) {
            movie->Frames[frameIndex].Action = FrameAction::BottomFieldOnly;
          } else if(typeAsString == u8"Progressive") {
            movie->Frames[frameIndex].Action = FrameAction::Progressive;
          } else if(typeAsString == u8"Average") {
            movie->Frames[frameIndex].Action = FrameAction::Average;
          } else if(typeAsString == u8"Duplicate") {
            movie->Frames[frameIndex].Action = FrameAction::Duplicate;
          } else if(typeAsString == u8"Triplicate") {
            movie->Frames[frameIndex].Action = FrameAction::Triplicate;
          } else if((typeAsString == u8"Blended") || (typeAsString == u8"Deblend")) {
            movie->Frames[frameIndex].Action = FrameAction::Deblend;
          } else if(typeAsString.startsWith(u8"InterpolateFrom(")) {
            int firstEndIndex = typeAsString.indexOf(u8'+', 16);
            if((firstEndIndex != -1) && (firstEndIndex > 16)) {
              int secondEndIndex = typeAsString.indexOf(u8')', firstEndIndex + 1);
              if((secondEndIndex != -1) && (secondEndIndex > (firstEndIndex + 1))) {
                std::size_t leftFrameIndex = Nuclex::Support::Text::lexical_cast<std::size_t>(
                  typeAsString.mid(16, firstEndIndex - 16).toStdString()
                );
                std::size_t rightFrameIndex = Nuclex::Support::Text::lexical_cast<std::size_t>(
                  typeAsString.mid(firstEndIndex + 1, secondEndIndex - (firstEndIndex + 1)).toStdString()
                );
                movie->Frames[frameIndex].Action = FrameAction::Interpolate;
                movie->Frames[frameIndex].InterpolationSourceIndices = (
                  std::pair<std::size_t, std::size_t>(leftFrameIndex, rightFrameIndex)
                );
              }
            }
          } else if(typeAsString.startsWith(u8"ReplaceWith(")) {
            int endIndex = typeAsString.indexOf(u8')', 12);
            if((endIndex != -1) && (endIndex > 12)) {
              std::size_t replacementFrameIndex = Nuclex::Support::Text::lexical_cast<std::size_t>(
                typeAsString.mid(12, endIndex - 12).toStdString()
              );
              movie->Frames[frameIndex].Action = FrameAction::Replace;
              movie->Frames[frameIndex].LeftOrReplacementIndex = replacementFrameIndex;
            }
          }

          if(tokens.size() > 3) {
            QString interpolationType = tokens[3].trimmed();
            if(interpolationType == u8"AlsoInsertInterpolatedFrameAfter") {
              movie->Frames[frameIndex].AlsoInsertInterpolatedAfter = true;
            }
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
        line.append(this->Frames[index].Filename);
        //if(this->Frames[index].Combedness.has_value()) {
        //  lexical_append(line, this->Frames[index].Combedness.value());
        //}
        line.append(u8", ");
        switch(this->Frames[index].Action) {
          case FrameAction::Discard: { line.append(u8"Discard"); break; }
          case FrameAction::TopFieldFirst: { line.append(u8"TopFieldFirst"); break; }
          case FrameAction::BottomFieldFirst: { line.append(u8"BottomFieldFirst"); break; }
          case FrameAction::TopFieldOnly: { line.append(u8"TopFieldOnly"); break; }
          case FrameAction::BottomFieldOnly: { line.append(u8"BottomFieldOnly"); break; }
          case FrameAction::Progressive: { line.append(u8"Progressive"); break; }
          case FrameAction::Average: { line.append(u8"Average"); break; }
          case FrameAction::Duplicate: { line.append(u8"Duplicate"); break; }
          case FrameAction::Triplicate: { line.append(u8"Triplicate"); break; }
          case FrameAction::Deblend: { line.append(u8"Deblend"); break; }
          case FrameAction::Interpolate: {
            line.append(u8"InterpolateFrom(");
            line.append(
              Nuclex::Support::Text::lexical_cast<std::string>(
                this->Frames[index].InterpolationSourceIndices.value().first
              )
            );
            line.append(u8"+");
            line.append(
              Nuclex::Support::Text::lexical_cast<std::string>(
                this->Frames[index].InterpolationSourceIndices.value().second
              )
            );
            line.append(u8")");
            break;
          }
          case FrameAction::Replace: {
            line.append(u8"ReplaceWith(");
            line.append(
              Nuclex::Support::Text::lexical_cast<std::string>(
                this->Frames[index].LeftOrReplacementIndex.value()
              )
            );
            line.append(u8")");
            break;
          }
          default: { break; } // Others are not manually assignable and not saved
        }
        if(this->Frames[index].AlsoInsertInterpolatedAfter.has_value()) {
          line.append(u8", ");
          line.append(u8"AlsoInsertInterpolatedFrameAfter");
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

} // namespace Nuclex::FrameFixer
