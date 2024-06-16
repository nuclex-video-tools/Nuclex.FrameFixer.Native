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

#ifndef NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H
#define NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H

#include "Nuclex/FrameFixer/Config.h"

#include <QAbstractItemModel>
#include <QSize>

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
    /// <summary>Size the individual thumbnails should have</summary>
    private: QSize thumbnailResolution;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FRAMETHUMBNAILITEMMODEL_H
