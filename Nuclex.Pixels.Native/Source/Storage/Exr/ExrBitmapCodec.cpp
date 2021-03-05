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

#include "ExrBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)

// OpenEXR:
// - is non endian-aware
// - assumes int is 32 bits or the whole file format breaks
// - assumes 2 byte alignment in its Rgba type or the whole file format breaks
#if defined(NUCLEX_PIXELS_BIG_ENDIAN) || !defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
#error The OpenEXR library does not support big endian, please unset HAVE_OPENEXR
#endif

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "OpenExrHelpers.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Exr {

  // ------------------------------------------------------------------------------------------- //

  ExrBitmapCodec::ExrBitmapCodec() :
    name(u8"Extended Range (.exr) via OpenEXR") {
    this->knownFileExtensions.push_back(u8"exr");
  }

  // ------------------------------------------------------------------------------------------- //

  bool ExrBitmapCodec::CanLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {

    // If a file extension is offered, do an early exit if it doesn't match.
    // Should the codec be used through the BitmapSerializer (which is very likely
    // always the case), the extension will either match or be missing.
    bool mightBeExr;
    {
      std::size_t hintLength = extensionHint.length();
      if(hintLength == 3) { // extension without dot possible
        mightBeExr = (
          ((extensionHint[0] == 'e') || (extensionHint[0] == 'E')) &&
          ((extensionHint[1] == 'x') || (extensionHint[1] == 'X')) &&
          ((extensionHint[2] == 'r') || (extensionHint[2] == 'R'))
        );
      } else if(hintLength == 4) { // extension with dot possible
        mightBeExr = (
          (extensionHint[0] == '.') &&
          ((extensionHint[1] == 'e') || (extensionHint[1] == 'E')) &&
          ((extensionHint[2] == 'x') || (extensionHint[2] == 'X')) &&
          ((extensionHint[3] == 'r') || (extensionHint[3] == 'R'))
        );
      } else if(extensionHint.empty()) { // extension missing
        mightBeExr = true;
      } else { // extension wrong
        mightBeExr = false;
      }
    }

    // If the extension indicates an .exr file (or no extension was provided),
    // check the file header to see if this is really an .exr file
    if(mightBeExr) {
      std::size_t fileLength = source.GetSize();
      if(fileLength >= SmallestPossibleExrSize) {
        std::uint8_t fileHeader[16];
        source.ReadAt(0, 16, fileHeader); // Yes, we read a few bytes more than needed.
        return Helpers::IsValidExrHeader(fileHeader);
      } else { // file is too short to be an .exr file
        return false;
      }
    } else { // wrong file extension
      return false;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  bool ExrBitmapCodec::CanSave() const {
    return false; // Still working in this...
  }

  // ------------------------------------------------------------------------------------------- //

  BitmapInfo ExrBitmapCodec::TryReadInfo(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    VirtualFileInputStream inputStream(source);
    try {
      Imf::InputFile inputFile(inputStream);

      const Imath::Box2i &dataWindow = inputFile.header().dataWindow();

      BitmapInfo result;
      result.Loadable = true;
      result.Width = static_cast<std::size_t>(dataWindow.max.x - dataWindow.min.x + 1);
      result.Height = static_cast<std::size_t>(dataWindow.max.y - dataWindow.min.y + 1);
      result.PixelFormat = PixelFormat::R16_G16_B16_A16_Float; // Always. See Imf::Rgba
      result.MemoryUsage = (
        (CountRequiredBytes(result.PixelFormat, result.Width) * result.Height) +
        (sizeof(std::intptr_t) * 3) +
        (sizeof(std::size_t) * 3) +
        (sizeof(int) * 2)
      );

      return result;
    }
    catch(const Iex::BaseExc &error) { // Convert exception to a FileFormatError
      throw Errors::FileFormatError(error.message());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  OptionalBitmap ExrBitmapCodec::TryLoad(
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint; // Unused

    VirtualFileInputStream inputStream(source);
    try {
      Imf::InputFile inputFile(inputStream);

      const Imath::Box2i &dataWindow = inputFile.header().dataWindow();
      std::size_t imageWidth = dataWindow.max.x - dataWindow.min.x + 1;
      std::size_t imageHeight = dataWindow.max.y - dataWindow.min.y + 1;

      // CHECK: OpenEXR has some intermedia 'YCA' format. When does this apply?

      // TODO: Should be 16 bit float, not 8 bit signed integer
      Bitmap result(imageWidth, imageHeight, PixelFormat::R16_G16_B16_A16_Float);
      const BitmapMemory &memory = result.Access();

      Imf::FrameBuffer frameBuffer;
      Helpers::AddChannelsToFrameBuffer(frameBuffer, memory.PixelFormat, memory.Pixels);
      inputFile.setFrameBuffer(frameBuffer);
      inputFile.readPixels(dataWindow.min.y, dataWindow.max.y);

      return OptionalBitmap(std::move(result));
    }
    catch(const Iex::BaseExc &error) { // Convert exception to a FileFormatError
      throw Errors::FileFormatError(error.message());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool ExrBitmapCodec::TryReload(
    Bitmap &exactlyFittingBitmap,
    const VirtualFile &source, const std::string &extensionHint /* = std::string() */
  ) const {
    (void)exactlyFittingBitmap;
    (void)extensionHint;

    (void)extensionHint; // Unused

    VirtualFileInputStream inputStream(source);
    try {
      Imf::InputFile inputFile(inputStream);

      const Imath::Box2i &dataWindow = inputFile.header().dataWindow();
      std::size_t imageWidth = dataWindow.max.x - dataWindow.min.x + 1;
      std::size_t imageHeight = dataWindow.max.y - dataWindow.min.y + 1;

      const BitmapMemory &memory = exactlyFittingBitmap.Access();
      bool sizeMatches = (
        (imageWidth == memory.Width) &&
        (imageHeight == memory.Height)
      );
      if(!sizeMatches) {
        throw std::runtime_error(u8"Provided bitmap does not have the correct dimensions");
      }

      // If OpenEXR works correctly, we can load:
      // - Any R, G, B, A order or subset
      // - UINT, HALF and FLOAT (8, 16, 32 bits)
      Imf::FrameBuffer frameBuffer;
      Helpers::AddChannelsToFrameBuffer(frameBuffer, memory.PixelFormat, memory.Pixels);
      inputFile.setFrameBuffer(frameBuffer);
      inputFile.readPixels(dataWindow.min.y, dataWindow.max.y);
    }
    catch(const Iex::BaseExc &error) { // Convert exception to a FileFormatError
      throw Errors::FileFormatError(error.message());
    }

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void ExrBitmapCodec::Save(const Bitmap &bitmap, VirtualFile &target) const {
    (void)bitmap;
    (void)target;

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Exr

#endif //defined(NUCLEX_PIXELS_HAVE_OPENEXR)
