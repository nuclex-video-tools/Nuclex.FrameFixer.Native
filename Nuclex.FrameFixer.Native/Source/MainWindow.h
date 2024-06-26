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

#ifndef NUCLEX_FRAMEFIXER_MAINWINDOW_H
#define NUCLEX_FRAMEFIXER_MAINWINDOW_H

#include "Nuclex/FrameFixer/Config.h"

#include "./Model/DeinterlaceMode.h"
#include "./Model/FrameAction.h"
#include "./Algorithm/Deinterlacing/Deinterlacer.h"

#include <QMainWindow> // for QMainWindow
#include <QItemSelection> // for QItemSelection
#include <QMutex> // for QMutex

#include <memory> // for std::unique_ptr
#include <optional> // for std::optional
#include <map> // for std::pair

namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  class ServicesRoot;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  class Deinterlacer;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer::Algorithm::Interpolation{

  // ------------------------------------------------------------------------------------------- //

  class FrameInterpolator;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  class FrameThumbnailItemModel;
  class FrameThumbnailPaintDelegate;
  class DeinterlacerItemModel;
  class Movie;
  class Frame;

  // ------------------------------------------------------------------------------------------- //

}

namespace Ui {

  // ------------------------------------------------------------------------------------------- //

  class MainWindow;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

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
    ///   Lets the user browse for the frames folder when the button is clicked
    /// </summary>
    private: void browseClicked();

    /// <summary>
    ///   Detelecines the entire movie using the currently assigned frame types
    /// </summary>
    private: void renderClicked();

    /// <summary>Marks the current frame as a non-interlaced frame</param>
    private: void markProgressiveClicked() { toggleFrameType(FrameAction::Progressive); }
    /// <summary>Marks the current frame as a B-C frame in the telecine rhythm</param>
    private: void markTopFieldFirstClicked()  { toggleFrameType(FrameAction::TopFieldFirst); }
    /// <summary>Marks the current frame as a C-D frame in the telecine rhythm</param>
    private: void markBottomFieldFirstClicked()  { toggleFrameType(FrameAction::BottomFieldFirst); }
    /// <summary>Marks the current frame as a C frame wiht only its top field</param>
    private: void markTopFieldOnlyClicked() { toggleFrameType(FrameAction::TopFieldOnly); }
    /// <summary>Marks the current frame as a C frame with only its bottom field</param>
    private: void markBottomFieldOnlyClicked() { toggleFrameType(FrameAction::BottomFieldOnly); }

    /// <summary>Marks the current frame to be discarded</param>
    private: void markDiscardClicked() { toggleFrameType(FrameAction::Discard); }
    /// <summary>Marks the current frame for averaging</param>
    private: void markAverageClicked() { toggleFrameType(FrameAction::Average); }
    /// <summary>Marks the current frame for duplication</param>
    private: void markDuplicateClicked() { toggleFrameType(FrameAction::Duplicate); }
    /// <summary>Marks the current frame for triplication</param>
    private: void markTriplicateClicked() { toggleFrameType(FrameAction::Triplicate); }

    /// <summary>Marks the current frame for deblending</param>
    private: void markDeblendClicked() { toggleFrameType(FrameAction::Deblend); }
    /// <summary>Marks the current frame to repeat the most recent interpolation</param>
    private: void markReplacedClicked();
    /// <summary>Marks the current frame to repeat the most recent interpolation</param>
    private: void markReplaceWithInterpolatedClicked();
    /// <summary>Marks the current frame for neighbor interpolation</param>
    private: void markAppendInterpolatedClicked() {
      setupInterpolation(1);
      toggleFrameType(FrameAction::Interpolate);
    }

    /// <summary>Flood-fills the provisional frame types for previewing</param>
    private: void showStatisticsClicked();

    /// <summary>Toggles the current frame between the specified type and none</summary>
    /// <param name="frameType">Frame type to apply or remove from the current frame</param>
    private: void toggleFrameType(FrameAction frameType);

    /// <summary>Sets the current frame's interpolation source frames</summary>
    /// <param name="distance">Distance of the interpolation source frames</param>
    private: void setupInterpolation(int distance);

    /// <summary>Updates the preview image when the swap fields option is toggled</summary>
    /// <param name="checked">Whether the swap fields option is on or off</param>
    private: void swapFieldsOptionToggled(bool checked);

    /// <summary>Updates the preview image when the enhance option is toggled</summary>
    /// <param name="checked">Whether the enhance option is on or off</param>
    private: void enhanceOptionToggled(bool checked);

    /// <summary>Updates the preview image when the preview option is toggled</summary>
    /// <param name="checked">Whether the preview option is on or off</param>
    private: void previewOptionToggled(bool checked);

    /// <summary>Updates the displayed frame when another thumbnail is selected</summary>
    /// <param name="selected">List of frames that have been newly selected</param>
    /// <param name="deselected">List of frames that are no longer selected</param>
    private: void selectedThumbnailChanged(
      const QItemSelection &selected, const QItemSelection &deselected
    );

    private: void selectedDeinterlacerChanged(int selectedIndex);

    private: std::size_t getSelectedFrameIndex() const;

    private: std::size_t getLastTaggedFrameIndex() const;

    /// <summary>Displays the preview for the specified frame in the main view</summary>
    /// <param name="frame">Frame that will be shown in the main view</param>
    private: void displayFrameInView(const Frame &frame);

    private: void exportDetelecinedFrames(
      const std::string &directory,
      const std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> &deinterlacer,
      const std::shared_ptr<Algorithm::Interpolation::FrameInterpolator> &interpolator,
      std::optional<std::pair<std::size_t, std::size_t>> inputFrameRange = (
        std::optional<std::pair<std::size_t, std::size_t>>()
      ),
      std::optional<std::pair<std::size_t, std::size_t>> outputFrameRange = (
        std::optional<std::pair<std::size_t, std::size_t>>()
      )
    );

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
    /// <summary>Item model that manages the deinterlacers</summary>
    private: std::unique_ptr<DeinterlacerItemModel> deinterlacerItemModel;
    /// <summary>Root service container doing the application's work</summary>
    private: std::shared_ptr<Services::ServicesRoot> servicesRoot;
    /// <summary>The movie whose frames are currently loaded for processing</summary>
    private: std::shared_ptr<Movie> currentMovie;
    /// <summary>The currently selected deinterlacer</summary>
    private: std::shared_ptr<Algorithm::Deinterlacing::Deinterlacer> deinterlacer;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_MAINWINDOW_H
