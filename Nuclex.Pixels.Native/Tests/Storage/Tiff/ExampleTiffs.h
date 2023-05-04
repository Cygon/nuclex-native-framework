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

#ifndef NUCLEX_PIXELS_STORAGE_TIFF_EXAMPLETIFFS_H
#define NUCLEX_PIXELS_STORAGE_TIFF_EXAMPLETIFFS_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny TIFF file encoding a 1x1 pixel black square</summary>
  extern const std::uint8_t verySmallTiff[202];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 17x7 TIFF with colored letters and an alpha channel</summary>
  extern const std::uint8_t testTiff[1259];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The colored test TIFF saved using deflate (ZLib) compression</summary>
  extern const std::uint8_t deflateCompressedTiff[389];

  // ------------------------------------------------------------------------------------------- //
  
  /// <summary>The colored test TIFF saved using DCT (libjpeg) compression</summary>
  extern const std::uint8_t dctCompressedTiff[1081];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The colored test TIFF saved using Pixar compression</summary>
  extern const std::uint8_t pixarCompressedTiff[427];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A damaged TIFF file with inconsistent internal data</summary>
  extern const std::uint8_t corruptTiff[202];

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif // NUCLEX_PIXELS_STORAGE_TIFF_EXAMPLETIFFS_H
