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

You should have received a copy of  the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATQUERY_H
#define NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATQUERY_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"

#include <optional> // for std::optional

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides informations about pixel formats</summary>
  /// <remarks>
  ///   <para>
  ///     These methods provide additional informations (on top of the inlined methods
  ///     provided in the PixelFormat.h file, <see cref="CountBitsPerPixel" />,
  ///     <see cref="CountRequiredBytes" /> and <see cref="CountChannels" />) which
  ///     you can use for checking and - if necessary - for runtime-conversion of
  ///     unsupported pixel formats into a supported but variable pixel format.
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE PixelFormatQuery {

    /// <summary>Checks whether the specified pixel format has a red channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the specified pixel format has a red channel</returns>
    public: NUCLEX_PIXELS_API static bool HasRedChannel(PixelFormat pixelFormat);
    /// <summary>Checks whether the specified pixel format has a green channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the specified pixel format has a green channel</returns>
    public: NUCLEX_PIXELS_API static bool HasGreenChannel(PixelFormat pixelFormat);
    /// <summary>Checks whether the specified pixel format has a blue channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the specified pixel format has a blue channel</returns>
    public: NUCLEX_PIXELS_API static bool HasBlueChannel(PixelFormat pixelFormat);
    /// <summary>Checks whether the specified pixel format has an alpha channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the specified pixel format has an alpha channel</returns>
    public: NUCLEX_PIXELS_API static bool HasAlphaChannel(PixelFormat pixelFormat);

    /// <summary>Reports whether the pixel format uses of signed integers or floats</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>
    ///   True if the pixel format is either a floating point format or an integer format
    ///   that uses unsigned integers
    /// </returns>
    public: NUCLEX_PIXELS_API static bool IsSignedFormat(PixelFormat pixelFormat);
    /// <summary>Reports whether the pixel format uses floating point channels</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the pixel format stores channels in floating point format</returns>
    public: NUCLEX_PIXELS_API static bool IsFloatFormat(PixelFormat pixelFormat);

    /// <summary>
    ///   Checks whether the pixel format has channels that differ in their number of bits
    /// </summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if the pixel format has channels with different widths</returns>
    /// <remarks>
    ///   This will return false for formats such as A16-R16-G16-B16 or R8-G8 and it
    ///   will return true for formats such as A2-R10-G10-B10 or R5-G6-B5.
    /// </remarks>
    public: NUCLEX_PIXELS_API static bool HasDifferentlySizedChannels(PixelFormat pixelFormat);

    /// <summary>
    ///   Checks whether all channels in the pixel format start on a byte boundary
    /// </summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>True if all channels in the pixel format start in a byte boundary</returns>
    /// <remarks>
    ///   This will return true only if each color channel's width is a multiple of 8
    ///   (and each channel begins at a bit index that's a multiple of 8).
    /// </remarks>
    public: NUCLEX_PIXELS_API static bool AreAllChannelsByteAligned(PixelFormat pixelFormat);

    /// <summary>Reports whether the pixel format uses the opposite endianness</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>
    ///   <para>
    ///     True if the pixel format stores data in the opposite endianness from
    ///     the current platform. If different endianness can be covered by simply stating
    ///     the positions of color channels differently, that is preferred to flipping.
    ///   </para>
    ///   <para>
    ///     For example, the pixel format A8-R8-G8-B8 is 32 bits wide. On little endian
    ///     platforms, it will report that the alpha channel is in bits 0-7. On big endian
    ///     platforms, <see cref="RequiresEndianFlip" /> will still be false, but it will
    ///     report that the alpha channel is in bits 24-31.
    ///   </para>
    ///   <para>
    ///     Formats with a _Flipped16, _Flipped32 or higher postfix always need to be
    ///     flipped (in steps as indicated by the postfix, i.e. in _Flipped16 flip every
    ///     two bytes, but don't flip the whole channel).
    ///   </para>
    /// </returns>
    public: NUCLEX_PIXELS_API static bool RequiresEndianFlip(PixelFormat pixelFormat);

    /// <summary>Looks up the index of the lowest bit in the red channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The index of the lowest red bit in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> GetLowestRedBitIndex(
      PixelFormat pixelFormat
    );
    /// <summary>Looks up the index of the lowest bit in the green  channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The index of the lowest green bit in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> GetLowestGreenBitIndex(
      PixelFormat pixelFormat
    );
    /// <summary>Looks up the index of the lowest bit in the blue channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The index of the lowest blue bit in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> GetLowestBlueBitIndex(
      PixelFormat pixelFormat
    );
    /// <summary>Looks up the index of the lowest bit in the alpha channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The index of the lowest alpha bit in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> GetLowestAlphaBitIndex(
      PixelFormat pixelFormat
    );

    /// <summary>Counts the number of bits used for the red channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The number of red bits used in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> CountRedBits(
      PixelFormat pixelFormat
    );
    /// <summary>Counts the number of bits used for the green channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The number of green bits used in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> CountGreenBits(
      PixelFormat pixelFormat
    );
    /// <summary>Counts the number of bits used for the blue channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The number of blue bits used in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> CountBlueBits(
      PixelFormat pixelFormat
    );
    /// <summary>Counts the number of bits used for the alpha channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The number of alpha bits used in the pixel format</returns>
    public: NUCLEX_PIXELS_API static std::optional<std::size_t> CountAlphaBits(
      PixelFormat pixelFormat
    );

    /// <summary>Counts the number of bits of the pixel format's widest channel</summary>
    /// <param name="pixelFormat">Pixel format that will be checked</param>
    /// <returns>The number of bits used by the pixel format's widest channel</returns>
    public: NUCLEX_PIXELS_API static std::size_t CountWidestChannelBits(
      PixelFormat PixelFormat
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATQUERY_H
