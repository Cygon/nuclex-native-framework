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

#include "../../../Source/Compression/Csc/CscDecompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compressed string used to test the CSC decompressor</summary>
  const std::uint8_t compressedString[] = {
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x80, 0x00,
    0x00, 0x3d, 0x00, 0xdb, 0xe5, 0xdf, 0x30, 0xa3, 0xb4, 0xd7, 0x9f, 0xa2,
    0xb4, 0x9f, 0x41, 0xb9, 0x46, 0x34, 0x8b, 0xc1, 0x2f, 0x6e, 0x56, 0x65,
    0x7f, 0x8f, 0xf7, 0x4b, 0x91, 0x69, 0x9f, 0x7b, 0xeb, 0x6a, 0x6a, 0x64,
    0x8d, 0x49, 0xd1, 0xfe, 0x49, 0xef, 0xa9, 0x12, 0xb5, 0x2a, 0xdc, 0x80,
    0x74, 0xe8, 0xd6, 0x79, 0x0a, 0x21, 0x55, 0x86, 0x81, 0x29, 0xb5, 0xcd,
    0xf4, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x03, 0x06, 0x08, 0x00, 0x80, 0x00,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Text that should result went decompressing the test data</summary>
  const char uncompressedString[] =
    u8"Hello World, this is text that has been CSC-compressed";

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  TEST(CscDecompressorTest, CanBeCreatedAndDestroyed) {
    EXPECT_NO_THROW(
      CscDecompressor decompressor;
      (void)decompressor;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(CscDecompressorTest, MemoryChunkCanBeDecompressed) {

    // Message that will be compressed
    const std::uint8_t *input = compressedString;

    // Set up a buffer to hold the compressed data. We assume compression will not
    // make result in generated output that is more than twice the size of the input data :)
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(uncompressedString) * 2);

    // Now compress the data with the deflate compressor
    {
      CscDecompressor decompressor;

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

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)
