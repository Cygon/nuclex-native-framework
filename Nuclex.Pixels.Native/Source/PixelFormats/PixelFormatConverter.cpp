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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"

#include "ConvertPixel.h"
#include "OnPixelFormat.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts one row of pixels from one pixel format to another</summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format of the source pixels</typeparam>
  /// <typeparam name="TTargetPixelFormat">
  ///   Pixel format the pixels will be converted to
  /// </typeparam>
  /// <param name="sourcePixels">Starting address to read source pixels from</param>
  /// <param name="targetPixels">
  ///   Starting address at which converted pixels will be written
  /// </param>
  /// <param name="pixelCount">Number of pixels that will be converted</param>
  template<
    Nuclex::Pixels::PixelFormat TSourcePixelFormat,
    Nuclex::Pixels::PixelFormat TTargetPixelFormat
  >
  void ConvertRow(
    const std::uint8_t *sourcePixels, std::uint8_t *targetPixels, std::size_t pixelCount
  ) {
    typedef Nuclex::Pixels::PixelFormats::PixelTypeFromFormat<TSourcePixelFormat> SourceType;
    typedef Nuclex::Pixels::PixelFormats::PixelTypeFromFormat<TTargetPixelFormat> TargetType;

    constexpr std::size_t bytesPerSourcePixel = (
      Nuclex::Pixels::CountBitsPerPixel(TSourcePixelFormat) / 8
    );
    constexpr std::size_t bytesPerTargetPixel = (
      Nuclex::Pixels::CountBitsPerPixel(TTargetPixelFormat) / 8
    );

    while(pixelCount > 0) {
      Nuclex::Pixels::PixelFormats::ConvertPixel<TSourcePixelFormat, TTargetPixelFormat>(
        reinterpret_cast<const SourceType *>(sourcePixels),
        reinterpret_cast<TargetType *>(targetPixels)
      );

      sourcePixels += bytesPerSourcePixel;
      targetPixels += bytesPerTargetPixel;
      --pixelCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Retrieves a full row pixel format conversion function from the specified source
  ///   pixel format (to a target specified at runtime via parameter)
  /// </summary>
  /// <typeparam name="TSourcePixelFormat">Pixel format of the source pixels</typeparam>
  template<Nuclex::Pixels::PixelFormat TSourcePixelFormat>
  class GetConvertRowFunction {

    /// <summary>
    ///   Retrieves a full row pixel format conversion function from the source pixel
    ///   format (specified to the outer template class) to the specified target pixel format
    /// </summary>
    /// <typeparam name="TTargetPixelFormat">
    ///   Pixel format the returned conversion function will convert to
    /// </typeparam>
    template<Nuclex::Pixels::PixelFormat TTargetPixelFormat>
    class GetConvertRowFunctionWithKnownSourcePixelFormat {

      /// <summary>
      ///   Returns a function that converts from the source to the target pixel format
      /// </summary>
      /// <returns>The pixel format conversion function</returns>
      public: Nuclex::Pixels::PixelFormats::ConvertRowFunction *operator()() const {
        return &ConvertRow<TSourcePixelFormat, TTargetPixelFormat>;
      }

    };

    /// <summary>
    ///   Returns a function that converts from the source to the specified target pixel format
    /// </summary>
    /// <param name="targetPixelFormat">
    ///   Pixel format to which the returned function will be converting
    /// </param>
    /// <returns>The pixel format conversion function</returns>
    public: Nuclex::Pixels::PixelFormats::ConvertRowFunction *operator()(
      Nuclex::Pixels::PixelFormat targetPixelFormat
    ) const {
      return Nuclex::Pixels::PixelFormats::OnPixelFormat<
        GetConvertRowFunctionWithKnownSourcePixelFormat,
        Nuclex::Pixels::PixelFormats::ConvertRowFunction *
      >(targetPixelFormat);
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  ConvertRowFunction *GetPixelFormatConverter(
    PixelFormat sourcePixelFormat, PixelFormat targetPixelFormat
  ) {
    return Nuclex::Pixels::PixelFormats::OnPixelFormat<
      GetConvertRowFunction, ConvertRowFunction *
    >(sourcePixelFormat, targetPixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
