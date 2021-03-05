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
#define NUCLEX_STORAGE_SOURCE 1

#include "CscCompressionAlgorithm.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "CscCompressor.h"
#include "CscDecompressor.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a human-readable name for this compression algorithm</summary>
  /// <param name="level">ZLib compression level used when compressing</param>
  /// <returns>A human-readable name for the compression algorithm</returns>
  std::string buildAlgorithmName(int level) {
    const static std::string cscCompressorName(u8"CSC compression via libcsc ");
    const static std::string compressionLevel(u8" (compression level ");
    const static std::string closingBrace(u8")");
    const static std::string versionString(u8"2016-10-13");

    std::string name;
    name.reserve(
      cscCompressorName.size() + 12 + compressionLevel.size() + 2 + closingBrace.size()
    );

    name.append(cscCompressorName);
    name.append(versionString);
    name.append(compressionLevel);
    name.append(std::to_string(level));
    name.append(closingBrace);
    
    return name;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  const int CscCompressionAlgorithm::FastestQuality = 1;

  // ------------------------------------------------------------------------------------------- //

  const int CscCompressionAlgorithm::StrongestQuality = 5;

  // ------------------------------------------------------------------------------------------- //

  const int CscCompressionAlgorithm::DefaultQuality = 3;

  // ------------------------------------------------------------------------------------------- //

  CscCompressionAlgorithm::CscCompressionAlgorithm(int level) :
    name(buildAlgorithmName(level)),
    level(level) {}

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Compressor> CscCompressionAlgorithm::CreateCompressor() const {
    return std::make_unique<CscCompressor>(this->level);
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Decompressor> CscCompressionAlgorithm::CreateDecompressor() const {
    return std::make_unique<CscDecompressor>();
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)
