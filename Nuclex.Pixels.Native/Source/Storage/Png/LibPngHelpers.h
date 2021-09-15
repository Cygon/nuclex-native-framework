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

#ifndef NUCLEX_PIXELS_STORAGE_PNG_LIBPNGHELPERS_H
#define NUCLEX_PIXELS_STORAGE_PNG_LIBPNGHELPERS_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)

#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"

#include <png.h> // LibPNG main header

namespace Nuclex { namespace Pixels { namespace Storage { namespace Png {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid PNG file possible</summary>
  /// <remarks>
  ///   From https://garethrees.org/2007/11/14/pngcrush/
  /// </remarks>
  constexpr const std::size_t SmallestPossiblePngSize = 67; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading PNG files using libpng</summary>
  class Helpers {

    /// <summary>Checks if the specified file extension indicates a .png file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates a .png</returns>
    public: static bool DoesFileExtensionSayPng(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid .png header</summary>
    /// <param name="source">File that will be checked for a valid .png header</param>
    /// <returns>True if a valid .png header was found, false otherwise</returns>
    public: static bool CheckIfPngHeaderPresent(const VirtualFile &source);

    /// <summary>Selects the pixel format in which a .png file will be loaded</summary>
    /// <param name="pngRead">
    ///   PNG read structure from which the .png pixel format will be queried
    ///   (will receive necessary adjustments if non-const reference)
    /// </param>
    /// <param name="pngInfo">
    ///   PNG information structure, required by some of the LibPNG query methods
    /// </param>
    /// <returns>
    ///   The pixel format that is closest/matches the .png file and if non-const,
    ///   for which LibPNG has been configured to load the image as
    /// </returns>
    /// <remarks>
    ///   LibPNG can perform some pixel format adjustments on its own. We use these to adapt
    ///   formats that would have no representation in Nuclex.Pixels (such as 1, 2 and 4 bits
    ///   per channel which is space-saving for storage but useless on modern graphics hardware).
    /// </remarks>
    public: static PixelFormat SelectPixelFormatForLoad(
      ::png_struct &pngRead, const ::png_info &pngInfo
    );

    /// <summary>Finds the supported pixel format that is closest to the PNG's</summary>
    /// <param name="pngRead">Main PNG structure storing libpng settings</param>
    /// <param name="pngInfo">PNG info structure storing information about an image</param>
    /// <returns>The pixel format that's most like the one of the PNG image</returns>
    public: static PixelFormat GetClosestPixelFormat(
      const ::png_struct &pngRead, const ::png_info &pngInfo
    );

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Data passed along to the custom read function for libpng</summary>
  struct PngReadEnvironment {

    /// <summary>Initializes a new libpng read environment</summary>
    /// <param name="pngRead">Main PNG structure initialized for reading</param>
    /// <param name="file">File from which libpng should be reading</param>
    public: PngReadEnvironment(
      ::png_struct &pngRead, const Nuclex::Pixels::Storage::VirtualFile &file
    ) :
      IsReadOnly(true),
      File(file),
      Position(0) {
      SetupFunctionPointers(*this, pngRead);
    }

    /// <summary>Sets up the functions pointers used by libpng</summary>
    /// <param name="pngReadEnvironment">
    ///   Environment on which the function pointers will be set up
    /// </param>
    /// <param name="pngWrite">Main PNG structure initialized for reading</param>
    protected: static void SetupFunctionPointers(
      PngReadEnvironment &pngReadEnvironment, ::png_struct &pngRead
    );

    /// <summary>Whether the file is read-only, always true for this structure</summary>
    public: bool IsReadOnly;
    /// <summary>File from which the read method is reading data</summary>
    public: const Nuclex::Pixels::Storage::VirtualFile &File;
    /// <summary>Current position of the file pointer</summary>
    public: std::uint64_t Position;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Data passed along to the custom write function for libpng</summary>
  struct PngWriteEnvironment {

    /// <summary>Initializes a new libpng write environment</summary>
    /// <param name="pngWrite">Main PNG structure initialized for writing</param>
    /// <param name="file">File to which libpng should be writing</param>
    public: PngWriteEnvironment(
      ::png_struct &pngWrite, Nuclex::Pixels::Storage::VirtualFile &file
    ) :
      IsReadOnly(false),
      File(file),
      Position(0) {
      SetupFunctionPointers(*this, pngWrite);
    }

    /// <summary>Sets up the functions pointers used by libpng</summary>
    /// <param name="pngWriteEnvironment">
    ///   Environment on which the function pointers will be set up
    /// </param>
    /// <param name="pngWrite">Main PNG structure initialized for writing</param>
    protected: static void SetupFunctionPointers(
      PngWriteEnvironment &pngWriteEnvironment, ::png_struct &pngWrite
    );

    /// <summary>Whether the file is read-only, always false for this structure</summary>
    public: bool IsReadOnly;
    /// <summary>File to which the write method is writing data</summary>
    public: Nuclex::Pixels::Storage::VirtualFile &File;
    /// <summary>Current position of the file pointer</summary>
    public: std::uint64_t Position;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Png

#endif // defined(NUCLEX_PIXELS_HAVE_LIBPNG)

#endif // NUCLEX_PIXELS_STORAGE_PNG_LIBPNGHELPERS_H
