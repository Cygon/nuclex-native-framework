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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/BitTricks.h"
#include <gtest/gtest.h>

#include <random> // lots, for testing with random numbers

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the n-th power of 10</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  template<typename TResult>
  inline TResult Pow10(std::size_t power) = delete;

  /// <summary>Calculates the n-th power of 10 as a 32 bit integer</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  template<>
  inline std::uint32_t Pow10<std::uint32_t>(std::size_t power) {
    static const std::uint32_t powersOfTen[] = {
      1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U
    };
    return powersOfTen[power];
  }

  /// <summary>Calculates the n-th power of 10 as a 64 bit integer</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  /// <remarks>
  ///   Doing this with std::pow() will start to yield imprecise results at the higher
  ///   ranges of 64 bit integers (at least with fast math enabled), this integer-only
  ///   version of the method gives an accurate result in all cases
  /// </remarks>
  template<>
  inline std::uint64_t Pow10<std::uint64_t>(std::size_t power) {
    static const std::uint64_t powersOfTen[20] = {
      1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL,
      100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
      10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
      100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL
    };
    return powersOfTen[power];
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanCountBitsIn32BitsValue) {
    EXPECT_EQ(0, BitTricks::CountBits(std::uint32_t(0U)));

    EXPECT_EQ(1, BitTricks::CountBits(std::uint32_t(1U)));
    EXPECT_EQ(2, BitTricks::CountBits(std::uint32_t(3U)));
    EXPECT_EQ(3, BitTricks::CountBits(std::uint32_t(7U)));

    EXPECT_EQ(1, BitTricks::CountBits(std::uint32_t(2147483648U)));
    EXPECT_EQ(2, BitTricks::CountBits(std::uint32_t(3221225472U)));
    EXPECT_EQ(3, BitTricks::CountBits(std::uint32_t(3758096384U)));

    EXPECT_EQ(32, BitTricks::CountBits(std::uint32_t(4294967295U)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanCountBitsIn64BitsValue) {
    EXPECT_EQ(0, BitTricks::CountBits(std::uint64_t(0ULL)));

    EXPECT_EQ(1, BitTricks::CountBits(std::uint64_t(1ULL)));
    EXPECT_EQ(2, BitTricks::CountBits(std::uint64_t(3ULL)));
    EXPECT_EQ(3, BitTricks::CountBits(std::uint64_t(7ULL)));

    EXPECT_EQ(1, BitTricks::CountBits(std::uint64_t(9223372036854775808ULL)));
    EXPECT_EQ(2, BitTricks::CountBits(std::uint64_t(13835058055282163712ULL)));
    EXPECT_EQ(3, BitTricks::CountBits(std::uint64_t(16140901064495857664ULL)));

    EXPECT_EQ(64, BitTricks::CountBits(std::uint64_t(18446744073709551615U)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanCountLeadingZeroBitsIn32BitsValue) {
    for(std::size_t index = 0; index < 31; ++index) {
      EXPECT_EQ(
        31 - index,
        BitTricks::CountLeadingZeroBits(std::uint32_t(1U << index))
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanCountLeadingZeroBitsIn64BitsValue) {
    for(std::size_t index = 0; index < 63; ++index) {
      EXPECT_EQ(
        63 - index,
        BitTricks::CountLeadingZeroBits(std::uint64_t(1ULL << index))
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanFindPowerOfTwoFor32BitsValue) {
    std::mt19937 generator;

    for(std::size_t index = 0; index < 31; ++index) {
      EXPECT_EQ(
        (1U << index),
        BitTricks::GetUpperPowerOfTwo(std::uint32_t(1U << index))
      );

      // Do some random checks for 10 numbers below the searched for power-of-two
      {
        std::uint32_t upperBound = 1U << index;
        std::uint32_t lowerBound = (upperBound >> 1) + 1U;
        std::uniform_int_distribution<std::uint32_t> distribution(lowerBound, upperBound);
        for(std::size_t extra = 0; extra < 10; ++extra) {
          EXPECT_EQ(
            (1U << index),
            BitTricks::GetUpperPowerOfTwo(distribution(generator))
          );
        }
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanFindPowerOfTwoFor64BitsValue) {
    std::mt19937 generator;

    for(std::size_t index = 0; index < 63; ++index) {
      EXPECT_EQ(
        (1ULL << index),
        BitTricks::GetUpperPowerOfTwo(std::uint64_t(1ULL << index))
      );

      // Do some random checks for 10 numbers below the searched for power-of-two
      {
        std::uint64_t upperBound = 1ULL << index;
        std::uint64_t lowerBound = (upperBound >> 1) + 1ULL;
        std::uniform_int_distribution<std::uint64_t> distribution(lowerBound, upperBound);
        for(std::size_t extra = 0; extra < 10; ++extra) {
          EXPECT_EQ(
            (1ULL << index),
            BitTricks::GetUpperPowerOfTwo(distribution(generator))
          );
        }
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanGetLogBase2Of32BitsValue) {
    for(std::size_t index = 0; index < 31; ++index) {
      if(index >= 1) {
        EXPECT_EQ(
          index - 1U,
          BitTricks::GetLogBase2(std::uint32_t((1U << index) - 1))
        );
      }
      EXPECT_EQ(
        index,
        BitTricks::GetLogBase2(std::uint32_t(1U << index))
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanGetLogBase2Of64BitsValue) {
    for(std::size_t index = 0; index < 63; ++index) {
      if(index >= 1) {
        EXPECT_EQ(
          index - 1U,
          BitTricks::GetLogBase2(std::uint64_t((1ULL << index) - 1))
        );
      }
      EXPECT_EQ(
        index,
        BitTricks::GetLogBase2(std::uint64_t(1ULL << index))
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanGetLogBase10Of32BitsValue) {
    EXPECT_EQ(0U, BitTricks::GetLogBase10(std::uint32_t(1)));

    for(std::size_t log10 = 1; log10 < 10; ++log10) {
      std::uint32_t nextHigher = Pow10<std::uint32_t>(log10);
      std::uint32_t nextLower = nextHigher - 1;

      EXPECT_EQ(log10 - 1, BitTricks::GetLogBase10(nextLower));
      EXPECT_EQ(log10, BitTricks::GetLogBase10(nextHigher));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, CanGetLogBase10Of64BitsValue) {
    EXPECT_EQ(0U, BitTricks::GetLogBase10(std::uint64_t(1)));

    for(std::size_t log10 = 1; log10 < 20; ++log10) {
      std::uint64_t nextHigher = Pow10<std::uint64_t>(log10);
      std::uint64_t nextLower = nextHigher - 1;

      EXPECT_EQ(log10 - 1, BitTricks::GetLogBase10(nextLower));
      EXPECT_EQ(log10, BitTricks::GetLogBase10(nextHigher));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, XorShiftRandomNumberGeneratorWorksWith32Bits) {
    std::uint32_t randomNumber = 0x12345678;

    for(std::size_t index = 0; index < 1000; ++index) {
      std::uint32_t nextRandomNumber = BitTricks::XorShiftRandom(randomNumber);
      EXPECT_NE(nextRandomNumber, randomNumber);
      randomNumber = nextRandomNumber;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitTricksTest, XorShiftRandomNumberGeneratorWorksWith64Bits) {
    std::uint64_t randomNumber = 0x1234567812345678;

    for(std::size_t index = 0; index < 1000; ++index) {
      std::uint64_t nextRandomNumber = BitTricks::XorShiftRandom(randomNumber);
      EXPECT_NE(nextRandomNumber, randomNumber);
      randomNumber = nextRandomNumber;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
