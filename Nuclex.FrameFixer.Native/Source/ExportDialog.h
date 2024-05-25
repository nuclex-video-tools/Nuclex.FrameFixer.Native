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

#ifndef NUCLEX_FRAMEFIXER_EXPORTDIALOG_H
#define NUCLEX_FRAMEFIXER_EXPORTDIALOG_H

#include "Nuclex/FrameFixer/Config.h"

#include <QDialog> // for QDialog
#include <memory> // for std::unique_ptr
#include <optional> // for std::optional
#include <map> // for std::pair

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  class ServicesRoot;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Services

namespace Ui {

  // ------------------------------------------------------------------------------------------- //

  class ExportDialog;

  // ------------------------------------------------------------------------------------------- //

} // namespace Ui

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dialog that asks the user for the range of frames to export</summary>
  class ExportDialog : public QDialog {
    Q_OBJECT

    /// <summary>Initializes a new settings dialog</summary>
    /// <param name="parent">Outer dialog that is displaying this dialog</param>
    public: explicit ExportDialog(QWidget *parent = nullptr);

    /// <summary>Frees all memory used by the dialog</summary>
    public: ~ExportDialog();

    public: void SetInitialExportDirectory(const QString &directory);
    public: void SetMaximumFrameCount(std::size_t frameCount);
    public: void SetInitialframeCount(std::size_t frameCount);

    public: std::string GetExportDirectory() const;
    public: std::optional<std::pair<std::size_t, std::size_t>> GetInputFrameRange() const;
    public: std::optional<std::pair<std::size_t, std::size_t>> GetOutputFrameRange() const;

#if 0
    /// <summary>Binds the dialog to the specified service provide</summary>
    /// <param name="servicesRoot">Application service provider to bind to</param>
    public: void BindToServicesRoot(const std::shared_ptr<Services::ServicesRoot> &servicesRoot);
#endif

    /// <summary>Verifies the settings when the dialog is closed via the okay button</summary>
    protected: void accept() override;

    /// <summary>Enables or disables some controls when export all is chosen</summary>
    /// <param name="checked">
    ///   True if the export all option is selected, false otherwise
    /// </param>
    private: void exportAllChosen(bool checked);

    /// <summary>Enables or disables some controls when export input frames is chosen</summary>
    /// <param name="checked">
    ///   True if the export input frame range option is selected, false otherwise
    /// </param>
    private: void exportInputFrameRangeChosen(bool checked);

    /// <summary>Enables or disables some controls when export output frames is chosen</summary>
    /// <param name="checked">
    ///   True if the export output frame range option is selected, false otherwise
    /// </param>
    private: void exportOutputFrameRangeChosen(bool checked);

    /// <summary>Opens the directory browser when the user clicks on the browse button</summary>
    private: void browseExportDirectoryClicked();

    /// <summary>
    ///   Verifies the current settings and generates a message indicating the problem
    /// </summary>
    /// <returns>An empty string if the settings are okay, otherwise a problem message</returns>
    private: QString verifySettingsOrStateProblem();

    /// <summary>
    ///   Displays a discard/cancel message box when the user tries to close the dialog
    ///   while there are unsaved changes that are also invalid.
    /// <summary>
    /// <param name="issuesPreventingClose">
    ///   Issues that prevent the changes from being applied
    /// </param>
    /// <param name="closeEvent">
    ///   Event that was sent to the <see cref="closeEvent" /> method
    /// </param>
    private: void handleCloseEventWithDiscardCancel(
      const QString &issuesPreventingClose,
      QCloseEvent *closeEvent
    );

    /// <summary>The user interface arrangement generated from the .ui file</summary>
    private: std::unique_ptr<Ui::ExportDialog> ui;
#if 0
    /// <summary>Service provider the dialog has been bound to, can be null</summary>
    private: std::shared_ptr<Services::ServicesRoot> servicesRoot;
#endif

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_EXPORTDIALOG_H
