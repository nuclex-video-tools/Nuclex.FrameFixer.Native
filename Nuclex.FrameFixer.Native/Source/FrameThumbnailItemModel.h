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

#ifndef NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H
#define NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H

#include "Nuclex/FrameFixer/Config.h"

#include <QAbstractItemModel>

#include <memory> // for std::shared_ptr

#include <Nuclex/Support/Collections/SequentialSlotCache.h> // for SequentialSlotCache

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  class Movie;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Presents the frames of a movie as dynamically generated thumbnails</summary>
  class FrameThumbnailItemModel : public QAbstractListModel {
    Q_OBJECT

    /// <summary>Initializes a new item model for frame thumbnails</summary>
    /// <param name="parent">Always null</param>
    public: explicit FrameThumbnailItemModel(QObject *parent = nullptr);

    /// <summary>Stops all background processing and frees all memory</summary>
    public: ~FrameThumbnailItemModel() override;

    /// <summary>Sets the movie for whose frames thumbnails will be provided</summary>
    /// <param name="movie">Movie for which thumbnails will be provided</param>
    public: void SetMovie(const std::shared_ptr<Movie> &movie);

    /// <summary>Sets the resolution in which thumbnails will be generated</summary>
    /// <param name="resolution">The desired thumbnail resolution</param>
    public: void SetThumbnailResolution(const QSize &resolution);

    /// <summary>Retrieves the number of rows present in the virtual list</summary>
    /// <param name="parent">Parent item, only relevant when items form a tree</param>
    /// <returns>The number of rows in the virtual list</returns>
    public: int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /// <summary>Retrieves the data stored in the specified list row</summary>
    /// <param name="index">Holds the row of the item from which data is retrieved</param>
    /// <param name="role">Indicates the property (by index) to be retrieved</param>
    /// <returns>The value of the requested property in the specified list item</returns>
    public: QVariant data(
      const QModelIndex &index, int role = Qt::DisplayRole
    ) const override;

    /// <summary>The movie for which the model provides thumbnails</summary>
    private: std::shared_ptr<Movie> movie;
    /// <summary>Cached thumbnails for the movie's frames</summary>
    private: std::unique_ptr<
      Nuclex::Support::Collections::SequentialSlotCache<std::size_t, QVariant>
    > thumbnailCache;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H
