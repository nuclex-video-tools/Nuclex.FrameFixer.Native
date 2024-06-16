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

#ifndef NUCLEX_FRAMEFIXER_QZOOMABLEGRAPHICSVIEW_H
#define NUCLEX_FRAMEFIXER_QZOOMABLEGRAPHICSVIEW_H

#include "Nuclex/FrameFixer/Config.h"

#include <QGraphicsView>

// no namespace

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Graphics view which allows zooming in and out using the mouse wheel</summary>
  class QZoomableGraphicsView : public QGraphicsView {
    Q_OBJECT

    /// <summary>Initializes a new item model for frame thumbnails</summary>
    /// <param name="parent">Always null</param>
    public: explicit QZoomableGraphicsView(QWidget *parent = nullptr) :
      QGraphicsView(parent) {
      setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
    }

    /// <summary>Zooms the view in or out when the user turns the mouse wheel</summary>
    /// <param name="wheelEvent">Describes how far the mouse wheel was turned</param>
    public: void wheelEvent(QWheelEvent *wheelEvent) override;

  };

  // ------------------------------------------------------------------------------------------- //

// no namespace

#endif // NUCLEX_FRAMEFIXER_QZOOMABLEGRAPHICSVIEW_H
