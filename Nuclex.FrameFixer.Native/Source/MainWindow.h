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

#include "Nuclex/Telecide/Config.h"

#include <QMainWindow> // for QMainWindow
#include <QItemSelection> // for QItemSelection
#include <QMutex> // for QMutex

#include <memory> // for std::unique_ptr

namespace Nuclex::Telecide::Services {

  // ------------------------------------------------------------------------------------------- //

  class ServicesRoot;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  class FrameThumbnailItemModel;
  class FrameThumbnailPaintDelegate;
  class Movie;
  class Frame;

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

    /// <summary>Sets up callbacks for the relevant notifications of UI widgets</summary>
    private: void connectUiSignals();

    /// <summary>Loads the currently selected movie's frames</summary>
    private: void ingestMovieFrames();

    /// <summary>
    ///   Let's the user browse for the frames folder when the button is clicked
    /// </summary>
    private: void browseClicked();

    /// <summary>Marks the current frame as a B-C frame in the telecine rhythm</param>
    private: void markBcFrameClicked();
    /// <summary>Marks the current frame as a C-D frame in the telecine rhythm</param>
    private: void markCdFrameClicked();
    /// <summary>Marks the current frame as a non-interlaced frame</param>
    private: void markProgressiveFrameClicked();

    /// <summary>Updates the displayed frame when another thumbnail is selected</summary>
    /// <param name="selected">List of frames that have been newly selected</param>
    /// <param name="deselected">List of frames that are no longer selected</param>
    private: void selectedThumbnailChanged(
      const QItemSelection &selected, const QItemSelection &deselected
    );

    private: std::size_t getSelectedFrameIndex() const;

    /// <summary>Displays the preview for the specified frame in the main view</summary>
    /// <param name="frame">Frame that will be shown in the main view</param>
    private: void displayFrameInView(const Frame &frame);

    private: void stopAnalysisThread();
    private: void startAnaylsisThread();
    private: void analyzeMovieFramesInThread();
    private: static void callAnalyzeMovieFramesInThread(MainWindow *self) {
      self->analyzeMovieFramesInThread();
    }

    /// <summary>Saves the status of all frames when the user clicks on save</summary>
    private: void saveClicked();
    /// <summary>Quits the application when the user clicks the quit button</summary>
    private: void quitClicked();

    /// <summary>The user interface arrangement generated from the .ui file</summary>
    private: std::unique_ptr<Ui::MainWindow> ui;
    /// <summary>Item model that manages the thumbnails</summary>
    private: std::unique_ptr<FrameThumbnailItemModel> thumbnailItemModel;
    /// <summary>Delegate that draws the thumbnails with decoration</summary>
    private: std::unique_ptr<FrameThumbnailPaintDelegate> thumbnailPaintDelegate;
    /// <summary>Root service container doing the application's work</summary>
    private: std::shared_ptr<Services::ServicesRoot> servicesRoot;
    /// <summary>The movie whose frames are currently loaded for processing</summary>
    private: std::shared_ptr<Movie> currentMovie;
    /// <summary>Thread that analyzes the combiness of frames</summary>
    private: std::unique_ptr<QThread> analysisThread;
    /// <summary>Mutex controlling access to the analysis thread</summary>
    private: std::unique_ptr<QMutex> analysisThreadMutex;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_MAINWINDOW_H
