#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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
#include "LibJpegHelpers.h"

#include <cassert>
#include <algorithm>

#include <jpeglib.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Handles an error inside libjpeg</summary>
  /// <param name="info">Main structure containing all libjpeg configuration</param>
  /// <remarks>
  ///   <para>
  ///     libjpeg  is a C library, but its error handling scheme expects this function to
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

  /// <summary>RAII helper that frees a JPEG decompression structure upon scope exit</summary>
  class JpegDecompressStructScope {

    /// <summary>Frees a JPEG decompression structure upon termination</summary>
    /// <param name="commonInfo">JPEG decompression structure that will be freed</summary>
    public: JpegDecompressStructScope(::jpeg_decompress_struct *commonInfo) :
      commonInfo(commonInfo) {}

    /// <summary>Frees the JPEG decompression structure</summary>
    public: ~JpegDecompressStructScope() {
      ::jpeg_destroy_decompress(this->commonInfo);
    }

    /// <summary>JPEG decompression structure that will be freed</summary>
    private: ::jpeg_decompress_struct *commonInfo;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper that frees a JPEG compression structure upon scope exit</summary>
  class JpegCompressStructScope {

    /// <summary>Frees a JPEG compression structure upon termination</summary>
    /// <param name="commonInfo">JPEG compression structure that will be freed</summary>
    public: JpegCompressStructScope(::jpeg_compress_struct *commonInfo) :
      commonInfo(commonInfo) {}

    /// <summary>Frees the JPEG compression structure</summary>
    public: ~JpegCompressStructScope() {
      ::jpeg_destroy_compress(this->commonInfo);
    }

    /// <summary>JPEG compression structure that will be freed</summary>
    private: ::jpeg_compress_struct *commonInfo;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  JpegBitmapCodec::JpegBitmapCodec() :
    name(u8"Joint Photography Experts Group File Interchange Format(.jpeg) via libjpeg") {
    this->knownFileExtensions.push_back(u8"jpg");
    this->knownFileExtensions.push_back(u8"jpeg");
    this->knownFileExtensions.push_back(u8"jpe");
  }

  // ------------------------------------------------------------------------------------------- //

  bool JpegBitmapCodec::CanLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {

    // If a file extension is offered, do an early exit if it doesn't match.
    // Should the codec be used through the BitmapSerializer (which is very likely
    // always the case), the extension will either match or be missing.
    bool mightBeJpeg;
    {
      std::size_t hintLength = extensionHint.length();
      if(hintLength == 3) { // extension without dot possible
        mightBeJpeg = (
          ((extensionHint[0] == 'j') || (extensionHint[0] == 'J')) &&
          ((extensionHint[1] == 'p') || (extensionHint[1] == 'P')) &&
          (
            ((extensionHint[2] == 'e') || (extensionHint[2] == 'E')) ||
            ((extensionHint[2] == 'g') || (extensionHint[2] == 'G'))
          )
        );
      } else if(hintLength == 4) { // extension with dot or long form possible
        mightBeJpeg = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 'j') || (extensionHint[1] == 'J')) &&
          ((extensionHint[2] == 'p') || (extensionHint[2] == 'P')) &&
          (
            ((extensionHint[3] == 'e') || (extensionHint[3] == 'E')) ||
            ((extensionHint[3] == 'g') || (extensionHint[3] == 'G'))
          )
        );
        mightBeJpeg |= (
          ((extensionHint[0] == 'j') || (extensionHint[0] == 'J')) &&
          ((extensionHint[1] == 'p') || (extensionHint[1] == 'P')) &&
          ((extensionHint[2] == 'e') || (extensionHint[2] == 'E')) &&
          ((extensionHint[3] == 'g') || (extensionHint[3] == 'G'))
        );
      } else if(hintLength == 5) { // long form with dot possible
        mightBeJpeg = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 'j') || (extensionHint[1] == 'J')) &&
          ((extensionHint[2] == 'p') || (extensionHint[2] == 'P')) &&
          ((extensionHint[3] == 'e') || (extensionHint[3] == 'E')) &&
          ((extensionHint[4] == 'g') || (extensionHint[4] == 'G'))
        );
      } else if(extensionHint.empty()) { // extension missing
        mightBeJpeg = true;
      } else { // extension wrong
        mightBeJpeg = false;
      }
    }

    // If the extension indicates a JPEG file (or no extension was provided),
    // check the file header to see if this is really a JPEG file
    if(mightBeJpeg) {
      std::size_t fileLength = source.GetSize();
      if(fileLength >= SmallestPossibleJpegSize) {
        std::uint8_t fileHeader[16];
        source.ReadAt(0, 16, fileHeader);
        return Helpers::IsValidJpegHeader(fileHeader);
      } else { // file is too short to be a PNG
        return false;
      }
    } else { // extension was wrong
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool JpegBitmapCodec::CanSave() const {
    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  BitmapInfo JpegBitmapCodec::TryReadInfo(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    ::jpeg_decompress_struct commonInfo;
    ::jpeg_create_decompress(&commonInfo);
    {
      JpegDecompressStructScope decompressScope(&commonInfo);

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      commonInfo.err = &errorManager;

      // Set up a custom data source that reads from a virtual file
      JpegReadEnvironment virtualFileSource(source);
      commonInfo.src = &virtualFileSource;

      // If the file is too small to be a JPEG image, bail out
      if(virtualFileSource.Length < SmallestPossibleJpegSize) {
        BitmapInfo result;
        result.Loadable = false;
        return result;
      }

      // Do the first fill ourselves so we can check the file's identity
      // and exit early if it doesn't look like a JPEG file
      virtualFileSource.fill_input_buffer(&commonInfo);
      if(!Helpers::IsValidJpegHeader(virtualFileSource.Buffer)) {
        BitmapInfo result;
        result.Loadable = false;
        return result;
      }

      // Finally, we can read the JPEG file header to get file infos
      int result = ::jpeg_read_header(&commonInfo, TRUE);
      if(result != JPEG_HEADER_OK) {
        throw Errors::FileFormatError(u8"libjpeg failed to read the file header");
      }

      // Create an information structure holding the informations we found
      BitmapInfo info;
      info.Loadable = true;
      info.Width = static_cast<std::size_t>(commonInfo.image_width);
      info.Height = static_cast<std::size_t>(commonInfo.image_height);
      info.PixelFormat = Helpers::GetEquivalentPixelFormat(commonInfo);
      info.MemoryUsage = (
        (CountRequiredBytes(info.PixelFormat, info.Width) * info.Height) +
        (sizeof(std::intptr_t) * 3) +
        (sizeof(std::size_t) * 3) +
        (sizeof(int) * 2)
      );
      return info;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  OptionalBitmap JpegBitmapCodec::TryLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    ::jpeg_decompress_struct commonInfo;
    ::jpeg_create_decompress(&commonInfo);
    {
      JpegDecompressStructScope decompressScope(&commonInfo);

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      commonInfo.err = &errorManager;

      // Set up a custom data source that reads from a virtual file
      JpegReadEnvironment virtualFileSource(source);
      commonInfo.src = &virtualFileSource;

      // If the file is too small for even the JPEG/JFIF header, bail out
      if(virtualFileSource.Length < SmallestPossibleJpegSize) {
        return OptionalBitmap();
      }

      // Do the first fill ourselves so we can check the file's identity
      // and exit early if it doesn't look like a JPEG file
      virtualFileSource.fill_input_buffer(&commonInfo);
      if(!Helpers::IsValidJpegHeader(virtualFileSource.Buffer)) {
        return OptionalBitmap(); // file header did not indicate a JPEG file
      }

      // Finally, we can read the JPEG file header to get file infos
      int result = ::jpeg_read_header(&commonInfo, TRUE);
      if(result != JPEG_HEADER_OK) {
        throw std::runtime_error(u8"libjpeg failed to read the file header");
      }

      // TODO: Use fastest color space and copy/convert to Bitmap
      //       Bitmap of type returned by TryReadInfo() MUST be able to hold this data!
      // Force libjpeg to convert to 24 bit RGB for us
      commonInfo.output_components = 3;
      commonInfo.out_color_space = JCS_RGB;

      // Begin decompression, this will update output_width and output_height,
      // usually to the same as image_width, image_height unless scaling is set up.
      ::boolean startedWithoutSuspension = ::jpeg_start_decompress(&commonInfo);
      if(startedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw std::runtime_error(u8"Input file truncated");
      }

      // Create the bitmap so we can directly decode into its pixel buffer 
      Bitmap decodedBitmap(
        static_cast<std::size_t>(commonInfo.output_width),
        static_cast<std::size_t>(commonInfo.output_height),
        PixelFormat::R8_G8_B8_Unsigned
      );
      const BitmapMemory &memory = decodedBitmap.Access();

      // Read the bitmap scanline by scanline. The function can also take an array of
      // scanlines, which may be faster than decoding line-by-line, but this is
      // the most straightforward way to do it.
      std::uint8_t *currentRowPointer = reinterpret_cast<std::uint8_t *>(memory.Pixels);
      while(commonInfo.output_scanline < commonInfo.output_height) {
        JDIMENSION readScanlineCount = ::jpeg_read_scanlines(&commonInfo, &currentRowPointer, 1);
        if(readScanlineCount != 1) {
          throw std::runtime_error(u8"Unknown error reading scanline from jpeg");
        }
        currentRowPointer += memory.Stride;
      }

      // Finish decompression. This does some additional sanity checks, verifying that
      // the image was decompressed completely and reading the input stream up to the EOI
      // market (in case it contains multiple images).
      ::boolean endedWithoutSuspension = ::jpeg_finish_decompress(&commonInfo);
      if(endedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw std::runtime_error(u8"Input file truncated");
      }

      return OptionalBitmap(std::move(decodedBitmap));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool JpegBitmapCodec::TryReload(
    Bitmap &exactlyFittingBitmap,
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    ::jpeg_decompress_struct commonInfo;
    ::jpeg_create_decompress(&commonInfo);
    {
      JpegDecompressStructScope decompressScope(&commonInfo);

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      commonInfo.err = &errorManager;

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
        throw std::runtime_error(u8"libjpeg failed to read the file header");
      }

      // TODO: Use fastest color space and copy/convert to Bitmap
      //       Bitmap of type returned by TryReadInfo() MUST be able to hold this data!
      // Force libjpeg to convert to 24 bit RGB for us
      commonInfo.output_components = 3;
      commonInfo.out_color_space = JCS_RGB;

      // Begin decompression, this will update output_width and output_height,
      // usually to the same as image_width, image_height unless scaling is set up.
      ::boolean startedWithoutSuspension = ::jpeg_start_decompress(&commonInfo);
      if(startedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw std::runtime_error(u8"Input file truncated");
      }

      const BitmapMemory &memory = exactlyFittingBitmap.Access();
      bool matchesExpectations = (
        (memory.Width == commonInfo.output_width) &&
        (memory.Height == commonInfo.output_height) &&
        (memory.PixelFormat == PixelFormat::R8_G8_B8_Unsigned)
      );
      if(!matchesExpectations) {
        throw std::runtime_error(
          u8"Bitmap provided to Reload() does not have a correct dimensions and pixel format"
        );
      }

      // Read the bitmap scanline by scanline. The function can also take an array of
      // scanlines, which may be faster than decoding line-by-line, but this is
      // the most straightforward way to do it.
      std::uint8_t *currentRowPointer = reinterpret_cast<std::uint8_t *>(memory.Pixels);
      while(commonInfo.output_scanline < commonInfo.output_height) {
        JDIMENSION readScanlineCount = ::jpeg_read_scanlines(&commonInfo, &currentRowPointer, 1);
        if(readScanlineCount != 1) {
          throw std::runtime_error(u8"Unknown error reading scanline from jpeg");
        }
        currentRowPointer += memory.Stride;
      }

      // Finish decompression. This does some additional sanity checks, verifying that
      // the image was decompressed completely and reading the input stream up to the EOI
      // market (in case it contains multiple images).
      ::boolean endedWithoutSuspension = ::jpeg_finish_decompress(&commonInfo);
      if(endedWithoutSuspension == FALSE) { // decompressor was suspended -- we don't support this
        throw std::runtime_error(u8"Input file truncated");
      }

      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void JpegBitmapCodec::Save(const Bitmap &bitmap, VirtualFile &target) const {
    (void)bitmap;
    (void)target;

    ::jpeg_compress_struct commonInfo;
    ::jpeg_create_compress(&commonInfo);
    {
      JpegCompressStructScope decompressScope(&commonInfo);
    }

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
