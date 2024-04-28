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

namespace {

  bool compareFramesByFilename(
    const Nuclex::Telecide::Frame &left, const Nuclex::Telecide::Frame &right
  ) {
    return left.Filename < right.Filename;
  }

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

      std::size_t frameIndex = 0;
      while(directoryEnumerator.hasNext()) {
        directoryEnumerator.next();
        movie->Frames.emplace_back(-1, directoryEnumerator.fileName().toStdString());
        ++frameIndex;

        if((frameIndex % 100) == 0) {
          if(static_cast<bool>(cancellationWatcher)) {
            cancellationWatcher->ThrowIfCanceled();
          }
        }
      }

      std::sort(movie->Frames.begin(), movie->Frames.end(), &compareFramesByFilename);
      for(std::size_t index = 0; index < movie->Frames.size(); ++index) {
        movie->Frames[index].Index = index;
      }
    }

    return movie;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
