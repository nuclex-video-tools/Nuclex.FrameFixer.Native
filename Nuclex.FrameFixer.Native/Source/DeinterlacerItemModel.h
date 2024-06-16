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

#ifndef NUCLEX_FRAMEFIXER_DEINTERLACERITEMMODEL_H
#define NUCLEX_FRAMEFIXER_DEINTERLACERITEMMODEL_H

#include "Nuclex/FrameFixer/Config.h"

#include <QAbstractItemModel>
#include <QStringList>

#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Presents a list of deinterlacers with human-readable names</summary>
  class DeinterlacerItemModel : public QAbstractListModel {
    Q_OBJECT

    // List of deinterlacers
    public: typedef std::vector<
      std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer>
    > DeinterlacerList;

    /// <summary>Initializes a new item model for deinterlacers</summary>
    /// <param name="parent">Always null</param>
    public: explicit DeinterlacerItemModel(QObject *parent = nullptr);

    /// <summary>Stops all background processing and frees all memory</summary>
    public: ~DeinterlacerItemModel() override;

    /// <summary>Sets the list of deinterlacers that will be exposed for Qt</summary>
    /// <param name="deinterlacers">Deinterlacers the item model will expose</param>
    public: void SetDeinterlacers(const DeinterlacerList &deinterlacers);

    /// <summary>Retrieves a deinterlacer by its index</summary>
    /// <param name="index">Index of the deinterlacer that will be retrieved</param>
    /// <returns>The deinterlacer with the specified index</returns>
    public: std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> GetDeinterlacer(
      std::size_t index
    ) const;

    /// <summary>Retrieves the number of rows present in the virtual list</summary>
    /// <param name="parent">Parent item, only relevant when items form a tree</param>
    /// <returns>The number of rows in the virtual list</returns>
    public: int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /// <summary>Retrieves the data stored in the specified list row</summary>
    /// <param name="index">Holds the row of the item from which data is retrieved</param>
    /// <param name="role">Indicates the property (by index) to be retrieved</param>
    /// <returns>The value of the requested property in the specified list item</returns>
    public: QVariant data(
      const QModelIndex &index, int role = Qt::DisplayRole
    ) const override;

    /// <summary>Deinterlacers whose names will be provided as an item model</summary>
    private: DeinterlacerList deinterlacers;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_DEINTERLACERITEMMODEL_H
