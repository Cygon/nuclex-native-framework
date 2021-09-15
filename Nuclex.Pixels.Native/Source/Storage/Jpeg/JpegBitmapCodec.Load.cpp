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

#include "JpegBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "Nuclex/Pixels/Errors/WrongSizeError.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"

#include "Nuclex/Support/ScopeGuard.h"

#include "LibJpegHelpers.h"

#include <cassert> // for assert()
#include <algorithm>

#include <jpeglib.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Ignores diagnostic messages from jpeglib</summary>
  /// <param name="cinfo">Jpeg common info containing the diagnostic message</param>
  void discardJpegMessage(struct ::jpeg_common_struct *cinfo) { (void)cinfo; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Handles an error inside libjpeg</summary>
  /// <param name="info">Main structure containing all libjpeg configuration</param>
  /// <remarks>
  ///   <para>
  ///     libjpeg is a C library, but its error handling scheme expects this function to
  ///     never return (either it calls abort() or longjmp()). To allow this, all memory
  ///     libjpeg allocates must be tracked in the jpeg_common_struct and there must be no
  ///     open ends on the stack when the error handler is called.
  ///   </para>
  ///   <para>
  ///     This gives us all the guarantees we need to fire a C++ exception right through
  ///     libjpeg back to our original call site.
  ///   </para>
  /// </remarks>
  void handleJpegError(struct ::jpeg_common_struct *cinfo) {
    if(cinfo != nullptr) {
      if(cinfo->err != nullptr) {
        const ::jpeg_error_mgr &errorManager = *cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*errorManager.format_message)(cinfo, buffer);

        throw Nuclex::Pixels::Errors::FileFormatError(buffer);
      }
    }

    throw Nuclex::Pixels::Errors::FileFormatError(u8"Error occurred in libjpeg");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Selects the pixel format in which a .jpg file will be loaded</summary>
  /// <param name="commonInfo">
  ///   JPEG decompression informations containing the JPEG's stored pixel format,
  ///   will have the output pixel format set to match the returned load pixel format
  /// </param>
  /// <returns>
  ///   The pixel format that is closest/matches the .jpeg file and for which LibJPEG has
  ///   been configured to load the image as
  /// </returns>
  Nuclex::Pixels::PixelFormat selectPixelFormatForLoad(
    ::jpeg_decompress_struct &commonInfo
  ) {

    // From the libjpeg docs:
    //
    //   J_COLOR_SPACE out_color_space
    //
    //     Output color space. jpeg_read_header() sets an appropriate default based on
    //     jpeg_color_space; typically it will be RGB or grayscale. The application can
    //     change this field to request output in a different colorspace. For example,
    //     set it to JCS_GRAYSCALE to get grayscale output from a color file.
    //

    // Force libjpeg to decode either as 8 bit grayscale or as 24 bit RGB. The 'else'
    // branch also avoids the new formats introduced with libjpeg-turbo, which are all
    // just for convenience (XRGB and ARGB, but the extra byte is either garbage or always
    // set to 0xFF. The only interesting format would be R5-G6-B5, but we can convert
    // to that ourselves more efficiently and avoid requiring libjpeg-turbo.
    if(commonInfo.jpeg_color_space == JCS_GRAYSCALE) {
      commonInfo.output_components = 1;
      commonInfo.out_color_space = JCS_GRAYSCALE;
      return Nuclex::Pixels::PixelFormat::R8_Unsigned;
    } else {
      commonInfo.output_components = 3;
      commonInfo.out_color_space = JCS_RGB;
      return Nuclex::Pixels::PixelFormat::R8_G8_B8_Unsigned;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Loads a .jpg file into a Bitmap's memory keeping the pixel format</summary>
  /// <param name="commonInfo">LibJPEG decompression state and settings</param>
  /// <param name="memory">
  ///   Description of the bitmap memory layout the loaded file will be stored in
  /// </param>
  void loadJpegIntoBitmapMemoryDirect(
    ::jpeg_decompress_struct &commonInfo,
    const Nuclex::Pixels::BitmapMemory &memory
  ) {

    // Begin decompression, this will update output_width and output_height,
    // usually to the same as image_width, image_height unless scaling is set up.
    ::boolean startedWithoutSuspension = ::jpeg_start_decompress(&commonInfo);
    if(startedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
      throw Nuclex::Pixels::Errors::FileFormatError(u8"Input file truncated");
    }

    // Read the bitmap scanline by scanline. The function can also take an array of
    // scanlines, which may be faster than decoding line-by-line, but this is
    // the most straightforward way to do it.
    std::uint8_t *currentRowPointer = reinterpret_cast<std::uint8_t *>(memory.Pixels);
    while(commonInfo.output_scanline < commonInfo.output_height) {
      ::JDIMENSION readScanlineCount = ::jpeg_read_scanlines(
        &commonInfo, &currentRowPointer, 1
      );
      if(readScanlineCount != 1) {
        throw Nuclex::Pixels::Errors::FileFormatError(
          u8"Unknown error reading scanline from jpeg"
        );
      }
      currentRowPointer += memory.Stride;
    }

    // Finish decompression. This does some additional sanity checks, verifying that
    // the image was decompressed completely and reading the input stream up to the EOI
    // market (in case it contains multiple images).
    ::boolean endedWithoutSuspension = ::jpeg_finish_decompress(&commonInfo);
    if(endedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
      throw Nuclex::Pixels::Errors::FileFormatError(u8"Input file truncated");
    }

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Loads a .jpeg file into a Bitmap's memory, converting the pixel format on the fly
  /// </summary>
  /// <param name="commonInfo">LibJPEG decompression state and settings</param>
  /// <param name="storagePixelFormat">
  ///   Pixel format as which LibJPEG will load the .jpg file (usually determined by the call
  ///   to <see cref="selectPixelFormatForLoad" />)
  // </param>
  /// <param name="memory">
  ///   Description of the bitmap memory layout the loaded file will be stored in
  /// </param>
  void loadJpegIntoBitmapMemoryWithConversion(
    ::jpeg_decompress_struct &commonInfo,
    Nuclex::Pixels::PixelFormat storagePixelFormat,
    const Nuclex::Pixels::BitmapMemory &memory
  ) {

    // Allocate memory for 1 row (we're converting the pixel format of the image
    // row by row, this should yield good performance without wasting megabytes of memory)
    std::vector<std::uint8_t> rowBytes(
      Nuclex::Pixels::CountRequiredBytes(storagePixelFormat, memory.Width)
    );
    {
      std::size_t jpegRowByteCount = commonInfo.out_color_components * commonInfo.output_width;
      if(jpegRowByteCount > rowBytes.size()) {
        rowBytes.resize(jpegRowByteCount);
      }
    }

    {
      using Nuclex::Pixels::PixelFormats::PixelFormatConverter;

      PixelFormatConverter::ConvertRowFunction *convertRow = (
        PixelFormatConverter::GetRowConverter(storagePixelFormat, memory.PixelFormat)
      );

      // Begin decompression, this will update output_width and output_height,
      // usually to the same as image_width, image_height unless scaling is set up.
      ::boolean startedWithoutSuspension = ::jpeg_start_decompress(&commonInfo);
      if(startedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Input file truncated");
      }

      // Let LibJPEG load the image successively row-by-row and convert each
      // row from the temporary buffer into the correct location in the Bitmap's memory
      std::uint8_t *targetRowStart = (
        reinterpret_cast<std::uint8_t *>(memory.Pixels)
      );
      for(std::size_t rowIndex = 0; rowIndex < memory.Height; ++rowIndex) {
        std::uint8_t *intermediateRowAddress = rowBytes.data();
        ::JDIMENSION readScanlineCount = ::jpeg_read_scanlines(
          &commonInfo, &intermediateRowAddress, 1
        );
        if(readScanlineCount != 1) {
          throw Nuclex::Pixels::Errors::FileFormatError(
            u8"Unknown error reading scanline from jpeg"
          );
        }

        convertRow(
          rowBytes.data(), // + CountBitsPerPixel(storagePixelFormat),
          targetRowStart, // + CountBitsPerPixel(memory.PixelFormat),
          memory.Width
        );

        targetRowStart += memory.Stride;
      }

      // Finish decompression. This does some additional sanity checks, verifying that
      // the image was decompressed completely and reading the input stream up to the EOI
      // market (in case it contains multiple images).
      ::boolean endedWithoutSuspension = ::jpeg_finish_decompress(&commonInfo);
      if(endedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw Nuclex::Pixels::Errors::FileFormatError(u8"Input file truncated");
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  std::optional<Bitmap> JpegBitmapCodec::TryLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    {
      ::jpeg_decompress_struct commonInfo;

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      errorManager.output_message = &discardJpegMessage;
      commonInfo.err = &errorManager;

      ::jpeg_create_decompress(&commonInfo);
      ON_SCOPE_EXIT {
        ::jpeg_destroy_decompress(&commonInfo);
      };

      {
        // Set up a custom data source that reads from a virtual file
        JpegReadEnvironment virtualFileSource(source);
        commonInfo.src = &virtualFileSource;

        // If the file is too small for even the JPEG/JFIF header, bail out
        if(virtualFileSource.Length < SmallestPossibleJpegSize) {
          return std::optional<Bitmap>();
        }

        // Do the first fill ourselves so we can check the file's identity
        // and exit early if it doesn't look like a JPEG file
        virtualFileSource.fill_input_buffer(&commonInfo);
        if(!Helpers::IsValidJpegHeader(virtualFileSource.Buffer)) {
          return std::optional<Bitmap>(); // file header did not indicate a JPEG file
        }

        // Finally, we can read the JPEG file header to get file infos
        int result = ::jpeg_read_header(&commonInfo, TRUE);
        if(result != JPEG_HEADER_OK) {
          throw Errors::FileFormatError(u8"libjpeg failed to read the file header");
        }

        // Get the target pixel format and possibly adjust the output format LibJPEG
        // will decode the image as so it is compatible to the selected pixel format.
        PixelFormat storagePixelFormat = selectPixelFormatForLoad(commonInfo);

        // Normally, the output dimensions will only be known after jpeg_start_decompress(),
        // but we'd like to encapsulate the whole decompression stage into the load...()
        // methods, so to get the actual output image size, we have to call this:
        ::jpeg_calc_output_dimensions(&commonInfo);

        // Create the bitmap so we can directly decode into its pixel buffer
        Bitmap loadedBitmap(
          static_cast<std::size_t>(commonInfo.image_width),
          static_cast<std::size_t>(commonInfo.image_height),
          storagePixelFormat
        );

        // Since we created the bitmap with the matching pixel format, the decode can always
        // be performed without pixel format conversion
        loadJpegIntoBitmapMemoryDirect(commonInfo, loadedBitmap.Access());

        return std::optional<Bitmap>(std::move(loadedBitmap));
      } // jpegReadEnvironment scope
    } // commonInfo scope
  }

  // ------------------------------------------------------------------------------------------- //

  bool JpegBitmapCodec::TryReload(
    Bitmap &exactlyFittingBitmap,
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    {
      ::jpeg_decompress_struct commonInfo;

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      errorManager.output_message = &discardJpegMessage;
      commonInfo.err = &errorManager;

      ::jpeg_create_decompress(&commonInfo);
      ON_SCOPE_EXIT {
        ::jpeg_destroy_decompress(&commonInfo);
      };

      {
        // Set up a custom data source that reads from a virtual file
        JpegReadEnvironment virtualFileSource(source);
        commonInfo.src = &virtualFileSource;

        // If the file is too small for even the JPEG/JFIF header, bail out
        if(virtualFileSource.Length < 16) {
          return false;
        }

        // Do the first fill ourselves so we can check the file's identity
        // and exit early if it doesn't look like a JPEG file
        virtualFileSource.fill_input_buffer(&commonInfo);
        if(!Helpers::IsValidJpegHeader(virtualFileSource.Buffer)) {
          return false;
        }

        // Finally, we can read the JPEG file header to get file infos
        int result = ::jpeg_read_header(&commonInfo, TRUE);
        if(result != JPEG_HEADER_OK) {
          throw Errors::FileFormatError(u8"libjpeg failed to read the file header");
        }

        // Get the target pixel format and possibly adjust the output format LibJPEG
        // will decode the image as so it is compatible to the selected pixel format.
        PixelFormat storagePixelFormat = selectPixelFormatForLoad(commonInfo);

        // Normally, the output dimensions will only be known after jpeg_start_decompress(),
        // but we'd like to encapsulate the whole decompression stage into the load...()
        // methods, so to get the actual output image size, we have to call this:
        ::jpeg_calc_output_dimensions(&commonInfo);

        {
          std::size_t width = static_cast<std::size_t>(commonInfo.output_width);
          std::size_t height = static_cast<std::size_t>(commonInfo.output_height);

          const BitmapMemory &memory = exactlyFittingBitmap.Access();
          if((width != memory.Width) || (height != memory.Height)) {
            throw Errors::WrongSizeError(
              u8"Size of existing target Bitmap did not match the image file being loaded"
            );
          }
        }

        // Perform the actual load. If the pixel format of the provided bitmap matches
        // the pixel format of the .png file, we can do a direct load, otherwise we will
        // load the .png file row-by-row and convert the pixel format while copying.
        const BitmapMemory &memory = exactlyFittingBitmap.Access();
        if(memory.PixelFormat == storagePixelFormat) {
          loadJpegIntoBitmapMemoryDirect(commonInfo, memory);
        } else {
          loadJpegIntoBitmapMemoryWithConversion(commonInfo, storagePixelFormat, memory);
        }

        return true;
      } // jpegReadEnvironment scope
    } // commonInfo scope
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
