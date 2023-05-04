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

  /// <summary>Converts a pixel in a floating point format into an integer format</summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format used by the input pixel</typeparam>
  /// <typeparam name="TTargetPixelFormat">Pixel format used by the output pixel</typeparam>
  /// <param name="pixel">Pixel that will be converted to the output pixel format</param>
  /// <returns>A pixel in the output pixel format that's equivalent to the input pixel</returns>
  template<
    PixelFormat TSourcePixelFormat, PixelFormat TTargetPixelFormat,
    typename std::enable_if_t<
      (TSourcePixelFormat != TTargetPixelFormat) &&
      (IsFloatFormat<TSourcePixelFormat>) &&
      (!IsFloatFormat<TTargetPixelFormat>)
    > * = nullptr
  >
  NUCLEX_PIXELS_ALWAYS_INLINE void ConvertPixel(
    const PixelTypeFromFormat<TSourcePixelFormat> *sourcePixel,
    PixelTypeFromFormat<TTargetPixelFormat> *targetPixel
  ) {
    (void)sourcePixel; // MSVC fantasizes a constellation where no channels exist
    (void)targetPixel; // then warns that these two parameters aren't used...

    // TODO: Float->Int converter currently does not support signed integer formats
    static_assert(
      !IsSignedFormat<TTargetPixelFormat> && u8"Signed pixel formats not implemented yet"
    );

    if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel1::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TTargetPixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TTargetPixelFormat>,
        0, //PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex,
        PixelFormatDescription<TTargetPixelFormat>::Channel1::BitCount
      >;
      const constexpr int ShiftOffset = (
        -static_cast<int>(PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex)
      );

      // Since we perform the conversion in the lowest bits, and since garbage
      // bits above the number can't happen, we don't need to bit mask the result!
      *targetPixel = BitShift<ShiftOffset>(
        static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          (*source) * static_cast<SourceFloatType>(channelBitMask)
        )
      );
    }

    if constexpr(NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TTargetPixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TTargetPixelFormat>,
        0, //PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex,
        PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
      >;
      const constexpr int ShiftOffset = (
        -static_cast<int>(PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex)
      );

      // Since we perform the conversion in the lowest bits, and since garbage
      // bits above the number can't happen, we don't need to bit mask the result!
      if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
        *targetPixel |= BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      } else {
        *targetPixel = BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      }
    }

    if constexpr(NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TTargetPixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TTargetPixelFormat>,
        0, //PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex,
        PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
      >;
      const constexpr int ShiftOffset = (
        -static_cast<int>(PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex)
      );

      // Since we perform the conversion in the lowest bits, and since garbage
      // bits above the number can't happen, we don't need to bit mask the result!
      if constexpr(
        NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>
      ) {
        *targetPixel |= BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      } else {
        *targetPixel = BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      }
    }

    if constexpr(NeedConvertChannel4<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex % 8) == 0) &&
        u8"Source floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount
      > SourceFloatType;
      const SourceFloatType *source = reinterpret_cast<const SourceFloatType *>(
        reinterpret_cast<const std::uint8_t *>(sourcePixel) +
        (PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TTargetPixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TTargetPixelFormat>,
        0, //PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex,
        PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
      >;
      const constexpr int ShiftOffset = (
        -static_cast<int>(PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex)
      );

      // Since we perform the conversion in the lowest bits, and since garbage
      // bits above the number can't happen, we don't need to bit mask the result!
      if constexpr(
        NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>
      ) {
        *targetPixel |= BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      } else {
        *targetPixel = BitShift<ShiftOffset>(
          static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
            (*source) * static_cast<SourceFloatType>(channelBitMask)
          )
        );
      }
    } else if constexpr(HasAlphaChannel<TTargetPixelFormat>) {
      if constexpr(
        NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>
      ) {
        *targetPixel |= BitMask<
          PixelTypeFromFormat<TTargetPixelFormat>,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
        >;
      } else {
        *targetPixel = BitMask<
          PixelTypeFromFormat<TTargetPixelFormat>,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
        >;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
