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

#ifndef NUCLEX_FRAMEFIXER_DEINTERLACERITEMMODEL_H
#define NUCLEX_FRAMEFIXER_DEINTERLACERITEMMODEL_H

#include "Nuclex/FrameFixer/Config.h"

#include <QAbstractItemModel>
#include <QStringList>

#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Presents a list of deinterlacers with human-readable names</summary>
  class DeinterlacerItemModel : public QAbstractListModel {
    Q_OBJECT

    // List of deinterlacers
    typedef std::vector<std::shared_ptr<Algorithm::Deinterlacer>> DeinterlacerList;

    /// <summary>Initializes a new item model for deinterlacers</summary>
    /// <param name="parent">Always null</param>
    public: explicit DeinterlacerItemModel(QObject *parent = nullptr);

    /// <summary>Stops all background processing and frees all memory</summary>
    public: ~DeinterlacerItemModel() override;

    /// <summary>Loads the movie frames from the specified folder</summary>
    /// <param name="path">Path from which the movie frames will be loaded</param>
    public: void SetDeinterlacers(const DeinterlacerList &deinterlacers);

    /// <summary>Retrieves a deinterlacer by its index</summary>
    /// <param name="index">Index of the deinterlacer that will be retrieved</param>
    /// <returns>The deinterlacer wit hthe specified index</returns>
    public: std::shared_ptr<Algorithm::Deinterlacer> &GetDeinterlacer(std::size_t index) const;

    /// <summary>Sets the resolution in which thumbnails will be generated</summary>
    /// <param name="resolution">The desired thumbnail resolution</param>
    public: void SetThumbnailResolution(const QSize &resolution);

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
