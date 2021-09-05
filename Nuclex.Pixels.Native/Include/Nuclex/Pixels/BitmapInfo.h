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

#ifndef NUCLEX_PIXELS_BITMAPINFO_H
#define NUCLEX_PIXELS_BITMAPINFO_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Informations about a bitmap</summary>
  /// <remarks>
  ///   This structure is returned if you ask a codec to provide informations about
  ///   a bitmap before actually loading it.
  /// </remarks>
  struct NUCLEX_PIXELS_TYPE BitmapInfo {

    /// <summary>Width of the bitmap in pixels</summary>
    public: std::size_t Width;

    /// <summary>Height of the bitmap in pixels</summary>
    public: std::size_t Height;

    /// <summary>Memory layout and contents of pixels in the bitmap</summary>
    public: enum PixelFormat PixelFormat;

    // TODO: Expose color space (sRGB, Linear, other?)
    //public: enum ColorSpace ColorSpace;

    /// <summary>Memory usage of the bitmap when loaded into memory</summary>
    /// <remarks>
    ///   This is as accurate as possible and assumes one Bitmap instance with the pixel
    ///   data in it. Each additional Bitmap instance will add sizeof(BitmapMemory) +
    ///   sizeof(std::intptr_t) bytes, but these may just be stack memory depending on
    ///   how you use this library.
    /// </remarks>
    public: std::size_t MemoryUsage;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_BITMAPINFO_H
