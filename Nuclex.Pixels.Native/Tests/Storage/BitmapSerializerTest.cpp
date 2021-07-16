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

#include "TemporaryDirectoryScope.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny PNG file encoding a 1x1 pixel black quare</summary>
  const std::uint8_t verySmallPng[73] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x37, 0x6e, 0xf9, 0x24, 0x00, 0x00, 0x00,
    0x10, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x62, 0x60, 0x01, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x03, 0x00, 0x00, 0x06, 0x00, 0x05, 0x57, 0xbf, 0xab,
    0xd4, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
    0x82
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 17x7 PNG with colored letters and an alpha channel</summary>
  const std::uint8_t testPng[161] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x07,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xEE, 0xE2, 0xA6, 0x7C, 0x00, 0x00, 0x00,
    0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x2E, 0x23, 0x00, 0x00, 0x2E,
    0x23, 0x01, 0x78, 0xA5, 0x3F, 0x76, 0x00, 0x00, 0x00, 0x53, 0x49, 0x44,
    0x41, 0x54, 0x18, 0xD3, 0x63, 0x60, 0xA0, 0x02, 0x60, 0x04, 0x11, 0xFF,
    0x19, 0x44, 0xFE, 0x23, 0x04, 0xDE, 0x30, 0x32, 0xFF, 0x17, 0x80, 0xF3,
    0xFF, 0x32, 0x7E, 0x60, 0x64, 0xB0, 0xF8, 0x0F, 0xE7, 0x33, 0x9C, 0x60,
    0x64, 0x04, 0x6B, 0x41, 0x33, 0x83, 0x01, 0xDD, 0x20, 0x64, 0x43, 0xC0,
    0x00, 0xD9, 0x10, 0x06, 0x24, 0x2D, 0x98, 0x22, 0xA8, 0x86, 0x10, 0x61,
    0x10, 0x61, 0x43, 0xB0, 0x7A, 0x1E, 0xD5, 0x20, 0x38, 0x9B, 0x05, 0x57,
    0x60, 0xC1, 0x0C, 0xC2, 0x08, 0x13, 0x5A, 0x01, 0x00, 0x34, 0x62, 0x29,
    0x55, 0xE9, 0x90, 0x52, 0xAE, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E,
    0x44, 0xAE, 0x42, 0x60, 0x82
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny JPEG file encoding a 1x1 pixel white quare</summary>
  const std::uint8_t verySmallJpeg[283] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
    0x01, 0x01, 0x01, 0x2C, 0x01, 0x2C, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x01, 0x00, 0x01, 0x03,
    0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x00,
    0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xFF, 0xC4, 0x00, 0x14, 0x10,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xC4, 0x00, 0x14, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xC4, 0x00, 0x14, 0x11, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11,
    0x00, 0x3F, 0x00, 0x7F, 0x00, 0xFF, 0xD9
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A 17x7 JPEG with colored letters</summary>
  const std::uint8_t testJpeg[460] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
    0x01, 0x01, 0x01, 0x2C, 0x01, 0x2C, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
    0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x03,
    0x03, 0x03, 0x03, 0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x06,
    0x06, 0x05, 0x06, 0x09, 0x08, 0x0A, 0x0A, 0x09, 0x08, 0x09, 0x09, 0x0A,
    0x0C, 0x0F, 0x0C, 0x0A, 0x0B, 0x0E, 0x0B, 0x09, 0x09, 0x0D, 0x11, 0x0D,
    0x0E, 0x0F, 0x10, 0x10, 0x11, 0x10, 0x0A, 0x0C, 0x12, 0x13, 0x12, 0x10,
    0x13, 0x0F, 0x10, 0x10, 0x10, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x03, 0x03,
    0x03, 0x04, 0x03, 0x04, 0x08, 0x04, 0x04, 0x08, 0x10, 0x0B, 0x09, 0x0B,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x07, 0x00, 0x11, 0x03,
    0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x00,
    0x16, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x06, 0xFF, 0xC4, 0x00,
    0x21, 0x10, 0x00, 0x02, 0x01, 0x04, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x00, 0x01, 0x05,
    0x06, 0x07, 0x11, 0x12, 0x13, 0x14, 0x15, 0x22, 0xFF, 0xC4, 0x00, 0x15,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xC4, 0x00, 0x23, 0x11,
    0x00, 0x01, 0x03, 0x03, 0x02, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x11, 0x00, 0x12, 0x21, 0x03,
    0x31, 0x05, 0x13, 0x14, 0x22, 0x32, 0x41, 0x91, 0xFF, 0xDA, 0x00, 0x0C,
    0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0xBD, 0x1F,
    0xA5, 0x6E, 0x90, 0x33, 0x9B, 0x1C, 0xF4, 0x16, 0xCF, 0x32, 0x0C, 0xFD,
    0xBD, 0x12, 0xD3, 0x1C, 0x36, 0x27, 0x9D, 0xEF, 0x8A, 0xB6, 0x30, 0x3D,
    0x82, 0x81, 0x39, 0x6B, 0xB2, 0x07, 0xE6, 0xB2, 0x47, 0xE0, 0x48, 0x3A,
    0xE8, 0x2F, 0xE0, 0x4B, 0x05, 0x8D, 0x62, 0xF6, 0xFE, 0x2F, 0xE4, 0x7D,
    0x9E, 0x3E, 0xD5, 0x85, 0xC7, 0xC5, 0xE4, 0x88, 0x20, 0xDD, 0xEB, 0x5F,
    0xCA, 0x43, 0xCA, 0x5F, 0x7E, 0x93, 0x09, 0xD2, 0xB1, 0x85, 0x98, 0xB7,
    0xDA, 0xA1, 0x23, 0x13, 0x21, 0xD2, 0x63, 0x2E, 0x0B, 0x25, 0x5C, 0x06,
    0xD6, 0x41, 0x1A, 0xD8, 0x81, 0xF4, 0xDD, 0xF1, 0x81, 0xA6, 0xA5, 0x19,
    0x43, 0x04, 0xB8, 0x4E, 0x9E, 0x79, 0xD6, 0xCC, 0xFA, 0xB4, 0xC8, 0xB6,
    0x23, 0xE2, 0xAE, 0x19, 0x90, 0x01, 0xCD, 0x45, 0xC6, 0x8F, 0x52, 0xFD,
    0x5C, 0x43, 0x51, 0x44, 0xAD, 0x41, 0x00, 0xED, 0x1D, 0x88, 0x08, 0x1B,
    0x89, 0xC8, 0x12, 0x73, 0xE5, 0x24, 0x40, 0x24, 0x55, 0x27, 0x4A, 0x52,
    0x95, 0x6A, 0xFF, 0xD9
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A tiny EXR file encoding a 1x1 pixel white quare</summary>
  const std::uint8_t verySmallExr[355] = {
    0x76, 0x2F, 0x31, 0x01, 0x02, 0x00, 0x00, 0x00, 0x63, 0x68, 0x61, 0x6E,
    0x6E, 0x65, 0x6C, 0x73, 0x00, 0x63, 0x68, 0x6C, 0x69, 0x73, 0x74, 0x00,
    0x49, 0x00, 0x00, 0x00, 0x41, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x47, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x52, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6F, 0x6D, 0x70, 0x72, 0x65, 0x73,
    0x73, 0x69, 0x6F, 0x6E, 0x00, 0x63, 0x6F, 0x6D, 0x70, 0x72, 0x65, 0x73,
    0x73, 0x69, 0x6F, 0x6E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x64, 0x61,
    0x74, 0x61, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x00, 0x62, 0x6F, 0x78,
    0x32, 0x69, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64,
    0x69, 0x73, 0x70, 0x6C, 0x61, 0x79, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77,
    0x00, 0x62, 0x6F, 0x78, 0x32, 0x69, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x6C, 0x69, 0x6E, 0x65, 0x4F, 0x72, 0x64, 0x65, 0x72,
    0x00, 0x6C, 0x69, 0x6E, 0x65, 0x4F, 0x72, 0x64, 0x65, 0x72, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x70, 0x69, 0x78, 0x65, 0x6C, 0x41, 0x73, 0x70,
    0x65, 0x63, 0x74, 0x52, 0x61, 0x74, 0x69, 0x6F, 0x00, 0x66, 0x6C, 0x6F,
    0x61, 0x74, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x73,
    0x63, 0x72, 0x65, 0x65, 0x6E, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x43,
    0x65, 0x6E, 0x74, 0x65, 0x72, 0x00, 0x76, 0x32, 0x66, 0x00, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x63,
    0x72, 0x65, 0x65, 0x6E, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x57, 0x69,
    0x64, 0x74, 0x68, 0x00, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x53, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C
  };

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
    public: Nuclex::Pixels::BitmapInfo TryReadInfo(
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
    public: virtual Nuclex::Pixels::Storage::OptionalBitmap TryLoad(
      const Nuclex::Pixels::Storage::VirtualFile &source,
      const std::string &extensionHint = std::string()
    ) const override {
      (void)source;
      (void)extensionHint;
      return Nuclex::Pixels::Storage::OptionalBitmap();
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
      Nuclex::Pixels::Storage::VirtualFile &target
    ) const override {
      (void)bitmap;
      (void)target;
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
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
  TEST(BitmapSerializerTest, PngCanBeLoadedByPath) {
    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.WriteFullFile(
        u8"test.png",
        std::string(reinterpret_cast<const char *>(verySmallPng),  sizeof(verySmallPng))
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
    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.WriteFullFile(
        u8"test.jpeg",
        std::string(reinterpret_cast<const char *>(verySmallJpeg),  sizeof(verySmallJpeg))
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
    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.WriteFullFile(
        u8"test.exr",
        std::string(reinterpret_cast<const char *>(verySmallExr),  sizeof(verySmallExr))
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
    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.WriteFullFile(
        u8"test.png",
        std::string(reinterpret_cast<const char *>(testPng),  sizeof(testPng))
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
    BitmapSerializer store;

    {
      TemporaryDirectoryScope temporaryDirectory;

      // Write our test PNG image into a file
      temporaryDirectory.WriteFullFile(
        u8"test.jpeg",
        std::string(reinterpret_cast<const char *>(testJpeg),  sizeof(testJpeg))
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
