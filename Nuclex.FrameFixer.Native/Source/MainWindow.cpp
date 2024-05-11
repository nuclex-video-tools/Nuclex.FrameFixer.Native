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
#include "./Algorithm/InterlaceDetector.h"
#include "./Algorithm/PreviewDeinterlacer.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>

#include <Nuclex/Support/Text/LexicalCast.h>
#include <Nuclex/Pixels/Storage/BitmapSerializer.h>

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    thumbnailItemModel(),
    thumbnailPaintDelegate(),
    servicesRoot(),
    currentMovie(),
    analysisThread(),
    analysisThreadMutex(new QMutex()) {

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
      this->ui->markDiscardButton, &QPushButton::clicked,
      this, &MainWindow::markDiscardClicked
    );
    connect(
      this->ui->markBcButton, &QPushButton::clicked,
      this, &MainWindow::markBcFrameClicked
    );
    connect(
      this->ui->markCdButton, &QPushButton::clicked,
      this, &MainWindow::markCdFrameClicked
    );
    connect(
      this->ui->markBottomCButton, &QPushButton::clicked,
      this, &MainWindow::markBottomCFrameClicked
    );
    connect(
      this->ui->markTopCButton, &QPushButton::clicked,
      this, &MainWindow::markTopCFrameClicked
    );
    connect(
      this->ui->markProgressiveButton, &QPushButton::clicked,
      this, &MainWindow::markProgressiveClicked
    );
    connect(
      this->ui->markAverageButton, &QPushButton::clicked,
      this, &MainWindow::markAverageClicked
    );
    connect(
      this->ui->markDuplicateButton, &QPushButton::clicked,
      this, &MainWindow::markDuplicateClicked
    );
    connect(
      this->ui->thumbnailList->selectionModel(), &QItemSelectionModel::selectionChanged,
      this, &MainWindow::selectedThumbnailChanged
    );

    connect(
      this->ui->exportButton, &QPushButton::clicked,
      this, &MainWindow::exportClicked
    );
    connect(
      this->ui->saveButton, &QPushButton::clicked,
      this, &MainWindow::saveClicked
    );
    connect(
      this->ui->quitButton, &QPushButton::clicked,
      this, &MainWindow::quitClicked
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::ingestMovieFrames() {
    std::string frameDirectoryPath = this->ui->frameDirectoryText->text().toStdString();
    this->currentMovie = Movie::FromImageFolder(frameDirectoryPath);

    this->thumbnailItemModel->SetMovie(this->currentMovie);
    this->thumbnailPaintDelegate->SetMovie(this->currentMovie);

    //startAnaylsisThread();
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

  void MainWindow::toggleFrameType(FrameType frameType) {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        if(selectedFrame.Type == frameType) {
          selectedFrame.Type = FrameType::Unknown;
        } else {
          selectedFrame.Type = frameType;
        }

        this->ui->thumbnailList->update();
        this->ui->thumbnailList->viewport()->update();

        displayFrameInView(selectedFrame);
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

      QImage bitmap(QString::fromStdString(imagePath));

      FrameType frameType = frame.Type;
      if(frameType == FrameType::Unknown) {
        frameType = frame.ProvisionalType; // this one is calculated
      }
      if((frame.Type == FrameType::BC) && (frame.Index >= 1)) {
        std::string previousImagePath = this->currentMovie->GetFramePath(frame.Index - 1);
        QImage previousBitmap(QString::fromStdString(previousImagePath));          
        PreviewDeinterlacer::Deinterlace(&previousBitmap, bitmap, true);
      } else if((frame.Type == FrameType::CD) && (frame.Index >= 1)) {
        std::string previousImagePath = this->currentMovie->GetFramePath(frame.Index - 1);
        QImage previousBitmap(QString::fromStdString(previousImagePath));          
        PreviewDeinterlacer::Deinterlace(&previousBitmap, bitmap, false);
      } else if((frame.Type == FrameType::BC) || (frame.Type == FrameType::BottomC)) {
        PreviewDeinterlacer::Deinterlace(nullptr, bitmap, true);
      } else if((frame.Type == FrameType::CD) || (frame.Type == FrameType::TopC)) {
        PreviewDeinterlacer::Deinterlace(nullptr, bitmap, false);
      }

      std::unique_ptr<QGraphicsScene> frameScene = std::make_unique<QGraphicsScene>();
      std::unique_ptr<QGraphicsPixmapItem> pixmapItem = (
        std::make_unique<QGraphicsPixmapItem>(QPixmap::fromImage(bitmap))
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

  void MainWindow::exportClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::unique_ptr<QImage> previousFrame;


      std::size_t frameCount = this->currentMovie->Frames.size();
      if(frameCount >= 1200) {
        frameCount = 1200;
      }
      for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {



      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::saveClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      this->currentMovie->SaveState();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::quitClicked() {
    close();

    // Another option, but I read it's akin to calling exit(0), aka crash and burn.
    //QApplication::quit();
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::analyzeMovieFramesInThread() {
    Nuclex::Pixels::Storage::BitmapSerializer serializer;

    for(std::size_t index = 0; index < this->currentMovie->Frames.size(); ++index) {
      Frame &frame = this->currentMovie->Frames[index];
      if(!frame.Combedness.has_value()) {
        std::string framePath = this->currentMovie->GetFramePath(index);
        Nuclex::Pixels::Bitmap frameBitmap = serializer.Load(framePath);

        double combedness = InterlaceDetector::GetInterlaceProbability(frameBitmap);
        frame.Combedness = combedness;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::stopAnalysisThread() {
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::startAnaylsisThread() {
    if(static_cast<bool>(this->currentMovie)) {
      QMutexLocker locker(this->analysisThreadMutex.get());
      if(!static_cast<bool>(this->analysisThread)) {
        this->analysisThread.reset(
          QThread::create(&MainWindow::callAnalyzeMovieFramesInThread, this)
        );
        this->analysisThread->start();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::CriuGui
