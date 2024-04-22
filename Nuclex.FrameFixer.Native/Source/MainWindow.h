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

#ifndef NUCLEX_CRIUGUI_MAINWINDOW_H
#define NUCLEX_CRIUGUI_MAINWINDOW_H

#include "./Config.h"

#include <QMainWindow> // for QMainWindow
#include <memory> // for std::unique_ptr

namespace Nuclex::Telecide::Services {

  // ------------------------------------------------------------------------------------------- //

  class ServicesRoot;

  // ------------------------------------------------------------------------------------------- //

}

namespace Ui {

  // ------------------------------------------------------------------------------------------- //

  class MainWindow;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages the main window of the application</summary>
  class MainWindow : public QMainWindow {
    Q_OBJECT

    /// <summary>Initializes a new main window for the CriuGui application</summary>
    /// <param name="parent">Parent widget the main window will be placed in. Null.</param>
    public: explicit MainWindow(QWidget *parent = nullptr);
    /// <summary>Frees all resources owned by the main window</summary>
    public: ~MainWindow();

    /// <summary>Binds the main window to the specified service container</summary>
    /// <param name="servicesRoot">Service container the main window will use</param>
    /// <remarks>
    ///   The root service container contains all the services that perform the actual work
    ///   of the application (while this dialog just displays the current state reported by
    ///   the services or calls into the relevant services when the user clicks a button to
    ///   enact a change).
    /// </remarks>
    public: void BindToServicesRoot(
      const std::shared_ptr<Services::ServicesRoot> &servicesRoot
      //const std::shared_ptr<Nuclex::Support::Services::ServiceProvider> &instanceFactory
    );

    /// <summary>Updates the list of managed processes from the database</summary>
    private: void updateProcessList();

    /// <summary>
    ///   Let's the user browse for the frames folder when the button is clicked
    /// </summary>
    private: void browseClicked();

    private: void markBcFrameClicked();
    private: void markCdFrameClicked();
    private: void markProgressiveFrameClicked();

    /// <summary>Quits the application when the user clicks the quit button</summary>
    private: void quitClicked();

    /// <summary>The user interface arrangement generated from the .ui file</summary>
    private: std::unique_ptr<Ui::MainWindow> ui;
    /// <summary>Root service container doing the application's work</summary>
    private: std::shared_ptr<Services::ServicesRoot> servicesRoot;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_MAINWINDOW_H