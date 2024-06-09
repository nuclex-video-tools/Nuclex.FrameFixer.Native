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

#include "./RenderProgressDialog.h"
#include "ui_RenderProgressDialog.h"

#include "./Services/ServicesRoot.h" // for ServicesRoot

#include <QCloseEvent> // for QCloseEvent

namespace Nuclex::FrameFixer {

  // ------------------------------------------------------------------------------------------- //

  RenderProgressDialog::RenderProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::RenderProgressDialog>()),
    servicesRoot() {

    this->ui->setupUi(this);
/*
    connect(
      this->ui->browseTargetDirectoryButton, &QPushButton::clicked,
      this, &RenderDialog::browseTargetDirectoryClicked
    );
*/
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

  void RenderProgressDialog::accept() {
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer
