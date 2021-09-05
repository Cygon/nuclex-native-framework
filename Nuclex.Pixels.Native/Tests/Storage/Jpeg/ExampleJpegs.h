#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_PIXELS_STORAGE_PNG_EXAMPLEJPEGS_H
#define NUCLEX_PIXELS_STORAGE_PNG_EXAMPLEJPEGS_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny JPEG file encoding a 1x1 pixel white square</summary>
  extern const std::uint8_t verySmallJpeg[283];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 17x7 JPEG with colored letters</summary>
  extern const std::uint8_t testJpeg[460];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A damaged JPEG file that will fail to decode</summary>
  extern const std::uint8_t corruptJpeg[283];

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // NUCLEX_PIXELS_STORAGE_JPEG_EXAMPLEJPEGS_H
