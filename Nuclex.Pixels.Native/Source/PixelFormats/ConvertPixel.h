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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_CONVERTPIXEL_H
#define NUCLEX_PIXELS_PIXELFORMATS_CONVERTPIXEL_H

#include "Nuclex/Pixels/Config.h"
#include "ChannelHelpers.h"

#include "UnsignedBitAdjust.h"
#include "PixelFormatDescription.h"

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>No-op conversion if both pixel formats are identical</summary>
  /// <typeparam name="SourceFormat">Pixel format used by the input pixel</typeparam>
  /// <typeparam name="TargetFormat">Pixel format used by the output pixel</typeparam>
  /// <param name="pixel">Pixel that will be converted to the output pixel format</param>
  /// <returns>A pixel in the output pixel format that's equivalent to the input pixel</returns>
  template<
    PixelFormat TSourcePixelFormat, PixelFormat TTargetPixelFormat,
    typename std::enable_if_t<
      (TSourcePixelFormat == TTargetPixelFormat)
    > * = nullptr
  >
  NUCLEX_PIXELS_ALWAYS_INLINE void ConvertPixel(
    const PixelTypeFromFormat<TSourcePixelFormat> *sourcePixel,
    PixelTypeFromFormat<TTargetPixelFormat> *targetPixel
  ) {
    *targetPixel = *sourcePixel;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#include "ConvertPixel.IntToInt.inl"
#include "ConvertPixel.IntToFloat.inl"
#include "ConvertPixel.FloatToInt.inl"
#include "ConvertPixel.FloatToFloat.inl"

#endif // NUCLEX_PIXELS_PIXELFORMATS_CONVERTPIXEL_H
