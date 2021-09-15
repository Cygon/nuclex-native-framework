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

#include "Nuclex/Pixels/Scaling/BitmapScaler.h"

#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"

// The AVIR headers cause some warnings with Microsoft's compilers about
// if statements whose outcome is static and known at compile time. Nothing
// we can do about that (apart from invasive changes to the AVIR code).
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif
#include "Avir/avir.h"
#include "Avir/lancir.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()

// This is already 100% working, but here are some possible optimization opportunities:
//
//   - The "resizer" is provided with an "avir::fpclass_def<double>" parameter,
//     so does all internal math use floats or doubles anyway? In this case, it might
//     be faster to simply convert any input bitmap to 32 bit float channels rather
//     than letting AVIR convert a second time from our intermediate format to float.
//
//   - Otherwise, we can eliminate more pixel format conversions by just checking
//     whether all channels are byte/word sized and byte/word aligned (in other words,
//     any swizzle is okay since AVIR doesn't distinguish between channel types anyway)
//
//   - The intermediate pixel format selectors involve hardcoded checks, too. This is
//     not ideal since anyone adding or changing a pixel format would have to dig into
//     the library to spot and extend these hardcoded checks. Not critical since not
//     extended the checks only causes performance cost but not nice either.
//

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Checks whether the end of each row in a bitmap is directly followed by the beginning
  ///   of the next row in memory (i.e. it's not a sub-bitmap or uses aligned rows)
  /// </summary>
  /// <param name="memory">Memory of the bitmap that will be checked</param>
  /// <returns>True if the pixels are one long stream with no gaps inbetween</returns>
  bool isContiguous(const Nuclex::Pixels::BitmapMemory &memory) {
    int calculatedStride = static_cast<int>(
      Nuclex::Pixels::CountRequiredBytes(memory.PixelFormat, memory.Width)
    );
    return (calculatedStride == memory.Stride);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines an adequate pixel format to use when passing a bitmap to AVIR for
  ///   resampling using 8 bit unsigned integer math
  /// </summary>
  /// <param name="pixelFormat">
  ///   Pixel format for which an adequate intermediate format will be determined
  /// </param>
  /// <returns>
  ///   A pixel format that doesn't lose information and can be passed to AVIR for resampling
  /// </returns>
  Nuclex::Pixels::PixelFormat getEightBitResamplingFormat(
    Nuclex::Pixels::PixelFormat pixelFormat
  ) {
    using Nuclex::Pixels::PixelFormats::PixelFormatQuery;

    // TODO: Any swizzle is okay so long as all channels are byte-sized and byte-aligned

    std::size_t channelCount = Nuclex::Pixels::CountChannels(pixelFormat);
    switch(channelCount) {
      case 1: {
        if(PixelFormatQuery::HasAlphaChannel(pixelFormat)) {
          return Nuclex::Pixels::PixelFormat::A8_Unsigned;
        } else {
          return Nuclex::Pixels::PixelFormat::R8_Unsigned;
        }
      }
      default: {
        return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines an adequate pixel format to use when passing a bitmap to AVIR for
  ///   resampling using 16 bit unsigned integer math
  /// </summary>
  /// <param name="pixelFormat">
  ///   Pixel format for which an adequate intermediate format will be determined
  /// </param>
  /// <returns>
  ///   A pixel format that doesn't lose information and can be passed to AVIR for resampling
  /// </returns>
  Nuclex::Pixels::PixelFormat getSixteenBitResamplingFormat(
    Nuclex::Pixels::PixelFormat pixelFormat
  ) {
    using Nuclex::Pixels::PixelFormats::PixelFormatQuery;

    // TODO: Any swizzle is okay so long as all channels are word-sized and word-aligned

    std::size_t channelCount = Nuclex::Pixels::CountChannels(pixelFormat);
    switch(channelCount) {
      case 1: {
        if(PixelFormatQuery::HasAlphaChannel(pixelFormat)) {
          return Nuclex::Pixels::PixelFormat::A16_Unsigned_Native16;
        } else {
          return Nuclex::Pixels::PixelFormat::R16_Unsigned_Native16;
        }
      }
      default: {
        return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines an adequate pixel format to use when passing a bitmap to AVIR for
  ///   resampling using floating point math
  /// </summary>
  /// <param name="pixelFormat">
  ///   Pixel format for which an adequate intermediate format will be determined
  /// </param>
  /// <returns>
  ///   A pixel format that doesn't lose information and can be passed to AVIR for resampling
  /// </returns>
  Nuclex::Pixels::PixelFormat getFloatingPointResamplingFormat(
    Nuclex::Pixels::PixelFormat pixelFormat
  ) {
    std::size_t channelCount = Nuclex::Pixels::CountChannels(pixelFormat);
    if(channelCount == 1) {
      if(pixelFormat == Nuclex::Pixels::PixelFormat::A32_Float_Native32) {
        return Nuclex::Pixels::PixelFormat::A32_Float_Native32;
      } else {
        return Nuclex::Pixels::PixelFormat::R32_Float_Native32;
      }
    } else {
      return Nuclex::Pixels::PixelFormat::R32_G32_B32_A32_Float_Native32;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using AVIR with 8 bit color channels</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  /// <param name="resizerParams">Parameters for the AVIR resizing algorithm</param>
  void resampleAvirWithEightBitChannels(
    const Nuclex::Pixels::BitmapMemory &source,
    const Nuclex::Pixels::BitmapMemory &target,
    const avir::CImageResizerParams &resizerParams
  ) {
    assert(isContiguous(source) && u8"Source pixel buffer has no gaps");
    assert(isContiguous(target) && u8"Target pixel buffer has no gaps");

    std::size_t channelCount = Nuclex::Pixels::CountChannels(source.PixelFormat);
    {
      avir::CImageResizer<avir::fpclass_def<double>> imageResizer(8, 8, resizerParams);
      imageResizer.resizeImage<std::uint8_t, std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(source.Pixels),
        static_cast<int>(source.Width), static_cast<int>(source.Height),
        static_cast<int>(channelCount * source.Width),
        reinterpret_cast<std::uint8_t *>(target.Pixels),
        static_cast<int>(target.Width), static_cast<int>(target.Height),
        static_cast<int>(channelCount),
        0.0, // resizing step, 0.0 = calculate automatically
        nullptr // additional variables
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using AVIR with 16 bit color channels</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  /// <param name="resizerParams">Parameters for the AVIR resizing algorithm</param>
  void resampleAvirWithSixteenBitChannels(
    const Nuclex::Pixels::BitmapMemory &source,
    const Nuclex::Pixels::BitmapMemory &target,
    const avir::CImageResizerParams &resizerParams
  ) {
    assert(isContiguous(source) && u8"Source pixel buffer has no gaps");
    assert(isContiguous(target) && u8"Target pixel buffer has no gaps");

    std::size_t channelCount = Nuclex::Pixels::CountChannels(source.PixelFormat);
    {
      avir::CImageResizer<avir::fpclass_def<double>> imageResizer(16, 16, resizerParams);
      imageResizer.resizeImage<std::uint16_t, std::uint16_t>(
        reinterpret_cast<const std::uint16_t *>(source.Pixels),
        static_cast<int>(source.Width), static_cast<int>(source.Height),
        static_cast<int>(channelCount * source.Width),
        reinterpret_cast<std::uint16_t *>(target.Pixels),
        static_cast<int>(target.Width), static_cast<int>(target.Height),
        static_cast<int>(channelCount),
        0.0, // resizing step, 0.0 = calculate automatically
        nullptr // additional variables
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using AVIR with floating point color channels</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  /// <param name="resizerParams">Parameters for the AVIR resizing algorithm</param>
  void resampleAvirWithFloatingPoint(
    const Nuclex::Pixels::BitmapMemory &source,
    const Nuclex::Pixels::BitmapMemory &target,
    const avir::CImageResizerParams &resizerParams
  ) {
    assert(isContiguous(source) && u8"Source pixel buffer has no gaps");
    assert(isContiguous(target) && u8"Target pixel buffer has no gaps");

    std::size_t channelCount = Nuclex::Pixels::CountChannels(source.PixelFormat);
    {
      avir::CImageResizer<avir::fpclass_def<double>> imageResizer(32, 32, resizerParams);
      imageResizer.resizeImage<float, float>(
        reinterpret_cast<const float *>(source.Pixels),
        static_cast<int>(source.Width), static_cast<int>(source.Height),
        static_cast<int>(channelCount * source.Width),
        reinterpret_cast<float *>(target.Pixels),
        static_cast<int>(target.Width), static_cast<int>(target.Height),
        static_cast<int>(channelCount),
        0.0, // resizing step, 0.0 = calculate automatically
        nullptr // additional variables
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using the embedded AVIR image resizing library</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  /// <param name="resizerParams">Parameters for the AVIR resizing algorithm</param>
  void resampleAvir(
    const Nuclex::Pixels::Bitmap &source,
    Nuclex::Pixels::Bitmap &target,
    const avir::CImageResizerParams &resizerParams
  ) {
    using Nuclex::Pixels::PixelFormats::PixelFormatQuery;
    using Nuclex::Pixels::PixelFormats::PixelFormatConverter;
    using Nuclex::Pixels::Bitmap;

    // This is just a long decision tree that figures out which formats to pass to
    // the AVIR library (uint8, uint16 or float), whether to convert the input bitmap
    // into an intermediate pixel format and wether to convert the output bitmap into
    // an intermediate pixel format.
    //
    // The complexity of cleverly solving this seems to outweight three pasted blocks
    // of it statements, so this appears more maintainable than going full DRY.
    //
    // Method:
    //
    //   1. if input/output is float, use Tin/Tout = float
    //   1a. if input/output is signed, also use Tin/Tout = float
    //   1b. if input/output is >8 bits, use Tin/Tout = std::uint16_t
    //   1c. otherwise use Tin/Tout = std::uint8_t
    //   2. Decide if input pixel format conversion or clone is needed
    //   2a. If pixel format doesn't match resizer's, convert
    //   2b. If stride is larger than line size, bitmap needs to be cloned
    //   3. Decide if output pixel format conversion or clone is needed
    //   3a. If pixel format doesn't match resizer's, use temporary bitmap
    //   3b. If stride is larger than line size, use temporary bitmap
    //   4. call resizeImage()
    //   5. if temporary bitmap used, copy back and/or convert pixel format
    //
    const Nuclex::Pixels::BitmapMemory &sourceMemory = source.Access();
    const Nuclex::Pixels::BitmapMemory &targetMemory = target.Access();

    // Use a floating point pixel format for conversion if either the input or the output
    // is a floating point format. We also need to do this for signed integer formats because
    // AVIR only supports signed channels when using floating point channels.
    bool involvesFloatsOrSignedChannels = (
      PixelFormatQuery::IsSignedFormat(sourceMemory.PixelFormat) ||
      PixelFormatQuery::IsSignedFormat(targetMemory.PixelFormat)
    );
    if(involvesFloatsOrSignedChannels) {
      Nuclex::Pixels::PixelFormat resamplePixelFormat = (
        getFloatingPointResamplingFormat(sourceMemory.PixelFormat)
      );

      // Dies the input happen to have the correct pixel format?
      if((sourceMemory.PixelFormat == resamplePixelFormat) && isContiguous(sourceMemory)) {
        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat) && isContiguous(targetMemory)) {
          resampleAvirWithFloatingPoint(sourceMemory, targetMemory, resizerParams);
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithFloatingPoint(sourceMemory, intermediate.Access(), resizerParams);
          PixelFormatConverter::Convert(intermediate, target);
        }
      } else { // No, input bitmap needs to be converted first
        Bitmap convertedSource = PixelFormatConverter::Convert(source, resamplePixelFormat);

        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat) && isContiguous(targetMemory)) {
          resampleAvirWithFloatingPoint(convertedSource.Access(), targetMemory, resizerParams);
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithFloatingPoint(
            convertedSource.Access(), intermediate.Access(), resizerParams
          );
          PixelFormatConverter::Convert(intermediate, target);
        }
      }

      return;
    }

    // If this point is reached, the pixel format is unsigned and integral. Now check if any
    // channel exceeds 8 bits, in which case we will perform the scaling with 16 bit math.
    bool exceedsEightBits = (
      (PixelFormatQuery::CountWidestChannelBits(sourceMemory.PixelFormat) >= 9) ||
      (PixelFormatQuery::CountWidestChannelBits(targetMemory.PixelFormat) >= 9)
    );
    if(exceedsEightBits) {
      Nuclex::Pixels::PixelFormat resamplePixelFormat = (
        getSixteenBitResamplingFormat(sourceMemory.PixelFormat)
      );

      // Dies the input happen to have the correct pixel format?
      if((sourceMemory.PixelFormat == resamplePixelFormat) && isContiguous(sourceMemory)) {
        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat)  && isContiguous(targetMemory)) {
          resampleAvirWithSixteenBitChannels(sourceMemory, targetMemory, resizerParams);
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithSixteenBitChannels(sourceMemory, intermediate.Access(), resizerParams);
          PixelFormatConverter::Convert(intermediate, target);
        }
      } else { // No, input bitmap needs to be converted first
        Bitmap convertedSource = PixelFormatConverter::Convert(source, resamplePixelFormat);

        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat) && isContiguous(targetMemory)) {
          resampleAvirWithSixteenBitChannels(
            convertedSource.Access(), targetMemory, resizerParams
          );
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithSixteenBitChannels(
            convertedSource.Access(), intermediate.Access(), resizerParams
          );
          PixelFormatConverter::Convert(intermediate, target);
        }
      }

      return;
    }

    // At this point, we know the pixel format is an unsigned integer format of 8 bits per
    // color channel or less, so we'll use 8 bit integer math to rescale.
    {
      Nuclex::Pixels::PixelFormat resamplePixelFormat = (
        getEightBitResamplingFormat(sourceMemory.PixelFormat)
      );

      // Dies the input happen to have the correct pixel format?
      if((sourceMemory.PixelFormat == resamplePixelFormat) && isContiguous(sourceMemory)) {
        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat)  && isContiguous(targetMemory)) {
          resampleAvirWithEightBitChannels(sourceMemory, targetMemory, resizerParams);
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithEightBitChannels(sourceMemory, intermediate.Access(), resizerParams);
          PixelFormatConverter::Convert(intermediate, target);
        }
      } else { // No, input bitmap needs to be converted first
        Bitmap convertedSource = PixelFormatConverter::Convert(source, resamplePixelFormat);

        // Does the output happen to have the correct pixel format?
        if((targetMemory.PixelFormat == resamplePixelFormat) && isContiguous(targetMemory)) {
          resampleAvirWithEightBitChannels(
            convertedSource.Access(), targetMemory, resizerParams
          );
        } else { // No, intermediate output bitmap is required
          Bitmap intermediate(targetMemory.Width, targetMemory.Height, resamplePixelFormat);
          resampleAvirWithEightBitChannels(
            convertedSource.Access(), intermediate.Access(), resizerParams
          );
          PixelFormatConverter::Convert(intermediate, target);
        }
      }

      return;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Scaling {

  // ------------------------------------------------------------------------------------------- //

  void BitmapScaler::rescaleAvir(
    const Bitmap &source, Bitmap &target, ResamplingMethod method
  ) {
    switch(method) {
      case ResamplingMethod::AvirSmooth: {
        resampleAvir(source, target, avir::CImageResizerParamsULR());
        break;
      }
      case ResamplingMethod::Avir: {
        resampleAvir(source, target, avir::CImageResizerParamsDef());
        break;
      }
      case ResamplingMethod::AvirSharp: {
        resampleAvir(source, target, avir::CImageResizerParamsUltra());
        break;
      }
      default: {
        throw std::logic_error(u8"Invalid resampling method specified");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Scaling
