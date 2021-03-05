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
#define NUCLEX_STORAGE_SOURCE 1

#if defined(NUCLEX_STORAGE_HAVE_LZIP)

#include "../../../Source/Compression/LZip/LzmaDecompressor.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compressed string used to test the LZMA decompressor</summary>
  const std::uint8_t compressedString[] = {
    0x4c, 0x5a, 0x49, 0x50, 0x01, 0xfd, 0x00, 0x24, 0x19, 0x49, 0x98, 0x6f,
    0x10, 0x11, 0xc8, 0x5f, 0xe6, 0xd5, 0x8a, 0x97, 0x69, 0x8a, 0x3d, 0x59,
    0x3b, 0xd4, 0xfd, 0x2d, 0x54, 0x97, 0xac, 0x17, 0x79, 0xc6, 0x29, 0x94,
    0xe9, 0x80, 0x7c, 0x1a, 0xca, 0x60, 0xbf, 0xaf, 0xda, 0x40, 0xea, 0x27,
    0xe3, 0x3a, 0xcf, 0x31, 0x2e, 0x50, 0xd1, 0x89, 0x1d, 0x34, 0x74, 0xa5,
    0x35, 0x78, 0xaf, 0xff, 0xfd, 0xa2, 0x50, 0x00, 0xce, 0xe4, 0x2f, 0x9b,
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Text that should result went decompressing the test data</summary>
  const char uncompressedString[] =
    u8"Hello World, this is text that has been LZMA-compressed";

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaDecompressorTest, CanBeCreatedAndDestroyed) {
    EXPECT_NO_THROW(
      LzmaDecompressor decompressor;
      (void)decompressor;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaDecompressorTest, MemoryChunkCanBeDecompressed) {

    // Message that will be decompressed
    const std::uint8_t *input = compressedString;

    // Set up a buffer to hold the decompressed data + some extra space
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(uncompressedString) * 2);

    // Now decompress the data with the LZMA decompressor
    {
      LzmaDecompressor decompressor;

      std::size_t inputByteCount = sizeof(compressedString);
      std::size_t outputByteCount = outputBuffer.size();
      StopReason stopReason = decompressor.Process(
        input, inputByteCount, &outputBuffer[0], outputByteCount
      );
      EXPECT_EQ(stopReason, StopReason::InputBufferExhausted);

      std::size_t secondOutputByteCount = outputBuffer.size() - outputByteCount;
      stopReason = decompressor.Finish(&outputBuffer[outputByteCount], secondOutputByteCount);
      EXPECT_EQ(stopReason, StopReason::Finished);

      outputBuffer.resize(outputByteCount + secondOutputByteCount);
    }

    ASSERT_EQ(outputBuffer.size(), sizeof(uncompressedString));
    for(std::size_t index = 0; index < sizeof(uncompressedString); ++index) {
      EXPECT_EQ(static_cast<char>(outputBuffer[index]), uncompressedString[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::LZip

#endif // defined(NUCLEX_STORAGE_HAVE_LZIP)
