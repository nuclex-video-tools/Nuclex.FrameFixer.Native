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

#include "Nuclex/FrameFixer/QZoomableGraphicsView.h"

#include <QWheelEvent>

// no namespace

  // ------------------------------------------------------------------------------------------- //

  void QZoomableGraphicsView::wheelEvent(QWheelEvent *wheelEvent) {
    if(wheelEvent->modifiers() == Qt::KeyboardModifier::NoModifier) {

      // Zoom in or out when the Control key is pressed
      if(wheelEvent->delta() > 0) {
        scale(1.1, 1.1); // Zoom in
      } else {
        scale(0.9, 0.9); // Zoom out
      }

      wheelEvent->accept();

    } else {

      // Pass the event to the base class if Control key is not pressed
      QGraphicsView::wheelEvent(wheelEvent);

    }    
  }

  // ------------------------------------------------------------------------------------------- //

// no namespace
