#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_PIXELS_PIXELFORMAT_H
#define NUCLEX_PIXELS_PIXELFORMAT_H

#include "Nuclex/Pixels/Size.h"

#include <cstddef>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  //#define SMALLEST_UNIT(size) (size << 24)
  //#define BITS_PER_PIXEL(count) (count << 16)
  //#define ID(number) (number)

  /// <summary>Data formats that can be used to describe a pixel</summary>
  /// <remarks>
  ///   <para>
  ///     All pixel formats specify the in-memory ordering of the color channels (anything
  ///     else would be insane, considering non-byte-sized color channels in R5-G6-B5).
  ///     Thus, R8-G8-B8-A8 is the same in memory, no matter if big endian or little endian,
  ///     only the code required to access individual color channels will change.
  ///   </para>
  ///   <para>
  ///     The enum values are defined as follows:
  ///   </para>
  ///   <code>
  ///     0sssssss pppppppp nnnnnnnn nnnnnnnn
  ///   </code>
  ///   <para>
  ///     Where 's' indicates the size of the smallest unit addressable in the pixel format
  ///     in bytes. For a 32 bit RGBA format, this would be 4 (if a write to the texture
  ///     was off by two bytes, R would become B, G would become A and so on). Compressed
  ///     pixel formats may have larger chunks - DXT5 for example would only be addressable
  ///     in units of 128 bits / 16 bytes.
  ///   </para>
  ///   <para>
  ///     The next byte, 'p' contains the number of bits per pixel. It is useful for
  ///     calculating the amount of memory required to hold an image of size x by y.
  ///   </para>
  ///   <para>
  ///     In the remaining two bytes, tagged as 'n', a unique id of each pixel format
  ///     will be stored.
  ///   </para>
  /// </remarks>
  enum class PixelFormat {

    /// <summary>8 bit single color stored in the red channel</summary>
    R8_Unsigned = (1 << 24) | (8 << 16) | 1,

    /// <summary>16 bit with two colors, each in an 8 bit channel</summary>
    /// <remarks>Useful for tangent-space normal maps with calculated up-vector</remarks>
    R8_G8_Signed = (2 << 24) | (16 << 16) | 2,

    /// <summary>16 bit with three colors<summary>
    R5_G6_B5_Unsigned = (2 << 24) | (16 << 16) | 3,

    /// <summary>24 bit color using 8 bits for each channel</summary>
    R8_G8_B8_Unsigned = (3 << 24) | (24 << 16) | 4,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>This is probably the most widely used and compatible texture format</remarks>
    R8_G8_B8_A8_Unsigned = (4 << 24) | (32 << 16) | 5,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    R8_G8_B8_A8_Signed = (4 << 24) | (32 << 16) | 6,

    /// <summary>Compressed RGB turning every 64 bits into 16 pixels</summary>
    BC1_Compressed = (8 << 24) | (4 << 16) | 1001,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC2_Compressed = (16 << 24) | (8 << 16) | 1002,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC3_Compressed = (16 << 24) | (8 << 16) | 1003,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC4_Compressed = (16 << 24) | (8 << 16) | 1004,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC5_Compressed = (16 << 24) | (8 << 16) | 1005,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC6_Compressed = (16 << 24) | (8 << 16) | 1006,

    /// <summary>Compressed RGB with alpha turning every 128 bits into 16 pixels</summary>
    BC7_Compressed = (16 << 24) | (8 << 16) | 1007,

    /// <summary>8 bit alpha channel only</summary>
    Modern_A8_Unsigned = (1 << 24) | (8 << 16) | 2001,

    /// <summary>16 bit red channel only</summary>
    Modern_R16_Unsigned = (2 << 24) | (16 << 16) | 2002,

    /// <summary>32 bit with two 16 bit color channels</summary>
    Modern_R16_G16_Unsigned = (4 << 24) | (32 << 16) | 2003,

    /// <summary>32 bit with two half-precision floating point color channels</summary>
    Modern_R16_G16_Float = (4 << 24) | (32 << 16) | 2004,

    /// <summary>32 bit floating point red channel only</summary>
    Modern_R32_Float = (4 << 24) | (32 << 16) | 2005,

    /// <summary>64 bit color with alpha using 16 bit color channels</summary>
    Modern_R16_G16_B16_A16_Unsigned = (8 << 24) | (64 << 16) | 2006,

    /// <summary>64 bit color with alpha as half-precision floating point channels</summary>
    Modern_R16_G16_B16_A16_Float = (8 << 24) | (64 << 16) | 2007,

    /// <summary>128 bit color with alpha as floating point channels</summary>
    Offline_R32_G32_B32_A32_Float = (16 << 24) | (128 << 16) | 3001

  }; // enum class PixelFormat

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest bits used per pixel in the specified pixel format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose bits per pixel will be determined</param>
  /// <returns>The bits per pixel in the specified pixel format</returns>
  constexpr inline std::size_t CountBitsPerPixel(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) >> 16) & 0xFF;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest number of bits that can be modified in the given format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose unit size will be determined</param>
  /// <returns>The smallest changeable number of bytes in the specified pixel format</returns>
  constexpr inline std::size_t CountBytesPerBlock(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) >> 24);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the number of bytes required to store the specified number of pixels
  /// </summary>
  /// <param name="pixelCount">
  ///   Number of pixels for which the memory required will be calculated
  /// </param>
  /// <param name="pixelFormat">Pixel format for which the size will be determined</param>
  /// <returns>The size of a single pixel in the specified pixel format</returns>
  constexpr inline std::size_t CountRequiredBytes(
    std::size_t pixelCount, PixelFormat pixelFormat
  ) {
    return ((CountBitsPerPixel(pixelFormat) * pixelCount) + 7) / 8; // Always round up
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the size of the smallest interdepenent pixel block</summary>
  /// <param name="pixelFormat">
  ///   Pixel format whose smallest interdependent block size will be returned
  /// </param>
  /// <returns>The size fo the smallest interdependent block in the pixel format</returns>
  inline Size GetBlockSize(PixelFormat pixelFormat) {
    switch(pixelFormat) {
      case PixelFormat::BC1_Compressed:
      case PixelFormat::BC2_Compressed:
      case PixelFormat::BC3_Compressed:
      case PixelFormat::BC4_Compressed:
      case PixelFormat::BC5_Compressed:
      case PixelFormat::BC6_Compressed:
      case PixelFormat::BC7_Compressed: {
        return Size(4, 4);
      }
      default: {
        return Size(1, 1);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_PIXELFORMAT_H
