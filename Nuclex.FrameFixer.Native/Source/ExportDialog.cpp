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

#include "./ExportDialog.h"
#include "ui_ExportDialog.h"

//#include "./Services/ServicesRoot.h" // for ServicesRoot

#include <QFileDialog> // for QFileDialog, shows file and folder selection dialogs
#include <QCloseEvent> // for QCloseEvent
#include <QMessageBox> // for QMessageBox
#include <QRadioButton>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::ExportDialog>()) {

    this->ui->setupUi(this);

    connect(
      this->ui->browseExportDirectoryButton, &QPushButton::clicked,
      this, &ExportDialog::browseExportDirectoryClicked
    );
    connect(
      this->ui->exportAllChoice, &QRadioButton::toggled,
      this, &ExportDialog::exportAllChosen
    );
    connect(
      this->ui->exportInputRangeChoice, &QRadioButton::toggled,
      this, &ExportDialog::exportInputFrameRangeChosen
    );
    connect(
      this->ui->exportOutputRangeChoice, &QRadioButton::toggled,
      this, &ExportDialog::exportOutputFrameRangeChosen
    );

    exportAllChosen(true);
  }

  // ------------------------------------------------------------------------------------------- //

  ExportDialog::~ExportDialog() {}

  // ------------------------------------------------------------------------------------------- //

  void ExportDialog::SetInitialExportDirectory(const QString &directory) {
    this->ui->exportDirectoryText->setText(directory);
  }

  // ------------------------------------------------------------------------------------------- //

  void ExportDialog::SetMaximumFrameCount(std::size_t frameCount) {
    this->ui->inputStartFrameNumber->setMaximum(static_cast<int>(frameCount));
    this->ui->inputEndFrameNumber->setMaximum(static_cast<int>(frameCount));    
    this->ui->inputEndFrameNumber->setValue(static_cast<int>(frameCount));    
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::pair<std::size_t, std::size_t>> ExportDialog::GetInputFrameRange() const {
    if(this->ui->exportInputRangeChoice->isChecked()) {
      return std::pair<std::size_t, std::size_t>(
        this->ui->inputStartFrameNumber->value(),
        this->ui->inputEndFrameNumber->value()
      );
    } else {
      return std::optional<std::pair<std::size_t, std::size_t>>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::pair<std::size_t, std::size_t>> ExportDialog::GetOutputFrameRange() const {
    if(this->ui->exportOutputRangeChoice->isChecked()) {
      return std::pair<std::size_t, std::size_t>(
        this->ui->outputStartFrameNumber->value(),
        this->ui->outputEndFrameNumber->value()
      );
    } else {
      return std::optional<std::pair<std::size_t, std::size_t>>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ExportDialog::SetInitialframeCount(std::size_t frameCount) {
    this->ui->inputEndFrameNumber->setValue(static_cast<int>(frameCount));
  }

  // ------------------------------------------------------------------------------------------- //

  std::string ExportDialog::GetExportDirectory() const {
    return this->ui->exportDirectoryText->text().toStdString();
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  void ExportDialog::BindToServicesRoot(
    const std::shared_ptr<Services::ServicesRoot> &servicesRoot
  ) {
    this->servicesRoot = servicesRoot;

    if(static_cast<bool>(servicesRoot)) {
      const std::shared_ptr<Services::Settings> &settings = servicesRoot->GetSettings();
      this->ui->stateDirectoryText->setText(
        QString::fromStdString(settings->GetStateDirectory())
      );
    }

    this->changed = false;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  void ExportDialog::accept() {
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

  void ExportDialog::exportAllChosen(bool checked) {
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

  void ExportDialog::exportInputFrameRangeChosen(bool checked) {
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

  void ExportDialog::exportOutputFrameRangeChosen(bool checked) {
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

  void ExportDialog::browseExportDirectoryClicked() {
    std::unique_ptr<QFileDialog> selectDirectoryDialog = (
      std::make_unique<QFileDialog>(this)
    );

    // Configure the dialog to let the user browse for a directory
    selectDirectoryDialog->setFileMode(QFileDialog::FileMode::Directory);
    selectDirectoryDialog->setOption(QFileDialog::Option::ShowDirsOnly);
    selectDirectoryDialog->setWindowTitle(
      QString(u8"Select directory to export deinterlaced frames to")
    );

    selectDirectoryDialog->setDirectory(this->ui->exportDirectoryText->text());

    // Display the dialog, the user can select a directory or hit cancel
    int result = selectDirectoryDialog->exec();

    // If the user selected a directory and did not cancel,
    // store its full path in the working directory text box.
    if(result == QDialog::Accepted) {
      QStringList selectedFiles = selectDirectoryDialog->selectedFiles();
      if(!selectedFiles.empty()) {
        this->ui->exportDirectoryText->setText(selectedFiles[0]);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  QString ExportDialog::verifySettingsOrStateProblem() {
    //if(this->ui->stateDirectoryText->text().isEmpty()) {
    //  return QString(u8"No state directory has been selected");
    //}

    return QString();
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
