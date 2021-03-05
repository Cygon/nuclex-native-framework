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

#ifndef NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSIONALGORITHM_H
#define NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSIONALGORITHM_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "Nuclex/Storage/Compression/CompressionAlgorithm.h"

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides compressors and decompressors using the CSC algorithm</summary>
  class CscCompressionAlgorithm : public CompressionAlgorithm {

    /// <summary>Quality parameter that results in the fastest compression</summary>
    public: static const int FastestQuality;
    /// <summary>Quality parameter that results in the highest ratio compression</summary>
    public: static const int StrongestQuality;
    /// <summary>Quality parameter that results in reasonable compression</summary>
    /// <remarks>
    ///   There's usually a point where a compression algorithm's time requirement
    ///   rises exponentially while the gains on terms of compression ratio are just
    ///   minimal. This quality tries to reflect a point before.
    /// </remarks>
    public: static const int DefaultQuality;

    /// <summary>Initializes the CSCcompressor and decompressor factory</summary>
    /// <param name="level">CSC compression level that will be used</param>
    public: CscCompressionAlgorithm(int level);

    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~CscCompressionAlgorithm() override = default;

    /// <summary>Returns the human-readable name of the compression algorithm</summary>
    /// <returns>The name of the compression algorithm the factory provides</returns>
    public: const std::string &GetName() const override { 
      return this->name;
    }

    /// <summary>Returns a unique id for the compression algorithm</summary>
    /// <returns>The compression algorithm's unique id</returns>
    public: std::array<std::uint8_t, 8> GetId() const override {
      return std::array<std::uint8_t, 8> {
        'C', 'S', 'A', 'C', '0', '0', '0', '1'
      };
    }

    /// <summary>
    ///   Returns the average number of CPU cycles this algorithm runs for to
    ///   compress one kilobyte of data
    /// <summary>
    /// <returns>The avergae number of CPU cycles to comrpess one kilobyte</returns>
    public: std::size_t GetCompressionCyclesPerKilobyte() const override {
      return 10000;
    };

    /// <summary>
    ///   Returns the average size of data compressed with this algorithm as compared
    ///   to its uncompressed size
    /// </summary>
    /// <returns>The average ratio of compressed size to uncompressed size</returns>
    public: float GetAverageCompressionRatio() const override {
      return 0.8f;
    }

    /// <summary>Whether this compression algorithm is experimental</summary>
    /// <returns>True if the compression algorithm is experimental, false otherwise</returns>
    public: bool IsExperimental() const override { return true; }

    /// <summary>Creates a new data compressor</summary>
    /// <returns>A new data compressor of the algorithm's type</returns>
    public: std::unique_ptr<Compressor> CreateCompressor() const override;

    /// <summary>Creates a new data decompressor</summary>
    /// <returns>A new data decompressor of the algorithm's type</returns>
    public: std::unique_ptr<Decompressor> CreateDecompressor() const override;

    /// <summary>The name of the compression algorithm</summary>
    private: std::string name;
    /// <summary>Compression level that will be used when compressing things</summary>
    private: int level;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)

#endif // NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSOR_H
