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

#ifndef NUCLEX_PIXELS_STORAGE_JPEG_LIBJPEGHELPERS_H
#define NUCLEX_PIXELS_STORAGE_JPEG_LIBJPEGHELPERS_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)

#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"

#include <cstdint>

#include <jpeglib.h>

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the input buffer for reading file data into libjpeg</summary>
  /// <remarks>
  ///   This is the same size as used by the (FILE *) implementation set up by
  ///   the init_source() function in jdatasrc.c.
  /// </remarks>
  constexpr const std::size_t JpegInputBufferSize = 4096;

  /// <summary>Size of the smallest valid JPEG file possible</summary>
  /// <remarks>
  ///   From https://stackoverflow.com/questions/2253404
  /// </remarks>
  constexpr const std::size_t SmallestPossibleJpegSize = 119; // byte

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading JPEG files using libjpeg</summary>
  class Helpers {

    /// <summary>
    ///   Checks whether the first 13 bytes in a file are a valid JPEG file header
    /// </summary>
    /// <param name="fileHeader">File header that will be checked</param>
    /// <returns>True if the file header is a valid JPEG file header</returns>
    /// <remarks>
    ///   The file header must contain at least the first 13 bytes of the file,
    ///   otherwise this will segfault.
    /// </remarks>
    public: static bool IsValidJpegHeader(const std::uint8_t *fileHeader);

    /// <summary>Finds the supported pixel format that is closest to the JPEG's</summary>
    /// <param name="commonInfo">JPEG decompression structure with image information</param>
    /// <returns>The pixel format that's most like the one of the JPEG image</returns>
    public: static PixelFormat GetEquivalentPixelFormat(
      const ::jpeg_decompress_struct &commonInfo
    );

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Data required by the libjpeg IO functions to read from virtual files</summary>
  struct JpegReadEnvironment : public ::jpeg_source_mgr {

    /// <summary>Initializes a new libjpeg read environment</summary>
    /// <param name="file">Virtual file from which data will be read</param>
    public: JpegReadEnvironment(const Nuclex::Pixels::Storage::VirtualFile &file) :
      IsReadOnly(true),
      File(file),
      Position(0) {
      this->Length = file.GetSize();
      SetupFunctionPointers(*this);
    }

    /// <summary>Sets up the functions pointers used by libjpeg</summary>
    /// <param name="jpegReadEnvironment">
    ///   Environment on which the function pointers will be set up
    /// </param>
    protected: static void SetupFunctionPointers(JpegReadEnvironment &jpegReadEnvironment);

    /// <summary>Whether the virtual file is opened in read-only mode</summary>
    public: bool IsReadOnly;
    /// <summary>Virtual file the read environment is taking data from</summary>
    public: const VirtualFile &File;
    /// <summary>Current position of the file cursor</summary>
    public: std::uint64_t Position;
    /// <summary>Total length of the file in bytes</summary>
    public: std::uint64_t Length;
    /// <summary>Buffer in which read data will be stored for libjpeg</summary>
    public: std::uint8_t Buffer[JpegInputBufferSize];

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Data required by the libjpeg IO functions to write to virtual files</summary>
  struct JpegWriteEnvironment : public ::jpeg_source_mgr {

    /// <summary>Initializes a new libjpeg write  environment</summary>
    /// <param name="file">Virtual file into which data will be written</param>
    public: JpegWriteEnvironment(Nuclex::Pixels::Storage::VirtualFile &file) :
      IsReadOnly(false),
      File(file),
      Position(0) {
      SetupFunctionPointers(*this);
    }

    /// <summary>Sets up the functions pointers used by libjpeg</summary>
    /// <param name="jpegWriteEnvironment">
    ///   Environment on which the function pointers will be set up
    /// </param>
    protected: static void SetupFunctionPointers(JpegWriteEnvironment &jpegWriteEnvironment);

    /// <summary>Whether the virtual file is opened in read-only mode</summary>
    public: bool IsReadOnly;
    /// <summary>Virtual file the read environment is taking data from</summary>
    public: VirtualFile &File;
    /// <summary>Current position of the file cursor</summary>
    public: std::uint64_t Position;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)

#endif // NUCLEX_PIXELS_STORAGE_JPEG_LIBJPEGHELPERS_H
