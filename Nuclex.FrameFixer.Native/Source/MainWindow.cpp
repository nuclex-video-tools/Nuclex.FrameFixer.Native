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

#include <QFileDialog>

// Qt Image Viewer example (uses QLabel in QScrollArea)
// https://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html
//

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()) {

    this->ui->setupUi(this);

    this->ui->thumbnailsImage->setViewMode(QListView::ViewMode::IconMode);
    this->ui->thumbnailsImage->setIconSize(QSize(100, 100));
    this->ui->thumbnailsImage->setWrapping(false);
    this->ui->thumbnailsImage->setUniformItemSizes(true);
    this->thumbnailItemModel.reset(new FrameThumbnailItemModel());
    this->ui->thumbnailsImage->setModel(this->thumbnailItemModel.get());

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
    /*
    connect(
      this->ui->quitButton, &QPushButton::clicked,
      this, &MainWindow::quitClicked
    );
    */
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

  void MainWindow::ingestMovieFrames() {
    std::string frameDirectoryPath = this->ui->frameDirectoryText->text().toStdString();
    this->currentMovie = Movie::FromImageFolder(frameDirectoryPath);

    this->thumbnailItemModel->SetMovie(this->currentMovie);
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
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markCdFrameClicked() {
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markProgressiveFrameClicked() {
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::quitClicked() {
    close();

    // Another option, but I read it's akin to calling exit(0), aka crash and burn.
    //QApplication::quit();
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::CriuGui
