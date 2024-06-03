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

#include "./RenderDialog.h"
#include "ui_RenderDialog.h"

#include "./DeinterlacerItemModel.h"
#include "./InterpolatorItemModel.h"

#include "./Services/ServicesRoot.h" // for ServicesRoot
#include "./Services/DeinterlacerRepository.h"
#include "./Services/InterpolatorRepository.h"

#include <QFileDialog> // for QFileDialog, shows file and folder selection dialogs
#include <QCloseEvent> // for QCloseEvent
#include <QMessageBox> // for QMessageBox
#include <QRadioButton> // for QRadioButton

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  RenderDialog::RenderDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::RenderDialog>()),
    servicesRoot(),
    deinterlacerModel(std::make_unique<DeinterlacerItemModel>()),
    interpolatorModel(std::make_unique<InterpolatorItemModel>()) {

    this->ui->setupUi(this);

    connect(
      this->ui->browseTargetDirectoryButton, &QPushButton::clicked,
      this, &RenderDialog::browseTargetDirectoryClicked
    );
    connect(
      this->ui->renderAllChoice, &QRadioButton::toggled,
      this, &RenderDialog::everythingChosen
    );
    connect(
      this->ui->renderInputRangeChoice, &QRadioButton::toggled,
      this, &RenderDialog::inputFrameRangeChosen
    );
    connect(
      this->ui->renderOutputRangeChoice, &QRadioButton::toggled,
      this, &RenderDialog::outputFrameRangeChosen
    );

    this->ui->deinterlacerCombo->setModel(this->deinterlacerModel.get());
    this->ui->interpolatorCombo->setModel(this->interpolatorModel.get());

    everythingChosen(true);
  }

  // ------------------------------------------------------------------------------------------- //

  RenderDialog::~RenderDialog() {}

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::BindToServicesRoot(
    const std::shared_ptr<Services::ServicesRoot> &servicesRoot
  ) {
    this->servicesRoot = servicesRoot;

    if(static_cast<bool>(servicesRoot)) {
      this->deinterlacerModel->SetDeinterlacers(
        servicesRoot->Deinterlacers()->GetDeinterlacers()
      );
      this->interpolatorModel->SetInterpolators(
        servicesRoot->Interpolators()->GetInterpolators()
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::SetInitialTargetDirectory(const QString &directory) {
    this->ui->targetDirectoryText->setText(directory);
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::SetMaximumFrameCount(std::size_t frameCount) {
    this->ui->inputStartFrameNumber->setMaximum(static_cast<int>(frameCount));
    this->ui->inputEndFrameNumber->setMaximum(static_cast<int>(frameCount));    
    this->ui->inputEndFrameNumber->setValue(static_cast<int>(frameCount));    
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::pair<std::size_t, std::size_t>> RenderDialog::GetInputFrameRange() const {
    if(this->ui->renderInputRangeChoice->isChecked()) {
      return std::pair<std::size_t, std::size_t>(
        this->ui->inputStartFrameNumber->value(),
        this->ui->inputEndFrameNumber->value()
      );
    } else {
      return std::optional<std::pair<std::size_t, std::size_t>>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::pair<std::size_t, std::size_t>> RenderDialog::GetOutputFrameRange() const {
    if(this->ui->renderOutputRangeChoice->isChecked()) {
      return std::pair<std::size_t, std::size_t>(
        this->ui->outputStartFrameNumber->value(),
        this->ui->outputEndFrameNumber->value()
      );
    } else {
      return std::optional<std::pair<std::size_t, std::size_t>>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::SetInitialframeCount(std::size_t frameCount) {
    this->ui->inputEndFrameNumber->setValue(static_cast<int>(frameCount));
  }

  // ------------------------------------------------------------------------------------------- //

  std::string RenderDialog::GetTargetDirectory() const {
    return this->ui->targetDirectoryText->text().toStdString();
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::accept() {
    QString issuesPreventingClose = verifySettingsOrStateProblem();
    if(issuesPreventingClose.isEmpty()) {
      //if(static_cast<bool>(this->servicesRoot)) {
      //  applyToSettings();
      //  servicesRoot->GetSettings()->Save();
      //}
      QDialog::accept();
    } else {
      std::unique_ptr<QMessageBox> messageBox = (
        std::make_unique<QMessageBox>(this)
      );
      messageBox->setText(
        QString(u8"The current settings are not valid.\n\n") +
        issuesPreventingClose
      );
      messageBox->setIcon(QMessageBox::Icon::Warning);
      messageBox->exec();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::everythingChosen(bool checked) {
    this->ui->inputStartFrameLabel->setEnabled(!checked);
    this->ui->inputStartFrameNumber->setEnabled(!checked);
    this->ui->inputEndFrameLabel->setEnabled(!checked);
    this->ui->inputEndFrameNumber->setEnabled(!checked);

    this->ui->outputStartFrameLabel->setEnabled(!checked);
    this->ui->outputStartFrameNumber->setEnabled(!checked);
    this->ui->outputEndFrameLabel->setEnabled(!checked);
    this->ui->outputEndFrameNumber->setEnabled(!checked);
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::inputFrameRangeChosen(bool checked) {
    this->ui->inputStartFrameLabel->setEnabled(checked);
    this->ui->inputStartFrameNumber->setEnabled(checked);
    this->ui->inputEndFrameLabel->setEnabled(checked);
    this->ui->inputEndFrameNumber->setEnabled(checked);

    this->ui->outputStartFrameLabel->setEnabled(!checked);
    this->ui->outputStartFrameNumber->setEnabled(!checked);
    this->ui->outputEndFrameLabel->setEnabled(!checked);
    this->ui->outputEndFrameNumber->setEnabled(!checked);
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::outputFrameRangeChosen(bool checked) {
    this->ui->inputStartFrameLabel->setEnabled(!checked);
    this->ui->inputStartFrameNumber->setEnabled(!checked);
    this->ui->inputEndFrameLabel->setEnabled(!checked);
    this->ui->inputEndFrameNumber->setEnabled(!checked);

    this->ui->outputStartFrameLabel->setEnabled(checked);
    this->ui->outputStartFrameNumber->setEnabled(checked);
    this->ui->outputEndFrameLabel->setEnabled(checked);
    this->ui->outputEndFrameNumber->setEnabled(checked);
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderDialog::browseTargetDirectoryClicked() {
    std::unique_ptr<QFileDialog> selectDirectoryDialog = (
      std::make_unique<QFileDialog>(this)
    );

    // Configure the dialog to let the user browse for a directory
    selectDirectoryDialog->setFileMode(QFileDialog::FileMode::Directory);
    selectDirectoryDialog->setOption(QFileDialog::Option::ShowDirsOnly);
    selectDirectoryDialog->setWindowTitle(
      QString(u8"Select directory to export deinterlaced frames to")
    );

    selectDirectoryDialog->setDirectory(this->ui->targetDirectoryText->text());

    // Display the dialog, the user can select a directory or hit cancel
    int result = selectDirectoryDialog->exec();

    // If the user selected a directory and did not cancel,
    // store its full path in the working directory text box.
    if(result == QDialog::Accepted) {
      QStringList selectedFiles = selectDirectoryDialog->selectedFiles();
      if(!selectedFiles.empty()) {
        this->ui->targetDirectoryText->setText(selectedFiles[0]);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  QString RenderDialog::verifySettingsOrStateProblem() {
    //if(this->ui->stateDirectoryText->text().isEmpty()) {
    //  return QString(u8"No state directory has been selected");
    //}

    return QString();
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
