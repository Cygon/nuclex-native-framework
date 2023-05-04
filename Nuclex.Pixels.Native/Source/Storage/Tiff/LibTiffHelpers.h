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

#ifndef NUCLEX_PIXELS_STORAGE_TIFF_LIBTIFFHELPERS_H
#define NUCLEX_PIXELS_STORAGE_TIFF_LIBTIFFHELPERS_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"

#include <cstdint> // for std::size_t

#include <libtiff/tiff.h>
#include <libtiff/tiffio.h>

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid TIFF file possible</summary>
  /// <remarks>
  ///   From https://github.com/mathiasbynens/small/blob/master/tiff.tif
  /// </remarks>
  constexpr const std::size_t SmallestPossibleTiffSize = 46; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading TIFF files using libtiff</summary>
  class Helpers {

    /// <summary>
    ///   Checks whether the first 8 bytes in a file are a valid TIFF file header
    /// </summary>
    /// <param name="fileHeader">File header that will be checked</param>
    /// <returns>True if the file header is a valid TIFF file header</returns>
    /// <remarks>
    ///   The file header must contain at least the first 8 bytes of the file,
    ///   otherwise this will segfault.
    /// </remarks>
    public: static bool IsValidTiffHeader(const std::uint8_t *fileHeader);

    /// <summary>Opens a virtual file for reading as a TIFF file</summary>
    /// <param name="file">File that will be opened for reading as a TIFF file</param>
    /// <param name="headerOnly">Whether only the file header is going to be read</param>
    /// <returns>A TIFF file pointer that can be used in all LibTIFF methods</returns>
    public: static ::TIFF *OpenForReading(
      const Nuclex::Pixels::Storage::VirtualFile &file, bool headerOnly = false
    );

    /// <summary>Opens a virtual file as a TIFF file for writing</summary>
    /// <param name="file">File that will be opened as a TIFF file for writing</param>
    /// <returns>A TIFF file pointer that can be used in all LibTIFF methods</returns>
    public: static ::TIFF *OpenForWriting(Nuclex::Pixels::Storage::VirtualFile &file);

    /// <summary>Closes a TIFF file pointer opened by one of the Open...() methods</summary>
    /// <param name="tiffClose">
    ///   TIFF file pointer returned from one of the Open...() methods that will be closed
    /// </param>
    public: static void Close(::TIFF *tiffFile);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#endif // NUCLEX_PIXELS_STORAGE_TIFF_LIBTIFFHELPERS_H
