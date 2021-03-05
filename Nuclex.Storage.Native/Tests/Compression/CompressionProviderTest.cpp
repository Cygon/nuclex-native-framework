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
#include "Nuclex/Storage/Compression/CompressionAlgorithm.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  TEST(CompressionProviderTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      CompressionProvider test;
      (void)test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(CompressionProviderTest, CanCountCompressionAlgorithms) {
    CompressionProvider test;

    std::size_t expectedMinimumCount = 0;
#if defined(NUCLEX_STORAGE_HAVE_BROTLI)
    expectedMinimumCount += 3;
#endif
#if defined(NUCLEX_STORAGE_HAVE_CSC)
    expectedMinimumCount += 3;
#endif
#if defined(NUCLEX_STORAGE_HAVE_LZIP)
    expectedMinimumCount += 3;
#endif
#if defined(NUCLEX_STORAGE_HAVE_ZLIB)
    expectedMinimumCount += 3;
#endif

    EXPECT_GE(test.CountAlgorithms(), expectedMinimumCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(CompressionProviderTest, CanAccessCompressionAlgorithmsByIndex) {
    CompressionProvider test;

    std::size_t algorithmCount = test.CountAlgorithms();
    for(std::size_t index = 0; index < algorithmCount; ++index) {
      const CompressionAlgorithm &algorithm = test.GetAlgorithm(index);
      EXPECT_GE(algorithm.GetName().length(), 4);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(CompressionProviderTest, AlgorithmsCanBeLookedUpById) {
    CompressionProvider test;

    std::size_t algorithmCount = test.CountAlgorithms();
    for(std::size_t index = 0; index < algorithmCount; ++index) {
      const CompressionAlgorithm &algorithm = test.GetAlgorithm(index);

      std::array<std::uint8_t, 8> algorithmId = algorithm.GetId();
      const CompressionAlgorithm &algorithmById = test.GetAlgorithm(algorithmId);

      EXPECT_EQ(typeid(algorithm), typeid(algorithmById));
    }
  }
  
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression
