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

#include "InterpolatorItemModel.h"

#include "./Algorithm/Interpolation/FrameInterpolator.h"

#include <QPixmap>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  InterpolatorItemModel::InterpolatorItemModel(QObject *parent) :
    QAbstractListModel(parent),
    interpolators() {}

  // ------------------------------------------------------------------------------------------- //

  InterpolatorItemModel::~InterpolatorItemModel() {}

  // ------------------------------------------------------------------------------------------- //

  void InterpolatorItemModel::SetInterpolators(const FrameInterpolatorList &interpolators) {
    beginResetModel();
    this->interpolators = interpolators;
    endResetModel();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<
    Algorithm::Interpolation::FrameInterpolator
  > InterpolatorItemModel::GetInterpolator(std::size_t index) const {
    return this->interpolators.at(index);
  }

  // ------------------------------------------------------------------------------------------- //

  int InterpolatorItemModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return static_cast<int>(this->interpolators.size());
  }

  // ------------------------------------------------------------------------------------------- //

  QVariant InterpolatorItemModel::data(
    const QModelIndex &index, int role /* = Qt::DisplayRole */
  ) const {

    // Verify that the widget is asking for a valid index, otherwise return nothing
    int rowIndex = index.row();
    if((rowIndex < 0) || (static_cast<std::size_t>(rowIndex) >= this->interpolators.size())) {
      return QVariant();
    }

    if(role != Qt::DisplayRole) {
      return QVariant();
    }

    return QString::fromStdString(this->interpolators.at(rowIndex)->GetName());
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
