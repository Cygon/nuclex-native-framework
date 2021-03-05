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

#include "Nuclex/Storage/Compression/CompressionProvider.h"

#include "Brotli/BrotliCompressionAlgorithm.h"
#include "Csc/CscCompressionAlgorithm.h"
#include "LZip/LzmaCompressionAlgorithm.h"
#include "ZLib/DeflateCompressionAlgorithm.h"

#include <cassert> // assert()
#include <stdexcept> // std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Vector of compression algorithms</summary>
  typedef std::vector<
    std::unique_ptr<Nuclex::Storage::Compression::CompressionAlgorithm>
  > CompressionAlgorithmVector;

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_STORAGE_HAVE_BROTLI)
  /// <summary>Adds variations of Google's brotli compression algorithm to a vector</summary>
  /// <param name="algorithms">Vector the algorithms will be added to</param>
  void addBrotliAlgorithms(CompressionAlgorithmVector &algorithms) {
    using Nuclex::Storage::Compression::Brotli::BrotliCompressionAlgorithm;

    algorithms.push_back(
      std::make_unique<BrotliCompressionAlgorithm>(BrotliCompressionAlgorithm::FastestQuality)
    );
    algorithms.push_back(
      std::make_unique<BrotliCompressionAlgorithm>(BrotliCompressionAlgorithm::DefaultQuality)
    );
    algorithms.push_back(
      std::make_unique<BrotliCompressionAlgorithm>(BrotliCompressionAlgorithm::StrongestQuality)
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_STORAGE_HAVE_CSC)
  /// <summary>Adds variations of the CSC compression algorithm to a vector</summary>
  /// <param name="algorithms">Vector the algorithms will be added to</param>
  void addCscAlgorithms(CompressionAlgorithmVector &algorithms) {
    using Nuclex::Storage::Compression::Csc::CscCompressionAlgorithm;

    algorithms.push_back(
      std::make_unique<CscCompressionAlgorithm>(CscCompressionAlgorithm::FastestQuality)
    );
    algorithms.push_back(
      std::make_unique<CscCompressionAlgorithm>(CscCompressionAlgorithm::DefaultQuality)
    );
    algorithms.push_back(
      std::make_unique<CscCompressionAlgorithm>(CscCompressionAlgorithm::StrongestQuality)
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_STORAGE_HAVE_ZLIB)
  /// <summary>Adds variations of ZLib's deflate compression algorithm to a vector</summary>
  /// <param name="algorithms">Vector the algorithms will be added to</param>
  void addDeflateAlgorithms(CompressionAlgorithmVector &algorithms) {
    using Nuclex::Storage::Compression::ZLib::DeflateCompressionAlgorithm;

    algorithms.push_back(
      std::make_unique<DeflateCompressionAlgorithm>(DeflateCompressionAlgorithm::FastestLevel)
    );
    algorithms.push_back(
      std::make_unique<DeflateCompressionAlgorithm>(DeflateCompressionAlgorithm::DefaultLevel)
    );
    algorithms.push_back(
      std::make_unique<DeflateCompressionAlgorithm>(DeflateCompressionAlgorithm::StrongestLevel)
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_STORAGE_HAVE_LZIP)
  /// <summary>Adds variations of LZip's LZMA compression algorithm to a vector</summary>
  /// <param name="algorithms">Vector the algorithms will be added to</param>
  void addLzmaAlgorithms(CompressionAlgorithmVector &algorithms) {
    using Nuclex::Storage::Compression::LZip::LzmaCompressionAlgorithm;

    algorithms.push_back(
      std::make_unique<LzmaCompressionAlgorithm>(LzmaCompressionAlgorithm::FastestQuality)
    );
    algorithms.push_back(
      std::make_unique<LzmaCompressionAlgorithm>(LzmaCompressionAlgorithm::DefaultQuality)
    );
    algorithms.push_back(
      std::make_unique<LzmaCompressionAlgorithm>(LzmaCompressionAlgorithm::StrongestQuality)
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  CompressionAlgorithmVector getBuiltInAlgorithms() {
    CompressionAlgorithmVector algorithms;

#if defined(NUCLEX_STORAGE_HAVE_BROTLI)
    addBrotliAlgorithms(algorithms);
#endif
#if defined(NUCLEX_STORAGE_HAVE_CSC)
    addCscAlgorithms(algorithms);
#endif
#if defined(NUCLEX_STORAGE_HAVE_LZIP)
    addLzmaAlgorithms(algorithms);
#endif
#if defined(NUCLEX_STORAGE_HAVE_ZLIB)
    addDeflateAlgorithms(algorithms);
#endif

    return algorithms;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  CompressionProvider::CompressionProvider() :
    algorithms(getBuiltInAlgorithms()) {}

  // ------------------------------------------------------------------------------------------- //

  CompressionProvider::~CompressionProvider() {
  }

  // ------------------------------------------------------------------------------------------- //

  const CompressionAlgorithm &CompressionProvider::GetAlgorithm(
    const std::array<std::uint8_t, 8> &algorithmId
  ) const {
    std::size_t count = this->algorithms.size();
    for(std::size_t index = 0; index < count; ++index) {
      if(this->algorithms[index]->GetId() == algorithmId) {
        return *this->algorithms[index].get();
      }
    }

    throw std::runtime_error(
      u8"Compression algorithm with specified id is not known. If you're attempting to "
      u8"decompress existing data using this algorithm, perhaps the version of Nuclex.Storage "
      u8"you're using was built without having this algorithm enabled."
    );
  }

  // ------------------------------------------------------------------------------------------- //

  const CompressionAlgorithm &CompressionProvider::GetOptimalAlgorithm(
    std::size_t uncompressedDataSize, float compressionTimeSeconds
  ) const {

    // If no algorithms have been registered, we can bail out early
    std::size_t count = this->algorithms.size();
    if(count == 0) {
      throw std::runtime_error(u8"No compression algorithms registered");
    }

    // Calculate the number of cycles per kilobyte the algorithm may burn
    // while staying below the time limit given by the caller
    std::size_t maximumCyclesPerKilobyte;
    {
      // TODO: Query maximum CPU speed to use as a guesstimate
      const std::size_t cpuCyclesPerSecond = 2000000000U; // 2 GHz assumption

      std::size_t uncompressedKilobyteCount = uncompressedDataSize / 1024U;
      maximumCyclesPerKilobyte = static_cast<std::size_t>(
        cpuCyclesPerSecond * static_cast<double>(compressionTimeSeconds) /
        uncompressedKilobyteCount
      );
    }

    return getStrongestAlgorithmFasterThan(maximumCyclesPerKilobyte);
  }

  // ------------------------------------------------------------------------------------------- //

  const CompressionAlgorithm &CompressionProvider::GetStrongAlgorithm(
    float performanceFactor /* = 0.75f */
  ) const {

    // If no algorithms have been registered, we can bail out early
    std::size_t count = this->algorithms.size();
    if(count == 0) {
      throw std::runtime_error(u8"No compression algorithms registered");
    }

    // Figure out the fastest and slowest algorithms we have
    std::size_t cyclesPerKilobyte = this->algorithms[0]->GetCompressionCyclesPerKilobyte();
    std::size_t highestCyclesPerKilobyte = cyclesPerKilobyte;
    std::size_t lowestCyclesPerKilobyte = cyclesPerKilobyte;

    for(std::size_t index = 1; index < count; ++index) {
      cyclesPerKilobyte = this->algorithms[index]->GetCompressionCyclesPerKilobyte();

      if(cyclesPerKilobyte < lowestCyclesPerKilobyte) {
        lowestCyclesPerKilobyte = cyclesPerKilobyte;
      }
      if(cyclesPerKilobyte >= lowestCyclesPerKilobyte) {
        highestCyclesPerKilobyte = cyclesPerKilobyte;
      }
    }

    // Calculate the performance in cycles per kilobyte the target algrithm must be below
    std::size_t maximumCyclesPerKilobygte = static_cast<std::size_t>(
      (highestCyclesPerKilobyte - lowestCyclesPerKilobyte) * performanceFactor
    ) + lowestCyclesPerKilobyte;

    return getStrongestAlgorithmFasterThan(maximumCyclesPerKilobygte);
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(_MSC_VER) // The Microsoft compiler's static analysis fails here...
#pragma warning(push) // Both GCC (9.2) and clang (8.0) handle it just fine
#pragma warning(disable:4701) // potentially uninitialized local variable used
#endif

  const CompressionAlgorithm &CompressionProvider::getStrongestAlgorithmFasterThan(
    std::size_t maximumCyclesPerKilobyte
  ) const {
    std::size_t count = this->algorithms.size();
    assert((count >= 1) && u8"Number of registered algorithms is at least 1");

    // Loop task 1: Find the algorithm that's closest to the requested performance.
    // Since we're also including algorithms that are slower than the requested
    // performance, this loop is guaranteed to find a result.
    std::size_t closestIndex, closestDistance;

    // Loop task 2: find the strongest algorithm that's faster than the requested
    // performance. If this cannot be found, we can fall back to the closest one.
    std::size_t strongestIndex;
    float strongestRatio;
    bool strongestFound = false;

    // Loop that will scan the registered algorithms to complete task 1 and task 2.
    for(std::size_t index = 0; index < count; ++index) {
      std::size_t distance;

      // Calculate the difference in cycles per kilobyte. We can't used std::abs() here
      // because we're dealing with unsigned values.
      std::size_t cyclesPerKilobyte = this->algorithms[index]->GetCompressionCyclesPerKilobyte();
      if(cyclesPerKilobyte > maximumCyclesPerKilobyte) {
        distance = cyclesPerKilobyte - maximumCyclesPerKilobyte;
      } else {
        distance = maximumCyclesPerKilobyte - cyclesPerKilobyte;
      }

      // Keep track of the algorithm that's closest to the requested performance (task 1)
      if((index == 0) || (distance < closestDistance)) {
        closestIndex = index;
        closestDistance = distance;
      }

      // Look for the strongest algorithm that meets the performance requirement (task 2)
      if(cyclesPerKilobyte < maximumCyclesPerKilobyte) {
        float ratio = this->algorithms[index]->GetAverageCompressionRatio();
        if(!strongestFound || (ratio < strongestRatio)) {
          strongestFound = true;
          strongestRatio = ratio;
          strongestIndex = index;
        }
      }
    }

    // Now we either have the strongest algorithm that meets the required performance or
    // we fall back to the fastest algorithm that did *not* meet the required performance
    if(strongestFound) {
      return *this->algorithms[strongestIndex].get();
    } else {
      return *this->algorithms[closestIndex].get();
    }
  }

#if defined(_MSC_VER) // See above, static analysis in Microsoft compiler fails
#pragma warning(pop)
#endif

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression
