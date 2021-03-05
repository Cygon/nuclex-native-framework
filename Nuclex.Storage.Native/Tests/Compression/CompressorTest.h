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

#ifndef NUCLEX_STORAGE_COMPRESSION_COMPRESSORTEST_H
#define NUCLEX_STORAGE_COMPRESSION_COMPRESSORTEST_H

#include "Nuclex/Storage/Compression/Compressor.h"
#include "Nuclex/Storage/Compression/Decompressor.h"

#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tries to compress a longer stream of generated data</summary>
  /// <param name="compressor">Compressor that will be used to compress the data</param>
  /// <param name="target">Vector into which the compressed data will be written</param>
  /// <param name="totalByteCount">Number of uncompressed bytes that will be generated</param>
  /// <remarks>
  ///   The data that will be compressed is generated using a semi-random pattern that
  ///   can be exactly reproduced be the decompression check to verify each byte.
  /// </remarks>
  void checkStreamCompression(
    Compressor &compressor,
    std::vector<std::uint8_t> &target,
    std::size_t totalByteCount
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Decompresses and veriefies a longer stream created by
  ///   the <see cref="checkStreamCompression" /> method
  /// </summary>
  /// <param name="decompressor">Compressor that will be used to compress the data</param>
  /// <param name="target">Vector into which the compressed data will be written</param>
  /// <param name="totalByteCount">Number of uncompressed bytes that should come out</param>
  /// <remarks>
  ///   This not only checks whether the data decompresses cleanly and has the expected length,
  ///   but also verifies each byte by checking it against the generated pseudo-random data.
  /// </remarks>
  void checkStreamDecompression(
    Decompressor &decompressor,
    const std::vector<std::uint8_t> &source,
    std::size_t totalByteCount
  );

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_COMPRESSORTEST_H
