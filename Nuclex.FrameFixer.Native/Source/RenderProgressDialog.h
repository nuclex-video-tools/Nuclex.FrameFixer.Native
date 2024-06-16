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

#ifndef NUCLEX_FRAMEFIXER_RENDERPROGRESSDIALOG_H
#define NUCLEX_FRAMEFIXER_RENDERPROGRESSDIALOG_H

#include "Nuclex/FrameFixer/Config.h"

#include <memory> // for std::unique_ptr
#include <optional> // for std::optional
#include <map> // for std::pair

#include <QDialog> // for QDialog
#include <QThread>
#include <QTimer>

#include <Nuclex/Platform/Tasks/CancellationTrigger.h>

namespace Ui {

  // ------------------------------------------------------------------------------------------- //

  class RenderProgressDialog;

  // ------------------------------------------------------------------------------------------- //

}


namespace Nuclex::FrameFixer::Services {

  // ------------------------------------------------------------------------------------------- //

  class ServicesRoot;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  class Renderer;
  class Movie;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dialog that shows a progress bar while frames are being rendered</summary>
  class RenderProgressDialog : public QDialog {
    Q_OBJECT

    /// <summary>Initializes a new render dialog</summary>
    /// <param name="parent">Outer dialog that is displaying this dialog</param>
    public: explicit RenderProgressDialog(QWidget *parent = nullptr);

    /// <summary>Frees all memory used by the dialog</summary>
    public: ~RenderProgressDialog();

    /// <summary>Binds the dialog to the specified service provide</summary>
    /// <param name="servicesRoot">Application service provider to bind to</param>
    public: void BindToServicesRoot(const std::shared_ptr<Services::ServicesRoot> &servicesRoot);

    /// <summary>Selects the renderer that will be used to render in the background</summary>
    /// <param name="renderer">Renderer (already configured) that will be used</param>
    public: void SetRenderer(const std::shared_ptr<Renderer> &renderer);

    /// <summary>Begins the background rendering process</summary>
    /// <param name="movie">Movie that will be processed (deinterlaced) and saved</param>
    /// <param name="directory">Directory in which the processed frames will be saved</param>
    public: void Start(const std::shared_ptr<Movie> &movie, const std::string &directory);

    /// <summary>Cancels the rendering process and closes the dialog</summary>
    public: void CancelClicked();

    /// <summary>Called once per second to check if rendering has finished</summary>
    private: void checkForCompletionAndUpdateUi();

    /// <summary>Called in a background thread to render all the frames</summary>
    private: void renderInBackgroundThread();

    /// <summary>The user interface arrangement generated from the .ui file</summary>
    private: std::unique_ptr<Ui::RenderProgressDialog> ui;

    /// <summary>Service provider the dialog has been bound to, can be null</summary>
    private: std::shared_ptr<Services::ServicesRoot> servicesRoot;

    /// <summary>Timer that updates the ETA and checks for completion</summary>
    private: std::unique_ptr<QTimer> checkTimer;
    /// <summary>Renderer that will be used to render the frames</summary>
    private: std::unique_ptr<QThread> renderThread;
    /// <summary>Renderer that will be used to render the frames</summary>
    private: std::shared_ptr<Renderer> renderer;
    /// <summary>Movies that will be rendered</summary>
    private: std::shared_ptr<Movie> movie;
    /// <summary>Output directory for the rendered movie frames</summary>
    private: std::string directory;
    /// <summary>Allows the background processing thread to be cancelled</summary>
    private: std::shared_ptr<Nuclex::Platform::Tasks::CancellationTrigger> cancelTrigger;
    /// <summary>Total number of frames that will be rendered</summary>
    private: std::size_t totalFrameCount;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer

#endif // NUCLEX_FRAMEFIXER_RENDERPROGRESSDIALOG_H
