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

#include "Nuclex/Pixels/Storage/BitmapSerializer.h"
#include "Nuclex/Pixels/Storage/BitmapCodec.h"
#include <gtest/gtest.h>

#include "Nuclex/Support/TemporaryDirectoryScope.h"

// Disable things that have not yet made their way into
// the GitHub repository. If you want these, use the Subversion repo
#undef NUCLEX_PIXELS_HAVE_LIBTIFF
#undef NUCLEX_PIXELS_HAVE_OPENEXR
#undef NUCLEX_PIXELS_HAVE_LIBWEBP
#undef NUCLEX_PIXELS_HAVE_LIBAVIF

#include "Png/ExamplePngs.h"
#include "Jpeg/ExampleJpegs.h"
//#include "Exr/ExampleExrs.h"
//#include "Tiff/ExampleTiffs.h"
//#include "WebP/ExampleWebPs.h"
//#include "WebP/ExampleAVIFs.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  class DummyBitmapCodec : public Nuclex::Pixels::Storage::BitmapCodec {

    /// <summary>Initializes a new dummy codec for the unit test/summary>
    public: DummyBitmapCodec() :
      name("Dummy Codec") {
      this->knownFileExtensions.push_back("dummy");
      this->knownFileExtensions.push_back(".moo");
    }

    /// <summary>Gives the name of the file format implemented by this codec</summary>
    /// <returns>The name of the file format this codec implements</returns>
    public: const std::string &GetName() const override {
      return this->name;
    }

    /// <summary>Provides commonly used file extensions for this codec</summary>
    /// <returns>The commonly used file extensions in order of preference</returns>
    public: const std::vector<std::string> &GetFileExtensions() const override {
      return this->knownFileExtensions;
    }

    /// <summary>Tries to read informations for a bitmap</summary>
    /// <param name="source">Source data from which the informations should be extracted</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>
    ///   Informations about the bitmap, if the codec is able to load it, otherwise
    ///   a BitmapInfo structure with 'Loadable' set to false.
    /// </returns>
    public: std::optional<Nuclex::Pixels::BitmapInfo> TryReadInfo(
      const Nuclex::Pixels::Storage::VirtualFile &source,
      const std::string &extensionHint = std::string()
    ) const override {
      (void)source;
      (void)extensionHint;
      throw -1;
    }

    /// <summary>Checks if the codec is able to load the specified file</summary>
    /// <param name="source">Source data that will be checked for loadbility</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec is able to load the specified file</returns>
    public: bool CanLoad(
      const Nuclex::Pixels::Storage::VirtualFile &source,
      const std::string &extensionHint = std::string()
    ) const override {
      (void)source;
      (void)extensionHint;
      return true;
    }

    /// <summary>Checks if the codec is able to save bitmaps to storage</summary>
    /// <returns>True if the codec supports saving bitmaps</returns>
    public: bool CanSave() const override {
      return true;
    }

    /// <summary>Tries to load the specified file as a bitmap</summary>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>
    ///   The bitmap loaded from the specified file data or an empty value if the file format
    ///   is not supported by the codec
    /// </returns>
    public: virtual std::optional<Nuclex::Pixels::Bitmap> TryLoad(
      const Nuclex::Pixels::Storage::VirtualFile &source,
      const std::string &extensionHint = std::string()
    ) const override {
      (void)source;
      (void)extensionHint;
      return std::optional<Nuclex::Pixels::Bitmap>();
    }

    /// <summary>Tries to load the specified file into an exciting bitmap</summary>
    /// <param name="exactlyFittingBitmap">
    ///   Bitmap matching the exact dimensions of the file to be loaded
    /// </param>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec was able to load the bitmap, false otherwise</returns>
    public: bool TryReload(
      Nuclex::Pixels::Bitmap &exactlyFittingBitmap,
      const Nuclex::Pixels::Storage::VirtualFile &source,
      const std::string &extensionHint = std::string()
    ) const override {
      (void)exactlyFittingBitmap;
      (void)source;
      (void)extensionHint;
      throw -1;
    }

    /// <summary>Saves the specified bitmap into a file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="target">File into which the bitmap will be saved</param>
    public: void Save(
      const Nuclex::Pixels::Bitmap &bitmap,
      Nuclex::Pixels::Storage::VirtualFile &target,
      float compressionStrengthHint = 0.75f,
      float outputQualityHint = 0.95f
    ) const override {
      (void)bitmap;
      (void)target;
      (void)compressionStrengthHint;
      (void)outputQualityHint;
    }

    /// <summary>Human-readable name of the file format this codec implements</summary>
    private: std::string name;
    /// <summary>File extensions this file format is known to use</summary>
    private: std::vector<std::string> knownFileExtensions;

  };

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapSerializerTest, CustomCodecsCanBeRegistered) {
    BitmapSerializer store;

    EXPECT_NO_THROW(
      store.RegisterCodec(std::make_unique<DummyBitmapCodec>())
    );
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBWEBP)
  TEST(BitmapSerializerTest, WebPCanBeLoadedByPath) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test WebP image into a file
      temporaryDirectory.PlaceFile(
        u8"test.webp", WebP::verySmallWebP, sizeof(WebP::verySmallWebP)
      );

      std::string testWebPPath = temporaryDirectory.GetPath(u8"test.webp");
      EXPECT_TRUE(store.CanLoad(testWebPPath));

      //std::optional<BitmapInfo> bitmapInfo = store.TryReadInfo(testWebPPath);
      Bitmap bitmap = store.Load(testWebPPath);
      EXPECT_EQ(bitmap.GetWidth(), 1U);
      EXPECT_EQ(bitmap.GetHeight(), 1U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_G8_B8_A8_Unsigned);
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
  TEST(BitmapSerializerTest, TiffCanBeLoadedByPath) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test TIFF image into a file
      temporaryDirectory.PlaceFile(
        u8"test.tif", Tiff::verySmallTiff, sizeof(Tiff::verySmallTiff)
      );

      std::string testTiffPath = temporaryDirectory.GetPath(u8"test.tif");
      EXPECT_TRUE(store.CanLoad(testTiffPath));

      Bitmap bitmap = store.Load(testTiffPath);
      EXPECT_EQ(bitmap.GetWidth(), 1U);
      EXPECT_EQ(bitmap.GetHeight(), 1U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_G8_B8_A8_Unsigned);
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
  TEST(BitmapSerializerTest, PngCanBeLoadedByPath) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.PlaceFile(
        u8"test.png", Png::verySmallPng, sizeof(Png::verySmallPng)
      );

      std::string testPngPath = temporaryDirectory.GetPath(u8"test.png");
      EXPECT_TRUE(store.CanLoad(testPngPath));

      Bitmap bitmap = store.Load(testPngPath);
      EXPECT_EQ(bitmap.GetWidth(), 1U);
      EXPECT_EQ(bitmap.GetHeight(), 1U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_Unsigned); // grayscale
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
  TEST(BitmapSerializerTest, JpegCanBeLoadedByPath) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.PlaceFile(
        u8"test.jpeg", Jpeg::verySmallJpeg, sizeof(Jpeg::verySmallJpeg)
      );

      std::string testJpegPath = temporaryDirectory.GetPath(u8"test.jpeg");
      EXPECT_TRUE(store.CanLoad(testJpegPath));

      Bitmap bitmap = store.Load(testJpegPath);
      EXPECT_EQ(bitmap.GetWidth(), 1U);
      EXPECT_EQ(bitmap.GetHeight(), 1U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_G8_B8_Unsigned);
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)
  TEST(BitmapSerializerTest, ExrCanBeLoadedByPath) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.PlaceFile(
        u8"test.exr", Exr::verySmallExr, sizeof(Exr::verySmallExr)
      );

      std::string testExrPath = temporaryDirectory.GetPath(u8"test.exr");
      EXPECT_TRUE(store.CanLoad(testExrPath));

      Bitmap bitmap = store.Load(testExrPath);
      EXPECT_EQ(bitmap.GetWidth(), 1U);
      EXPECT_EQ(bitmap.GetHeight(), 1U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R16_G16_B16_A16_Float);
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_OPENEXR)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
  TEST(BitmapSerializerTest, PngsAreLoadedCorrectly) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.PlaceFile(
        u8"test.png", Png::testPng, sizeof(Png::testPng)
      );

      std::string testPngPath = temporaryDirectory.GetPath(u8"test.png");
      Bitmap bitmap = store.Load(testPngPath);

      EXPECT_EQ(bitmap.GetWidth(), 17U);
      EXPECT_EQ(bitmap.GetHeight(), 7U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_G8_B8_A8_Unsigned);
      // TODO: Check some of the pixels
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
  TEST(BitmapSerializerTest, JpegsAreLoadedCorrectly) {
    using Nuclex::Support::TemporaryDirectoryScope;

    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.PlaceFile(
        u8"test.jpeg", Jpeg::testJpeg, sizeof(Jpeg::testJpeg)
      );

      std::string testJpegPath = temporaryDirectory.GetPath(u8"test.jpeg");
      Bitmap bitmap = store.Load(testJpegPath);

      EXPECT_EQ(bitmap.GetWidth(), 17U);
      EXPECT_EQ(bitmap.GetHeight(), 7U);
      EXPECT_EQ(bitmap.GetPixelFormat(), PixelFormat::R8_G8_B8_Unsigned);
      // TODO: Check some of the pixels
    }
  }
#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
