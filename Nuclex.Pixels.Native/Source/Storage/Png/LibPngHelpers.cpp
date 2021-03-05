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

#include "LibPngHelpers.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)

#include "Nuclex/Pixels/Errors/FileFormatError.h"

#include <cassert>
#include <algorithm>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom libpng read function that reads data from a virtual file</summary>
  /// <param name="pngRead">PNG read main structure</param>
  /// <param name="data">Buffer that should receive the data</param>
  /// <param name="length">Number of bytes that should be read from the file</param>
  void readFromVirtualFile(
    ::png_struct *pngRead, ::png_byte *data, ::png_size_t length
  ) {
    // Here's some more internal libpng knowledge: png_error directly invokes
    // the user error handler, which is supposed to longjmp(), so exceptions are fine!
    //try {
    Nuclex::Pixels::Storage::Png::PngReadEnvironment &readEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Png::PngReadEnvironment *
    >(::png_get_io_ptr(pngRead));
    if(!readEnvironment.IsReadOnly) {
      throw std::runtime_error(u8"libpng tried to read from a file opened for writing");
    }

    readEnvironment.File.ReadAt(readEnvironment.Position, length, data);
    readEnvironment.Position += length;
    //}
    //catch(const std::exception &error) {
    //  png_error(pngRead, "Call to NULL read function");
    //}
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom libpng write function that write data into a virtual file</summary>
  /// <param name="pngRead">PNG write main structure</param>
  /// <param name="data">Buffer that contains the data to be written</param>
  /// <param name="length">Number of bytes that should be written to the file</param>
  void writeToVirtualFile(
    ::png_struct *pngWrite, ::png_byte *data, ::png_size_t length
  ) {
    // Here's some more internal libpng knowledge: png_error directly invokes
    // the user error handler, which is supposed to longjmp(), so exceptions are fine!
    //try {
    Nuclex::Pixels::Storage::Png::PngWriteEnvironment &writeEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Png::PngWriteEnvironment *
    >(::png_get_io_ptr(pngWrite));
    if(writeEnvironment.IsReadOnly) {
      throw std::runtime_error(u8"libpng tried to write to a read-only file");
    }

    writeEnvironment.File.WriteAt(writeEnvironment.Position, length, data);
    writeEnvironment.Position += length;
    //}
    //catch(const std::exception &error) {
    //  png_error(pngRead, "Call to NULL read function");
    //}
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom libpng flush function, called after writing has finished</summary>
  /// <param name="pngRead">PNG main structure</param>
  void flushVirtualFile(::png_struct *png_ptr) {
    (void)png_ptr; // Unused

    // Not needed in this implementation
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Finds the supported pixel format that is closest to the PNG's</summary>
  /// <param name="pngRead">Main PNG structure storing libpng settings</param>
  /// <param name="pngInfo">PNG info structure storing information about an image</param>
  /// <returns>The pixel format that's most like the one of the PNG image</returns>
  PixelFormat Helpers::GetEquivalentPixelFormat(
    const ::png_struct &pngRead, const ::png_info &pngInfo
  ) {
    ::png_byte bitDepth = ::png_get_bit_depth(&pngRead, &pngInfo);
    ::png_byte colorType = ::png_get_color_type(&pngRead, &pngInfo);
    ::png_byte channelCount = ::png_get_channels(&pngRead, &pngInfo);

    // From the docs:
    //
    // bitDepth
    // bit depth of one of the image channels.  (valid values are 1, 2, 4, 8, 16
    //
    // channelCount:
    // 1 (GRAY, PALETTE), 2 (GRAY_ALPHA), 3 (RGB), 4 (RGB_ALPHA or RGB + filler byte))
    //
    // colorType
    // PNG_COLOR_TYPE_GRAY (bit depths 1, 2, 4, 8, 16)
    // PNG_COLOR_TYPE_GRAY_ALPHA (bit depths 8, 16)
    // PNG_COLOR_TYPE_PALETTE (bit depths 1, 2, 4, 8)
    // PNG_COLOR_TYPE_RGB (bit_depths 8, 16)
    // PNG_COLOR_TYPE_RGB_ALPHA (bit_depths 8, 16)
    //  PNG_COLOR_MASK_PALETTE
    //  PNG_COLOR_MASK_COLOR
    //  PNG_COLOR_MASK_ALPHA
    switch(channelCount) {
      case 1: {
        if(colorType == PNG_COLOR_TYPE_PALETTE) {
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(bitDepth == 16) {
          return Nuclex::Pixels::PixelFormat::R16_Unsigned_Native16;
        } else {
          return Nuclex::Pixels::PixelFormat::R8_Unsigned;
        }
      }
      case 2: {
        if(colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
          if(bitDepth == 16) {
            throw std::runtime_error(u8"Required pixel format not implemented yet");
            //return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
          }
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case 3: {
        if(colorType == PNG_COLOR_TYPE_RGB) {
          if(bitDepth == 16) {
            throw std::runtime_error(u8"Required pixel format not implemented yet");
            //return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
          }
        } else {
            throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
      }
      case 4: {
        if(colorType == PNG_COLOR_TYPE_RGB) {
          if(bitDepth == 16) {
            throw std::runtime_error(u8"Required pixel format not implemented yet");
            //return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
          }
        } else {
          if(bitDepth == 16) {
            throw std::runtime_error(u8"Required pixel format not implemented yet");
            //return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
          }
        }
      }
      default: {
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PngReadEnvironment::SetupFunctionPointers(
    PngReadEnvironment &pngReadEnvironment, ::png_struct &pngRead
  ) {
    ::png_set_read_fn(&pngRead, &pngReadEnvironment, &readFromVirtualFile);
  }

  // ------------------------------------------------------------------------------------------- //

  void PngWriteEnvironment::SetupFunctionPointers(
    PngWriteEnvironment &pngWriteEnvironment, ::png_struct &pngWrite
  ) {
    ::png_set_write_fn(&pngWrite, &pngWriteEnvironment, &writeToVirtualFile, &flushVirtualFile);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)
