#pragma region CPL License
/*
Nuclex CriuGui
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

#include "FrameThumbnailPaintDelegate.h"

#include "./Model/Movie.h"

#include <QPainter>

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  FrameThumbnailPaintDelegate::FrameThumbnailPaintDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    movie() {
    //rootItem.reset(
    //  new ProcessItem({tr("Title"), tr("Summary")})
    //);
  }

  // ------------------------------------------------------------------------------------------- //

  FrameThumbnailPaintDelegate::~FrameThumbnailPaintDelegate() {
    //delete rootItem;
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailPaintDelegate::SetMovie(const std::shared_ptr<Movie> &movie) {
    std::size_t frameCount = movie->Frames.size();

    //beginResetModel();
    this->movie = movie;
    //endResetModel();
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailPaintDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
  ) const {
    QStyledItemDelegate::paint(painter, option, index);

    if(static_cast<bool>(this->movie)) {
      std::size_t frameIndex = static_cast<std::size_t>(index.row());
      const Frame &frame = this->movie->Frames[frameIndex];

      // Red - Yellow - Green - Cyan - Blue - White (PR)

      QRect decorationRect(option.rect.bottomLeft(), QSize(20, 20));
      decorationRect.adjust(0, -20, 0, -20);

      if(frame.Type == FrameType::Discard) {
        painter->save();
        painter->setBrush(QBrush(Qt::red));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "X");
        painter->restore();    
      } else if(frame.Type == FrameType::BC) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkGreen));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "BC");
        painter->restore();    
      } else if(frame.Type == FrameType::CD) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkCyan));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "CD");
        painter->restore();    
      } else if(frame.Type == FrameType::BottomC) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkGreen));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "C▼");
        painter->restore();    
      } else if(frame.Type == FrameType::TopC) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkCyan));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "C▲");
        painter->restore();    
      } else if(frame.Type == FrameType::Progressive) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkGray));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "PR");
        painter->restore();    
      }

      if(frame.Combedness.has_value()) {
        /*
        QPainterPath path;
        path.moveTo(0, combHeight);
        for (int i = 0; i < numTeeth; ++i) {
            path.lineTo(i * toothWidth, 0);
            path.lineTo((i + 1) * toothWidth, 0);
            path.lineTo((i + 1) * toothWidth, combHeight);
        }
        path.closeSubpath();

        painter.drawPath(path);
        */        
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide
