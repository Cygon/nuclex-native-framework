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

#ifndef NUCLEX_STORAGE_COMPRESSION_COMPRESSIONPROVIDER_H
#define NUCLEX_STORAGE_COMPRESSION_COMPRESSIONPROVIDER_H

#include "Nuclex/Storage/Config.h"

#include <cstddef> // for std::size_t
#include <vector> // for std::vector
#include <memory> // for std::array

// CHECK: Is there a better name for the CompressionProvider class?
//   CompressionSystem? CompressionManager? CompressionHandler?

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  class CompressionAlgorithm;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides data compression algorithms</summary>
  class CompressionProvider {

    /// <summary>Initializes a new compression provider with the built-in algorithms</summary>
    public: NUCLEX_STORAGE_API CompressionProvider();
    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API ~CompressionProvider();

    /// <summary>Counts the number of available compression algorithms</summary>
    /// <returns>The number of algorithms known to the compression provider</returns>
    public: NUCLEX_STORAGE_API std::size_t CountAlgorithms() const {
      return this->algorithms.size();
    }

    /// <summary>Accesses the compression algorithm with the specified index</summary>
    /// <param name="index">Index of the compression algorithm that will be accessed</param>
    /// <returns>The compression algorithm with the specified index</returns>
    /// <remarks>
    ///   Normally, you should use the <see cref="GetOptimalAlgorithm" /> method to
    ///   directly obtain a compression algorithm that fits your requirements, but if
    ///   you want to access the registered algorithms individually (for example to run
    ///   benchmarks), this method can be helpful.
    /// </remarks>
    public: NUCLEX_STORAGE_API const CompressionAlgorithm &GetAlgorithm(
      std::size_t index
    ) const {
      return *this->algorithms.at(index).get();
    }

    /// <summary>Looks up a compression algorithm by its ID</summary>
    /// <param name="algorithmId">Unique ID of the compression algorithm</param>
    /// <returns>The compression algorithm with the specified ID</returns>
    /// <remarks>
    ///   The ID of a compression algorithm can be obtained by calling the
    ///   <see cref="CompressionAlgorithm.GetId" /> method. This method is useful if
    ///   you wish to use a specific compression algorithm (i.e. because you need it
    ///   to decompressed existing compressed data or want to compress using
    ///   a specific format for compatibility purposes.
    /// </remarks>
    public: NUCLEX_STORAGE_API const CompressionAlgorithm &GetAlgorithm(
      const std::array<std::uint8_t, 8> &algorithmId
    ) const;

    /// <summery>
    ///   Returns the strongest compression algorithm that promises to compress
    ///   the specified amount of data within the specified time
    /// <summary>
    /// <param name="uncompressedDataSize">Amount of data that will be compressed</param>
    /// <param name="compressionTimeSeconds">Time that is available for compression</param>
    /// <returns>
    ///   A compression algorithm fitting the specified performance constraints
    /// </returns>
    /// <remarks>
    ///   How fast a compressor can process data depends on the system's performance
    ///   and on the data being compressed, so it is not possible to make an accurate
    ///   prediction for how long a compressor will need in the end. The average performance
    ///   is calculated by running a benchmark, allowing this method to produce somewhat
    ///   realistic estimate on how fast each algorithm would complete.
    /// </remarks>
    public: NUCLEX_STORAGE_API const CompressionAlgorithm &GetOptimalAlgorithm(
      std::size_t uncompressedDataSize, float compressionTimeSeconds
    ) const;

    /// <summary>
    ///   Returns the compression algorithm with the strongest compression whose
    ///   time per kilobyte is under than the specified percentile of all known algorithms
    /// </summary>
    /// <param name="performanceFactor">
    ///   Performance percentile the requested algorithm should be below
    /// </param>
    /// <returns>
    ///   The strongest compression algorithm that takes less time then the specified
    ///   percentile of all registered algorithms.
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method is useful to obtain a strong compression algorithm while filtering out
    ///     any esoteric algorithms that would be extremely slow. Because such algorithms
    ///     reliably explode in processing time, even picking a high percentile, such as
    ///     requiring the compression algorithm to complete in under 90 percent (0.9) of
    ///     the time the slowest algorithm would require, will eliminate the dangerously
    ///     slow ones.
    ///   </para>
    ///   <para>
    ///     You can also use this method to obtain a fast compression algorithm that
    ///     is not merely shoveling uncompressed data by specifying a very low percentile.
    ///   </para>
    /// </remarks>
    public: NUCLEX_STORAGE_API const CompressionAlgorithm &GetStrongAlgorithm(
      float performanceFactor = 0.75f
    ) const;

    /// <summary>
    ///   Searches for the strongest compression algorithm that promises to run faster
    ///   than the specified cycle count
    /// </summary>
    /// <param name="maximumCyclesPerKilobyte">
    ///   Highest number of CPU cycles the selected algorithm is allowed to have
    /// </param>
    /// <returns>
    ///   The strongest algorithm out of those that take less CPU cycles that the specified
    ///   maximum, or the closest one, if all algorithms are too slow
    /// </returns>
    private: const CompressionAlgorithm &getStrongestAlgorithmFasterThan(
      std::size_t maximumCyclesPerKilobyte
    ) const;

    /// <summary>Compression algorithms available for use by the compression provider</summary>
    private: std::vector<std::unique_ptr<CompressionAlgorithm>> algorithms;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_COMPRESSIONPROVIDER_H
