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

#include "../../../Source/Storage/Tiff/TiffBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#include <gtest/gtest.h>

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "ExampleTiffs.h"

#include <algorithm> // for std::copy_n()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simple virtual file implementation that accesses an in-memory buffer</summary>
  class InMemoryFile : public Nuclex::Pixels::Storage::VirtualFile {

    /// <summary>Initializes a new memory buffer based file</summary>
    /// <param name="data">Memory buffer the virtual file will access</param>
    /// <param name="length">Size of the memory buffer in bytes</param>
    public: InMemoryFile(const std::uint8_t *data, std::uint64_t length) :
      data(data),
      length(length) {}

    /// <summary>Frees all memory used by the instance</summary>
    public: ~InMemoryFile() override = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: std::uint64_t GetSize() const override { return this->length; }

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
    ) const override {
      std::copy_n(this->data + start, byteCount, buffer);
    }

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) override {
      (void)start;
      (void)byteCount;
      (void)buffer;
      assert(!u8"Write method of unit test dummy file is never called");
    }

    /// <summary>Memory buffer the virtual file implementation is serving data from</summary>
    private: const std::uint8_t *data;
    /// <summary>Length of the memory buffer in bytes</summary>
    private: std::uint64_t length;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TiffBitmapCodec codec;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, ProvidesName) {
    TiffBitmapCodec codec;
    std::string codecName = codec.GetName();
    EXPECT_GT(codecName.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, FileExtensionsIncludeTifAndTiff) {
    TiffBitmapCodec codec;
    const std::vector<std::string> &extensions = codec.GetFileExtensions();

    bool tifFound = false;
    bool tiffFound = false;
    for(std::size_t index = 0; index < extensions.size(); ++index) {
      if((extensions[index] == "tif") || (extensions[index] == ".tif")) {
        tifFound = true;
      }
      if((extensions[index] == "tiff") || (extensions[index] == ".tiff")) {
        tiffFound = true;
      }
    }

    EXPECT_TRUE(tifFound);
    EXPECT_TRUE(tiffFound);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, CanLoadMethodRecognizesTiffs) {
    TiffBitmapCodec codec;

    {
      std::uint8_t dummyData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      EXPECT_FALSE(codec.CanLoad(dummyFile));
    }

    {
      const InMemoryFile tiffFile(Tiff::testTiff, sizeof(Tiff::testTiff));
      EXPECT_TRUE(codec.CanLoad(tiffFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, TryReadInfoReturnsEmptyOnWrongFileType) {
    TiffBitmapCodec codec;
    {
      std::uint8_t dummyData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      std::optional<BitmapInfo> dummyBitmapInfo = codec.TryReadInfo(dummyFile);
      EXPECT_FALSE(dummyBitmapInfo.has_value());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, TryReadInfoThrowsOnCorruptedFile) {
    TiffBitmapCodec codec;
    {
      const InMemoryFile corruptTiffFile(corruptTiff, sizeof(corruptTiff));
      EXPECT_THROW(
        { codec.TryReadInfo(corruptTiffFile); },
        Errors::FileFormatError
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TiffBitmapCodecTest, TryReadInfoSucceedsForTiffs) {
    TiffBitmapCodec codec;
    {
      const InMemoryFile testTiffFile(Tiff::testTiff, sizeof(Tiff::testTiff));
      std::optional<BitmapInfo> testTiffBitmapInfo = codec.TryReadInfo(testTiffFile);
      EXPECT_TRUE(testTiffBitmapInfo.has_value());
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
