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

#include <string>
#include <vector>
#include <memory>
#include <map>

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
  ///     file from a path, turning the loading and saving of common image formats
  ///     to one-liners. For advanced usage, you can use your own VirtualFile
  ///     implementation to read data from any data source you like.
  ///   </para>
  ///   <para>
  ///     The bitmap serializer will select the correct codec either by file extension,
  ///     or, if no file extension was provided, by letting each registered codec
  ///     check the file header. The order in which codecs are tested tries the most
  ///     recently used codecs first (assuming that if your game or application commonly
  ///     uses only one or two file formats)
  ///   </para>
  /// </remarks>
  class BitmapSerializer {

    /// <summary>Initializes a new bitmap serializer</summary>
    public: NUCLEX_PIXELS_API BitmapSerializer();

    /// <summary>Frees all resources owned by a bitmap serializer</summary>
    public: NUCLEX_PIXELS_API ~BitmapSerializer();

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="TCodec">Type of bitmap codec that will be registered</param>
    public: template<typename TCodec>
    void RegisterCodec() {
      RegisterCodec(std::make_unique<TCodec>());
    }

#if 0 // This method can be written, but is it needed?
    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="TCodec">Type of bitmap codec that will be registered</param>
    public: template<typename TCodec>
    void UnregisterCodec() {
      UnregisterCodec(typeid(TCodec));
    }
#endif

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="codec">Bitmap codec that will be registered</param>
    public: NUCLEX_PIXELS_API void RegisterCodec(std::unique_ptr<BitmapCodec> &&codec);

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
    public: NUCLEX_PIXELS_API void Reload(
      Bitmap &exactFittingBitmap,
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="path">Path of the file the bitmap store will load</param>
    /// <returns>The bitmap loaded from the specified file</returns>
    public: NUCLEX_PIXELS_API void Reload(
      Bitmap &exactFittingBitmap, const std::string &path
    ) const;

    /// <summary>Saves a bitmap into the specified file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="file">File the bitmap will be saved into</param>
    /// <param name="extension">File extension used to select the file format</param>
    public: NUCLEX_PIXELS_API void Save(
      const Bitmap &bitmap, VirtualFile &file, const std::string &extension
    ) const;

    /// <summary>Saves a bitmap into the specified file</summary>
    /// <param name="bitmap">Bitmap that will be saved into a file</param>
    /// <param name="path">Path under which the file will be saved</param>
    /// <param name="extension">File extension used to select the file format</param>
    public: NUCLEX_PIXELS_API void Save(
      const Bitmap &bitmap, const std::string &path, const std::string &extension
    ) const;

    /// <summary>Builds a new iterator that checks the codecs in most likely order</summary>
    /// <param name="extension">File extension, if known</param>
    /// <returns>An iterator that returns the codec indices to try in order</returns>
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

    /// <summary>Maps file extensions to codecs</summary>
    private: typedef std::map<std::string, std::size_t> ExtensionCodecIndexMap;
    /// <summary>Stores a sequential list of codecs</summary>
    private: typedef std::vector<std::unique_ptr<BitmapCodec>> CodecVector;

    /// <summary>Allows the bitmap store to look up a codec by its file extension</summary>
    private: ExtensionCodecIndexMap codecsByExtension;
    /// <summary>Codecs that have been registered with the bitmap store</summary>
    private: CodecVector codecs;
    /// <summary>Codec that was most recently accessed, -1 if none</summary>
    private: mutable std::size_t mostRecentCodecIndex;
    /// <summary>Codec that was second-most recently accessed, -1 if none</summary>
    private: mutable std::size_t secondMostRecentCodecIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_BITMAPSERIALIZER_H
