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

#ifndef NUCLEX_TELECIDE_FRAMETHUMBNAILPAINTDELEGATE_H
#define NUCLEX_TELECIDE_FRAMETHUMBNAILPAINTDELEGATE_H

#include "Nuclex/Telecide/Config.h"

#include <QStyledItemDelegate>
#include <memory> // for std::shared_ptr

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  class Movie;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Draws thumbnails with added decorations indicating the frame type</summary>
  class FrameThumbnailPaintDelegate : public QStyledItemDelegate {
    Q_OBJECT

    /// <summary>Initializes a new thumbnail painting delegate</summary>
    /// <param name="parent">Always null</param>
    public: explicit FrameThumbnailPaintDelegate(QObject *parent = nullptr);

    /// <summary>Stops all background processing and frees all memory</summary>
    public: ~FrameThumbnailPaintDelegate() override;

    /// <summary>Loads the movie frames from the specified folder</summary>
    /// <param name="path">Path from which the movie frames will be loaded</param>
    public: void SetMovie(const std::shared_ptr<Movie> &movie);

    public: void paint(
      QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const override;

    /// <summary>The movie for which the model provides thumbnails</summary>
    private: std::shared_ptr<Movie> movie;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_FRAMETHUMBNAILPAINTDELEGATE_H
