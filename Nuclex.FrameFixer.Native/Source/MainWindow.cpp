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

#include "./MainWindow.h"
#include "./ExportDialog.h"
#include "ui_MainWindow.h"

#include "./Model/Movie.h"
#include "./FrameThumbnailItemModel.h"
#include "./FrameThumbnailPaintDelegate.h"
#include "./DeinterlacerItemModel.h"
#include "./Algorithm/Analysis/InterlaceDetector.h"
#include "./Exporter.h"

#include "./Algorithm/Deinterlace/PreviewDeinterlacer.h"
#include "./Algorithm/Deinterlace/LibAvNNedi3Deinterlacer.h"
#include "./Algorithm/Deinterlace/LibAvYadifDeinterlacer.h"
#include "./Algorithm/Deinterlace/LibAvEstdifDeinterlacer.h"
#include "./Algorithm/Filter.h"
#include "./Algorithm/Deblend/GradientMatrix.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QComboBox>

#include <Nuclex/Support/Text/LexicalCast.h>
#include <Nuclex/Pixels/Storage/BitmapSerializer.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  void SaveImage(const QImage &image, const std::string &directory, std::size_t frameIndex) {
    std::string path = directory;

    std::string::size_type length = directory.length();
    if((length >= 1) && (directory[length - 1] != '/')) {
      path = directory + u8"/";
    } else {
      path = directory;
    }

    std::string filename = Nuclex::Support::Text::lexical_cast<std::string>(frameIndex);
    for(std::size_t index = filename.length(); index < 8; ++index) {
      path.push_back(u8'0');
    }
    path.append(filename);
    path.append(u8".png");

    image.save(QString::fromStdString(path), u8"PNG");
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    thumbnailItemModel(),
    thumbnailPaintDelegate(),
    deinterlacerItemModel(),
    servicesRoot(),
    currentMovie(),
    deinterlacer(),
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

    this->deinterlacerItemModel.reset(new DeinterlacerItemModel());
    DeinterlacerItemModel::DeinterlacerList deinterlacers;
    deinterlacers.push_back(std::make_shared<Algorithm::Deinterlace::PreviewDeinterlacer>());
    deinterlacers.push_back(std::make_shared<Algorithm::Deinterlace::LibAvNNedi3Deinterlacer>());
    deinterlacers.push_back(std::make_shared<Algorithm::Deinterlace::LibAvYadifDeinterlacer>(false));
    deinterlacers.push_back(std::make_shared<Algorithm::Deinterlace::LibAvYadifDeinterlacer>(true));
    deinterlacers.push_back(std::make_shared<Algorithm::Deinterlace::LibAvEstdifDeinterlacer>());
    this->deinterlacerItemModel->SetDeinterlacers(deinterlacers);

    this->ui->deinterlacerCombo->setModel(this->deinterlacerItemModel.get());
    selectedDeinterlacerChanged(0); // Make sure deinterlace instance is set up

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
      this->ui->markTopFieldFirstButton, &QPushButton::clicked,
      this, &MainWindow::markTopFieldFirstClicked
    );
    connect(
      this->ui->markBottomFieldFirstButton, &QPushButton::clicked,
      this, &MainWindow::markBottomFieldFirstClicked
    );
    connect(
      this->ui->markBottomFieldOnlyButton, &QPushButton::clicked,
      this, &MainWindow::markBottomFieldOnlyClicked
    );
    connect(
      this->ui->markTopFieldOnlyButton, &QPushButton::clicked,
      this, &MainWindow::markTopFieldOnlyClicked
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
      this->ui->markTriplicateButton, &QPushButton::clicked,
      this, &MainWindow::markTriplicateClicked
    );
    connect(
      this->ui->markBlendedButton, &QPushButton::clicked,
      this, &MainWindow::markBlendedClicked
    );

    connect(
      this->ui->thumbnailList->selectionModel(), &QItemSelectionModel::selectionChanged,
      this, &MainWindow::selectedThumbnailChanged
    );
    connect(
      this->ui->deinterlacerCombo,
       static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, &MainWindow::selectedDeinterlacerChanged
    );

    connect(
      this->ui->swapFieldsOption, &QCheckBox::toggled,
      this, &MainWindow::swapFieldsOptionToggled
    );
    connect(
      this->ui->enhanceOption, &QCheckBox::toggled,
      this, &MainWindow::enhanceOptionToggled
    );
    connect(
      this->ui->previewOption, &QCheckBox::toggled,
      this, &MainWindow::previewOptionToggled
    );

    connect(
      this->ui->exportButton, &QPushButton::clicked,
      this, &MainWindow::exportClicked
    );
    connect(
      this->ui->showStatisticsButton, &QPushButton::clicked,
      this, &MainWindow::showStatisticsClicked
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

    std::size_t lastTaggedFrameIndex = getLastTaggedFrameIndex();
    if(lastTaggedFrameIndex != std::size_t(-1)) {
      this->ui->thumbnailList->scrollTo(
        this->thumbnailItemModel->index(static_cast<int>(lastTaggedFrameIndex))
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t MainWindow::getLastTaggedFrameIndex() const {
    std::size_t lastTaggedFrameIndex = std::size_t(-1);

    std::size_t frameCount = this->currentMovie->Frames.size();
    for(std::size_t index = 0; index < frameCount; ++index) {
      if(this->currentMovie->Frames[index].Type != FrameType::Unknown) {
        lastTaggedFrameIndex = index;
      }
    }

    return lastTaggedFrameIndex;
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
      QString(u8"Select directory containing movie frames")
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

  void MainWindow::showStatisticsClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t lastMatchingIndex = 0;

      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex == std::size_t(-1)) {
        return;
      }

      std::size_t emittedFrameCount = 0;
      for(std::size_t index = 0; index <= selectedFrameIndex; ++index) {
        FrameType frameType = this->currentMovie->Frames[index].Type;
        if(frameType == FrameType::Triplicate) {
          emittedFrameCount += 3;
        } else if(frameType == FrameType::Duplicate) {
          emittedFrameCount += 2;
        } else if(frameType != FrameType::Discard) {
          ++emittedFrameCount;
        }

        if(emittedFrameCount == ((index + 2) * 4 / 5)) {
          lastMatchingIndex = index;
        }
      }

      std::string status(u8"Frame: ");
      status += Nuclex::Support::Text::lexical_cast<std::string>(selectedFrameIndex);
      status += u8"\n";
      status += u8"Expected: ";
      status += Nuclex::Support::Text::lexical_cast<std::string>((selectedFrameIndex + 2) * 4 / 5);
      status += u8"\n";
      status += u8"Emitted: ";
      status += Nuclex::Support::Text::lexical_cast<std::string>(emittedFrameCount);
      status += u8"\n";
      status += u8"Sync At: ";
      status += Nuclex::Support::Text::lexical_cast<std::string>(lastMatchingIndex);
      status += u8"\n";
      this->ui->frameStatusLabel->setText(QString::fromStdString(status));      
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

  void MainWindow::swapFieldsOptionToggled(bool checked) {
    (void)checked;
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        displayFrameInView(selectedFrame);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::enhanceOptionToggled(bool checked) {
    (void)checked;
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        displayFrameInView(selectedFrame);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::previewOptionToggled(bool checked) {
    (void)checked;
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
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
      QImage frameImage;

      if(this->ui->previewOption->isChecked()) {
        Exporter movieExporter;
        movieExporter.SetDeinterlacer(this->deinterlacer);
        
        if(this->ui->swapFieldsOption->isChecked()) {
          movieExporter.FlipTopAndBottomField();
        }

        frameImage = movieExporter.Preview(this->currentMovie, frame.Index);
      } else {
        std::string imagePath = this->currentMovie->GetFramePath(frame.Index);
        frameImage.load(QString::fromStdString(imagePath));
      }

      if(this->ui->enhanceOption->isChecked()) {
        Filter::LuminanceHighPass(frameImage);
      }

      // Display the frame in Qt's graphics view
      {
        std::unique_ptr<QGraphicsScene> frameScene = std::make_unique<QGraphicsScene>();
        std::unique_ptr<QGraphicsPixmapItem> pixmapItem = (
          std::make_unique<QGraphicsPixmapItem>(QPixmap::fromImage(frameImage))
        );
        frameScene->addItem(pixmapItem.get());

        this->ui->frameInspectionImage->setScene(frameScene.get());
        frameScene.release();
        pixmapItem.release();
      }

      // Update the frame index and path displayed in the status corner
      {
        std::string status(u8"Frame: ");
        status += Nuclex::Support::Text::lexical_cast<std::string>(frame.Index);
        status += u8"\n";
        status += u8"File: ";
        status += frame.Filename;
        this->ui->frameStatusLabel->setText(QString::fromStdString(status));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::selectedDeinterlacerChanged(int selectedIndex) {
    if(static_cast<bool>(this->deinterlacer)) {
      this->deinterlacer->CoolDown();
    }

    this->deinterlacer = this->deinterlacerItemModel->GetDeinterlacer(selectedIndex);

    if(static_cast<bool>(this->deinterlacer)) {
      this->deinterlacer->WarmUp();
    }

    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        displayFrameInView(selectedFrame);
      }
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
      std::unique_ptr<ExportDialog> exportDialog = (
        std::make_unique<ExportDialog>(this)
      );

      // Stuff that I should be able to remove if I add a proper model class
      // that will manage the data shoveling properly.
      {
        std::string::size_type length = this->currentMovie->FrameDirectory.length();

        std::string exportPath;
        if((length >= 1) && (this->currentMovie->FrameDirectory[length - 1] == '/')) {
          exportPath = this->currentMovie->FrameDirectory.substr(0, length - 1) + u8".export/";
        } else {
          exportPath = this->currentMovie->FrameDirectory + u8".export/";
        }
        exportDialog->SetInitialExportDirectory(QString::fromStdString(exportPath));

        exportDialog->SetMaximumFrameCount(this->currentMovie->Frames.size());

        std::size_t lastTaggedFrameIndex = getLastTaggedFrameIndex();
        if(lastTaggedFrameIndex != std::size_t(-1)) {
          exportDialog->SetInitialframeCount(lastTaggedFrameIndex);
        } else {
          exportDialog->SetInitialframeCount(this->currentMovie->Frames.size());
        }
      }

      //if(static_cast<bool>(this->servicesRoot)) {
      //  exportDialog->BindToServicesRoot(this->servicesRoot);
      //}

      int result = exportDialog->exec();
      if(result == QDialog::DialogCode::Accepted) {
        exportDetelecinedFrames(
          exportDialog->GetExportDirectory(),
          exportDialog->GetInputFrameRange(),
          exportDialog->GetOutputFrameRange()
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::exportDetelecinedFrames(
    const std::string &directory,
    std::optional<std::pair<std::size_t, std::size_t>> inputFrameRange /* = (
      std::optional<std::pair<std::size_t, std::size_t>>()
    ) */,
    std::optional<std::pair<std::size_t, std::size_t>> outputFrameRange /* = (
      std::optional<std::pair<std::size_t, std::size_t>>()
    ) */
  ) {
    Exporter movieExporter;
    movieExporter.SetDeinterlacer(this->deinterlacer);
    
    if(this->ui->swapFieldsOption->isChecked()) {
      movieExporter.FlipTopAndBottomField();
    }

    if(inputFrameRange.has_value()) {
      movieExporter.RestrictRangeOfInputFrames(
        inputFrameRange.value().first, inputFrameRange.value().second
      );
    }
    if(outputFrameRange.has_value()) {
      movieExporter.RestrictRangeOfOutputFrames(
        outputFrameRange.value().first, outputFrameRange.value().second
      );
    }
    movieExporter.Export(this->currentMovie, directory);
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

        double combedness = Algorithm::Analysis::InterlaceDetector::GetInterlaceProbability(frameBitmap);
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

} // namespace Nuclex::FrameFixer
