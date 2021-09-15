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
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"

#include <cassert> // for assert()
#include <type_traits> // for std::is_const()

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
    Nuclex::Pixels::Storage::Png::PngReadEnvironment &readEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Png::PngReadEnvironment *
    >(::png_get_io_ptr(pngRead));
    assert(readEnvironment.IsReadOnly && u8"File read is performed on read environment");

    readEnvironment.File.ReadAt(readEnvironment.Position, length, data);
    readEnvironment.Position += length;
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
    Nuclex::Pixels::Storage::Png::PngWriteEnvironment &writeEnvironment = *reinterpret_cast<
      Nuclex::Pixels::Storage::Png::PngWriteEnvironment *
    >(::png_get_io_ptr(pngWrite));
    assert(!writeEnvironment.IsReadOnly && u8"File write is performed on write environment");

    writeEnvironment.File.WriteAt(writeEnvironment.Position, length, data);
    writeEnvironment.Position += length;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom libpng flush function, called after writing has finished</summary>
  /// <param name="pngRead">PNG main structure</param>
  void flushVirtualFile(::png_struct *png_ptr) {
    (void)png_ptr; // Unused
    // Not needed in this implementation
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Selects the pixel format in which a .png file will be loaded</summary>
  /// <param name="pngRead">
  ///   PNG read structure from which the .png pixel format will be queried and
  ///   which will, optionally, receive necessary adjustments (if a non-const reference
  ///   is passed to the method)
  /// </param>
  /// <param name="pngInfo">
  ///   PNG information structure, required by some of the LibPNG query methods
  /// </param>
  /// <returns>
  ///   The pixel format that is closest/matches the .png file and for which LibPNG
  ///   has optionally been configured to load the image as
  /// </returns>
  /// <remarks>
  ///   LibPNG can perform some pixel format adjustments on its own. We use these to adapt
  ///   formats that would have no representation in Nuclex.Pixels (such as 1, 2 and 4 bits
  ///   per channel which is space-saving for storage but useless on modern graphics hardware).
  /// </remarks>
  template<typename TPngReadStruct = ::png_struct>
  Nuclex::Pixels::PixelFormat selectPixelFormatForLoad(
    TPngReadStruct &pngRead, const ::png_info &pngInfo
  ) {

    // From the docs:
    //
    // bitDepth
    //   bit depth of one of the image channels.  (valid values are 1, 2, 4, 8, 16
    //
    // channelCount:
    //   1 (GRAY, PALETTE), 2 (GRAY_ALPHA), 3 (RGB), 4 (RGB_ALPHA or RGB + filler byte))
    //
    // colorType
    //   PNG_COLOR_TYPE_GRAY (bit depths 1, 2, 4, 8, 16)
    //   PNG_COLOR_TYPE_GRAY_ALPHA (bit depths 8, 16)
    //   PNG_COLOR_TYPE_PALETTE (bit depths 1, 2, 4, 8)
    //   PNG_COLOR_TYPE_RGB (bit_depths 8, 16)
    //   PNG_COLOR_TYPE_RGB_ALPHA (bit_depths 8, 16)
    //
    //   PNG_COLOR_MASK_PALETTE
    //   PNG_COLOR_MASK_COLOR
    //   PNG_COLOR_MASK_ALPHA
    //
    // png_set_filler(png_ptr, filler, PNG_FILLER_BEFORE);
    //   Can produce XRGB or RGBX output
    //
    // PNG_FORMAT_FLAG_AFIRST
    //   Can put alpha channel first or last
    //
    // png_set_bgr(png_ptr);
    //   Can produce RGB or BGR output
    //
    // png_set_gray_to_rgb(png_ptr);
    //   Can convert grayscale to RGB
    //
    // png_set_strip_16(png_ptr);
    //   Can convert 16 bit to 8 bit channels
    //
    // png_set_strip_alpha(png_ptr);
    //   Can remove the alpha channel
    //
    // png_set_rgb_to_gray_fixed(png_ptr, error_action, int red_weight, int green_weight);
    //   Can convert RGB to grayscale
    //

    // Make sure the bit depth is at least 8 bits per pixel. Fortunately, LibPNG can
    // help us out if the image is saved at a lower bit depth
    ::png_byte bitDepth = ::png_get_bit_depth(&pngRead, &pngInfo);
    if(bitDepth < 8) {
      if constexpr(!std::is_const<TPngReadStruct>::value) {
        ::png_set_expand_gray_1_2_4_to_8(&pngRead);
      }
      bitDepth = 8;
    }

    // Get the actual format of the input pixels
    ::png_byte colorType = ::png_get_color_type(&pngRead, &pngInfo);
    ::png_byte channelCount = ::png_get_channels(&pngRead, &pngInfo);
    switch(channelCount) {
      case 1: {
        // If this is a palette-base image, convert it to 24 bit RGB. There is no
        // support for a 16 bit per channel color palette.
        if(colorType == PNG_COLOR_TYPE_PALETTE) {
          if constexpr(!std::is_const<TPngReadStruct>::value) {
            ::png_set_palette_to_rgb(&pngRead);
          }
          return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
        } else if(bitDepth == 16) {
          return Nuclex::Pixels::PixelFormat::R16_Unsigned_Native16;
        } else {
          return Nuclex::Pixels::PixelFormat::R8_Unsigned;
        }
        break;
      }
      case 2: {
        if(colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
          if(bitDepth == 16) {
            return Nuclex::Pixels::PixelFormat::R16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_A8_Unsigned;
          }
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format");
        }
        break;
      }
      case 3: {
        if(colorType == PNG_COLOR_TYPE_RGB) {
          if(bitDepth == 16) {
            if constexpr(!std::is_const<TPngReadStruct>::value) {
              ::png_set_filler(&pngRead, 0xFFFFFFFF, PNG_FILLER_AFTER);
            }
            return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
          }
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format (non-RGB)");
        }
        break;
      }
      case 4: {
        if(colorType == PNG_COLOR_TYPE_RGB) {
          if(bitDepth == 16) {
            if constexpr(!std::is_const<TPngReadStruct>::value) {
              ::png_set_filler(&pngRead, 0xFFFFFFFF, PNG_FILLER_AFTER);
            }
            return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
          }
        } else if(colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
          if(bitDepth == 16) {
            return Nuclex::Pixels::PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
          } else {
            return Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
          }
        } else {
          throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format (non-RGB)");
        }
        break;
      }
      default: {
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Unsupported pixel format (>4 channels)");
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::DoesFileExtensionSayPng(const std::string &extension) {
    std::size_t extensionLength = extension.length();
    if(extensionLength == 3) { // extension without dot possible
      return (
        ((extension[0] == 'p') || (extension[0] == 'P')) &&
        ((extension[1] == 'n') || (extension[1] == 'N')) &&
        ((extension[2] == 'g') || (extension[2] == 'G'))
      );
    } else if(extensionLength == 4) { // extension with dot possible
      return (
        (extension[0] == '.') &&
        ((extension[1] == 'p') || (extension[1] == 'P')) &&
        ((extension[2] == 'n') || (extension[2] == 'N')) &&
        ((extension[3] == 'g') || (extension[3] == 'G'))
      );
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::CheckIfPngHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossiblePngSize) {
      return false; // File is too small to be a .png file
    }

    // If the file header is not indicative of a PNG file, bail out
    {
      std::uint8_t fileHeader[16];
      source.ReadAt(0, 16, fileHeader);
      if(::png_sig_cmp(fileHeader, 0, 16) != 0) {
        return false;
      }
    }

    // File was long enough to be a valid .png and the file header checks out
    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  PixelFormat Helpers::SelectPixelFormatForLoad(
    ::png_struct &pngRead, const ::png_info &pngInfo
  ) {
    return selectPixelFormatForLoad(pngRead, pngInfo);
  }

  // ------------------------------------------------------------------------------------------- //

  PixelFormat Helpers::GetClosestPixelFormat(
    const ::png_struct &pngRead, const ::png_info &pngInfo
  ) {
    return selectPixelFormatForLoad(pngRead, pngInfo);
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

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)
