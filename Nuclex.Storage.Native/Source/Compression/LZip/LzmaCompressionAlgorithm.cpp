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

#include "LzmaCompressionAlgorithm.h"

#if defined(NUCLEX_STORAGE_HAVE_LZIP)

#include "LzmaCompressor.h"
#include "LzmaDecompressor.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a human-readable name for this compression algorithm</summary>
  /// <param name="quality">Brotli compression quality used when compressing</param>
  /// <returns>A human-readable name for the compression algorithm</returns>
  std::string buildAlgorithmName(int quality) {
    const static std::string lzmaCompressorName(
      u8"LZMA compression via LZip "
    );
    const static std::string compressionQuality(u8" (compression quality ");
    const static std::string closingBrace(u8")");

    std::string name;
    name.reserve(
      lzmaCompressorName.size() + 8 + compressionQuality.size() + 2 + closingBrace.size()
    );

    name.append(lzmaCompressorName);
    name.append(::LZ_version_string);
    name.append(compressionQuality);
    name.append(std::to_string(quality));
    name.append(closingBrace);
    
    return name;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  const int LzmaCompressionAlgorithm::FastestQuality = 1;

  // ------------------------------------------------------------------------------------------- //

  const int LzmaCompressionAlgorithm::StrongestQuality = 9;

  // ------------------------------------------------------------------------------------------- //

  const int LzmaCompressionAlgorithm::DefaultQuality = 6;

  // ------------------------------------------------------------------------------------------- //

  LzmaCompressionAlgorithm::LzmaCompressionAlgorithm(int quality) :
    name(buildAlgorithmName(quality)),
    quality(quality) {}

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Compressor> LzmaCompressionAlgorithm::CreateCompressor() const {
    return std::make_unique<LzmaCompressor>(this->quality);
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Decompressor> LzmaCompressionAlgorithm::CreateDecompressor() const {
    return std::make_unique<LzmaDecompressor>();
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Brotli

#endif // defined(NUCLEX_STORAGE_HAVE_LZIP)
