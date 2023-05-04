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

  /// <summary>Converts a pixel in an integer format into a floating point format</summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format used by the input pixel</typeparam>
  /// <typeparam name="TTargetPixelFormat">Pixel format used by the output pixel</typeparam>
  /// <param name="pixel">Pixel that will be converted to the output pixel format</param>
  /// <returns>A pixel in the output pixel format that's equivalent to the input pixel</returns>
  template<
    PixelFormat TSourcePixelFormat, PixelFormat TTargetPixelFormat,
    typename std::enable_if_t<
      (TSourcePixelFormat != TTargetPixelFormat) &&
      (!IsFloatFormat<TSourcePixelFormat>) &&
      (IsFloatFormat<TTargetPixelFormat>)
    > * = nullptr
  >
  NUCLEX_PIXELS_ALWAYS_INLINE void ConvertPixel(
    const PixelTypeFromFormat<TSourcePixelFormat> *sourcePixel,
    PixelTypeFromFormat<TTargetPixelFormat> *targetPixel
  ) {
    (void)sourcePixel; // MSVC fantasizes a constellation where no channels exist
    (void)targetPixel; // then warns that these two parameters aren't used...

    // TODO: Int->Float converter currently does not support signed integer formats
    static_assert(
      !IsSignedFormat<TSourcePixelFormat> && u8"Signed pixel formats not implemented yet"
    );

    if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel1::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TSourcePixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TSourcePixelFormat>,
        PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex,
        PixelFormatDescription<TSourcePixelFormat>::Channel1::BitCount
      >;

#if defined(_MSC_VER) // MSVC invents warnings here that don't apply.
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
#endif

      // Floating point accuracy depends on how large the value is. At 1<<12 accuracy
      // it reaches an ULP of 0.5 (meaning it can only represent numbers in steps of 0.5),
      // at 1<<13 the ULP is 1. We can only use the more efficient method up to here.
      if constexpr(channelBitMask < 4194304) { // TODO: Use different value for half float
        *target = (
          static_cast<TargetFloatType>(*sourcePixel & channelBitMask) /
          static_cast<TargetFloatType>(channelBitMask)
        );
      } else { // Shift right, then divide by channel max if value too large
        *target = (
          static_cast<TargetFloatType>(
            BitShift<PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex>(
              *sourcePixel & channelBitMask
            )
          ) / (
            static_cast<TargetFloatType>(
              BitMask<
                std::uint_fast32_t,
                0,
                PixelFormatDescription<TSourcePixelFormat>::Channel1::BitCount
              >
            )
          )
        );
      }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    if constexpr(NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TSourcePixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TSourcePixelFormat>,
        PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex,
        PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount
      >;

#if defined(_MSC_VER) // MSVC invents warnings here that don't apply.
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
#endif

      // Floating point accuracy depends on how large the value is. At 1<<12 accuracy
      // reaches an ULP of 0.5 (meaning it canonly represent numbers in steps of 0.5),
      // at 1<<13 the ULP is 1. We can only use the more efficient method up to here.
      if constexpr(channelBitMask < 4194304) { // TODO: Use different value for half float
        *target = (
          static_cast<TargetFloatType>(*sourcePixel & channelBitMask) /
          static_cast<TargetFloatType>(channelBitMask)
        );
      } else { // Shift right, then divide by channel max if value too large
        *target = (
          static_cast<TargetFloatType>(
            BitShift<PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex>(
              *sourcePixel & channelBitMask
            )
          ) / (
            static_cast<TargetFloatType>(
              BitMask<
                std::uint_fast32_t,
                0,
                PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount
              >
            )
          )
        );
      }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    // Optimization idea (hard to implement)
    // If some channels have bits above 1<<12, perhaps the higher channels can be shifted
    // down to save one bit shift in total. Gain is small or nonexistent, though, instructions
    // become dependent and shifting is generally a 1 clock or half-clock instruction.

    if constexpr(NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TSourcePixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TSourcePixelFormat>,
        PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex,
        PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount
      >;

#if defined(_MSC_VER) // MSVC invents warnings here that don't apply.
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
#endif

      // Floating point accuracy depends on how large the value is. At 1<<12 accuracy
      // reaches an ULP of 0.5 (meaning it canonly represent numbers in steps of 0.5),
      // at 1<<13 the ULP is 1. We can only use the more efficient method up to here.
      if constexpr(channelBitMask < 4194304) { // TODO: Use different value for half float
        *target = (
          static_cast<TargetFloatType>(*sourcePixel & channelBitMask) /
          static_cast<TargetFloatType>(channelBitMask)
        );
      } else { // Shift right, then divide by channel max if value too large
        *target = (
          static_cast<TargetFloatType>(
            BitShift<PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex>(
              *sourcePixel & channelBitMask
            )
          ) / (
            static_cast<TargetFloatType>(
              BitMask<
                std::uint_fast32_t,
                0,
                PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount
              >
            )
          )
        );
      }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    if constexpr(NeedConvertChannel4<TSourcePixelFormat, TTargetPixelFormat>) {
      static_assert(
        ((PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex % 8) == 0) &&
        u8"Target floating point channel bits start at a byte boundary"
      );

      typedef ChannelFloatType<
        PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
      > TargetFloatType;
      TargetFloatType *target = reinterpret_cast<TargetFloatType *>(
        reinterpret_cast<std::uint8_t *>(targetPixel) +
        (PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex / 8)
      );

      const constexpr PixelTypeFromFormat<TSourcePixelFormat> channelBitMask = BitMask<
        PixelTypeFromFormat<TSourcePixelFormat>,
        PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex,
        PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount
      >;

#if defined(_MSC_VER) // MSVC invents warnings here that don't apply.
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
#endif

      // Floating point accuracy depends on how large the value is. At 1<<12 accuracy
      // reaches an ULP of 0.5 (meaning it canonly represent numbers in steps of 0.5),
      // at 1<<13 the ULP is 1. We can only use the more efficient method up to here.
      if constexpr(channelBitMask < 4194304) { // TODO: Use different value for half float
        *target = (
          static_cast<TargetFloatType>(*sourcePixel & channelBitMask) /
          static_cast<TargetFloatType>(channelBitMask)
        );
      } else { // Shift right, then divide by channel max if value too large
        *target = (
          static_cast<TargetFloatType>(
            BitShift<PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex>(
              *sourcePixel & channelBitMask
            )
          ) / (
            static_cast<TargetFloatType>(
              BitMask<
                std::uint_fast32_t,
                0,
                PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount
              >
            )
          )
        );
      }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
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
