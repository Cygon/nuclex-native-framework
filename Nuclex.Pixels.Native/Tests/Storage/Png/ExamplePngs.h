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

#ifndef NUCLEX_PIXELS_STORAGE_PNG_EXAMPLEPNGS_H
#define NUCLEX_PIXELS_STORAGE_PNG_EXAMPLEPNGS_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 24 bit RGB PNG file containing some colors and gradients</summary>
  /// <remarks>
  ///   This should be useful to check color channel layout and image geometry.
  ///   The top line should be black-red-green-blue-white.
  //    The leftmost row should be black-20% grey-40% grey-60% grey-80%grey.
  /// </remarks>
  extern const std::uint8_t colorTestPng[486];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 48 bit RGB PNG file containing some colors and gradients</summary>
  extern const std::uint8_t depth48TestPng[1190];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny PNG file encoding a 1x1 pixel black square</summary>
  extern const std::uint8_t verySmallPng[73];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 17x7 PNG with colored letters and an alpha channel</summary>
  extern const std::uint8_t testPng[161];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A damaged PNG file with inconsistent internal data</summary>
  extern const std::uint8_t corruptPng[161];

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif // NUCLEX_PIXELS_STORAGE_PNG_EXAMPLEPNGS_H
