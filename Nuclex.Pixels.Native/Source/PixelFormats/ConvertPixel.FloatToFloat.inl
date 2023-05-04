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
#error This file is intended to be included through ConvertPixel.h
#endif

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a pixel between two different floating point-based formats</summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format used by the input pixel</typeparam>
  /// <typeparam name="TTargetPixelFormat">Pixel format used by the output pixel</typeparam>
  /// <param name="pixel">Pixel that will be converted to the output pixel format</param>
  /// <returns>A pixel in the output pixel format that's equivalent to the input pixel</returns>
  template<
    PixelFormat TSourcePixelFormat, PixelFormat TTargetPixelFormat,
    typename std::enable_if_t<
      (TSourcePixelFormat != TTargetPixelFormat) &&
      (IsFloatFormat<TSourcePixelFormat>) &&
      (IsFloatFormat<TTargetPixelFormat>)
    > * = nullptr
  >
  NUCLEX_PIXELS_ALWAYS_INLINE void ConvertPixel(
    const PixelTypeFromFormat<TSourcePixelFormat> *sourcePixel,
    PixelTypeFromFormat<TTargetPixelFormat> *targetPixel
  ) {
    (void)sourcePixel; // MSVC fantasizes a constellation where no channels exist
    (void)targetPixel; // then warns that these two parameters aren't used...

    // Convert the red color channel, if one is present
    if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel1::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex / 8)
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel1::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex / 8)
      );

      *target = static_cast<TargetFloatType>(*source);
    }

    // Convert the green color channel, if one is present
    if constexpr(NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex / 8)
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex / 8)
      );

      *target = static_cast<TargetFloatType>(*source);
    }

    // Convert the blue color channel, if one is present
    if constexpr(NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex / 8)
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex / 8)
      );

      *target = static_cast<TargetFloatType>(*source);
    }

    // Convert the alpha channel, if one is present
    if constexpr(NeedConvertChannel4<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex / 8)
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex / 8)
      );

      *target = static_cast<TargetFloatType>(*source);
    } else if constexpr(HasAlphaChannel<TTargetPixelFormat>) {
      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex / 8)
      );
      *target = static_cast<TargetFloatType>(1);
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
