#pragma region CPL License
/*
Nuclex CriuEncoder
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

#ifndef NUCLEX_TELECIDE_PREVIEWDEINTERLACER_H
#define NUCLEX_TELECIDE_PREVIEWDEINTERLACER_H

#include "Nuclex/Telecide/Config.h"

#include <QImage>

namespace Nuclex::Telecide {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Cheapest possible deinterlacer that simply interpolates a field</summary>
  class PreviewDeinterlacer {

    public: static void Deinterlace(
      QImage *previousImage, QImage &image, bool topField = true
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Telecide

#endif // NUCLEX_TELECIDE_PREVIEWDEINTERLACER_H
