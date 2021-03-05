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

#include "BrotliCompressionAlgorithm.h"

#if defined(NUCLEX_STORAGE_HAVE_BROTLI)

#include "BrotliCompressor.h"
#include "BrotliDecompressor.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a human-readable name for this compression algorithm</summary>
  /// <param name="quality">Brotli compression quality used when compressing</param>
  /// <returns>A human-readable name for the compression algorithm</returns>
  std::string buildAlgorithmName(int quality) {
    const static std::string brotliCompressorName(
      u8"Brotli compression via reference implementation "
    );
    const static std::string compressionQuality(u8" (compression quality ");
    const static std::string closingBrace(u8")");

    std::string name;
    name.reserve(
      brotliCompressorName.size() + 8 + compressionQuality.size() + 2 + closingBrace.size()
    );

    // from version.h:
    //   Semantic version, calculated as (MAJOR << 24) | (MINOR << 12) | PATCH */
    //   #define BROTLI_VERSION 0x1000007

    std::uint32_t encoderVersion = ::BrotliEncoderVersion();
    std::uint32_t majorVersion = (encoderVersion >> 24);
    std::uint32_t minorVersion = (encoderVersion >> 12) & 0x00000FFF;
    std::uint32_t patchVersion = (encoderVersion & 0x00000FFF);

    name.append(brotliCompressorName);
    name.append(std::to_string(majorVersion));
    name.append(u8".");
    name.append(std::to_string(minorVersion));
    name.append(u8".");
    name.append(std::to_string(patchVersion));
    name.append(compressionQuality);
    name.append(std::to_string(quality));
    name.append(closingBrace);
    
    return name;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace Brotli {

  // ------------------------------------------------------------------------------------------- //

  const int BrotliCompressionAlgorithm::FastestQuality = BROTLI_MIN_QUALITY + 1;

  // ------------------------------------------------------------------------------------------- //

  const int BrotliCompressionAlgorithm::StrongestQuality = BROTLI_MAX_QUALITY;

  // ------------------------------------------------------------------------------------------- //

  const int BrotliCompressionAlgorithm::DefaultQuality = 7; //BROTLI_DEFAULT_QUALITY;

  // ------------------------------------------------------------------------------------------- //

  BrotliCompressionAlgorithm::BrotliCompressionAlgorithm(int quality) :
    name(buildAlgorithmName(quality)),
    quality(quality) {}

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Compressor> BrotliCompressionAlgorithm::CreateCompressor() const {
    return std::make_unique<BrotliCompressor>(this->quality);
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<Decompressor> BrotliCompressionAlgorithm::CreateDecompressor() const {
    return std::make_unique<BrotliDecompressor>();
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Brotli

#endif // defined(NUCLEX_STORAGE_HAVE_BROTLI)
