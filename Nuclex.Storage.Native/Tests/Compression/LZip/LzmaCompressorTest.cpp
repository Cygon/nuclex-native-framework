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

#include "../../../Source/Compression/LZip/LzmaCompressor.h"
#include "../../../Source/Compression/LZip/LzmaDecompressor.h"

#include "../CompressorTest.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compression level to use for the LZMA encoder in unit tests</summary>
  /// <remarks>
  ///   The LZMA encoder can allocate a lot of memory on higher compression levels.
  ///   This may cause delays (large heap allocations of hundreds of megabytes cause
  ///   certain operating systems to stall for a second or two) or at least be wasteful
  ///   of resources on continuous integration build systems.
  /// </remarks>
  const int UnitTestCompressionLevel = 2;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaCompressorTest, CanBeCreatedAndDestroyed) {
    EXPECT_NO_THROW(
      LzmaCompressor compressor(UnitTestCompressionLevel);
      (void)compressor;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaCompressorTest, MemoryChunkCanBeCompressed) {

    // Message that will be compressed
    char message[] = u8"Hello World, this text will be compressed by LZip";
    const std::uint8_t *input = reinterpret_cast<const std::uint8_t *>(message);

    // Set up a buffer to hold the compressed data. We assume compression will not
    // make result in generated output that is more than twice the size of the input data :)
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(message) * 2);

    // Now compress the data with the LZMA compressor
    {
      LzmaCompressor compressor(UnitTestCompressionLevel);

      std::size_t inputByteCount = sizeof(message);
      std::size_t outputByteCount = outputBuffer.size();
      StopReason stopReason = compressor.Process(
        input, inputByteCount, &outputBuffer[0], outputByteCount
      );
      EXPECT_EQ(stopReason, StopReason::InputBufferExhausted);

      std::size_t secondOutputByteCount = outputBuffer.size() - outputByteCount;
      stopReason = compressor.Finish(&outputBuffer[outputByteCount], secondOutputByteCount);
      EXPECT_EQ(stopReason, StopReason::Finished);

      outputBuffer.resize(outputByteCount + secondOutputByteCount);
    }

    // Output compressed bytes as hex codes for debugging
    //for(std::size_t index = 0; index < outputBuffer.size(); ++index) {
    //  std::cout << std::hex << static_cast<int>(outputBuffer[index]) << std::endl;
    //}

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaCompressorTest, MemoryChunkSurvivesCompressionRoundTrip) {

    // Message that will be compressed
    char message[] = u8"This text will be compressed and then decompressed again by LZip";
    const std::uint8_t *input = reinterpret_cast<const std::uint8_t *>(message);

    // Set up a buffer to hold the compressed data. We assume compression will not
    // make result in generated output that is more than twice the size of the input data :)
    std::vector<std::uint8_t> outputBuffer;
    outputBuffer.resize(sizeof(message) * 2);

    // Compress the data with the LZMA compressor
    {
      LzmaCompressor compressor(UnitTestCompressionLevel);

      std::size_t inputByteCount = sizeof(message);
      std::size_t outputByteCount = outputBuffer.size();
      StopReason stopReason = compressor.Process(
        input, inputByteCount, &outputBuffer[0], outputByteCount
      );
      EXPECT_EQ(stopReason, StopReason::InputBufferExhausted);

      std::size_t secondOutputByteCount = outputBuffer.size() - outputByteCount;
      stopReason = compressor.Finish(&outputBuffer[outputByteCount], secondOutputByteCount);
      EXPECT_EQ(stopReason, StopReason::Finished);

      outputBuffer.resize(outputByteCount + secondOutputByteCount);
    }

    // Now the output becomes the round trip's input
    std::vector<std::uint8_t> inputBuffer;
    inputBuffer.swap(outputBuffer);

    // Set up a buffer to hold the decompressed data. Same logic as before.
    outputBuffer.resize(sizeof(message) * 2);

    // Decompress the data with the LZMA decompressor
    {
      LzmaDecompressor decompressor;

      std::size_t inputByteCount = inputBuffer.size();
      std::size_t outputByteCount = outputBuffer.size();
      StopReason stopReason = decompressor.Process(
        &inputBuffer[0], inputByteCount, &outputBuffer[0], outputByteCount
      );
      EXPECT_EQ(stopReason, StopReason::InputBufferExhausted);

      std::size_t secondOutputByteCount = outputBuffer.size() - outputByteCount;
      stopReason = decompressor.Finish(&outputBuffer[outputByteCount], secondOutputByteCount);
      EXPECT_EQ(stopReason, StopReason::Finished);

      outputBuffer.resize(outputByteCount + secondOutputByteCount);
    }

    ASSERT_EQ(outputBuffer.size(), sizeof(message));
    for(std::size_t index = 0; index < sizeof(message); ++index) {
      EXPECT_EQ(static_cast<char>(outputBuffer[index]), message[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LzmaCompressorTest, LongStreamSurvivesCompressionRoundTrip) {
    const std::size_t totalByteCount = 262144 - 123; // 256 KiB minus a bit to confuse chunking

    std::vector<std::uint8_t> compressedData;

    {
      LzmaCompressor compressor(UnitTestCompressionLevel);
      checkStreamCompression(compressor, compressedData, totalByteCount);
    }
    {
      LzmaDecompressor decompressor;
      checkStreamDecompression(decompressor, compressedData, totalByteCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //
  
}}}} // namespace Nuclex::Storage::Compression::LZip

#endif // defined(NUCLEX_STORAGE_HAVE_LZIP)
