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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATDESCRIPTION_H
#define NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATDESCRIPTION_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"

#include "Nuclex/Pixels/UInt128.h" // for UInt128

//#include <cstdint> // for std::uint8_t, std::uint16_t, std::uint32_t and std::uint64_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the encoded color and bits used to store it</summary>
  /// <typeparam name="channelIndex">
  ///   Index of the color channel, 0:R/Y/X 1:G/U/Y 2:B/V/Z 3:A
  /// </typeparam>
  /// <typeparam name="lowestBitIndex">
  ///   Index of the first bit storing the channel (in the integral, endian-dependent type)
  /// </typeparam>
  /// <typeparam name="bitCount">Number of bits the channel is wide</typeparam>
  /// <remarks>
  ///   <para>
  ///     The pixel format converter functions by accessing the pixel as an integer value
  ///     and shifting bits around. Thus, little endian and big endian will have an effect
  ///     and pixel formats that defined a memory order (nearly all of them) will have to
  ///     be described here differently depending on endianness.
  ///   </para>
  ///   <para>
  ///     For formats where color channels are split in half by endianness (R5_G6_B5 or
  ///     R10_G10_B10_A2 currently), the PixelFormatConverter must be specialized because
  ///     such cases cannot be dealt with by the generic pixel format converter in a manner
  ///     that would work on different endian systems.
  ///   </para>
  ///   <para>
  ///     Overall, big endian support is of no importance for game development. However,
  ///     since there are array formats (memory-ordered) and packed formats (endian ordered)
  ///     and the little endian architecture has the bits all in reverse, we'll end up
  ///     having to deal with this at great length anyway.
  ///   </para>
  /// </remarks>
  template<std::size_t channelIndex, std::size_t lowestBitIndex, std::size_t bitCount>
  struct ColorChannelDescription {

    /// <summary>Index of the color channel, 0:R/Y/X 1:G/U/Y 2:B/V/Z</summary>
    public: static constexpr std::size_t ChannelIndex = channelIndex;

    /// <summary>Index of lowest bit occupied by the color channel</summary>
    public: static constexpr std::size_t LowestBitIndex = lowestBitIndex;

    /// <summary>Number of bits the channel is wide</summary>
    public: static constexpr std::size_t BitCount = bitCount;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Data type stored in a pixel format's color channels</summary>
  enum class PixelFormatDataType {

    /// <summary>Channels are stored as unsigned integers</summary>
    UnsignedInteger,

    /// <summary>Channels are stored as symmetric signed integers</summary>
    /// <remarks>
    ///   To have identical quantization for positive and negative values, rather than
    ///   go from -128 to +127 or from -32768 to +32767, a signed integer channel goes
    ///   from -127 to +127 or from -32767 ot +32767. The extra value shouldn't be used
    ///   but by established convention is equal to the closest valid value within range.
    /// </remarks>
    SignedInteger,

    /// <summary>Channels are stored as floating point values</summary>
    /// <remarks>
    ///   Depending on the color channel's bit count, this is either a half or a float
    ///   or potentially even a double precision float.
    /// </remarks>
    FloatingPoint,

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>What kind of endian flip to perform when reading/writing pixels</summary>
  enum class EndianFlipOperation {

    /// <summary>No endian flipping is required on the current platform</summary>
    None,

    /// <summary>The endianness of each color channels needs to be flipped individually</summary>
    /// <remarks>
    ///   This mode should be set when the pixel format has a _NativeXX or _FlippedXX
    ///   postfix that is smaller than the data type of the pixel. For example,
    ///   A16_R16_G16_B16_Flipped16 indicates that each 16 bit value is byte-flipped,
    ///   this mode would be appropriate whereas A8_R8_G8_B8_Flipped32 would mean
    ///   the whole pixel must be flipped and <see cref="FlipWholePixel" /> should be used.
    /// </remarks>
    FlipEachChannel,

    /// <summary>The data of the whole pixel should be flipped</summary>
    /// <remarks>
    ///   If this endian flip operation is used, the data type storing the pixel is
    ///   flipped altogether. Usually only found in smaller pixel types where it is convenient
    ///   to treat the whole pixel as, for example, a 32 but integer.
    /// </remarks>
    FlipWholePixel

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a pixel format</summary>
  /// <remarks>
  ///   <para>
  ///     Each pixel format requires the following members
  ///   </para>
  ///   <para>
  ///     DataFormat - Set to a member of the PixelFormatDataType enumeration which
  ///     indicates whether the pixel formats stores unsigned integers, signed integers
  ///     or floating point values.
  ///   </para>
  ///   <para>
  ///     PixelType - Typedef for an integral type wide enough to store a pixel. Will be
  ///     used for all internal calculations. In special cases, this may be larger than
  ///     the actual pixel (see 24-bit RGB formats), in which case a ReadPixel() and
  ///     WritePixel() method needs to be provided.
  ///   </para>
  ///   <para>
  ///     Channel1, Channel2, Channel3, Channel4 - Up to 4 color channels in order
  ///     R, G, B, A / Y, U, V, A / X, Y, Z, W. I decided on this design because there
  ///     are not pixel formats with more than 4 channels out in the wild. Occupied
  ///     channels need to be a typedef of a ColorChannelDescription specialization,
  ///     unused channels need to be a typedef to std::nullptr_t.
  ///   </para>
  /// </remarks>
  template<PixelFormat pixelFormat>
  struct PixelFormatDescription;

  // ------------------------------------------------------------------------------------------- //

#if 0
  #if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    #define NUCLEX_PIXELS_NEED_FLIP_IF_LITTLE_ENDIAN true
    #define NUCLEX_PIXELS_NEED_FLIP_IF_BIG_ENDIAN false
  #else
    #define NUCLEX_PIXELS_NEED_FLIP_IF_LITTLE_ENDIAN false
    #define NUCLEX_PIXELS_NEED_FLIP_IF_BIG_ENDIAN true
  #endif
#endif

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#include "PixelFormatDescription.R.inl"
#include "PixelFormatDescription.A.inl"
#include "PixelFormatDescription.RG.inl"
#include "PixelFormatDescription.RA.inl"
#include "PixelFormatDescription.RGB.inl"
#include "PixelFormatDescription.BGR.inl"
#include "PixelFormatDescription.ARGB.inl"
#include "PixelFormatDescription.BGRA.inl"
#include "PixelFormatDescription.RGBA.inl"
#include "PixelFormatDescription.ABGR.inl"

#endif // NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATDESCRIPTION_H
