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

#ifndef NUCLEX_FRAMEFIXER_FRAMETHUMBNAILPAINTDELEGATE_H
#define NUCLEX_FRAMEFIXER_FRAMETHUMBNAILPAINTDELEGATE_H

#include "Nuclex/FrameFixer/Config.h"

#include <QStyledItemDelegate>
#include <memory> // for std::shared_ptr

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  class Movie;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

namespace Nuclex::FrameFixer {

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

    /// <summary>Draws an overlay in the frame that crosses the frame out</summary>
    /// <param name="painter">Painter that will be used to draw the overlay</param>
    /// <param name="option">Contains active style options and item rectangle</param>
    private: void paintCrossedOutOverlay(
      QPainter &painter, const QStyleOptionViewItem &option
    ) const;

    /// <summary>Draws an overlay that indicates a single field being used</summary>
    /// <param name="painter">Painter that will be used to draw the overlay</param>
    /// <param name="option">Contains active style options and item rectangle</param>
    private: void paintSingleFieldOverlay(
      QPainter &painter, const QStyleOptionViewItem &option, bool topFieldFilled
    ) const;

    /// <summary>The movie for which the model provides thumbnails</summary>
    private: std::shared_ptr<Movie> movie;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_FRAMETHUMBNAILPAINTDELEGATE_H
