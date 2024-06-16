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

#include "DeinterlacerItemModel.h"

#include "./Algorithm/Deinterlacing/Deinterlacer.h"

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

  std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> DeinterlacerItemModel::GetDeinterlacer(
    std::size_t index
  ) const {
    return this->deinterlacers.at(index);
  }

  // ------------------------------------------------------------------------------------------- //

  int DeinterlacerItemModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return static_cast<int>(this->deinterlacers.size());
  }

  // ------------------------------------------------------------------------------------------- //

  QVariant DeinterlacerItemModel::data(
    const QModelIndex &index, int role /* = Qt::DisplayRole */
  ) const {

    // Verify that the widget is asking for a valid index, otherwise return nothing
    int rowIndex = index.row();
    if((rowIndex < 0) || (static_cast<std::size_t>(rowIndex) >= this->deinterlacers.size())) {
      return QVariant();
    }

    if(role != Qt::DisplayRole) {
      return QVariant();
    }

    return QString::fromStdString(this->deinterlacers.at(rowIndex)->GetName());
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
