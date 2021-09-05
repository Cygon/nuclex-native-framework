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

#ifndef NUCLEX_PIXELS_BITMAPMEMORY_H
#define NUCLEX_PIXELS_BITMAPMEMORY_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the complete memory layout of a bitmap</summary>
  /// <remarks>
  ///   This structure contains all informations needed to calculate the address of any
  ///   pixel in a bitmap and interpret it according to its pixel format. Nearly any
  ///   bitmap-based graphics API that allows raw-memory access to bitmaps will provide
  ///   you with these informations, so it is the universal key to access bitmaps.
  /// </remarks>
  struct NUCLEX_PIXELS_TYPE BitmapMemory {

    /// <summary>Width of the bitmap in pixels</summary>
    public: std::size_t Width;

    /// <summary>Height of the bitmap in pixels</summary>
    public: std::size_t Height;

    /// <summary>Offset in bytes to go from one line to the next</summary>
    /// <remarks>
    ///   <para>
    ///     Bitmaps can use a larger memory area than is needed for the pixels they contain.
    ///     This may be due to certain bitmap implementations aligning a bitmap's lines in
    ///     memory or because the bitmap is part of a larger image (this library's sub-bitmaps
    ///     or texture atlases).
    ///   </para>
    ///   <para>
    ///     Because of that it is not safe to go forward by (width x bytes per pixel) bytes
    ///     to move to the next line in an image. The stride is the number of bytes that you
    ///     need to skip to go exactly one pixel down.
    ///   </para>
    ///   <para>
    ///     Strides can be negative to put an image upside-down. In that case, the start address
    ///     should be the first pixel of the bitmap's last line and the stride should be
    ///     the number of bytes to advance (or rather, since it's a negative number, retreat) to
    ///     go up by exactly one pixel.
    ///   </para>
    /// </remarks>
    public: int Stride;

    /// <summary>Describes the memory layout and contents of a pixel</summary>
    /// <remarks>
    ///   <para>
    ///     The most common way for an image to store the color of a pixel is to store its
    ///     red, green and blue components as individual bytes. That way, 3 bytes are used for
    ///     each pixel and you can easily address the individual color channels by accessing
    ///     these bytes.
    ///   </para>
    ///   <para>
    ///     However, the world is not so simple. Sometimes, size trumps simplicity and pixels
    ///     store their color channels in just two bytes (for example, 5 bits for red,
    ///     6 bits for green, 5 bits for blue). And sometimes you have more or less channels,
    ///     a grayscale bitmap only needs a single channel. A texture for a video game might
    ///     have an additional alpha channel that contains the opacity of a pixel.
    ///   </para>
    ///   <para>
    ///     The pixel format thus defines how large a single pixel is in memory how the pixel
    ///     can be deciphered. Some pixel formats are compressed and do not allow individual
    ///     pixels to be addressed at all, in which case the bits per pixel is an average.
    ///   </para>
    /// </remarks>
    public: enum PixelFormat PixelFormat;

    /// <summary>Memory area storing the bitmap's pixels</summary>
    public: void *Pixels;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_BITMAPMEMORY_H
