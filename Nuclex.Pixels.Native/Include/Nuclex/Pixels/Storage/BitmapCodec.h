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

#ifndef NUCLEX_PIXELS_STORAGE_BITMAPCODEC_H
#define NUCLEX_PIXELS_STORAGE_BITMAPCODEC_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/BitmapInfo.h"
#include "Nuclex/Pixels/Bitmap.h"

#include <string> // for std::string
#include <vector> // for std::vector
#include <optional> // for std::optional

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Codec that loads and saves bitmaps in a predefined file format</summary>
  class NUCLEX_PIXELS_TYPE BitmapCodec {

    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~BitmapCodec() = default;

    /// <summary>Gives the name of the file format implemented by this codec</summary>
    /// <returns>The name of the file format this codec implements</returns>
    public: virtual const std::string &GetName() const = 0;

    /// <summary>Provides commonly used file extensions for this codec</summary>
    /// <returns>The commonly used file extensions in order of preference</returns>
    public: virtual const std::vector<std::string> &GetFileExtensions() const = 0;

    /// <summary>Tries to read informations for a bitmap</summary>
    /// <param name="source">Source data from which the informations should be extracted</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the bitmap, if the codec is able to load it</returns>
    public: virtual std::optional<BitmapInfo> TryReadInfo(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const = 0;

    /// <summary>Checks if the codec is able to load the specified file</summary>
    /// <param name="source">Source data that will be checked for loadbility</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec is able to load the specified file</returns>
    public: virtual bool CanLoad(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const = 0;

    /// <summary>Checks if the codec is able to save bitmaps to storage</summary>
    /// <returns>True if the codec supports saving bitmaps</returns>
    public: virtual bool CanSave() const = 0;

    /// <summary>Tries to load the specified file as a bitmap</summary>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>
    ///   The bitmap loaded from the specified file data or an empty value if the file format
    ///   is not supported by the codec
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method should, like CanLoad(), check if the provided data source contains
    ///     an image file of the file format implemented by the codec. If the file format
    ///     doesn't match up, it should return an empty OptionalBitmap instance (that's
    ///     the 'try' part).
    ///   </para>
    ///   <para>
    ///     On any other error (i.e. exception from the data source, corrupted image data,
    ///     unsupported version of the file format, etc.), the codec must throw an exception
    ///     rather than return an empty <code>std::optional&lt;Bitmap&gt;</code> instance.
    ///   </para>
    /// </remarks>
    public: virtual std::optional<Bitmap> TryLoad(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const = 0;

    /// <summary>Tries to load the specified file into an exciting bitmap</summary>
    /// <param name="exactlyFittingBitmap">
    ///   Bitmap matching the exact dimensions of the file to be loaded
    /// </param>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec was able to load the bitmap, false otherwise</returns>
    /// <remarks>
    ///   <para>
    ///     The behavior of this method should be identical to the TryLoad() method, namely
    ///     it should only return false if the data source seems to contain a different file
    ///     format than is implemented by the codec. If the image is broken or unloadable
    ///     for any other reason, an exception must be thrown.
    ///   </para>
    ///   <para>
    ///     This variant of the load function is useful for game engines that want to load
    ///     images directly into memory provided by their 3D API (they can construct a Bitmap
    ///     that accesses foreign memory, then load into that). This reduces the overhead of
    ///     having to load an image into freshly allocated memory only to copy it over into
    ///     a texture.
    ///   </para>
    /// </remarks>
    public: virtual bool TryReload(
      Bitmap &exactlyFittingBitmap,
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const = 0;

    /// <summary>Saves the specified bitmap into a file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="target">File into which the bitmap will be saved</param>
    /// <param name="compressionEffortHint">
    ///   How much effort (CPU time) should be put into reducing the size of the image.
    /// </param>
    /// <param name="outputQualityHint">
    ///   How much image quality should be prioritized over achieving small file sizes.
    /// </param>
    public: virtual void Save(
      const Bitmap &bitmap, VirtualFile &target,
      float compressionEffortHint = 0.75f, float outputQualityHint = 0.95f
    ) const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_BITMAPCODEC_H
