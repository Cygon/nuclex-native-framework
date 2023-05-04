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

  /// <summary>Converts between two different integer-based pixel formats</summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format used by the input pixel</typeparam>
  /// <typeparam name="TTargetPixelFormat">Pixel format used by the output pixel</typeparam>
  /// <param name="pixel">Pixel that will be converted to the output pixel format</param>
  /// <returns>A pixel in the output pixel format that's equivalent to the input pixel</returns>
  template<
    PixelFormat TSourcePixelFormat, PixelFormat TTargetPixelFormat,
    typename std::enable_if_t<
      (TSourcePixelFormat != TTargetPixelFormat) &&
      (!IsFloatFormat<TSourcePixelFormat>) &&
      (!IsFloatFormat<TTargetPixelFormat>)
    > * = nullptr
  >
  NUCLEX_PIXELS_ALWAYS_INLINE void ConvertPixel(
    const PixelTypeFromFormat<TSourcePixelFormat> *sourcePixel,
    PixelTypeFromFormat<TTargetPixelFormat> *targetPixel
  ) {
    typedef LargerPixelType<TSourcePixelFormat, TTargetPixelFormat> IntermediatePixelType;

    static_assert(
      (!IsSignedFormat<TSourcePixelFormat>) && (!IsSignedFormat<TTargetPixelFormat>) &&
      u8"Signed pixel formats not implemented yet, results will be wrong"
    );

    (void)sourcePixel; // MSVC fantasizes a constellation where no channels exist
    (void)targetPixel; // then warns that these two parameters aren't used...

    // Convert the red channel and assign it to the target pixel
    if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
      *targetPixel = static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
        PixelFormats::UnsignedBitAdjuster<
          PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel1::BitCount,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel1::BitCount
        >::template Adjust<
          PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel1::LowestBitIndex,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex
        >(
          static_cast<IntermediatePixelType>(*sourcePixel)
        ) & BitMask<
          IntermediatePixelType,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel1::LowestBitIndex,
          PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel1::BitCount
        >
      );
    }

    // Convert the green channel and assign it to the target pixel
    if constexpr(NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>) {
      if constexpr(NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat>) {
        *targetPixel |= static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
          >
        );
      } else {
        *targetPixel = static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel2::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel2::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel2::BitCount
          >
        );
      }
    }

    // Convert the blue channel and assign it to the target pixel
    if constexpr(NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>) {
      if constexpr(
        NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat>
      ) {
        *targetPixel |= static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
          >
        );
      } else {
        *targetPixel = static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel3::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel3::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel3::BitCount
          >
        );
      }
    }

    // Convert the alpha channel and assign it to the target pixel
    if constexpr(NeedConvertChannel4<TSourcePixelFormat, TTargetPixelFormat>) {
      if constexpr(
        NeedConvertChannel1<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel2<TSourcePixelFormat, TTargetPixelFormat> ||
        NeedConvertChannel3<TSourcePixelFormat, TTargetPixelFormat>
      ) {
        *targetPixel |= static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
          >
        );
      } else {
        *targetPixel = static_cast<PixelTypeFromFormat<TTargetPixelFormat>>(
          PixelFormats::UnsignedBitAdjuster<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel4::BitCount,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
          >::template Adjust<
            PixelFormats::PixelFormatDescription<TSourcePixelFormat>::Channel4::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex
          >(
            static_cast<IntermediatePixelType>(*sourcePixel)
          ) & BitMask<
            IntermediatePixelType,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::LowestBitIndex,
            PixelFormats::PixelFormatDescription<TTargetPixelFormat>::Channel4::BitCount
          >
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
