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
        if(frame.Type == FrameType::Discard) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::red));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "X");
          painter->restore();    
        } else if(frame.Type == FrameType::TopFieldFirst) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkBlue));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "TB");
          painter->restore();    
        } else if(frame.Type == FrameType::BottomFieldFirst) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGreen));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "BT");
          painter->restore();    
        } else if(frame.Type == FrameType::TopFieldOnly) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkBlue));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "T▲");
          painter->restore();    
        } else if(frame.Type == FrameType::BottomFieldOnly) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGreen));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "B▼");
          painter->restore();    
        } else if(frame.Type == FrameType::Progressive) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkGray));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "PR");
          painter->restore();    
        } else if(frame.Type == FrameType::Average) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkMagenta));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "A");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Type == FrameType::Duplicate) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkYellow));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "I  I");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Type == FrameType::Triplicate) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkYellow));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "I II");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Type == FrameType::Replace) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkRed));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "X*");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Type == FrameType::Deblend) {
          painter->save();
          painter->setBrush(QBrush(Qt::GlobalColor::darkRed));
          painter->drawEllipse(decorationRect);
          painter->setPen(QPen(Qt::GlobalColor::white));
          painter->drawText(decorationRect, Qt::AlignCenter, "<>");
          decorationRect.setLeft(decorationRect.left() + decorationRect.width() /2);
          painter->drawLine(decorationRect.topLeft(), decorationRect.bottomLeft());
          painter->restore();    
        } else if(frame.Type == FrameType::Interpolate) {
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

      if(frame.Type == FrameType::Interpolate) {
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

      bool isBridgedFrame = false;
      bool isLeftSourceFrame = false;
      bool isRightSourceFrame = false;
      for(int checkOffset = -4; checkOffset <= +4; ++checkOffset) {
        int checkedFrameIndex = static_cast<int>(frameIndex) + checkOffset;
        if((checkedFrameIndex >= 0) && (checkedFrameIndex < this->movie->Frames.size())) {
          if(this->movie->Frames[checkedFrameIndex].Type == FrameType::Interpolate) {
            if(this->movie->Frames[checkedFrameIndex].InterpolationSourceIndices.has_value()) {
              std::pair<std::size_t, std::size_t> sourceIndices = (
                this->movie->Frames[checkedFrameIndex].InterpolationSourceIndices.value()
              );
              isBridgedFrame |= (
                (sourceIndices.first < frameIndex) &&
                (checkedFrameIndex > frameIndex)
              );
              isBridgedFrame |= (
                (sourceIndices.second > frameIndex) &&
                (checkedFrameIndex < frameIndex)
              );
              isLeftSourceFrame |= (sourceIndices.first == frameIndex);
              isRightSourceFrame |= (sourceIndices.second == frameIndex);
            }
          }
        }
      }

      if(isBridgedFrame) {
        painter->save();
        painter->setPen(QPen(Qt::GlobalColor::white));
        QPoint upperLeft = option.rect.topLeft();
        upperLeft.setY(upperLeft.y() + 2);
        QPoint upperRight = option.rect.topRight();
        upperRight.setY(upperRight.y() + 2);
        painter->drawLine(upperLeft, upperRight);
        painter->restore();    
      } else if(isLeftSourceFrame) {
        painter->save();
        painter->setPen(QPen(Qt::GlobalColor::white));
        QPoint upperLeft = option.rect.topLeft();
        upperLeft.setY(upperLeft.y() + 2);
        QPoint upperRight = option.rect.topRight();
        upperRight.setY(upperRight.y() + 2);
        QPoint center = (upperLeft + upperRight) / 2;
        QPoint down(center.x(), center.y() + 10);
        center.setX(center.x() + 10);
        painter->drawLine(down, center);
        painter->drawLine(center, upperRight);
        painter->restore();    
      } else if(isRightSourceFrame) {
        painter->save();
        painter->setPen(QPen(Qt::GlobalColor::white));
        QPoint upperLeft = option.rect.topLeft();
        upperLeft.setY(upperLeft.y() + 2);
        QPoint upperRight = option.rect.topRight();
        upperRight.setY(upperRight.y() + 2);
        QPoint center = (upperLeft + upperRight) / 2;
        QPoint down(center.x(), center.y() + 10);
        center.setX(center.x() - 10);
        painter->drawLine(upperLeft, center);
        painter->drawLine(center, down);
        painter->restore();    
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
