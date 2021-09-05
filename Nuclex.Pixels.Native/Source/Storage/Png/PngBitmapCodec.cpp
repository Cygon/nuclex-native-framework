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

#include "LibPngHelpers.h"

#include <png.h> // libpng main header

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

  /// <summary>RAII helper class that frees a PNG struct again</summary>
  class PngReadScope {

    /// <summary>Initializes a new png_struct deleter</summary>
    /// <param name="pngStruct">
    ///   PNG main structure that should be deleted on scope exit
    /// </param>
    public: PngReadScope(::png_struct *pngStruct) :
      pngStruct(pngStruct) {}

    /// <summary>Frees the PNG main structure</summary>
    public: ~PngReadScope() {
      ::png_destroy_read_struct(&this->pngStruct, nullptr, nullptr);
    }

    /// <summary>PNG main structure that will be deleted</summary>
    private: ::png_struct *pngStruct;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper class that frees a PNG struct again</summary>
  class PngInfoScope {

    /// <summary>Initializes a new png_struct deleter</summary>
    /// <param name="pngStruct">PNG main structure needed for the API call</param>
    /// <param name="pngInfo">
    ///   PNG infomration structure that should be deleted on scope exit
    /// </param>
    public: PngInfoScope(const ::png_struct *pngStruct, ::png_info *pngInfo) :
      pngStruct(pngStruct),
      pngInfo(pngInfo) {}

    /// <summary>Frees the PNG information structure</summary>
    public: ~PngInfoScope() {
      ::png_destroy_info_struct(this->pngStruct, &this->pngInfo);
    }

    /// <summary>PNG main structure required for the API call</summary>
    private: const ::png_struct *pngStruct;
    /// <summary>PNG info structure that will be deleted</summary>
    private: ::png_info *pngInfo;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  PngBitmapCodec::PngBitmapCodec() :
    name(u8"Portable Network Graphics (.png) via libpng") {
    this->knownFileExtensions.push_back(u8"png");
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<BitmapInfo> PngBitmapCodec::TryReadInfo(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    // If this doesn't look like a .png file, bail out immediately
    if(!Helpers::CheckIfPngHeaderPresent(source)) {
      return std::optional<BitmapInfo>();
    }

    // Allocate the main LibPNG structure. It contains all pointers to user-defined
    // functions (IO, error handling and custom chunk processing, etc.)
    ::png_struct *pngRead = ::png_create_read_struct(
      PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
    );
    if(pngRead == nullptr) {
      throw std::bad_alloc();
    }
    {
      PngReadScope pngReadScope(pngRead);

      // Install a custom error handler function that simply throws a C++ exception.
      // LibPNG is one of the few C libraries designed to allow exceptions passing through.
      ::png_set_error_fn(pngRead, nullptr, &handlePngError, &handlePngWarning);

      // We also need the info structure. This holds all importing informations describing
      // the image's dimensions, pixel format, palette, gamma etc.
      ::png_info *pngInfo = ::png_create_info_struct(pngRead);
      if(pngInfo == nullptr) {
        throw std::bad_alloc();
      }
      {
        PngInfoScope pngInfoScope(pngRead, pngInfo);

        // Install a custom read function. This is used to read data from the virtual
        // file. The read environment emulates a file cursor.
        PngReadEnvironment environment(*pngRead, source);

        // Now we're ready for actually accessing a PNG file, attempt to obtain the image's
        // resolution, pixel format and so on
        ::png_read_info(pngRead, pngInfo);

        BitmapInfo result;
        result.Width = ::png_get_image_width(pngRead, pngInfo);
        result.Height = ::png_get_image_height(pngRead, pngInfo);
        result.PixelFormat = Helpers::GetClosestPixelFormat(*pngRead, *pngInfo);
        result.MemoryUsage = (
          (CountRequiredBytes(result.PixelFormat, result.Width) * result.Height) +
          (sizeof(std::intptr_t) * 3) +
          (sizeof(std::size_t) * 3) +
          (sizeof(int) * 2)
        );
        return result;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool PngBitmapCodec::CanLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {

    // If a file extension is offered, do an early exit if it doesn't match.
    // Should the codec be used through the BitmapSerializer (which is very likely
    // always the case), the extension will either match or be missing.
    bool mightBePng;
    {
      std::size_t hintLength = extensionHint.length();
      if(hintLength == 3) { // extension without dot possible
        mightBePng = (
          ((extensionHint[0] == 'p') || (extensionHint[0] == 'P')) &&
          ((extensionHint[1] == 'n') || (extensionHint[1] == 'N')) &&
          ((extensionHint[2] == 'g') || (extensionHint[2] == 'G'))
        );
      } else if(hintLength == 4) { // extension with dot possible
        mightBePng = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 'p') || (extensionHint[1] == 'P')) &&
          ((extensionHint[2] == 'n') || (extensionHint[2] == 'N')) &&
          ((extensionHint[3] == 'g') || (extensionHint[3] == 'G'))
        );
      } else if(extensionHint.empty()) { // extension missing
        mightBePng = true;
      } else { // extension wrong
        mightBePng = false;
      }
    }

    // If the extension indicates a PNG file (or no extension was provided),
    // check the file header to see if this is really a PNG file
    if(mightBePng) {
      std::uint64_t fileLength = source.GetSize();
      if(fileLength >= SmallestPossiblePngSize) {
        std::uint8_t fileHeader[16];
        source.ReadAt(0, 16, fileHeader);
        return (::png_sig_cmp(fileHeader, 0, 16) == 0);
      } else { // file is too short to be a PNG
        return false;
      }
    } else { // wrong file extension
      return false;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif //defined(NUCLEX_PIXELS_HAVE_LIBPNG)
