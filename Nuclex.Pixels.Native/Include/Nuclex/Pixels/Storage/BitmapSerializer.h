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

#ifndef NUCLEX_PIXELS_STORAGE_BITMAPSERIALIZER_H
#define NUCLEX_PIXELS_STORAGE_BITMAPSERIALIZER_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/Bitmap.h"
#include "Nuclex/Pixels/BitmapInfo.h"

#include <atomic> // for std::atomic
#include <string> // for std::string
#include <vector> // for std::vector
#include <unordered_map> // for std::unordered_map
#include <memory> // for std::unique_ptr
#include <optional> // for std::optional

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class BitmapCodec;
  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allows reading and writing Bitmaps from/to a stream of bytes</summary>
  /// <remarks>
  ///   <para>
  ///     This class enables you to turn Bitmaps into byte streams using common file
  ///     format such as .png, .jpg or any other file format the serializer is given
  ///     a &quot;codec&quot; for.
  ///   </para>
  ///   <para>
  ///     For simple file operations, the serializer has overloads that access plain
  ///     files from a path, turning the loading and saving of common image formats
  ///     into one-liners. For advanced usage, you can use your own VirtualFile
  ///     implementation to read data from any data source you like.
  ///   </para>
  ///   <para>
  ///     The bitmap serializer will select the correct codec either by file extension,
  ///     or, if no file extension was provided, by letting each registered codec
  ///     check the file header. The order in which codecs are tested tries the most
  ///     recently used codecs first (assuming that if your game or application commonly
  ///     uses only one or two file formats)
  ///   </para>
  ///   <example>
  ///     <code>
  ///       int main() {
  ///         Nuclex::Pixels::Storage::BitmapSerializer serializer;
  ///
  ///         Bitmap splashScreen = serializer.Load(u8&quot;splash-screen.png&quot;);
  ///         // Do something with the splash screen bitmap
  ///
  ///         return 0;
  ///       }
  ///     </code>
  ///   </example>
  ///   <para>
  ///     You can use the BitmapSerializer to load and save images on multiple threads.
  ///     The only requirement is that you must not use the <see cref="RegisterCodec" />
  ///     method simultaneously. A good design would be to set up your BitmapSerializer
  ///     instance once, register your custom codecs up front, call
  ///     <code>std::atomic_thread_fence()</code> for good measure and then let your
  ///     threads access the BitmapSerialier as a <code>const</code> reference so they
  ///     can only access the <see cref="Load" /> and <see cref="Save" /> methods.
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE BitmapSerializer {

    /// <summary>Initializes a new bitmap serializer</summary>
    public: NUCLEX_PIXELS_API BitmapSerializer();

    /// <summary>Frees all resources owned by a bitmap serializer</summary>
    public: NUCLEX_PIXELS_API ~BitmapSerializer();

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="codec">Bitmap codec that will be registered</param>
    public: NUCLEX_PIXELS_API void RegisterCodec(std::unique_ptr<BitmapCodec> &&codec);

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="TCodec">Type of bitmap codec that will be registered</param>
    public: template<typename TCodec>
    void RegisterCodec() {
      std::unique_ptr<BitmapCodec> codec = std::make_unique<TCodec>();
      RegisterCodec(std::move(codec));
    }

#if 0 // This method can be written, but is it needed?
    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="TCodec">Type of bitmap codec that will be registered</param>
    public: template<typename TCodec>
    void UnregisterCodec() {
      UnregisterCodec(typeid(TCodec));
    }
#endif

    /// <summary>Tries to read informations about a bitmap</summary>
    /// <param name="file">File from which informations will be read</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the bitmap, if it is a supported format</returns>
    public: NUCLEX_PIXELS_API std::optional<BitmapInfo> TryReadInfo(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Tries to read informations about a bitmap</summary>
    /// <param name="path">Path of the file informations will be read from</param>
    /// <returns>Informations about the bitmap, if it is a supported format</returns>
    public: NUCLEX_PIXELS_API std::optional<BitmapInfo> TryReadInfo(const std::string &path) const;

    /// <summary>Checks whether the bitmap store can load the specified file</summary>
    /// <param name="file">File the bitmap store will check</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>True if the bitmap store thinks it can load the file</returns>
    public: NUCLEX_PIXELS_API bool CanLoad(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Checks whether the bitmap store can load the specified file</summary>
    /// <param name="path">Path of the file the bitmap store will check</param>
    /// <returns>True if the bitmap store thinks it can load the file</returns>
    public: NUCLEX_PIXELS_API bool CanLoad(const std::string &path) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="file">File the bitmap store will load</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>The bitmap loaded from the specified file</returns>
    public: NUCLEX_PIXELS_API Bitmap Load(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="path">Path of the file the bitmap store will load</param>
    /// <returns>The bitmap loaded from the specified file</returns>
    public: NUCLEX_PIXELS_API Bitmap Load(const std::string &path) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="file">File the bitmap store will load</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>The bitmap loaded from the specified file</returns>
    /// <remarks>
    ///   This method is useful if you have an application that can, for example, evict
    ///   textures from video memory (or use a graphics API where the OS can release textures
    ///   during task switching). Instead of keeping a system memory copy of each texture,
    ///   a bitmap can be constructed to access the texture memory directly and this method
    ///   can then restore the texture to its former glory).
    ///   copy, you can
    /// </remarks>
    public: NUCLEX_PIXELS_API void Reload(
      Bitmap &exactFittingBitmap,
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="path">Path of the file the bitmap store will load</param>
    /// <returns>The bitmap loaded from the specified file</returns>
    /// <remarks>
    ///   This method is useful if you have an application that can, for example, evict textures
    ///   from video memory (or use a graphics API where the OS can release textures during
    ///   task switching). Instead of keeping a system memory copy of each texture, a bitmap
    ///   can be constructed to access the texture memory directly and this method can then
    ///   restore the texture to its former glory).
    ///   copy, you can
    /// </remarks>
    public: NUCLEX_PIXELS_API void Reload(
      Bitmap &exactFittingBitmap, const std::string &path
    ) const;

    /// <summary>Saves a bitmap into the specified file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="file">File the bitmap will be saved into</param>
    /// <param name="extension">File extension used to select the file format</param>
    /// <param name="compressionEffortHint">
    ///   How much effort (CPU time) should be put into reducing the size of the image.
    ///   Image formats may ignore this parameter completely. If possible, setting it
    ///   to 0.0 will write an uncompressed image file.
    /// </param>
    /// <param name="outputQualityHint">
    ///   How much image quality should be prioritized over achieving small file sizes.
    ///   This parameter will only be used by lossy image formats supporting such
    ///   settings in their compression algorithms.
    /// </param>
    public: NUCLEX_PIXELS_API void Save(
      const Bitmap &bitmap, VirtualFile &file, const std::string &extension,
      float compressionEffortHint = 0.75f, float outputQualityHint = 0.95f
    ) const;

    /// <summary>Saves a bitmap into the specified file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="path">Path under which the file will be saved</param>
    /// <param name="extension">
    ///   File extension used to select the file format. Leave empty to try and obtain
    ///   the extension from the specified target path.
    /// </param>
    /// <param name="compressionEffortHint">
    ///   How much effort (CPU time) should be put into reducing the size of the image.
    ///   Image formats may ignore this parameter completely. If possible, setting it
    ///   to 0.0 will write an uncompressed image file.
    /// </param>
    /// <param name="outputQualityHint">
    ///   How much image quality should be prioritized over achieving small file sizes.
    ///   This parameter will only be used by lossy image formats supporting such
    ///   settings in their compression algorithms.
    /// </param>
    public: NUCLEX_PIXELS_API void Save(
      const Bitmap &bitmap, const std::string &path,
      const std::string &extension = std::string(),
      float compressionEffortHint = 0.75f, float outputQualityHint = 0.95f
    ) const;

    /// <summary>Builds a new iterator that checks codecs in the most likely order</summary>
    /// <param name="extension">File extension, if known</param>
    /// <param name="tryCodecCallback">
    ///   Address of a method that will be called to try each registered codec
    /// </param>
    /// <returns>True as soon as one codec reports success, false if none did</returns>
    /// <remarks>
    ///   This is only a template so I don't have do expose the iterator implementation
    ///   in a public header. There's exactly one specialization of the method.
    /// </remarks>
    private: template<typename TOutput>
    bool tryCodecsInOptimalOrder(
      const std::string &extension,
      bool (*tryCodecCallback)(
        const BitmapCodec &codec, const std::string &extension, TOutput &result
      ),
      TOutput &result
    ) const;

    /// <summary>Updates the list of most recently used codecs</summary>
    /// <param name="codecIndex">Index of the codec that was most recently used</param>
    private: void updateMostRecentCodecIndex(std::size_t codecIndex) const;

    /// <summary>Maps file extensions to codec indices</summary>
    private: typedef std::unordered_map<std::string, std::size_t> ExtensionCodecIndexMap;
    /// <summary>Stores a sequential list of codecs</summary>
    private: typedef std::vector<std::unique_ptr<BitmapCodec>> CodecVector;

    /// <summary>Allows the bitmap store to look up a codec by its file extension</summary>
    /// <remarks>
    ///   Extensions are stored in UTF-8 folded lowercase for case insensitivity.
    /// </remarks>
    private: ExtensionCodecIndexMap codecsByExtension;
    /// <summary>Codecs that have been registered with the bitmap store</summary>
    private: CodecVector codecs;
    /// <summary>Codec that was most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> mostRecentCodecIndex;
    /// <summary>Codec that was second-most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> secondMostRecentCodecIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_BITMAPSERIALIZER_H
