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

#ifndef NUCLEX_FRAMEFIXER_INTERPOLATORITEMMODEL_H
#define NUCLEX_FRAMEFIXER_INTERPOLATORITEMMODEL_H

#include "Nuclex/FrameFixer/Config.h"

#include <QAbstractItemModel>
#include <QStringList>

#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex::FrameFixer::Algorithm::Interpolation {

  // ------------------------------------------------------------------------------------------- //

  class FrameInterpolator;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Presents a list of deinterlacers with human-readable names</summary>
  class InterpolatorItemModel : public QAbstractListModel {
    Q_OBJECT

    // List of interpolators
    public: typedef std::vector<
      std::shared_ptr<Algorithm::Interpolation::FrameInterpolator>
    > FrameInterpolatorList;

    /// <summary>Initializes a new item model for interpolators</summary>
    /// <param name="parent">Always null</param>
    public: explicit InterpolatorItemModel(QObject *parent = nullptr);

    /// <summary>Stops all background processing and frees all memory</summary>
    public: ~InterpolatorItemModel() override;

    /// <summary>Sets the list of interpolators that will be exposed for Qt</summary>
    /// <param name="interpolators">Interpolators the item model will expose</param>
    public: void SetInterpolators(const FrameInterpolatorList &interpolators);

    /// <summary>Retrieves an interpolator by its index</summary>
    /// <param name="index">Index of the interpolator that will be retrieved</param>
    /// <returns>The interpolator with the specified index</returns>
    public: std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> GetInterpolator(
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

    /// <summary>Interpolators whose names will be provided as an item model</summary>
    private: FrameInterpolatorList interpolators;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_INTERPOLATORITEMMODEL_H
