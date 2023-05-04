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
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"

#include "Nuclex/Support/ScopeGuard.h"

#include "LibPngHelpers.h"

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

  /// <summary>Interpolates between a minimum and maximum value</summary>
  /// <typeparam name="TValue">
  ///   Type of value that will be interpolated, assumed to be  an integer
  /// </typeparam>
  /// <param name="t">Interpolation point between 0.0 .. 1.0</param>
  /// <param name="min">Minimum value, will be returned at <paramref name="t" /> 0.0</param>
  /// <param name="max">Maximum value, will be returned at <paramref name="t" /> 1.0</param>
  /// <returns>The interpolated value</returns>
  template<typename TValue>
  TValue lerpInclusive(float t, TValue min, TValue max) {
    float interpolated = static_cast<float>(max - min) * t + 0.5f;
    return min + static_cast<TValue>(interpolated);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  void PngBitmapCodec::Save(
    const Bitmap &bitmap, VirtualFile &target,
    float compressionEffortHint /* = 0.75f */, float outputQualityHint /* = 0.95f */
  ) const {
    (void)outputQualityHint;

    // Allocate the main LibPNG structure. It contains all pointers to user-defined
    // functions (IO, error handling and custom chunk processing, etc.)
    {
      ::png_struct *pngWrite = ::png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
      );
      if(pngWrite == nullptr) {
        throw std::bad_alloc();
      }
      ON_SCOPE_EXIT {
        ::png_destroy_write_struct(&pngWrite, nullptr);
      };

      // Install a custom error handler function that simply throws a C++ exception.
      // LibPNG is one of the few C libraries designed to allow exceptions passing through
      // because it's based on setjmp()/longjmp().
      ::png_set_error_fn(pngWrite, nullptr, &handlePngError, &handlePngWarning);

      {
        // We also need the info structure. This holds all importing informations describing
        // the image's dimensions, pixel format, palette, gamma etc.
        ::png_info *pngInfo = ::png_create_info_struct(pngWrite);
        if(pngInfo == nullptr) {
          throw std::bad_alloc();
        }
        ON_SCOPE_EXIT {
          ::png_destroy_info_struct(pngWrite, &pngInfo);
        };

        // Install a custom read function. This is used to read data from the virtual
        // file. The read environment emulates a file cursor.
        PngWriteEnvironment environment(*pngWrite, target);

        // Honor the caller's wish for the effort to put into compressing the image file
        // If libpng is compiled without this, the unused parameter warning is desirable.
        #if defined(PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED)
        {
          ::png_set_compression_level(
            pngWrite,
            lerpInclusive(compressionEffortHint, int(0), int(9))
          );
        }
        #endif

        const BitmapMemory &memory = bitmap.Access();

        // Determine the storage pixel format and the parameters that need to be passed
        // to libpng to correctly output a PNG in that pixel format
        PixelFormat storagePixelFormat;
        int colorType, bitDepth;
        {
          using Nuclex::Pixels::PixelFormats::PixelFormatQuery;

          // We generate either 8 bit or 16 bit PNGs with the criterion that we always
          // store the whole channel and data never gets lost.
          if(PixelFormatQuery::CountWidestChannelBits(memory.PixelFormat) >= 9) {
            bitDepth = 16;
          } else {
            bitDepth = 8;
          }

          // PNG files support only 4 color channel combinations that are relevant
          // to us. Select the one that doesn't lose data and is the closest to
          // the pixel format we're trying to save.
          std::size_t channelCount = CountChannels(memory.PixelFormat);
          if(PixelFormatQuery::HasAlphaChannel(memory.PixelFormat)) {
            if(channelCount == 2) {
              colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
              if(bitDepth == 8) {
                storagePixelFormat = PixelFormat::R8_A8_Unsigned;
              } else {
                storagePixelFormat = PixelFormat::R16_A16_Unsigned_Native16;
              }
            } else {
              colorType = PNG_COLOR_TYPE_RGB_ALPHA;
              if(bitDepth == 8) {
                storagePixelFormat = PixelFormat::R8_G8_B8_A8_Unsigned;
              } else {
                storagePixelFormat = PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
              }
            }
          } else {
            if(channelCount == 1) {
              colorType = PNG_COLOR_TYPE_GRAY;
              if(bitDepth == 8) {
                storagePixelFormat = PixelFormat::R8_Unsigned;
              } else {
                storagePixelFormat = PixelFormat::R16_Unsigned_Native16;
              }
            } else {
              colorType = PNG_COLOR_TYPE_RGB;
              if(bitDepth == 8) {
                storagePixelFormat = PixelFormat::R8_G8_B8_Unsigned;
              } else {
                storagePixelFormat = PixelFormat::R16_G16_B16_A16_Unsigned_Native16;
              }
            }
          }
        }

        // The 'IHDR' chunk (image header) contains vital image metadata like the width, height
        // and color depth the image is stored as. We can provide this data easily with
        // the informations we gathered above.
        ::png_set_IHDR(
          pngWrite,
          pngInfo,
          static_cast<::png_uint_32>(memory.Width),
          static_cast<::png_uint_32>(memory.Height),
          static_cast<int>(bitDepth),
          static_cast<int>(colorType),
          PNG_INTERLACE_NONE,
          PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT
        );

        // The sBIT chunk is a small 'hint' for the decoder how many bits in each color channel
        // were used by the source image. If we save a 10 bit image, LibPNG would force us to
        // scale all color channels to 16 bits depth, but we can at least place a hint in
        // the image file that remarks that it was upscaled from 10 bit color channels.
        #if defined(PNG_sBIT_SUPPORTED)
        {
          using Nuclex::Pixels::PixelFormats::PixelFormatQuery;

          ::png_color_8 significantBitCounts;

          significantBitCounts.red = static_cast<::png_byte>(
            PixelFormatQuery::CountRedBits(memory.PixelFormat).value_or(0)
          );
          significantBitCounts.green = static_cast<::png_byte>(
            PixelFormatQuery::CountGreenBits(memory.PixelFormat).value_or(0)
          );
          significantBitCounts.blue = static_cast<::png_byte>(
            PixelFormatQuery::CountBlueBits(memory.PixelFormat).value_or(0)
          );
          significantBitCounts.alpha = static_cast<::png_byte>(
            PixelFormatQuery::CountAlphaBits(memory.PixelFormat).value_or(0)
          );
          if(significantBitCounts.red > significantBitCounts.green) {
            if(significantBitCounts.blue > significantBitCounts.red) {
              significantBitCounts.gray = significantBitCounts.blue;
            } else {
              significantBitCounts.gray = significantBitCounts.red;
            }
          } else {
            if(significantBitCounts.blue > significantBitCounts.green) {
              significantBitCounts.gray = significantBitCounts.blue;
            } else {
              significantBitCounts.gray = significantBitCounts.green;
            }
          }

          ::png_set_sBIT(pngWrite, pngInfo, &significantBitCounts);
        }
        #endif

        // Let LibPNG write the image informations to the file.
        ::png_write_info(pngWrite, pngInfo);

        if constexpr(NUCLEX_PIXELS_LITTLE_ENDIAN) {
          if(bitDepth >= 9) {
            ::png_set_swap(pngWrite);
          }
        }

        // Can we save the image directly from the bitmap's data?
        // We can only do this if the bitmap's pixel format is natively supported by LibPNG.
        if(storagePixelFormat == memory.PixelFormat) {

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

            // Save entire bitmap. Error handling via assigned error handler (-> exceptions)
            ::png_write_image(pngWrite, &rowAddresses[0]);
            //::png_write_rows(pngWrite, &rowAddresses[0], memory.Height);
          }

        } else { // Direct save impossible, need pixel format conversion

          using Nuclex::Pixels::PixelFormats::PixelFormatConverter;

          // Allocate memory for 1 row (we're converting the pixel format of the image
          // row by row, this should yield good performance without wasting megabytes of memory)
          std::vector<std::uint8_t> rowBytes(CountRequiredBytes(storagePixelFormat, memory.Width));
          std::size_t pngRowByteCount = ::png_get_rowbytes(pngWrite, pngInfo);
          if(pngRowByteCount > rowBytes.size()) {
            rowBytes.resize(pngRowByteCount);
          }

          PixelFormatConverter::ConvertRowFunction *convertRow = (
            PixelFormatConverter::GetRowConverter(memory.PixelFormat, storagePixelFormat)
          );

          // Convert each row of the image to the pixel format LibPNG can save and let
          // LibPNG buffer or encode it in the new .png file
          const std::uint8_t *sourceRowStart = (
            reinterpret_cast<const std::uint8_t *>(memory.Pixels)
          );
          for(std::size_t rowIndex = 0; rowIndex < memory.Height; ++rowIndex) {
            convertRow(
              sourceRowStart, // + CountBitsPerPixel(memory.PixelFormat),
              rowBytes.data(), // + CountBitsPerPixel(storagePixelFormat),
              memory.Width
            );

            ::png_write_row(pngWrite, rowBytes.data());

            sourceRowStart += memory.Stride;
          }

        } // if pixel format conversion needed for save

        // We submitted all image pixels to LibPNG, tell it that we're done and to flush
        // all output. This guarantees that the IO interface will have received a full
        // image file containing all pixels and possible footer bytes.
        ::png_write_end(pngWrite, nullptr); // nullptr = no PNG info record (comments etc.)
        #if defined(PNG_WRITE_FLUSH_SUPPORTED)
        //::png_write_flush(pngWrite); // I don't think this is necessary.
        #endif

      } // pngInfo and pngWriteEnvironment scope
    } // pngWrite scope
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif //defined(NUCLEX_PIXELS_HAVE_LIBPNG)
