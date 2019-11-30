#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#include "LibJpegHelpers.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)

#include "Nuclex/Pixels/Errors/FileFormatError.h"

#include <cassert>
#include <algorithm>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Does absolutely nothing with a jpeg_decompress_struct</summary>
  /// <param name="commonInfo">JPEG common structure that is not used</param>
  void doNothingWithDecompressStruct(struct ::jpeg_decompress_struct *commonInfo) {
    // Just like the function name promises...
    (void)commonInfo;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data from a virtual file up to the buffer size</summary>
  /// <param name="commonInfo">
  ///   JPEG common decompression structure containing the read environment structure
  /// </param>
  /// <returns>
  ///   True if the read buffer has been filled, false if the decoder should suspend
  ///   (suspension is complex behavior not needed or implemented by this library!)
  /// </returns>
  ::boolean readVirtualFile(struct ::jpeg_decompress_struct *commonInfo) {
    assert(
      (commonInfo != nullptr) &&
      u8"Common decompression info structure must always be provided"
    );
    assert(
      (commonInfo->src != nullptr) &&
      u8"LibJPEG input data source must be set up to a JpegReadEnvironment"
    );

    Nuclex::Pixels::Storage::Jpeg::JpegReadEnvironment &readEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Jpeg::JpegReadEnvironment *
    >(commonInfo->src);

    if(!readEnvironment.IsReadOnly) {
      throw std::runtime_error(u8"libjpeg read method was called on a write environment");
    }

    // Read up to one buffer full of data from the virtual file
    std::size_t remainingByteCount = readEnvironment.Length - readEnvironment.Position;
    std::size_t targetByteCount = std::min(
      remainingByteCount, Nuclex::Pixels::Storage::Jpeg::JpegInputBufferSize
    );
    readEnvironment.File.ReadAt(
      readEnvironment.Position, targetByteCount, readEnvironment.Buffer
    );
    readEnvironment.Position += targetByteCount;

    // Update the libjpeg decoder's input counters
    readEnvironment.next_input_byte = readEnvironment.Buffer;
    readEnvironment.bytes_in_buffer = targetByteCount;

    // The return value does not indicate error/success, but whether the input stream
    // is temporarily out of data and the decoder should suspend decompression.
    return TRUE;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Skips the file cursor ahead when reading data</summary>
  /// <param name="commonInfo">
  ///   JPEG common decompression structure containing the read environment structure
  /// </param>
  /// <param name="byteCount">
  ///   Number of bytes by which the file cursor should be advanced
  /// </param>
  void advanceVirtualFileCursor(
    struct ::jpeg_decompress_struct *commonInfo, long byteCount
  ) {
    assert(
      (commonInfo != nullptr) &&
      u8"Common decompression info structure must always be provided"
    );
    assert(
      (commonInfo->src != nullptr) &&
      u8"LibJPEG input data source must be set up to a JpegReadEnvironment"
    );

    Nuclex::Pixels::Storage::Jpeg::JpegReadEnvironment &readEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Jpeg::JpegReadEnvironment *
    >(commonInfo->src);

    if(!readEnvironment.IsReadOnly) {
      throw std::runtime_error(u8"libjpeg advance method was called on a write environment");
    }

    if(readEnvironment.Position + byteCount > readEnvironment.Length) {
      throw std::runtime_error(u8"Attempt to seek past end of file");
      //throw Nuclex::Pixels::Errors::FileAccessError(u8"Attempt to seek past end of file");
    }
    //if(byteCount < 0)

    readEnvironment.Position += byteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Resyncs after the jpeg decompressor has been suspended after running out of data
  /// </summary>
  /// <param name="commonInfo">
  ///   JPEG common decompression structure containing the read environment structure
  /// </param>
  /// <param name="desired">Desired stream offset to resync to (negative!)</param>
  /// <returns>True if the resync was possible, false otherwise</param>
  /// <remarks>
  ///   This method is only needed if the fill buffer (readVirtualFile()) method returns
  ///   FALSE to indicate a temporary data stall. Since we're not using that feature,
  ///   this is stubbed and will blast an exception right through libjpeg.
  /// </remarks>
  ::boolean throwResyncNotSupported(struct ::jpeg_decompress_struct *commonInfo, int desired) {
    (void)commonInfo;
    (void)desired;
    throw std::runtime_error(u8"Jpeg decompressor resync not supported");
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::IsValidJpegHeader(const std::uint8_t *fileHeader) {
    return (
      (fileHeader[0] == 0xff) && //  1 SOI ("Start of Image" marker)
      (fileHeader[1] == 0xd8) && //  1 
      (fileHeader[2] == 0xff) && //  2 JFIF marker (magic number #1)
      (fileHeader[3] == 0xe0) && //  2
      (
        (fileHeader[4] >= 1) ||  //  3 Header length (at least 10 bytes)
        (fileHeader[5] >= 10)    //  3
      ) &&
      (fileHeader[6] == 0x4a) && //  4 JFIF marker (magic number #2)
      (fileHeader[7] == 0x46) && //  4
      (fileHeader[8] == 0x49) && //  4
      (fileHeader[9] == 0x46) && //  4
      (fileHeader[10] == 0x00) && // 4
      (
        (fileHeader[12] == 0) || //  5 Density units (0, 1 or 2)
        (fileHeader[12] == 1) || //  5
        (fileHeader[12] == 2) //     5
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Finds the supported pixel format that is closest to the JPEG's</summary>
  /// <param name="commonInfo">JPEG decompression structure with image information</param>
  /// <returns>The pixel format that's most like the one of the JPEG image</returns>
  PixelFormat Helpers::GetEquivalentPixelFormat(const ::jpeg_decompress_struct &commonInfo) {
    switch(commonInfo.jpeg_color_space) {
      case JCS_UNKNOWN: { 
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
      }
      case JCS_GRAYSCALE: {
        if(commonInfo.num_components == 1) {
          return Nuclex::Pixels::PixelFormat::R8_Unsigned;
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case JCS_RGB: {
        if(commonInfo.num_components == 3) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(commonInfo.num_components == 4) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case JCS_YCbCr: { // This is converted to RGB / RGBA on load
        if(commonInfo.num_components == 3) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(commonInfo.num_components == 4) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case JCS_CMYK:
      case JCS_YCCK: {
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
      }

#if 0 // libjpeg-turbo only
      case JCS_EXT_RGB:
      case JCS_EXT_RGBX:
      case JCS_EXT_BGR:
      case JCS_EXT_BGRX:
      case JCS_EXT_XBGR:
      case JCS_EXT_XRGB: {
        if(commonInfo.num_components == 3) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(commonInfo.num_components == 4) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case JCS_EXT_RGBA:
      case JCS_EXT_BGRA:
      case JCS_EXT_ABGR:
      case JCS_EXT_ARGB: { // This is not really with alpha, just an output format
        if(commonInfo.num_components == 3) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(commonInfo.num_components == 4) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case JCS_RGB565: {
        return Nuclex::Pixels::PixelFormat::R5_G6_B5_Unsigned;
      }
#endif
      default: {
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void JpegReadEnvironment::SetupFunctionPointers(JpegReadEnvironment &jpegReadEnvironment) {
    jpegReadEnvironment.init_source = &doNothingWithDecompressStruct;
    jpegReadEnvironment.fill_input_buffer = &readVirtualFile;
    jpegReadEnvironment.skip_input_data = &advanceVirtualFileCursor;
    jpegReadEnvironment.resync_to_restart = &throwResyncNotSupported;
    jpegReadEnvironment.term_source = &doNothingWithDecompressStruct;

    jpegReadEnvironment.next_input_byte = nullptr;
    jpegReadEnvironment.bytes_in_buffer = 0;
  }

  // ------------------------------------------------------------------------------------------- //

  void JpegWriteEnvironment::SetupFunctionPointers(JpegWriteEnvironment &jpegWriteEnvironment) {
    jpegWriteEnvironment.init_source = &doNothingWithDecompressStruct;
    //jpegWriteEnvironment.fill_input_buffer = &readVirtualFile;
    //jpegWriteEnvironment.skip_input_data = &advanceVirtualFileCursor;
    //jpegWriteEnvironment.resync_to_restart = &throwResyncNotSupported;
    jpegWriteEnvironment.term_source = &doNothingWithDecompressStruct;

    jpegWriteEnvironment.next_input_byte = nullptr;
    jpegWriteEnvironment.bytes_in_buffer = 0;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
