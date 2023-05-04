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

#include "TiffBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "LibTiffHelpers.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper class that frees a TIFF file again</summary>
  class TiffFileScope {

    /// <summary>Initializes a new TIFF file deleter</summary>
    /// <param name="tiffFile">
    ///   TIFF file IO structure that should be deleted on scope exit
    /// </param>
    public: TiffFileScope(::TIFF *tiffFile) :
      tiffFile(tiffFile) {}
    
    /// <summary>Frees the TIFF file io structure</summary>
    public: ~TiffFileScope() {
      Nuclex::Pixels::Storage::Tiff::Helpers::Close(this->tiffFile);
    }

    /// <summary>TIFF file IO structure that will be deleted</summary>
    private: ::TIFF *tiffFile;

  };

#if 0
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
#endif
} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  TiffBitmapCodec::TiffBitmapCodec() :
    name(u8"Tag Image File Format (.tif) via libtiff") {
    this->knownFileExtensions.push_back(u8"tif");
    this->knownFileExtensions.push_back(u8"tiff");
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<BitmapInfo> TiffBitmapCodec::TryReadInfo(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // If the file is too small to be a PNG file, bail out
    if(source.GetSize() < SmallestPossibleTiffSize) {
      return std::optional<BitmapInfo>();
    }

    // If the file header is not indicative of a PNG file, bail out
    {
      std::uint8_t fileHeader[16];
      source.ReadAt(0, 16, fileHeader);
      if(!Helpers::IsValidTiffHeader(fileHeader)) {
        return std::optional<BitmapInfo>();
      }
    }

    std::uint32_t width, height;
    {
      TIFF *tiffFile = Helpers::OpenForReading(source);
      TiffFileScope tiffFileScope(tiffFile);

      int result = TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &width);
      if(result != 1) {
        throw Errors::FileFormatError(u8"TIFF file has no image width tag. Corrupt file?");
      }
      result = TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &height);
      if(result != 1) {
        throw Errors::FileFormatError(u8"TIFF file hs no image height tag. Corrupt file?");
      }
    }

    BitmapInfo info;
    info.Width = static_cast<std::size_t>(width);
    info.Height = static_cast<std::size_t>(height);
    info.PixelFormat = PixelFormat::R8_G8_B8_A8_Unsigned;
    info.MemoryUsage = info.Width * info.Height * 4;

    return info;
  }

  // ------------------------------------------------------------------------------------------- //

  bool TiffBitmapCodec::CanLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {

    // If a file extension is offered, do an early exit if it doesn't match.
    // Should the codec be used through the BitmapSerializer (which is very likely
    // always the case), the extension will either match or be missing.
    bool mightBeTiff;
    {
      std::size_t hintLength = extensionHint.length();
      if(hintLength == 3) { // extension without dot possible
        mightBeTiff = (
          ((extensionHint[0] == 't') || (extensionHint[0] == 'T')) &&
          ((extensionHint[1] == 'i') || (extensionHint[1] == 'I')) &&
          ((extensionHint[2] == 'f') || (extensionHint[2] == 'F'))
        );
      } else if(hintLength == 4) { // extension with dot or long possible
        mightBeTiff = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 't') || (extensionHint[1] == 'T')) &&
          ((extensionHint[2] == 'i') || (extensionHint[2] == 'I')) &&
          ((extensionHint[3] == 'f') || (extensionHint[3] == 'F'))
        );
        mightBeTiff |= (
          ((extensionHint[0] == 't') || (extensionHint[0] == 'T')) &&
          ((extensionHint[1] == 'i') || (extensionHint[1] == 'I')) &&
          ((extensionHint[2] == 'f') || (extensionHint[2] == 'F')) &&
          ((extensionHint[3] == 'f') || (extensionHint[3] == 'F'))
        );
      } else if(hintLength == 5) { // extension with dot and long possible
        mightBeTiff = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 't') || (extensionHint[1] == 'T')) &&
          ((extensionHint[2] == 'i') || (extensionHint[2] == 'I')) &&
          ((extensionHint[3] == 'f') || (extensionHint[3] == 'F')) &&
          ((extensionHint[4] == 'f') || (extensionHint[4] == 'F'))
        );
      } else if(extensionHint.empty()) { // extension missing
        mightBeTiff = true;
      } else { // extension wrong
        mightBeTiff = false;
      }
    }

    // If the extension indicates a TIFF file (or no extension was provided),
    // check the file header to see if this is really a TIFF file
    if(mightBeTiff) {
      std::size_t fileLength = source.GetSize();
      if(fileLength >= SmallestPossibleTiffSize) {
        std::uint8_t fileHeader[16];
        source.ReadAt(0, 16, fileHeader); // We only need 8
        return Helpers::IsValidTiffHeader(fileHeader);
      } else { // file is too short to be a TIFF
        return false;
      }
    } else { // wrong file extension
      return false;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  bool TiffBitmapCodec::CanSave() const {
    return false; // Still working on this...
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<Bitmap> TiffBitmapCodec::TryLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    std::uint32_t width, height;
    {
      TIFF *tiffFile = Helpers::OpenForReading(source);
      TiffFileScope tiffFileScope(tiffFile);

      int result = ::TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &width);
      if(result == 0) {
        throw Errors::FileFormatError(u8"TIFF file has no image width tag. Corrupt file?");
      }
      result = ::TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &height);
      if(result == 0) {
        throw Errors::FileFormatError(u8"TIFF file has no image height tag. Corrupt file?");
      }
    }

    Bitmap loadedBitmap(width, height);
    return std::optional<Bitmap>(std::move(loadedBitmap));
  }

  // ------------------------------------------------------------------------------------------- //

  bool TiffBitmapCodec::TryReload(
    Bitmap &exactlyFittingBitmap,
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)exactlyFittingBitmap;
    (void)source;
    (void)extensionHint;
    
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void TiffBitmapCodec::Save(
    const Bitmap &bitmap, VirtualFile &target,
    float compressionStrengthHint /* = 0.75f */, float outputQualityHint /* = 0.95f */
  ) const {
    (void)bitmap;
    (void)target;
    (void)compressionStrengthHint;
    (void)outputQualityHint;

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif //defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
