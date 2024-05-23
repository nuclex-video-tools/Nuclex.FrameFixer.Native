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
