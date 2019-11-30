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

#include "OpenExrHelpers.h"

#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)

#include "Nuclex/Pixels/Errors/FileFormatError.h"

#include <cassert>
#include <algorithm>

namespace Nuclex { namespace Pixels { namespace Storage { namespace Exr {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::IsValidExrHeader(const std::uint8_t *fileHeader) {

    // This essentially does the same as GenericInputFile::readMagicNumberAndVersionField(),
    // but doesn't throw an exception if the file doesn't look like an OpenEXR image.
    return (
      (fileHeader[0] == 0x76) &&         // 1 MAGIC (little endian magic integer)
      (fileHeader[1] == 0x2f) &&         // 1
      (fileHeader[2] == 0x31) &&         // 1
      (fileHeader[3] == 0x01) &&         // 1
      (fileHeader[4] == 0x02) &&         // 2 EXR_VERSION (file format version)
      (
        (fileHeader[5] == 0x00) &&       // 3 Flags
        ((fileHeader[6] & 0xE1) == 0) && // 3
        (fileHeader[7] == 0x00)          // 3
      )
    );

  }

  // ------------------------------------------------------------------------------------------- //

  void Helpers::AddChannelsToFrameBuffer(
    Imf::FrameBuffer &frameBuffer, PixelFormat pixelFormat, void *pixels
  ) {
    static const std::string redChannelName("R", 1);
    static const std::string greenChannelName("G", 1);
    static const std::string blueChannelName("B", 1);
    static const std::string alphaChannelName("A", 1);
    
    // TODO: I haven't filled this yet because I'm remaking the whole PixelFormat enum
    switch(pixelFormat) {
      case PixelFormat::R16_G16_B16_A16_Float: {
        std::uint16_t *pixelsAsHalf = reinterpret_cast<std::uint16_t *>(pixels);
        char *redBaseAddress = reinterpret_cast<char *>(&pixelsAsHalf[0]);
        char *greenBaseAddress = reinterpret_cast<char *>(&pixelsAsHalf[1]);
        char *blueBaseAddress = reinterpret_cast<char *>(&pixelsAsHalf[2]);
        char *alphaBaseAddress = reinterpret_cast<char *>(&pixelsAsHalf[3]);
        frameBuffer.insert(redChannelName, Imf::Slice(Imf::HALF, redBaseAddress, 64, 0));
        frameBuffer.insert(greenChannelName, Imf::Slice(Imf::HALF, greenBaseAddress, 64, 0));
        frameBuffer.insert(blueChannelName, Imf::Slice(Imf::HALF, blueBaseAddress, 64, 0));
        frameBuffer.insert(alphaChannelName, Imf::Slice(Imf::HALF, alphaBaseAddress, 64, 0));
        return;
      }
      default: {
        throw Errors::FileFormatError(u8"Requested pixel format not supported by OpenEXR");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Exr

#endif // defined(NUCLEX_PIXELS_HAVE_OPENEXR)
