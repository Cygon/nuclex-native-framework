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

#include "../../../Source/Compression/Brotli/BrotliDecompressor.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compressed string used to test the Brotli decompressor</summary>
  const std::uint8_t compressedString[] = {
    0x1b, 0x39, 0x00, 0x48, 0x1d, 0xa9, 0x51, 0x9f, 0x3b, 0xe2, 0x5a, 0x68,
    0xec, 0x4d, 0x4e, 0xb6, 0xb4, 0xd1, 0x25, 0x40, 0x1a, 0x1b, 0x38, 0xe4,
    0x80, 0xfd, 0x5f, 0x92, 0x62, 0xec, 0xc1, 0x06, 0x1c, 0x6b, 0x12, 0xbc,
    0x0e, 0x9b, 0xb0, 0xbc, 0xff, 0x8c, 0x79, 0x19, 0x83, 0xee, 0xb6, 0x7b,
    0xb8, 0x8f, 0x85, 0x6c, 0x01
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Text that should result went decompressing the test data</summary>
  const char uncompressedString[] =
    u8"Hello World, this is text that has been Brotli-compressed";

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace Storage { namespace Compression { namespace Brotli {

  // ------------------------------------------------------------------------------------------- //

  TEST(BrotliDecompressorTest, CanBeCreatedAndDestroyed) {
    EXPECT_NO_THROW(
      BrotliDecompressor decompressor;
      (void)decompressor;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BrotliDecompressorTest, MemoryChunkCanBeDecompressed) {

    // Message that will be decompressed
    const std::uint8_t *input = compressedString;

    // Set up a buffer to hold the decompressed data + a bit extra
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(uncompressedString) * 2);

    // Now decompress the data with the Brotli decompressor
    {
      BrotliDecompressor decompressor;

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

}}}} // namespace Nuclex::Storage::Compression::Brotli
