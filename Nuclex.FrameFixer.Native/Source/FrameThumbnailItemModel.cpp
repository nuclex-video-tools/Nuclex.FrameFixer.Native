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

#include "FrameThumbnailItemModel.h"

#include "./Model/Movie.h"

#include <QPixmap>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  FrameThumbnailItemModel::FrameThumbnailItemModel(QObject *parent) :
    QAbstractListModel(parent),
    movie(),
    thumbnailCache(),
    thumbnailResolution(128, 128) {}

  // ------------------------------------------------------------------------------------------- //

  FrameThumbnailItemModel::~FrameThumbnailItemModel() {}

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailItemModel::SetMovie(const std::shared_ptr<Movie> &movie) {
    std::size_t frameCount = movie->Frames.size();

    beginResetModel();
    this->movie = movie;
    this->thumbnailCache.reset(
      new Nuclex::Support::Collections::SequentialSlotCache<std::size_t, QVariant>(frameCount)
    );
    endResetModel();
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailItemModel::SetThumbnailResolution(const QSize &resolution) {
    beginResetModel();
    this->thumbnailResolution = resolution;
    if(static_cast<bool>(this->thumbnailCache)) {
      this->thumbnailCache->EvictDownTo(0);
    }
    endResetModel();
  }

  // ------------------------------------------------------------------------------------------- //

  int FrameThumbnailItemModel::rowCount(const QModelIndex &parent) const {
    if(static_cast<bool>(this->movie)) {
      return static_cast<int>(this->movie->Frames.size());
    } else {
      return 0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  QVariant FrameThumbnailItemModel::data(
    const QModelIndex &index, int role /* = Qt::DisplayRole */
  ) const {
    if(!static_cast<bool>(this->movie) || !index.isValid()) {
      return QVariant();
    }

    // Verify that the widget is asking for a valid index, otherwise return nothing
    int rowIndex = index.row();
    if((rowIndex < 0) || (rowIndex >= this->movie->Frames.size())) {
      return QVariant();
    }

    if(role != Qt::DecorationRole) {
      return QVariant();
    }

    QVariant thumbnail;

    bool wasCached = this->thumbnailCache->TryGet(rowIndex, thumbnail);
    if(!wasCached) {

      // Reconstitute the absolute path of the image file for this element
      std::string frameImagePath;
      {
        frameImagePath = this->movie->FrameDirectory;
        std::string::size_type length = frameImagePath.length();
        if(length >= 1) {
          if(frameImagePath[length - 1] != '/') {
            frameImagePath.push_back('/');
          }
        }
        frameImagePath.append(this->movie->Frames[rowIndex].Filename);
      }

      // Load the image, resize it to thumbnail format and return it for the 
      {
        QPixmap bitmap(QString::fromStdString(frameImagePath));

        int width = bitmap.width();
        int height = bitmap.height();
        if(width > this->thumbnailResolution.width()) {
          height = height * this->thumbnailResolution.width() / width;
          width = this->thumbnailResolution.width();
        }
        if(height > this->thumbnailResolution.height()) {
          width = width * this->thumbnailResolution.height() / height;
          height = this->thumbnailResolution.height();
        }
        bitmap = bitmap.scaled(width, height, Qt::KeepAspectRatio);
        if(bitmap.height() < this->thumbnailResolution.height()) {
          //bitmap.
          //thumbnail
        }

        thumbnail = bitmap;
      }

      this->thumbnailCache->Insert(rowIndex, thumbnail);
      this->thumbnailCache->EvictDownTo(1024);
    }

    return thumbnail;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
