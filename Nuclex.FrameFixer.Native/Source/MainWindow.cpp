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

#include "./MainWindow.h"
#include "./RenderDialog.h"
#include "./RenderProgressDialog.h"
#include "ui_MainWindow.h"

#include "./Services/ServicesRoot.h"
#include "./Services/DeinterlacerRepository.h"
#include "./Model/Movie.h"
#include "./Renderer.h"

#include "./DeinterlacerItemModel.h"
#include "./FrameThumbnailItemModel.h"
#include "./FrameThumbnailPaintDelegate.h"

#include <Nuclex/Support/Text/LexicalCast.h>

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QComboBox>

#include "./Algorithm/Filter.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    thumbnailItemModel(std::make_unique<FrameThumbnailItemModel>()),
    thumbnailPaintDelegate(std::make_unique<FrameThumbnailPaintDelegate>()),
    deinterlacerItemModel(std::make_unique<DeinterlacerItemModel>()),
    servicesRoot(),
    currentMovie(),
    deinterlacer() {

    this->ui->setupUi(this);

    this->thumbnailItemModel->SetThumbnailResolution(QSize(116, 116));

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

    // Prevent the check boxes from taking input focus. This helps keep the focus on
    // the thumbnail list rather than forcing the user to reach for the mouse after
    // each time they toggle preview and/or enhance.
    this->ui->enhanceOption->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->ui->previewOption->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->ui->swapFieldsOption->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    // Set up our thumbnail item view model, which will tell the QListView how many items
    // there are and load as well as cache the thumbnail images as needed. Without this,
    // performance would absolutely tank (average movies have between 150'000 frames).
    this->ui->thumbnailList->setModel(this->thumbnailItemModel.get());
    this->ui->thumbnailList->setItemDelegate(this->thumbnailPaintDelegate.get());
    this->ui->deinterlacerCombo->setModel(this->deinterlacerItemModel.get());

    connectUiSignals();
  }

  // ------------------------------------------------------------------------------------------- //

  MainWindow::~MainWindow() {}

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::BindToServicesRoot(
    const std::shared_ptr<Services::ServicesRoot> &servicesRoot
  ) {
    this->servicesRoot = servicesRoot;

    if(static_cast<bool>(this->servicesRoot)) {
      this->deinterlacerItemModel->SetDeinterlacers(
        servicesRoot->Deinterlacers()->GetDeinterlacers()
      );
    
      selectedDeinterlacerChanged(0); // Make sure deinterlace instance is set up
    }
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
      this->ui->markDeblendButton, &QPushButton::clicked,
      this, &MainWindow::markDeblendClicked
    );
    connect(
      this->ui->markDeblendButton, &QPushButton::clicked,
      this, &MainWindow::markDeblendClicked
    );
    connect(
      this->ui->markReplacedButton, &QPushButton::clicked,
      this, &MainWindow::markReplacedClicked
    );
    connect(
      this->ui->markReplaceWithInterpolatedButton, &QPushButton::clicked,
      this, &MainWindow::markReplaceWithInterpolatedClicked
    );
    connect(
      this->ui->markAppendInterpolatedButton, &QPushButton::clicked,
      this, &MainWindow::markAppendInterpolatedClicked
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
      this, &MainWindow::renderClicked
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
      if(this->currentMovie->Frames[index].Action != FrameAction::Unknown) {
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

  void MainWindow::markReplacedClicked() {
    if(static_cast<bool>(this->currentMovie)) {
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::markReplaceWithInterpolatedClicked() {
    if(static_cast<bool>(this->currentMovie)) {
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
        FrameAction frameType = this->currentMovie->Frames[index].Action;
        if(frameType == FrameAction::Triplicate) {
          emittedFrameCount += 3;
        } else if(frameType == FrameAction::Duplicate) {
          emittedFrameCount += 2;
        } else if(frameType != FrameAction::Discard) {
          ++emittedFrameCount;
        }
        if(this->currentMovie->Frames[index].AlsoInsertInterpolatedAfter.has_value()) {
          if(this->currentMovie->Frames[index].AlsoInsertInterpolatedAfter.value()) {
            ++emittedFrameCount;
          }
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

  void MainWindow::toggleFrameType(FrameAction frameType) {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        if(selectedFrame.Action == frameType) {
          selectedFrame.Action = FrameAction::Unknown;
        } else {
          selectedFrame.Action = frameType;
        }

        this->ui->thumbnailList->update();
        this->ui->thumbnailList->viewport()->update();

        displayFrameInView(selectedFrame);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::setupInterpolation(int distance) {
    if(static_cast<bool>(this->currentMovie)) {
      std::size_t selectedFrameIndex = getSelectedFrameIndex();
      if(selectedFrameIndex != std::size_t(-1)) {
        Frame &selectedFrame = this->currentMovie->Frames[selectedFrameIndex];
        selectedFrame.InterpolationSourceIndices = std::pair<std::size_t, std::size_t>(
          selectedFrameIndex - distance, selectedFrameIndex + distance
        );

        // TODO: Remove! Temporary for editing!
        selectedFrame.AlsoInsertInterpolatedAfter = true;
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
        Renderer movieRenderer;
        movieRenderer.SetDeinterlacer(this->deinterlacer);
        
        if(this->ui->swapFieldsOption->isChecked()) {
          movieRenderer.FlipTopAndBottomField();
        }

        frameImage = movieRenderer.Preview(this->currentMovie, frame.Index);
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

  void MainWindow::renderClicked() {
    if(static_cast<bool>(this->currentMovie)) {
      std::unique_ptr<RenderDialog> renderDialog = (
        std::make_unique<RenderDialog>(this)
      );
      if(static_cast<bool>(this->servicesRoot)) {
        renderDialog->BindToServicesRoot(this->servicesRoot);
      }

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
        renderDialog->SetInitialTargetDirectory(QString::fromStdString(exportPath));

        renderDialog->SetMaximumFrameCount(this->currentMovie->Frames.size());

        std::size_t lastTaggedFrameIndex = getLastTaggedFrameIndex();
        if(lastTaggedFrameIndex != std::size_t(-1)) {
          renderDialog->SetInitialframeCount(lastTaggedFrameIndex);
        } else {
          renderDialog->SetInitialframeCount(this->currentMovie->Frames.size());
        }
      }

      int result = renderDialog->exec();
      if(result == QDialog::DialogCode::Accepted) {
        exportDetelecinedFrames(
          renderDialog->GetTargetDirectory(),
          renderDialog->GetSelectedDeinterlacer(),
          renderDialog->GetSelectedInterpolator(),
          renderDialog->GetInputFrameRange(),
          renderDialog->GetOutputFrameRange()
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MainWindow::exportDetelecinedFrames(
    const std::string &directory,
    const std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> &deinterlacer,
    const std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> &interpolator,
    std::optional<std::pair<std::size_t, std::size_t>> inputFrameRange /* = (
      std::optional<std::pair<std::size_t, std::size_t>>()
    ) */,
    std::optional<std::pair<std::size_t, std::size_t>> outputFrameRange /* = (
      std::optional<std::pair<std::size_t, std::size_t>>()
    ) */
  ) {
    std::shared_ptr<Renderer> movieRenderer = std::make_shared<Renderer>();
    movieRenderer->SetDeinterlacer(deinterlacer);
    movieRenderer->SetInterpolator(interpolator);
    
    if(this->ui->swapFieldsOption->isChecked()) {
      movieRenderer->FlipTopAndBottomField();
    }

    if(inputFrameRange.has_value()) {
      movieRenderer->RestrictRangeOfInputFrames(
        inputFrameRange.value().first, inputFrameRange.value().second
      );
    }
    if(outputFrameRange.has_value()) {
      movieRenderer->RestrictRangeOfOutputFrames(
        outputFrameRange.value().first, outputFrameRange.value().second
      );
    }

    std::unique_ptr<RenderProgressDialog> progressDialog = (
      std::make_unique<RenderProgressDialog>(this)
    );
    if(static_cast<bool>(this->servicesRoot)) {
      progressDialog->BindToServicesRoot(this->servicesRoot);
    }
    progressDialog->SetRenderer(movieRenderer);

    progressDialog->Start(this->currentMovie, directory);
    progressDialog->exec();
    //movieRenderer.Render(this->currentMovie, directory);
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

} // namespace Nuclex::FrameFixer
