#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_STORAGE_COMPRESSION_COMPRESSIONALGORITHM_H
#define NUCLEX_STORAGE_COMPRESSION_COMPRESSIONALGORITHM_H

#include "Nuclex/Storage/Config.h"

#include <string>
#include <memory>
#include <array>

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  class Compressor;
  class Decompressor;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates data compressors/decompressors of its implemented type</summary>
  class CompressionAlgorithm {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API virtual ~CompressionAlgorithm() = default;

    /// <summary>Returns the human-readable name of the compression algorithm</summary>
    /// <returns>The name of the compression algorithm the factory provides</returns>
    public: virtual const std::string &GetName() const = 0;

    /// <summary>Returns a unique id for the compression algorithm</summary>
    /// <returns>The compression algorithm's unique id</returns>
    /// <remarks>
    ///   <para>
    ///     This should be a unique number that also incorporates the format version.
    ///     It must change when a newer version of the algorithm becomes incompatible with
    ///     older implementations (in both directions, either new can't decompress old or
    ///     old can't decompress new).
    ///   </para>
    ///   <para>
    ///     The recommended form of the ID is to use the first four bytes for a unique
    ///     abbreviation of the algorithm (i.e. 'DFLT' for deflate or 'BRTL' for brotli),
    ///     followed by an ascii format version, i.e. '0001' or '0715' that can either
    ///     start at zero or use the implementation's version number if said implementation
    ///     makes no guarantees about backwards compatibility.
    ///   </para>
    ///   <para>
    ///     This ID may be used to look up the compression algorithm that is used when
    ///     decompressing data, so do not change it lightly as it would render existing
    ///     archives unreadable.
    ///   </para>
    /// </remarks>
    public: virtual std::array<std::uint8_t, 8> GetId() const = 0;

    /// <summary>Whether this compression algorithm is experimental</summary>
    /// <returns>True if the compression algorithm is experimental, false otherwise</returns>
    /// <remarks>
    ///   <para>
    ///     This is a reliability rating: algorithms which are in widespread use are
    ///     regarded as safe (i.e. you're not going to encounter situations in the wild
    ///     where the data doesn't decompress or where the compressor segfaults).
    ///   </para>
    ///   <para>
    ///     Experimental compression algorithms may be just as safe, but aren't proven
    ///     yet. They're perfectly fine for offline compression (i.e. game assets)
    ///     where you can test ahead of time that they decompress correctly.
    ///   </para>
    /// </remarks>
    public: virtual bool IsExperimental() const { return false; }

    /// <summary>
    ///   Returns the average number of CPU cycles this algorithm takes to
    ///   compress one kilobyte of data
    /// </summary>
    /// <returns>The average number of CPU cycles to compress one kilobyte</returns>
    /// <remarks>
    ///   This number is established by running a compression benchmark on various
    ///   files (the pros call it a &quot;corpus&quot;) relevant to the library's usage,
    ///   using as many different CPUs as possible. It is useful to make educated guesses
    ///   about relative peformance between compression algorithms and perhaps even
    ///   roughly estimate compression times for a system.
    /// </remarks>
    public: virtual std::size_t GetCompressionCyclesPerKilobyte() const = 0;

    /// <summary>
    ///   Returns the average size of data compressed with this algorithm as compared
    ///   to its uncompressed size
    /// </summary>
    /// <returns>The average ratio of compressed size to uncompressed size</returns>
    /// <remarks>
    ///   This number is also established by running compression tests on sets of typical
    ///   files that will be accessed (in case of the values that ship with this library,
    ///   various images, 3D models and audio file formats were used to establish a ratio
    ///   that would be typical for packaged game assets)
    /// </remarks>
    public: virtual float GetAverageCompressionRatio() const = 0;

#if 0
    /// <summary>Returns the file extensions this codec is especially suited for</summary>
    /// <returns>A list of file extensions for which this codec is suitable</returns>
    /// <remarks>
    ///   Some algorithms are better suited for certain file types. This method returns
    ///   a list helping the caller decide whether the compression algorithm will achieve
    ///   good results for a known file extension.
    /// </remarks>
    public: virtual const std::vector<std::string> &GetSuitableExtensions() const;
#endif

    /// <summary>Creates a new data compressor</summary>
    /// <returns>A new data compressor of the algorithm's type</returns>
    public: virtual std::unique_ptr<Compressor> CreateCompressor() const = 0;

    /// <summary>Creates a new data decompressor</summary>
    /// <returns>A new data decompressor of the algorithm's type</returns>
    public: virtual std::unique_ptr<Decompressor> CreateDecompressor() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_COMPRESSORFACTORY_H

