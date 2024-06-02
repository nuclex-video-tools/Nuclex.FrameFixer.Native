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

#include "DeinterlacerItemModel.h"

#include "./Algorithm/Deinterlace/Deinterlacer.h"

#include <QPixmap>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  DeinterlacerItemModel::DeinterlacerItemModel(QObject *parent) :
    QAbstractListModel(parent),
    deinterlacers() {}

  // ------------------------------------------------------------------------------------------- //

  DeinterlacerItemModel::~DeinterlacerItemModel() {}

  // ------------------------------------------------------------------------------------------- //

  void DeinterlacerItemModel::SetDeinterlacers(const DeinterlacerList &deinterlacers) {
    beginResetModel();
    this->deinterlacers = deinterlacers;
    endResetModel();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<Algorithm::Deinterlace::Deinterlacer> DeinterlacerItemModel::GetDeinterlacer(
    std::size_t index
  ) const {
    return this->deinterlacers.at(index);
  }

  // ------------------------------------------------------------------------------------------- //

  int DeinterlacerItemModel::rowCount(const QModelIndex &parent) const {
    return static_cast<int>(this->deinterlacers.size());
  }

  // ------------------------------------------------------------------------------------------- //

  QVariant DeinterlacerItemModel::data(
    const QModelIndex &index, int role /* = Qt::DisplayRole */
  ) const {

    // Verify that the widget is asking for a valid index, otherwise return nothing
    int rowIndex = index.row();
    if((rowIndex < 0) || (rowIndex >= this->deinterlacers.size())) {
      return QVariant();
    }

    if(role != Qt::DisplayRole) {
      return QVariant();
    }

    return QString::fromStdString(this->deinterlacers.at(rowIndex)->GetName());
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
