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

#include "../../../Source/Storage/Png/PngBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)

#include <gtest/gtest.h>

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "Nuclex/Support/TemporaryDirectoryScope.h"
#include "ExamplePngs.h"

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

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      PngBitmapCodec codec;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, ProvidesName) {
    PngBitmapCodec codec;
    std::string codecName = codec.GetName();
    EXPECT_GT(codecName.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, FileExtensionsIncludePng) {
    PngBitmapCodec codec;
    const std::vector<std::string> &extensions = codec.GetFileExtensions();

    bool pngFound = false;
    for(std::size_t index = 0; index < extensions.size(); ++index) {
      if((extensions[index] == "png") || (extensions[index] == ".png")) {
        pngFound = true;
      }
    }

    EXPECT_TRUE(pngFound);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, CanLoadMethodRecognizesPngs) {
    PngBitmapCodec codec;

    {
      std::uint8_t dummyData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      EXPECT_FALSE(codec.CanLoad(dummyFile));
    }

    {
      const InMemoryFile pngFile(testPng, sizeof(testPng));
      EXPECT_TRUE(codec.CanLoad(pngFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, TryReadInfoReturnsEmptyOnWrongFileType) {
    PngBitmapCodec codec;
    {
      std::uint8_t dummyData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      std::optional<BitmapInfo> dummyBitmapInfo = codec.TryReadInfo(dummyFile);
      EXPECT_FALSE(dummyBitmapInfo.has_value());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, TryReadInfoThrowsOnCorruptedFile) {
    PngBitmapCodec codec;
    {
      const InMemoryFile corruptPngFile(corruptPng, sizeof(corruptPng));
      EXPECT_THROW(
        { codec.TryReadInfo(corruptPngFile); },
        Errors::FileFormatError
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, TryReadInfoSucceedsForPngs) {
    PngBitmapCodec codec;
    {
      const InMemoryFile testPngFile(colorTestPng, sizeof(colorTestPng));
      std::optional<BitmapInfo> testPngBitmapInfo = codec.TryReadInfo(testPngFile);
      EXPECT_TRUE(testPngBitmapInfo.has_value());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, Loads24BitColorPngs) {
    PngBitmapCodec codec;
    {
      const InMemoryFile testPngFile(colorTestPng, sizeof(colorTestPng));
      std::optional<Bitmap> bitmap = codec.TryLoad(testPngFile);
      ASSERT_TRUE(bitmap.has_value());

      Bitmap definitiveBitmap = bitmap.value();
      const BitmapMemory &memory = definitiveBitmap.Access();

      ASSERT_EQ(memory.PixelFormat, PixelFormat::R8_G8_B8_Unsigned);

      std::uint8_t r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[0];
      std::uint8_t g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[1];
      std::uint8_t b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[2];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[3];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[4];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[5];
      EXPECT_EQ(r, 255U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[6];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[7];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[8];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 255U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[9];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[10];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[11];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 255U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[12];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[13];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[14];
      EXPECT_EQ(r, 255U);
      EXPECT_EQ(g, 255U);
      EXPECT_EQ(b, 255U);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, Loads48BitColorPngs) {
    PngBitmapCodec codec;
    {
      const InMemoryFile testPngFile(depth48TestPng, sizeof(depth48TestPng));
      std::optional<Bitmap> bitmap = codec.TryLoad(testPngFile);
      ASSERT_TRUE(bitmap.has_value());

      Bitmap definitiveBitmap = bitmap.value();
      const BitmapMemory &memory = definitiveBitmap.Access();

      ASSERT_EQ(memory.PixelFormat, PixelFormat::R16_G16_B16_A16_Unsigned_Native16);

      std::uint16_t r = reinterpret_cast<std::uint16_t *>(memory.Pixels)[0];
      std::uint16_t g = reinterpret_cast<std::uint16_t *>(memory.Pixels)[1];
      std::uint16_t b = reinterpret_cast<std::uint16_t *>(memory.Pixels)[2];
      std::uint16_t a = reinterpret_cast<std::uint16_t *>(memory.Pixels)[3];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);
      EXPECT_EQ(a, 65535U);

      r = reinterpret_cast<std::uint16_t *>(memory.Pixels)[4];
      g = reinterpret_cast<std::uint16_t *>(memory.Pixels)[5];
      b = reinterpret_cast<std::uint16_t *>(memory.Pixels)[6];
      a = reinterpret_cast<std::uint16_t *>(memory.Pixels)[3];
      EXPECT_EQ(r, 65535U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);
      EXPECT_EQ(a, 65535U);

      r = reinterpret_cast<std::uint16_t *>(memory.Pixels)[8];
      g = reinterpret_cast<std::uint16_t *>(memory.Pixels)[9];
      b = reinterpret_cast<std::uint16_t *>(memory.Pixels)[10];
      a = reinterpret_cast<std::uint16_t *>(memory.Pixels)[3];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 65535U);
      EXPECT_EQ(b, 0U);
      EXPECT_EQ(a, 65535U);

      r = reinterpret_cast<std::uint16_t *>(memory.Pixels)[12];
      g = reinterpret_cast<std::uint16_t *>(memory.Pixels)[13];
      b = reinterpret_cast<std::uint16_t *>(memory.Pixels)[14];
      a = reinterpret_cast<std::uint16_t *>(memory.Pixels)[3];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 65535U);
      EXPECT_EQ(a, 65535U);

      r = reinterpret_cast<std::uint16_t *>(memory.Pixels)[16];
      g = reinterpret_cast<std::uint16_t *>(memory.Pixels)[17];
      b = reinterpret_cast<std::uint16_t *>(memory.Pixels)[18];
      a = reinterpret_cast<std::uint16_t *>(memory.Pixels)[3];
      EXPECT_EQ(r, 65535U);
      EXPECT_EQ(g, 65535U);
      EXPECT_EQ(b, 65535U);
      EXPECT_EQ(a, 65535U);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, Reloads48BitColorPngsInto24BitColorPngs) {
    PngBitmapCodec codec;
    {
      const InMemoryFile testPngFile(depth48TestPng, sizeof(depth48TestPng));

      Bitmap existingBitmap(5, 5, PixelFormat::R8_G8_B8_A8_Unsigned);
      bool wasPngFile = codec.TryReload(existingBitmap, testPngFile);
      ASSERT_TRUE(wasPngFile);

      const BitmapMemory &memory = existingBitmap.Access();

      std::uint8_t r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[0];
      std::uint8_t g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[1];
      std::uint8_t b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[2];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[4];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[5];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[6];
      EXPECT_EQ(r, 255U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[8];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[9];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[10];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 255U);
      EXPECT_EQ(b, 0U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[12];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[13];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[14];
      EXPECT_EQ(r, 0U);
      EXPECT_EQ(g, 0U);
      EXPECT_EQ(b, 255U);

      r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[16];
      g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[17];
      b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[18];
      EXPECT_EQ(r, 255U);
      EXPECT_EQ(g, 255U);
      EXPECT_EQ(b, 255U);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, LoadingGarbageReturnsEmptyValue) {
    std::vector<std::uint8_t> garbage(1024);
    for(std::size_t index = 0; index < 1024; ++index) {
      garbage[index] = static_cast<std::uint8_t>(index + 127);
    }

    PngBitmapCodec codec;
    {
      const InMemoryFile garbagePngFile(garbage.data(), 1024);

      std::optional<Bitmap> loadedBitmap = codec.TryLoad(garbagePngFile);
      EXPECT_FALSE(loadedBitmap.has_value());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, ReloadingGarbageReturnsFalse) {
    std::vector<std::uint8_t> garbage(1024);
    for(std::size_t index = 0; index < 1024; ++index) {
      garbage[index] = static_cast<std::uint8_t>(index + 127);
    }

    PngBitmapCodec codec;
    {
      const InMemoryFile garbagePngFile(garbage.data(), 1024);

      Bitmap dummy(10, 10);
      bool wasPngFile = codec.TryReload(dummy, garbagePngFile);
      EXPECT_FALSE(wasPngFile);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PngBitmapCodecTest, BitmapSurvivesRoundTrip) {
    PngBitmapCodec codec;
    {
      const InMemoryFile testPngFile(colorTestPng, sizeof(colorTestPng));
      std::optional<Bitmap> bitmap = codec.TryLoad(testPngFile);
      ASSERT_TRUE(bitmap.has_value());

      std::optional<Bitmap> reloadedBitmap;
      {
        Nuclex::Support::TemporaryDirectoryScope tempDir(u8"tst");

        // Save the bitmap
        {
          std::unique_ptr<VirtualFile> targetFile = VirtualFile::OpenRealFileForWriting(
            tempDir.GetPath(u8"save-png-unit-test.png")
          );
          codec.Save(bitmap.value(), *targetFile.get());
        }

        // Now load it again
        {
          std::unique_ptr<const VirtualFile> sourceFile = VirtualFile::OpenRealFileForReading(
            tempDir.GetPath(u8"save-png-unit-test.png")
          );
          reloadedBitmap = codec.TryLoad(*sourceFile.get());
        }
      }

      // Verify that the bitmap was loaded and ha the right dimensions
      ASSERT_TRUE(reloadedBitmap.has_value());
      EXPECT_EQ(reloadedBitmap->GetWidth(), bitmap->GetWidth());
      EXPECT_EQ(reloadedBitmap->GetHeight(), bitmap->GetHeight());
      EXPECT_EQ(reloadedBitmap->GetPixelFormat(), PixelFormat::R8_G8_B8_Unsigned);

      // Check contents
      {
        const BitmapMemory &memory = reloadedBitmap->Access();

        std::uint8_t r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[3];
        std::uint8_t g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[4];
        std::uint8_t b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[5];
        EXPECT_EQ(r, 255U);
        EXPECT_EQ(g, 0U);
        EXPECT_EQ(b, 0U);

        r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[6];
        g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[7];
        b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[8];
        EXPECT_EQ(r, 0U);
        EXPECT_EQ(g, 255U);
        EXPECT_EQ(b, 0U);

        r = reinterpret_cast<std::uint8_t *>(memory.Pixels)[9];
        g = reinterpret_cast<std::uint8_t *>(memory.Pixels)[10];
        b = reinterpret_cast<std::uint8_t *>(memory.Pixels)[11];
        EXPECT_EQ(r, 0U);
        EXPECT_EQ(g, 0U);
        EXPECT_EQ(b, 255U);
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)
