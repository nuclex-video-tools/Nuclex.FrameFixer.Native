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

      if(frame.Index % 5 == 0) {
        painter->save();
        painter->setPen(QPen(Qt::white));
        painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
        painter->restore();    
      } else if(frame.Index % 5 == 4) {
        painter->save();
        painter->setPen(QPen(Qt::white));
        painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
        painter->restore();    
      }


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
      } else if(frame.Type == FrameType::TopFieldFirst) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkGreen));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "BC");
        painter->restore();    
      } else if(frame.Type == FrameType::BottomFieldFirst) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkBlue));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "CD");
        painter->restore();    
      } else if(frame.Type == FrameType::BottomFieldOnly) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkGreen));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "C▼");
        painter->restore();    
      } else if(frame.Type == FrameType::TopFieldOnly) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkBlue));
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
      } else if(frame.Type == FrameType::Average) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkMagenta));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "A");
        decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
        painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
        painter->restore();    
      } else if(frame.Type == FrameType::Duplicate) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkYellow));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "I  I");
        decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
        painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
        painter->restore();    
      } else if(frame.Type == FrameType::Triplicate) {
        painter->save();
        painter->setBrush(QBrush(Qt::darkYellow));
        painter->drawEllipse(decorationRect);
        painter->setPen(QPen(Qt::white));
        painter->drawText(decorationRect, Qt::AlignCenter, "I II");
        decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
        painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
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
