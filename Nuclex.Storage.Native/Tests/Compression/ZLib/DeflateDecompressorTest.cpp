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

#include "../../../Source/Compression/ZLib/DeflateDecompressor.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  #if ONLY_USE_THIS_IF_TRAILERS_USED_IN_COMPRESSION

  /// <summary>Compressed string used to test the deflate decompressor</summary>
  const std::uint8_t compressedStringWithTrailer[] = {
    0x78, 0x9c, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x08, 0xcf, 0x2f, 0xca,
    0x49, 0xd1, 0x51, 0x28, 0xc9, 0xc8, 0x2c, 0x56, 0x00, 0xa2, 0x92, 0xd4,
    0x8a, 0x12, 0x20, 0x27, 0xb1, 0x44, 0x21, 0x23, 0xb1, 0x58, 0x21, 0x29,
    0x35, 0x35, 0x4f, 0x21, 0x25, 0x35, 0x2d, 0x27, 0xb1, 0x24, 0x55, 0x37,
    0x39, 0x3f, 0xb7, 0xa0, 0x28, 0xb5, 0xb8, 0x38, 0x35, 0x85, 0x01, 0x00,
    0x78, 0x71, 0x15, 0x40
  };

  #endif

  /// <summary>Compressed string used to test the deflate decompressor</summary>
  const std::uint8_t compressedString[] = {
    0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x08, 0xcf, 0x2f, 0xca, 0x49, 0xd1,
    0x51, 0x28, 0xc9, 0xc8, 0x2c, 0x56, 0x00, 0xa2, 0x92, 0xd4, 0x8a, 0x12,
    0x20, 0x27, 0xb1, 0x44, 0x21, 0x23, 0xb1, 0x58, 0x21, 0x29, 0x35, 0x35,
    0x4f, 0x21, 0x25, 0x35, 0x2d, 0x27, 0xb1, 0x24, 0x55, 0x37, 0x39, 0x3f,
    0xb7, 0xa0, 0x28, 0xb5, 0xb8, 0x38, 0x35, 0x85, 0x01, 0x00
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Text that should result went decompressing the test data</summary>
  const char uncompressedString[] =
    u8"Hello World, this is text that has been deflate-compressed";

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  TEST(DeflateDecompressorTest, CanBeCreatedAndDestroyed) {
    EXPECT_NO_THROW(
      DeflateDecompressor decompressor;
      (void)decompressor;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DeflateDecompressorTest, MemoryChunkCanBeDecompressed) {

    // Message that will be compressed
    const std::uint8_t *input = compressedString;

    // Set up a buffer to hold the compressed data. We assume compression will not
    // make result in generated output that is more than twice the size of the input data :)
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(uncompressedString) * 2);

    // Now compress the data with the deflate compressor
    {
      DeflateDecompressor decompressor;

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

}}}} // namespace Nuclex::Storage::Compression::ZLib
