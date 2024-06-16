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

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "FrameThumbnailPaintDelegate.h"

#include "./Model/Movie.h"

#include <QPainter>

namespace Nuclex::FrameFixer {

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
    this->movie = movie;
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailPaintDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
  ) const {
    QStyledItemDelegate::paint(painter, option, index);

    if(static_cast<bool>(this->movie)) {
      std::size_t frameIndex = static_cast<std::size_t>(index.row());
      const Frame &frame = this->movie->Frames[frameIndex];

      // Rhythm lines every 5 frames. Extra thick and apparently between frames
      // by having one line on each side of two neighboring frames.
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

      // Little round tag that visually indicates the frame type
      // Red - Yellow - Green - Cyan - Blue - White (PR)
      {
        QRect decorationRect(option.rect.bottomLeft(), QSize(20, 20));
        decorationRect.adjust(0, -20, 0, -20);
        if(frame.Action == FrameAction::Discard) {
          painter->save();
          paintCrossedOutOverlay(*painter, option);
          painter->restore();    
        } else if(frame.Action == FrameAction::TopFieldFirst) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkBlue));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "TB");
          painter->restore();    
        } else if(frame.Action == FrameAction::BottomFieldFirst) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGreen));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "BT");
          painter->restore();    
        } else if(frame.Action == FrameAction::TopFieldOnly) {
          painter->save();
          //paintSingleFieldOverlay(*painter, option, true);

          painter->setBrush(QBrush(Qt::GlobalColor::darkBlue));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "T▲");

          painter->restore();    
        } else if(frame.Action == FrameAction::BottomFieldOnly) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGreen));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "B▼");
          painter->restore();    
        } else if(frame.Action == FrameAction::Progressive) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGray));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "PR");
          painter->restore();    
        } else if(frame.Action == FrameAction::Average) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkMagenta));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "A");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Action == FrameAction::Duplicate) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkYellow));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "I  I");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Action == FrameAction::Triplicate) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkYellow));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "I II");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Action == FrameAction::Replace) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkRed));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "X*");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Action == FrameAction::Deblend) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkRed));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "<>");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Action == FrameAction::Interpolate) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkRed));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "]  [");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        }
      }

      if(frame.Action == FrameAction::Interpolate) {
        painter->save();
        painter->setPen(QPen(Qt::GlobalColor::white));
        QPoint upperLeft = option.rect.topLeft();
        upperLeft.setY(upperLeft.y() + 2);
        QPoint upperRight = option.rect.topRight();
        upperRight.setY(upperRight.y() + 2);
        QPoint center = (upperLeft + upperRight) / 2;
        QPoint centerLeft(center.x() - 10, center.y());
        painter->drawLine(upperLeft, centerLeft);
        QPoint centerRight(center.x() + 10, center.y());
        painter->drawLine(centerRight, upperRight);
        center.setY(center.y() + 10);
        painter->drawLine(centerLeft, center);
        painter->drawLine(center, centerRight);
        painter->restore();    
      }
      if(frame.AlsoInsertInterpolatedAfter.has_value()) {
        painter->save();
        painter->setPen(QPen(Qt::GlobalColor::white));
        QPoint upperLeft = option.rect.topLeft();
        QPoint bottomRight = option.rect.bottomRight();
        painter->drawLine(upperLeft, bottomRight);
        painter->restore();    
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailPaintDelegate::paintCrossedOutOverlay(
    QPainter &painter, const QStyleOptionViewItem &option
  ) const {
    QPen whitePen(Qt::GlobalColor::white, qreal(4.0));
    QPen redPen(Qt::GlobalColor::red, qreal(3.0));

    int centerX = (option.rect.left() + option.rect.right()) / 2;
    int bottomY = option.rect.bottom();

    painter.setPen(whitePen);
    painter.drawLine(centerX - 8, bottomY - 21, centerX + 8, bottomY - 5);
    painter.drawLine(centerX - 8, bottomY - 5, centerX + 8, bottomY - 21);
      
    painter.setPen(redPen);
    painter.drawLine(centerX - 8, bottomY - 21, centerX + 8, bottomY - 5);
    painter.drawLine(centerX - 8, bottomY - 5, centerX + 8, bottomY - 21);
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameThumbnailPaintDelegate::paintSingleFieldOverlay(
    QPainter &painter, const QStyleOptionViewItem &option, bool topFieldFilled
  ) const {
    (void)topFieldFilled;

    int centerX = (option.rect.left() + option.rect.right()) / 2;
    int bottomY = option.rect.bottom();

    QPen whitePen(Qt::GlobalColor::white, qreal(4.0));
    QPen greenPen(Qt::GlobalColor::green, qreal(3.0));

    QBrush whiteBrush(Qt::GlobalColor::white, Qt::BrushStyle::SolidPattern);
    QBrush greenBrush(Qt::GlobalColor::darkRed, Qt::BrushStyle::SolidPattern);
    //QPen greenPen(Qt::GlobalColor::darkGreen, qreal(3.0));

    //painter.fillRect(centerX - 11, bottomY - 23, 22, 20, whiteBrush);
    //painter.setPen(greenPen);
    painter.setPen(whitePen);
    painter.drawRect(centerX - 8, bottomY - 21, 16, 16);
    painter.setPen(greenPen);
    painter.drawRect(centerX - 8, bottomY - 21, 16, 16);
    //painter.fillRect(centerX - 6, bottomY - 19, 11, 12, greenBrush);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
