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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_CHANNELHELPERS_H
#define NUCLEX_PIXELS_PIXELFORMATS_CHANNELHELPERS_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Half.h"
#include "Nuclex/Pixels/UInt128.h"

#include "PixelFormatDescription.h"
#include "BitAdjust.h"
#include "BitMask.h"

#include <type_traits> // for std::conditional

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Integer type that can hold a pixel of the specified pixel format</summary>
  /// <typeparam name="pixelFormat">Pixel format for which to provide the type</typeparam>
  template<PixelFormat pixelFormat>
  using PixelTypeFromFormat = typename PixelFormatDescription<pixelFormat>::PixelType;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the pixel format is signed (allows negative color values)</summary>
  /// <typeparam name="pixelFormat">Pixel format that will be checked</typeparam>
  template<PixelFormat pixelFormat>
  constexpr bool IsSignedFormat = (
    (PixelFormatDescription<pixelFormat>::DataType == PixelFormatDataType::SignedInteger) ||
    (PixelFormatDescription<pixelFormat>::DataType == PixelFormatDataType::FloatingPoint)
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the pixel format uses floating point channels</summary>
  /// <typeparam name="pixelFormat">Pixel format that will be checked</typeparam>
  template<PixelFormat pixelFormat>
  constexpr bool IsFloatFormat = (
    (PixelFormatDescription<pixelFormat>::DataType == PixelFormatDataType::FloatingPoint)
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Floating point type a channel is using based on its bit count</summary>
  /// <typeparam name="bitCount">Number of bits allocated to this channel</typeparam>
  template<std::size_t bitCount>
  using ChannelFloatType = typename std::conditional<
    (bitCount >= 64),
    double,
    typename std::conditional<(bitCount >= 32), float, Nuclex::Pixels::Half>::type
  >::type;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the red color channel needs to be converted</summary>
  /// <typeparam name="sourcePixelFormat">Pixel format of the source pixel</typeparam>
  /// <typeparam name="targetPixelFormat">Pixel format of the target pixel</typeparam>
  template<PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat>
  constexpr bool NeedConvertChannel1 = (
    !std::is_same<
      typename PixelFormatDescription<sourcePixelFormat>::Channel1, std::nullptr_t
    >::value && !std::is_same<
      typename PixelFormatDescription<targetPixelFormat>::Channel1, std::nullptr_t
    >::value
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the green color channel needs to be converted</summary>
  /// <typeparam name="sourcePixelFormat">Pixel format of the source pixel</typeparam>
  /// <typeparam name="targetPixelFormat">Pixel format of the target pixel</typeparam>
  template<PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat>
  constexpr bool NeedConvertChannel2 = (
    !std::is_same<
      typename PixelFormatDescription<sourcePixelFormat>::Channel2, std::nullptr_t
    >::value && !std::is_same<
      typename PixelFormatDescription<targetPixelFormat>::Channel2, std::nullptr_t
    >::value
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the blue color channel needs to be converted</summary>
  /// <typeparam name="sourcePixelFormat">Pixel format of the source pixel</typeparam>
  /// <typeparam name="targetPixelFormat">Pixel format of the target pixel</typeparam>
  template<PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat>
  constexpr bool NeedConvertChannel3 = (
    !std::is_same<
      typename PixelFormatDescription<sourcePixelFormat>::Channel3, std::nullptr_t
    >::value && !std::is_same<
      typename PixelFormatDescription<targetPixelFormat>::Channel3, std::nullptr_t
    >::value
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the alpha channel needs to be converted</summary>
  /// <typeparam name="sourcePixelFormat">Pixel format of the source pixel</typeparam>
  /// <typeparam name="targetPixelFormat">Pixel format of the target pixel</typeparam>
  template<PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat>
  constexpr bool NeedConvertChannel4 = (
    !std::is_same<
      typename PixelFormatDescription<sourcePixelFormat>::Channel4, std::nullptr_t
    >::value && !std::is_same<
      typename PixelFormatDescription<targetPixelFormat>::Channel4, std::nullptr_t
    >::value
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Whether the specified pixel format contains an alpha channel</summary>
  /// <typeparam name="pixelFormat">
  ///   Pixel format that will be checked for an alpha channel
  /// </typeparam>
  template<PixelFormat pixelFormat>
  constexpr bool HasAlphaChannel = !std::is_same<
    typename PixelFormatDescription<pixelFormat>::Channel4, std::nullptr_t
  >::value;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The larger of the source and target pixel types</summary>
  /// <typeparam name="LeftFormat">First format to find the larger pixel type in</typeparam>
  /// <typeparam name="RightFormat">Second format to find the larger pixel type in</typeparam>
  template<PixelFormat LeftFormat, PixelFormat RightFormat>
  using LargerPixelType = typename std::conditional<
    (
      sizeof(typename PixelFormatDescription<LeftFormat>::PixelType) >=
      sizeof(typename PixelFormatDescription<RightFormat>::PixelType)
    ),
    typename PixelFormatDescription<LeftFormat>::PixelType,
    typename PixelFormatDescription<RightFormat>::PixelType
  >::type;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_CHANNELHELPERS_H
