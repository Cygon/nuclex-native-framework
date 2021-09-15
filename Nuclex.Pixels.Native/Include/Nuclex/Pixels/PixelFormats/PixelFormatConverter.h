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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATCONVERTER_H
#define NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATCONVERTER_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Bitmap.h"

#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts between different pixel formats</summary>
  /// <remarks>
  ///   <para>
  ///     This class converts pixels between different pixel formats as non-destructively
  ///     as possible. However, if the target pixel format has fewer bits or is lacking
  ///     a channel that is present in the source pixel format, information will be lost.
  ///   </para>
  ///   <para>
  ///     The general rules followed by the pixel format converter are the following:
  ///   </para>
  ///   <para>
  ///     <list type="bullet">
  ///       <item>
  ///         <term>Channel present -&gt; missing</term>
  ///         <description>
  ///           If a color channel is present in the source pixel format but not in the target
  ///           pixel format, its data will simply not be present in the target pixel.
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Channel missing -&gt; present</term>
  ///         <description>
  ///           Should the target pixel format have color channels that are not present in
  ///           the source pixel format, their contents will be zero for R, G and B channels
  ///           and the maximum value in case of the alpha channel.
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Grayscale images</term>
  ///         <description>
  ///           Grayscale pixel format use the red color channel in this library. This means
  ///           that when loading a grayscale image and converting it to an RGB image, its
  ///           data will sit in the red color channel and the resulting image will have
  ///           various intensities of red rather than showing as grayscale.
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Color channel widening</term>
  ///         <description>
  ///           If the target pixel format has wider color channels than the source pixel
  ///           format, accurate widening will be performed (using the &quot;repeat bits&quot;
  ///           method which is fast and 100% accurate).
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Color channel narrowing</term>
  ///         <description>
  ///           If the target pixel format has narrower color channels than the source pixel
  ///           format, then narrowing will be performed by rounding. This is the most accurate
  ///           way of narrowing a color channel without converting to floating point.
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Unsigned -&gt; signed conversion</term>
  ///         <description>
  ///           When converting unsigned color channels to signed ones, the midpoint + 1 of
  ///           the unsigned color channel becomes zero, any value below it is negative and
  ///           any value above it is positive. Plus one because signed formats have one unit
  ///           of reach further in negative range, though this is ignored because signed
  ///           pixel formats are symmetric (i.e. -128 is same as -127 for signed 8 bits).
  ///         </description>
  ///       </item>
  ///       <item>
  ///         <term>Signed -&gt; unsigned conversion</term>
  ///         <description>
  ///           The lowest possible value of the signed color channel becomes the new zero
  ///           and the unsigned color channel counts from there. This allows lossless
  ///           conversion back to an unsigned channel.
  ///         </description>
  ///       </item>
  ///     </list>
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE PixelFormatConverter {

    /// <summary>Converts an entire row of pixels to another pixel format</summary>
    /// <param name="sourcePixels">Starting address to read source pixels from</param>
    /// <param name="targetPixels">
    ///   Starting address at which converted pixels will be written
    /// </param>
    /// <param name="pixelCount">Number of pixels that will be converted</param>
    public: typedef void ConvertRowFunction(
      const std::uint8_t *sourcePixels, std::uint8_t *targetPixels, std::size_t pixelCount
    ); // TODO: Use void * instead of std::uint8_t * for the public API, like BitmapMemory?

    /// <summary>
    ///   Returns a converter function that converts a row of pixels from the specified
    ///   source pixel format to the specified target pixel format
    /// </summary>
    /// <param name="sourcePixelFormat">Pixel format of the source pixels</param>
    /// <param name="targetPixelFormat">Pixel format of the target pixels</param>
    /// <returns>
    ///   A converter function that will perform the conversion on a series of pixels
    /// </param>
    /// <remarks>
    ///   This is useful if you need to perform on-the-fly pixel format conversion,
    ///   for example when implementing a <see cref="Nuclex.Pixels.Storage.BitmapCodec" />
    ///   that needs to convert from a file's pixel format to an existing bitmap's
    ///   pixel format. Many reference libraries for popular image file formats offer
    ///   means to load an image line-by-line or at least in blocks.
    /// </remarks>
    public: NUCLEX_PIXELS_API static ConvertRowFunction *GetRowConverter(
      PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat
    );

    /// <summary>
    ///   Copies the contents of one bitmap into another bitmap while converting the pixel format
    /// </summary>
    /// <param name="source">Bitmap that will be copied to the target bitmap</param>
    /// <param name="target">
    ///   Bitmap that will be overwritten with the contents of the source bitmap
    /// </param>
    public: NUCLEX_PIXELS_API static void Convert(
      const Bitmap &source, Bitmap &target
    );

    /// <summary>Converts the whole bitmap to a different pixel format</summary>
    /// <param name="source">Bitmap that will be converted to a different pixel format</param>
    /// <param name="newPixelFormat">Pixel format the bitmap will be converted to</param>
    /// <returns>
    ///   A new bitmap with the contents of the input bitmap on the new pixel format
    /// </returns>
    public: NUCLEX_PIXELS_API static Bitmap Convert(
      const Bitmap &source, PixelFormat newPixelFormat
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATCONVERTER_H
