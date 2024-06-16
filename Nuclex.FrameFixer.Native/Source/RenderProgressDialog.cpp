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

#include "./RenderProgressDialog.h"
#include "ui_RenderProgressDialog.h"

#include "./Services/ServicesRoot.h" // for ServicesRoot
#include "./Renderer.h"

#include <QTimer>

#include <Nuclex/Support/Errors/CanceledError.h>
#include <Nuclex/Support/Text/LexicalAppend.h>

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  RenderProgressDialog::RenderProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::RenderProgressDialog>()),
    servicesRoot(),
    checkTimer(),
    renderThread(),
    renderer(),
    movie(),
    directory(),
    cancelTrigger(),
    totalFrameCount(std::size_t(-1)) {

    this->ui->setupUi(this);

    connect(
      this->ui->cancelButton, &QPushButton::clicked,
      this, &RenderProgressDialog::CancelClicked
    );
  }

  // ------------------------------------------------------------------------------------------- //

  RenderProgressDialog::~RenderProgressDialog() {}

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::BindToServicesRoot(
    const std::shared_ptr<Services::ServicesRoot> &servicesRoot
  ) {
    this->servicesRoot = servicesRoot;
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::SetRenderer(const std::shared_ptr<Renderer> &renderer) {
    this->renderer = renderer;
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::Start(
    const std::shared_ptr<Movie> &movie, const std::string &directory
  ) {
    this->movie = movie;
    this->directory = directory;
    this->cancelTrigger = Nuclex::Platform::Tasks::CancellationTrigger::Create();

    this->renderThread.reset(
      QThread::create(&RenderProgressDialog::renderInBackgroundThread, this)
    );

    this->totalFrameCount = this->renderer->GetTotalFrameCount(movie);

    std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
    {
      this->checkTimer = std::make_unique<QTimer>(this);
      connect(
        this->checkTimer.get(), &QTimer::timeout,
        this, &RenderProgressDialog::checkForCompletionAndUpdateUi
      );
      this->checkTimer->start(500);
    }
    this->renderThread->start();

    //QTimer::singleShot(0, this, &RenderProgressDialog::renderInBackgroundThread);
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::CancelClicked() {
    if(static_cast<bool>(this->renderThread)) {
      if(static_cast<bool>(this->cancelTrigger)) {
        this->cancelTrigger->Cancel();
        this->cancelTrigger.reset();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::checkForCompletionAndUpdateUi() {
    bool isStillRunning = false;
    if(static_cast<bool>(this->renderThread)) {
      if(this->renderThread->isRunning()) {
        isStillRunning = true;
      }
    }

    if(static_cast<bool>(this->renderer)) {
      std::size_t completedFrameCount = this->renderer->GetCompletedFrameCount();
      std::string status = "Frame ";
      Nuclex::Support::Text::lexical_append(status, completedFrameCount);
      if(this->totalFrameCount != std::size_t(-1)) {
        status.append(u8" out of ");
        Nuclex::Support::Text::lexical_append(status, this->totalFrameCount);
      }
      
      this->ui->currentFrameLabel->setText(QString::fromStdString(status));

      if(this->totalFrameCount != std::size_t(-1)) {
        if(this->ui->progressBar->minimum() != 0.0) {
          this->ui->progressBar->setMinimum(0.0);
        }
        if(this->ui->progressBar->maximum() != 100.0) {
          this->ui->progressBar->setMaximum(100.0);
        }
        this->ui->progressBar->setValue(
          static_cast<double>(completedFrameCount) / static_cast<double>(totalFrameCount) * 100.0
        );
      }

    }

    if(!isStillRunning) {
      this->renderThread.reset();
      close();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RenderProgressDialog::renderInBackgroundThread() {
    QThread::msleep(250); // just for fun
    try {
      this->renderer->Render(this->movie, this->directory, this->cancelTrigger->GetWatcher());
    }
    catch(const Nuclex::Support::Errors::CanceledError &) {}
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
