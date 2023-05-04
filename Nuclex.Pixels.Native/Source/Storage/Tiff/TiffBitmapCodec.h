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

#ifndef NUCLEX_PIXELS_STORAGE_TIFF_TIFFBITMAPCODEC_H
#define NUCLEX_PIXELS_STORAGE_TIFF_TIFFBITMAPCODEC_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#include "Nuclex/Pixels/Storage/BitmapCodec.h"

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Loads and saves images in the older TIFF file format</summary>
  class TiffBitmapCodec : public BitmapCodec {

    /// <summary>Initializes a new TIFF bitmap codec</summary>
    public: TiffBitmapCodec();
    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~TiffBitmapCodec() = default;

    /// <summary>Gives the name of the file format implemented by this codec</summary>
    /// <returns>The name of the file format this codec implements</returns>
    public: const std::string &GetName() const override { return this->name; }

    /// <summary>Provides commonly used file extensions for this codec</summary>
    /// <returns>The commonly used file extensions in order of preference</returns>
    public: const std::vector<std::string> &GetFileExtensions() const override {
      return this->knownFileExtensions;
    }

    /// <summary>Checks if the codec is able to load the specified file</summary>
    /// <param name="source">Source data that will be checked for loadbility</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec is able to load the specified file</returns>
    public: bool CanLoad(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const override;

    /// <summary>Checks if the codec is able to save bitmaps to storage</summary>
    /// <returns>True if the codec supports saving bitmaps</returns>
    public: bool CanSave() const override;

    /// <summary>Tries to read informations for a bitmap</summary>
    /// <param name="source">Source data from which the informations should be extracted</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the bitmap, if the codec is able to load it</returns>
    public: std::optional<BitmapInfo> TryReadInfo(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const override;

    /// <summary>Tries to load the specified file as a bitmap</summary>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>
    ///   The bitmap loaded from the specified file data or an empty value if the file format
    ///   is not supported by the codec
    /// </returns>
    public: virtual std::optional<Bitmap> TryLoad(
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const override;

    /// <summary>Tries to load the specified file into an exciting bitmap</summary>
    /// <param name="exactlyFittingBitmap">
    ///   Bitmap matching the exact dimensions of the file to be loaded
    /// </param>
    /// <param name="source">Source data the bitmap will be loaded from</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>True if the codec was able to load the bitmap, false otherwise</returns>
    public: bool TryReload(
      Bitmap &exactlyFittingBitmap,
      const VirtualFile &source, const std::string &extensionHint = std::string()
    ) const override;

    /// <summary>Saves the specified bitmap into a file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="target">File into which the bitmap will be saved</param>
    /// <param name="compressionEffortHint">
    ///   How much effort (CPU time) should be put into reducing the size of the image.
    /// </param>
    /// <param name="outputQualityHint">
    ///   How much image quality should be prioritized over achieving small file sizes.
    /// </param>
    public: void Save(
      const Bitmap &bitmap, VirtualFile &target,
      float compressionEffortHint = 0.75f, float outputQualityHint = 0.95f
    ) const override;

    /// <summary>Human-readable name of the file format this codec implements</summary>
    private: std::string name;
    /// <summary>File extensions this file format is known to use</summary>
    private: std::vector<std::string> knownFileExtensions;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#endif // NUCLEX_PIXELS_STORAGE_TIFF_TIFFBITMAPCODEC_H
