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
#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Storage/BitmapCodec.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "Nuclex/Support/Text/StringConverter.h"

// Disable things that have not yet made their way into
// the GitHub repository. If you want these, use the Subversion repo!
#undef NUCLEX_PIXELS_HAVE_LIBTIFF
#undef NUCLEX_PIXELS_HAVE_OPENEXR
#undef NUCLEX_PIXELS_HAVE_LIBWEBP
#undef NUCLEX_PIXELS_HAVE_LIBAVIF

#if defined(NUCLEX_PIXELS_HAVE_LIBWEBP)
#include "WebP/WebPBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
#include "Tiff/TiffBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
#include "Png/PngBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
#include "Jpeg/JpegBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)
#include "Exr/ExrBitmapCodec.h"
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invalid size marker for the most recent codec indices</summary>
  constexpr std::size_t InvalidIndex = std::size_t(-1);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper used to pass information through lambda methods</summary>
  struct FileAndBitmap {

    /// <summary>File the bitmap serializer has been tasked with reading</summary>
    public: const Nuclex::Pixels::Storage::VirtualFile *File;

    /// <summary>Container that receives the loaded bitmap if successful</summary>
    public: std::optional<Nuclex::Pixels::Bitmap> Bitmap;

    /// <summary>Bitmap into whih the TryLoad() methods will load the pixels</summary>
    public: Nuclex::Pixels::Bitmap *TargetBitmap;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper used to pass information through lambda methods</summary>
  struct FileAndBitmapInfo {

    /// <summary>File the bitmap serializer has been tasked with reading</summary>
    public: const Nuclex::Pixels::Storage::VirtualFile *File;

    /// <summary>Information container that will be filled if successful</summary>
    public: std::optional<Nuclex::Pixels::BitmapInfo> BitmapInfo;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  BitmapSerializer::BitmapSerializer() :
    mostRecentCodecIndex(InvalidIndex),
    secondMostRecentCodecIndex(InvalidIndex) {
#if defined(NUCLEX_PIXELS_HAVE_LIBWEBP)
    RegisterCodec(std::make_unique<WebP::WebPBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
    RegisterCodec(std::make_unique<Tiff::TiffBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
    RegisterCodec(std::make_unique<Png::PngBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
    RegisterCodec(std::make_unique<Jpeg::JpegBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)
    RegisterCodec(std::make_unique<Exr::ExrBitmapCodec>());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  BitmapSerializer::~BitmapSerializer() {}

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::RegisterCodec(std::unique_ptr<BitmapCodec> &&codec) {
    std::size_t codecCount = this->codecs.size();

    // This should be a one-liner, but clang has a nonsensical warning then typeid()
    // is called with an expression that needs to be evaluated at runtime :-(
    const BitmapCodec &newCodec = *codec.get();
    const std::type_info &newType = typeid(newCodec);

    // Make sure this exact type isn't registered yet
    for(std::size_t index = 0; index < codecCount; ++index) {
      const BitmapCodec &checkedCodec = *this->codecs[index].get();
      const std::type_info &existingType = typeid(checkedCodec);
      if(newType == existingType) {
        throw std::runtime_error(u8"Codec already registered");
      }
    }

    const std::vector<std::string> &extensions = codec->GetFileExtensions();

    // Register the new codec into our list
    this->codecs.push_back(std::move(codec));

    // Update the extension lookup map for quick codec finding
    std::size_t extensionCount = extensions.size();
    for(std::size_t index = 0; index < extensionCount; ++index) {
      using Nuclex::Support::Text::StringConverter;

      const std::string &extension = extensions[index];
      std::string::size_type extensionLength = extension.length();

      if(extensionLength > 0) {
        if(extension[0] == '.') {
          if(extensionLength > 1) {
            std::string lowerExtension = StringConverter::FoldedLowercaseFromUtf8(
              extension.substr(1)
            );
            this->codecsByExtension.insert(
              ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
            );
          }
        } else {
          std::string lowerExtension = StringConverter::FoldedLowercaseFromUtf8(extension);
          this->codecsByExtension.insert(
            ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
          );
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<BitmapInfo> BitmapSerializer::TryReadInfo(
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmapInfo fileProvider;
    fileProvider.File = &file;

    bool wasLoaded = tryCodecsInOptimalOrder<FileAndBitmapInfo>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmapInfo &fileAndBitmap) {
        fileAndBitmap.BitmapInfo = std::move(
          codec.TryReadInfo(*fileAndBitmap.File, extension)
        );
        return fileAndBitmap.BitmapInfo.has_value();
      },
      fileProvider
    );
    if(wasLoaded) {
      return fileProvider.BitmapInfo;
    } else {
      return std::optional<BitmapInfo>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<BitmapInfo> BitmapSerializer::TryReadInfo(const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WINDOWS)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        return TryReadInfo(*file.get(), path.substr(extensionDotIndex + 1));
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      return TryReadInfo(*file.get());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool BitmapSerializer::CanLoad(
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;

    return tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        return codec.CanLoad(*fileAndBitmap.File, extension);
      },
      fileProvider
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool BitmapSerializer::CanLoad(const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WINDOWS)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        return CanLoad(*file.get(), path.substr(extensionDotIndex + 1));
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      return CanLoad(*file.get());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap BitmapSerializer::Load(
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;

    bool wasLoaded = tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        std::optional<Bitmap> loadedBitmap = codec.TryLoad(*fileAndBitmap.File, extension);
        if(loadedBitmap.has_value()) {
          fileAndBitmap.Bitmap = std::move(loadedBitmap);
          return true;
        } else {
          return false;
        }
      },
      fileProvider
    );
    if(wasLoaded) {
      return fileProvider.Bitmap.value();
    } else {
      throw Errors::FileFormatError(u8"File format not supported by any registered codec");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap BitmapSerializer::Load(const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WINDOWS)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        return Load(*file.get(), path.substr(extensionDotIndex + 1));
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      return Load(*file.get());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Reload(
    Bitmap &exactFittingBitmap,
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;
    fileProvider.TargetBitmap = &exactFittingBitmap;

    bool wasLoaded = tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        if(codec.TryReload(*fileAndBitmap.TargetBitmap, *fileAndBitmap.File, extension)) {
          return true;
        } else {
          return false;
        }
      },
      fileProvider
    );

    if(!wasLoaded) {
      throw Errors::FileFormatError("File format not supported by any registered codec");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Reload(Bitmap &exactFittingBitmap, const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WINDOWS)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        Reload(exactFittingBitmap, *file.get(), path.substr(extensionDotIndex + 1));
        return;
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      Reload(exactFittingBitmap, *file.get());
      return;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Save(
    const Bitmap &bitmap, VirtualFile &file, const std::string &extension,
    float compressionEffortHint /* = 0.75f */, float outputQualityHint /* = 0.95f */
  ) const {
    const BitmapCodec &codec = getSavingCodecForExtension(extension);
    codec.Save(bitmap, file, compressionEffortHint, outputQualityHint);
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Save(
    const Bitmap &bitmap, const std::string &path,
    const std::string &extension /* = std::string() */,
    float compressionEffortHint /* = 0.75f */, float outputQualityHint /* = 0.95f */
  ) const {

    // Only try to pick the extension out of the specified path if the caller hasn't
    // explicitly filled the extension parameter
    if(extension.empty()) {
      std::string::size_type extensionDotIndex = path.find_last_of('.');
  #if defined(NUCLEX_PIXELS_WINDOWS)
      std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
  #else
      std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
  #endif

      // Check if the provided path contains a file extension and if so, use it to select
      // the bitmap codec to attempt to save the image file with.
      if(extensionDotIndex != std::string::npos) {
        bool dotBelongsToFilename = (
          (lastPathSeparatorIndex == std::string::npos) ||
          (extensionDotIndex > lastPathSeparatorIndex)
        );
        if(dotBelongsToFilename) {
          std::string detectedExtension = path.substr(extensionDotIndex + 1);
          const BitmapCodec &codec = getSavingCodecForExtension(detectedExtension);
          std::unique_ptr<VirtualFile> file = VirtualFile::OpenRealFileForWriting(path, true);
          codec.Save(bitmap, *file.get(), compressionEffortHint, outputQualityHint);
          return;
        }
      }

      // No explicit file extension specified and the target path didn't have one either,
      // so we've got no clue which image file format the caller wants us to use...
      std::string message(u8"Could not deduce saved image file format from file extension '", 62);
      message.append(path);
      message.push_back(u8'\'');
      throw Errors::FileFormatError(message);
    }

    // Extension was specified explicitly, look it up
    {
      const BitmapCodec &codec = getSavingCodecForExtension(extension);
      std::unique_ptr<VirtualFile> file = VirtualFile::OpenRealFileForWriting(path, true);
      codec.Save(bitmap, *file.get(), compressionEffortHint, outputQualityHint);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  const BitmapCodec &BitmapSerializer::getSavingCodecForExtension(
    const std::string &extension
  ) const {
    using Nuclex::Support::Text::StringConverter;

    // Do a lookup for the codec responsible for the specified extension
    std::string foldedLowercaseExtension = StringConverter::FoldedLowercaseFromUtf8(extension);
    ExtensionCodecIndexMap::const_iterator iterator = this->codecsByExtension.find(
      foldedLowercaseExtension
    );

    // If no registered codec is associated with the specified extension, we fail.
    // Since this method is used when saving, we can use a tailored error message.
    if(iterator == this->codecsByExtension.end()) {
      std::string message(u8"No codec registered to save image file with extension '", 55);
      message.append(extension);
      message.push_back(u8'\'');
      throw Errors::FileFormatError(message);
    }

    // If a codec has been registered to this file extension, it might still be that
    // the codec is a read-only implementation of the file format, so check this.
    const BitmapCodec &codec = *this->codecs[iterator->second].get();
    if(!codec.CanSave()) {
      std::string message(u8"Codec '", 7);
      message.append(codec.GetName());
      message.append(u8"' does not support image saving");
      throw Errors::FileFormatError(message);
    }

    return codec;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TOutput>
  bool BitmapSerializer::tryCodecsInOptimalOrder(
    const std::string &extension,
    bool (*tryCodecCallback)(
      const BitmapCodec &codec, const std::string &extension, TOutput &result
    ),
    TOutput &result
  ) const {
    std::size_t hintCodecIndex;

    // If an extension hint was provided, try the codec registered for the extension first
    if(extension.empty()) {
      hintCodecIndex = InvalidIndex;
    } else {
      using Nuclex::Support::Text::StringConverter;
      std::string foldedLowercaseExtension = StringConverter::FoldedLowercaseFromUtf8(extension);
      ExtensionCodecIndexMap::const_iterator iterator = (
        this->codecsByExtension.find(foldedLowercaseExtension)
      );
      if(iterator == this->codecsByExtension.end()) {
        hintCodecIndex = InvalidIndex;
      } else {
        hintCodecIndex = iterator->second;
        if(tryCodecCallback(*this->codecs[hintCodecIndex].get(), extension, result)) {
          updateMostRecentCodecIndex(hintCodecIndex);
          return true;
        }
      }
    }

    // Look up the two most recently used codecs (we don't care about race conditions here,
    // in the rare case of one occurring, we'll simple be a little less efficient and not
    // have the right codec in the MRU list...
    std::size_t mostRecent = this->mostRecentCodecIndex;
    std::size_t secondMostRecent = this->secondMostRecentCodecIndex;

    // Try the most recently used codec. It may be set to 'InvalidIndex' if this
    // is the first call to Load(). Don't try if it's the same as the extension hint.
    if((mostRecent != InvalidIndex) && (mostRecent != hintCodecIndex)) {
      if(tryCodecCallback(*this->codecs[mostRecentCodecIndex].get(), extension, result)) {
        updateMostRecentCodecIndex(mostRecent);
        return true;
      }
    }

    // Try the second most recently used logic. It, too, may be set to 'InvalidIndex'.
    // Also avoid retrying codecs we already tried.
    if(
      (secondMostRecent != InvalidIndex) &&
      (secondMostRecent != mostRecent) &&
      (secondMostRecent != hintCodecIndex)
    ) {
      if(tryCodecCallback(*this->codecs[secondMostRecent].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // Hint was not provided or wrong, most recently used codecs didn't work,
    // so go through all remaining codecs.
    std::size_t codecCount = this->codecs.size();
    for(std::size_t index = 0; index < codecCount; ++index) {
      if((index == mostRecent) || (index == secondMostRecent) || (index == hintCodecIndex)) {
        continue;
      }

      if(tryCodecCallback(*this->codecs[index].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // No codec can load the file, we give up
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::updateMostRecentCodecIndex(std::size_t codecIndex) const {
    this->secondMostRecentCodecIndex.store(
      this->mostRecentCodecIndex.load(std::memory_order::memory_order_relaxed),
      std::memory_order::memory_order_relaxed
    );

    this->mostRecentCodecIndex.store(codecIndex, std::memory_order::memory_order_relaxed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
