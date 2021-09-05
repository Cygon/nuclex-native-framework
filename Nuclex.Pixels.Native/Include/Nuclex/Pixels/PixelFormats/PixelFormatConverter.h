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

#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an entire row of pixels to another pixel format</summary>
  /// <param name="sourcePixels">Starting address to read source pixels from</param>
  /// <param name="targetPixels">
  ///   Starting address at which converted pixels will be written
  /// </param>
  /// <param name="pixelCount">Number of pixels that will be converted</param>
  typedef void ConvertRowFunction(
    const std::uint8_t *sourcePixels, std::uint8_t *targetPixels, std::size_t pixelCount
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Returns a converter function that converts a row of pixels from the specified
  ///   source pixel format to the specified target pixel format
  /// </summary>
  /// <param name="sourcePixelFormat">Pixel format of the source pixels</param>
  /// <param name="targetPixelFormat">Pixel format of the target pixels</param>
  /// <returns>
  ///   A converter function that will perform the conversion of a series of pixels
  /// </param>
  /// <remarks>
  ///   Usually, you
  /// </remarks>
  NUCLEX_PIXELS_API ConvertRowFunction *GetPixelFormatConverter(
    PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat
  );

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATCONVERTER_H
