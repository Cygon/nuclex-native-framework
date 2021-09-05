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

#include "PngBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "Nuclex/Pixels/Errors/WrongSizeError.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"

#include "Nuclex/Support/ScopeGuard.h"

#include "LibPngHelpers.h"

#include <png.h> // libpng main header

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

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Handles an error occuring while a PNG is being read</summary>
  /// <param name="png">PNG main structure, unused</param>
  /// <param name="errorMessage">Describes the error that has occurred</param>
  /// <remarks>
  ///   <para>
  ///     libpng is a C library, but its error handling scheme expects this function to never
  ///     return (either it calls abort() or longjmp()). To allow this, all memory libpng
  ///     allocates must be tracked in the png_struct and there must be no open ends on
  ///     the stack when the error handler is called.
  ///   </para>
  ///   <para>
  ///     This gives us all the guarantees we need to fire a C++ exception right through
  ///     libpng back to our original call site.
  ///   </para>
  /// </remarks>
  void handlePngError(::png_struct *png, const char *errorMessage) {
    (void)png;
    throw Nuclex::Pixels::Errors::FileFormatError(errorMessage);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Handles a warning being issues by libpng</summary>
  /// <param name="png">PNG main structure, unused</param>
  /// <param name="warningMessage">Describes the warning, unused</param>
  void handlePngWarning(::png_struct *png, const char *warningMessage) {
    (void)png;
    (void)warningMessage;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Selects the pixel format in which a .png file will be loaded</summary>
  /// <param name="pngRead">
  ///   PNG read structure from which the .png pixel format will be queried and which will
  ///   receive necessary adjustments
  /// </param>
  /// <param name="pngInfo">
  ///   PNG information structure, required by some of the LibPNG query methods
  /// </param>
  /// <returns>
  ///   The pixel format that is closest/matches the .png file and for which LibPNG has
  ///   been configured to load the image as
  /// </returns>
  /// <remarks>
  ///   LibPNG can perform some pixel format adjustments on its own. We use these to adapt
  ///   formats that would have no representation in Nuclex.Pixels (such as 1, 2 and 4 bits
  ///   per channel which is space-saving for storage but useless on modern graphics hardware).
  /// </remarks>
  Nuclex::Pixels::PixelFormat selectPixelFormatForLoad(
    ::png_struct &pngRead, const ::png_info &pngInfo
  ) {

    // Make sure the bit depth is at least 8 bits per pixel. Fortunately, LibPNG can
    // help us out if the image is saved at a lower bit depth
    ::png_byte bitDepth = ::png_get_bit_depth(&pngRead, &pngInfo);
    if(bitDepth < 8) {
      ::png_set_expand_gray_1_2_4_to_8(&pngRead);
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
          ::png_set_palette_to_rgb(&pngRead);
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
            ::png_set_filler(&pngRead, 0xFFFFFFFF, PNG_FILLER_AFTER);
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
            ::png_set_filler(&pngRead, 0xFFFFFFFF, PNG_FILLER_AFTER);
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

  /// <summary>Loads a .png file into a Bitmap's memory keeping the pixel format</summary>
  /// <param name="pngRead">LibPNG read structure through which reading will take place</param>
  /// <param name="pngInfo">LibPNG info structure required for some query functions</param>
  /// <param name="memory">
  ///   Description of the bitmap memory layout the loaded file will be stored in
  /// </param>
  void loadPngIntoBitmapMemoryDirect(
    ::png_struct &pngRead, const ::png_info &pngInfo,
    const Nuclex::Pixels::BitmapMemory &memory
  ) {

    // Obtain the number of bytes per row libpng thinks it requires. Since we're
    // providing the row start adresses to libpng ourselves, we only use this for
    // a safety check so we know the Bitmap's memory can hold what libpng writes.
    {
      std::size_t bytesPerRow = ::png_get_rowbytes(&pngRead, &pngInfo);
      if(bytesPerRow > static_cast<std::size_t>(std::abs(memory.Stride))) {
        throw Nuclex::Pixels::Errors::FileFormatError(
          u8"libpng row size unexpectedly large, wrong pixel format?"
        );
      }

      //assert()
    }

    // Finally, build an array of row addresses for libpng and use it to load
    // the whole image in one call. This minimizes the number of method calls and
    // should be the most efficient method to get the pixels into the Bitmap.
    {
      std::vector<::png_byte *> rowAddresses;
      {
        rowAddresses.reserve(memory.Height);

        std::uint8_t *rowStartPointer = reinterpret_cast<std::uint8_t *>(memory.Pixels);
        for(std::size_t index = 0; index < memory.Height; ++index) {
          rowAddresses.push_back(rowStartPointer);
          rowStartPointer += memory.Stride;
        }
      }

      // Load entire bitmap. Error handling via assigned error handler (-> exceptions)
      ::png_read_image(&pngRead, &rowAddresses[0]);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Loads a .png file into a Bitmap's memory, converting the pixel format on the fly
  /// </summary>
  /// <param name="pngRead">LibPNG read structure through which reading will take place</param>
  /// <param name="pngInfo">LibPNG info structure required for some query functions</param>
  /// <param name="storagePixelFormat">
  ///   Pixel format as which LibPNG will load the .png file (usually determined by the call
  ///   to <see cref="selectPixelFormatForLoad" />)
  // </param>
  /// <param name="memory">
  ///   Description of the bitmap memory layout the loaded file will be stored in
  /// </param>
  void loadPngIntoBitmapMemoryWithConversion(
    ::png_struct &pngRead, const ::png_info &pngInfo,
    Nuclex::Pixels::PixelFormat storagePixelFormat,
    const Nuclex::Pixels::BitmapMemory &memory
  ) {

    // Allocate memory for 1 row (we're converting the pixel format of the image
    // row by row, this should yield good performance without wasting megabytes of memory)
    std::vector<std::uint8_t> rowBytes(
      Nuclex::Pixels::CountRequiredBytes(storagePixelFormat, memory.Width)
    );
    {
      std::size_t pngRowByteCount = ::png_get_rowbytes(&pngRead, &pngInfo);
      if(pngRowByteCount > rowBytes.size()) {
        rowBytes.resize(pngRowByteCount);
      }
    }

    {
      using Nuclex::Pixels::PixelFormats::ConvertRowFunction;
      using Nuclex::Pixels::PixelFormats::GetPixelFormatConverter;

      ConvertRowFunction *convertRow = GetPixelFormatConverter(
        storagePixelFormat, memory.PixelFormat
      );

      // Let LibPNG load the image successively row-by-row and convert each
      // row from the temporary buffer into the correct location in the Bitmap's memory
      std::uint8_t *targetRowStart = (
        reinterpret_cast<std::uint8_t *>(memory.Pixels)
      );
      for(std::size_t rowIndex = 0; rowIndex < memory.Height; ++rowIndex) {
        ::png_read_row(&pngRead, rowBytes.data(), nullptr);

        convertRow(
          rowBytes.data(), // + CountBitsPerPixel(storagePixelFormat),
          targetRowStart, // + CountBitsPerPixel(memory.PixelFormat),
          memory.Width
        );

        targetRowStart += memory.Stride;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  std::optional<Bitmap> PngBitmapCodec::TryLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // If this doesn't look like a .png file, bail out immediately
    if(!Helpers::CheckIfPngHeaderPresent(source)) {
      return std::optional<Bitmap>();
    }

    {
      // Allocate the main LibPNG structure. It contains all pointers to user-defined
      // functions (IO, error handling and custom chunk processing, etc.)
      ::png_struct *pngRead = ::png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
      );
      if(pngRead == nullptr) {
        throw std::bad_alloc();
      }
      ON_SCOPE_EXIT {
        ::png_destroy_read_struct(&pngRead, nullptr, nullptr);
      };

      // Install a custom error handler function that simply throws a C++ exception.
      // LibPNG is one of the few C libraries designed to allow exceptions passing through
      // because it's based on setjmp()/longjmp().
      ::png_set_error_fn(pngRead, nullptr, &handlePngError, &handlePngWarning);

      {
        // We also need the info structure. This is filled with all important informations
        // describing the image's dimensions, pixel format, palette, gamma etc.
        ::png_info *pngInfo = ::png_create_info_struct(pngRead);
        if(pngInfo == nullptr) {
          throw std::bad_alloc();
        }
        ON_SCOPE_EXIT {
          ::png_destroy_info_struct(pngRead, &pngInfo);
        };

        // Install a custom read function. This is used to read data from the virtual
        // file. The read environment emulates a file cursor.
        PngReadEnvironment environment(*pngRead, source);

        // Now that we're ready to actually access the PNG file,
        // attempt to obtain the image's resolution, pixel format and so on
        ::png_read_info(pngRead, pngInfo);

        // Determine the pixel format used in the .png file (this will also configure
        // LibPNG to perform adjustment in case the native pixel format is not supported)
        PixelFormat storagePixelFormat = selectPixelFormatForLoad(*pngRead, *pngInfo);
        std::size_t width = ::png_get_image_width(pngRead, pngInfo);
        std::size_t height = ::png_get_image_height(pngRead, pngInfo);

        // Perform the actual load through the shared loading code
        // (since we can match the pixel format used for storage, this needs no conversion)
        Bitmap image(width, height, storagePixelFormat);
        loadPngIntoBitmapMemoryDirect(*pngRead, *pngInfo, image.Access());
        return std::optional<Bitmap>(std::move(image));
      } // pngInfo scope

    } // pngRead scope
  }

  // ------------------------------------------------------------------------------------------- //

  bool PngBitmapCodec::TryReload(
    Bitmap &exactlyFittingBitmap,
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // If this doesn't look like a .png file, bail out immediately
    if(!Helpers::CheckIfPngHeaderPresent(source)) {
      return false;
    }

    // Allocate the main LibPNG structure. It contains all pointers to user-defined
    // functions (IO, error handling and custom chunk processing, etc.)
    {
      ::png_struct *pngRead = ::png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
      );
      if(pngRead == nullptr) {
        throw std::bad_alloc();
      }
      ON_SCOPE_EXIT {
        ::png_destroy_read_struct(&pngRead, nullptr, nullptr);
      };

      // Install a custom error handler function that simply throws a C++ exception.
      // LibPNG is one of the few C libraries designed to allow exceptions passing through
      // because it's based on setjmp()/longjmp().
      ::png_set_error_fn(pngRead, nullptr, &handlePngError, &handlePngWarning);

      {
        // We also need the info structure. This is filled with all important informations
        // describing the image's dimensions, pixel format, palette, gamma etc.
        ::png_info *pngInfo = ::png_create_info_struct(pngRead);
        if(pngInfo == nullptr) {
          throw std::bad_alloc();
        }
        ON_SCOPE_EXIT {
          ::png_destroy_info_struct(pngRead, &pngInfo);
        };

        // Install a custom read function. This is used to read data from the virtual
        // file. The read environment emulates a file cursor.
        PngReadEnvironment environment(*pngRead, source);

        // Now that we're ready to actually access the PNG file,
        // attempt to obtain the image's resolution, pixel format and so on
        ::png_read_info(pngRead, pngInfo);

        std::size_t width = ::png_get_image_width(pngRead, pngInfo);
        std::size_t height = ::png_get_image_height(pngRead, pngInfo);

        const BitmapMemory &memory = exactlyFittingBitmap.Access();
        if((width != memory.Width) || (height != memory.Height)) {
          throw Errors::WrongSizeError(
            u8"Size of existing target Bitmap did not match the image file being loaded"
          );
        }

        // Determine the pixel format used in the .png file (this will also configure
        // LibPNG to perform adjustment in case the native pixel format is not supported)
        PixelFormat storagePixelFormat = selectPixelFormatForLoad(*pngRead, *pngInfo);

        // Perform the actual load. If the pixel format of the provided bitmap matches
        // the pixel format of the .png file, we can do a direct load, otherwise we will
        // load the .png file row-by-row and convert the pixel format while copying.
        if(memory.PixelFormat == storagePixelFormat) {
          loadPngIntoBitmapMemoryDirect(*pngRead, *pngInfo, memory);
        } else {
          loadPngIntoBitmapMemoryWithConversion(
            *pngRead, *pngInfo, storagePixelFormat, memory
          );
        }
      } // pngInfo scope
    } // pngRead scope

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif //defined(NUCLEX_PIXELS_HAVE_LIBPNG)
