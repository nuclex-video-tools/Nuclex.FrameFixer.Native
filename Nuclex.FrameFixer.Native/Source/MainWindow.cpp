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
#define NUCLEX_CRIUGUI_SOURCE 1

#include "./MainWindow.h"
#include "ui_MainWindow.h"

#include "./Model/Movie.h"

#include "./FrameThumbnailItemModel.h"
#include "./FrameThumbnailPaintDelegate.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <Nuclex/Support/Text/LexicalCast.h>

// Qt Image Viewer example (uses QLabel in QScrollArea)
// https://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html
//

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    thumbnailItemModel(),
    thumbnailPaintDelegate(),
    servicesRoot(),
    currentMovie() {

    this->ui->setupUi(this);

    this->ui->thumbnailList->setViewMode(QListView::ViewMode::IconMode);
    this->ui->thumbnailList->setIconSize(QSize(128, 128));
    // Prevent the thumbnails strip from wrapping around. Important, because otherwise
    // the thumbnails would be arranged in a grid instead of one long strip.
    this->ui->thumbnailList->setWrapping(false);
    // Tell the thumbnail list that all thumbnails have the exact same size. Important,
    // because otherwise the widget would query *all* thumbnails added to it for their
    // size in order to figure out its total size, forcing us to load each and every frame.
    this->ui->thumbnailList->setUniformItemSizes(true);
    this->ui->thumbnailList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection
    );

    // Set up our thumbnail item view model, which will tell the QListView how many items
    // there are and load as well as cache the thumbnail images as needed. Without this,
    // performance would absolutely tank (average movies have between 150'000 frames).
    this->thumbnailItemModel.reset(new FrameThumbnailItemModel());
    this->ui->thumbnailList->setModel(this->thumbnailItemModel.get());

    this->thumbnailPaintDelegate.reset(new FrameThumbnailPaintDelegate());
    this->ui->thumbnailList->setItemDelegate(this->thumbnailPaintDelegate.get());

    connectUiSignals();
  }

  // ------------------------------------------------------------------------------------------- //

  MainWindow::~MainWindow() {}

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::BindToServicesRoot(
    const std::shared_ptr<Services::ServicesRoot> &servicesRoot
  ) {
    this->servicesRoot = servicesRoot;

    //if(static_cast<bool>(this->servicesRoot)) {
    //  updateProcessList();
    //}
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::connectUiSignals() {
    connect(
      this->ui->browseButton, &QPushButton::clicked,
      this, &MainWindow::browseClicked
    );

    connect(
      this->ui->bcFrameButton, &QPushButton::clicked,
      this, &MainWindow::markBcFrameClicked
    );
    connect(
      this->ui->cdFrameButton, &QPushButton::clicked,
      this, &MainWindow::markCdFrameClicked
    );
    connect(
      this->ui->prFrameButton, &QPushButton::clicked,
      this, &MainWindow::markProgressiveFrameClicked
    );
    connect(
      this->ui->thumbnailList->selectionModel(), &QItemSelectionModel::selectionChanged,
      this, &MainWindow::selectedThumbnailChanged
    );
    /*
    connect(
      this->ui->quitButton, &QPushButton::clicked,
      this, &MainWindow::quitClicked
    );
    */
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::ingestMovieFrames() {
    std::string frameDirectoryPath = this->ui->frameDirectoryText->text().toStdString();
    this->currentMovie = Movie::FromImageFolder(frameDirectoryPath);

    this->thumbnailItemModel->SetMovie(this->currentMovie);
    this->thumbnailPaintDelegate->SetMovie(this->currentMovie);
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::browseClicked() {
    std::unique_ptr<QFileDialog> selectDirectoryDialog = (
      std::make_unique<QFileDialog>(this)
    );

    // Configure the dialog to let the user browse for a directory
    selectDirectoryDialog->setFileMode(QFileDialog::FileMode::Directory);
    selectDirectoryDialog->setOption(QFileDialog::Option::ShowDirsOnly);
    selectDirectoryDialog->setWindowTitle(
      QString(u8"Select directory to use for saved snapshots")
    );

    // Display the dialog, the user can select a directory or hit cancel
    int result = selectDirectoryDialog->exec();

    // If the user selected a directory and did not cancel,
    // store its full path in the working directory text box.
    if(result == QDialog::Accepted) {
      QStringList selectedFiles = selectDirectoryDialog->selectedFiles();
      if(!selectedFiles.empty()) {
        this->ui->frameDirectoryText->setText(selectedFiles[0]);
        ingestMovieFrames();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markBcFrameClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        if(selectedFrame.Type == FrameType::BC) {
          selectedFrame.Type = FrameType::Unknown;
        } else {
          selectedFrame.Type = FrameType::BC;
        }
        this->ui->thumbnailList->update();
        this->ui->thumbnailList->viewport()->update();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markCdFrameClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        if(selectedFrame.Type == FrameType::CD) {
          selectedFrame.Type = FrameType::Unknown;
        } else {
          selectedFrame.Type = FrameType::CD;
        }
        this->ui->thumbnailList->update();
        this->ui->thumbnailList->viewport()->update();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markProgressiveFrameClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        if(selectedFrame.Type == FrameType::Progressive) {
          selectedFrame.Type = FrameType::Unknown;
        } else {
          selectedFrame.Type = FrameType::Progressive;
        }
        this->ui->thumbnailList->update();
        this->ui->thumbnailList->viewport()->update();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::selectedThumbnailChanged(
    const QItemSelection &selected, const QItemSelection &deselected
  ) {
    (void)deselected;

    if(selected.size() >= 1) {
      const QModelIndexList &selectedRanges = selected.at(0).indexes();
      if(selectedRanges.size() >= 1) {
        const QModelIndex selectedItem = selectedRanges.at(0);
        if(static_cast<bool>(this->currentMovie)) {
          displayFrameInView(this->currentMovie->Frames[selectedItem.row()]);
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::displayFrameInView(const Frame &frame) {
    if(static_cast<bool>(this->currentMovie)) {
      std::string imagePath = this->currentMovie->GetFramePath(frame.Index);
      QPixmap bitmap(QString::fromStdString(imagePath));

      std::unique_ptr<QGraphicsScene> frameScene = std::make_unique<QGraphicsScene>();
      std::unique_ptr<QGraphicsPixmapItem> pixmapItem = (
        std::make_unique<QGraphicsPixmapItem>(bitmap)
      );
      frameScene->addItem(pixmapItem.get());

      this->ui->frameInspectionImage->setScene(frameScene.get());
      frameScene.release();
      pixmapItem.release();

      std::string status(u8"Frame: ");
      status += Nuclex::Support::Text::lexical_cast<std::string>(frame.Index);
      status += u8"\n";
      status += u8"File: ";
      status += frame.Filename;
      this->ui->frameStatusLabel->setText(QString::fromStdString(status));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t MainWindow::getSelectedFrameIndex() const {
    QItemSelectionModel *selectionModel = this->ui->thumbnailList->selectionModel();

    const QModelIndexList &selectedRanges = selectionModel->selectedIndexes();
    if(selectedRanges.size() >= 1) {
      const QModelIndex selectedItem = selectedRanges.at(0);
      if(static_cast<bool>(this->currentMovie)) {
        return static_cast<std::size_t>(selectedItem.row());
      }
    }

    return std::size_t(-1);
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::quitClicked() {
    close();

    // Another option, but I read it's akin to calling exit(0), aka crash and burn.
    //QApplication::quit();
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::CriuGui
