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
#define NUCLEX_PIXELS_SOURCE 1

#include "Nuclex/Pixels/UInt128.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Constructs a 128 bit integer from a low and a high 64 bit integer</summary>
  /// <param name="upper">64 bit integer which will donate the high 64 bits</param>
  /// <param name="upper">64 bit integer which will donate the low 64 bits</param>
  /// <returns>The complete 128 bit integer</returns>
  Nuclex::Pixels::uint128_t makeUInt128(std::uint64_t upper, std::uint64_t lower) {
#if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)
    return (
      (static_cast<Nuclex::Pixels::uint128_t>(upper) << 64) |
      static_cast<Nuclex::Pixels::uint128_t>(lower)
    );
#else
    return Nuclex::Pixels::UInt128(upper, lower);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeInitializedFromUInt8) {
    uint128_t test(std::uint8_t(123U));
    EXPECT_EQ(std::uint64_t(123U), static_cast<std::uint64_t>(test));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeInitializedFromUInt16) {
    uint128_t test(std::uint16_t(45678U));
    EXPECT_EQ(std::uint64_t(45678U), static_cast<std::uint64_t>(test));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeInitializedFromUInt32) {
    uint128_t test(std::uint32_t(3456789012U));
    EXPECT_EQ(std::uint64_t(3456789012U), static_cast<std::uint64_t>(test));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeInitializedFromUInt64) {
    uint128_t test(std::uint64_t(12345678901234567890UL));
    EXPECT_EQ(std::uint64_t(12345678901234567890UL), static_cast<std::uint64_t>(test));
  }
 
  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeCastToUInt8) {
    uint128_t test(std::uint8_t(123));
    EXPECT_EQ(static_cast<std::uint8_t>(test), std::uint8_t(123));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeCastToUInt16) {
    uint128_t test(std::uint16_t(12345));
    EXPECT_EQ(static_cast<std::uint16_t>(test), std::uint16_t(12345));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeCastToUInt32) {
    uint128_t test(std::uint32_t(1234567890));
    EXPECT_EQ(static_cast<std::uint32_t>(test), std::uint32_t(1234567890));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanBeCastToUInt64) {
    uint128_t test(std::uint64_t(1234567890000ULL));
    EXPECT_EQ(static_cast<std::uint64_t>(test), std::uint64_t(1234567890000ULL));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseOrOperator) {
    uint128_t zero = makeUInt128(0U, 0U);
    uint128_t evenBits = makeUInt128(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits = makeUInt128(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits = makeUInt128(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(zero, uint128_t(0U));
    EXPECT_EQ(zero | evenBits, evenBits);
    EXPECT_EQ(zero | oddBits, oddBits);
    EXPECT_EQ(zero | evenBits | oddBits, allBits);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseOrAssignmentOperator) {
    uint128_t test = makeUInt128(0U, 0U);
    uint128_t evenBits = makeUInt128(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits = makeUInt128(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits = makeUInt128(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(test, uint128_t(0U));
    test |= evenBits;
    EXPECT_EQ(test, evenBits);
    test |= oddBits;
    EXPECT_EQ(test, allBits);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseAndOperator) {
    uint128_t zero = makeUInt128(0U, 0U);
    uint128_t evenBits = makeUInt128(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits = makeUInt128(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits = makeUInt128(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(allBits & evenBits, evenBits);
    EXPECT_EQ(allBits & oddBits, oddBits);
    EXPECT_EQ(allBits & zero, zero);
    EXPECT_EQ(zero & evenBits & evenBits, zero);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseAndAssignmentOperator) {
    uint128_t test = makeUInt128(0U, 0U);
    uint128_t evenBits = makeUInt128(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits = makeUInt128(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits = makeUInt128(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    test = allBits;
    EXPECT_EQ(test, allBits);
    test &= evenBits;
    EXPECT_EQ(test, evenBits);
    test = allBits;
    test &= oddBits;
    EXPECT_EQ(test, oddBits);
    test = allBits;
    test &= uint128_t(0U);
    EXPECT_EQ(test, uint128_t(0U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasLeftShiftOperator) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint128_t expected = std::uint64_t(1) << value;
      uint128_t tested(1U);
      EXPECT_EQ(tested << static_cast<int>(value), expected);
      EXPECT_EQ(tested, uint128_t(1U));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasLeftShiftOperatorSupportingOver64Bits) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint64_t expected = std::uint64_t(1) << value;

      uint128_t tested(1U);
      EXPECT_EQ(
        static_cast<std::uint64_t>(tested << static_cast<int>(value + 64) >> 64),
        expected
      );
      EXPECT_EQ(tested, uint128_t(1U));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasLeftShiftAssignmentOperator) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint128_t expected = std::uint64_t(1) << value;
      uint128_t tested(1U);
      EXPECT_EQ((tested <<= static_cast<int>(value)), expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasLeftShiftAssignmentOperatorSupportingOver64Bits) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint64_t expected = std::uint64_t(1) << value;

      uint128_t tested(1U);
      tested <<= static_cast<int>(value + 64);
      EXPECT_EQ(
        static_cast<std::uint64_t>(tested >> 64),
        expected
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasRightShiftOperator) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint128_t expected = std::uint64_t(9223372036854775808U) >> value;
      uint128_t tested(9223372036854775808U);
      EXPECT_EQ(tested >> static_cast<int>(value), expected);
      EXPECT_EQ(tested, uint128_t(9223372036854775808U));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasRightShiftOperatorSupportingOver64Bits) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint64_t expected = std::uint64_t(9223372036854775808U) >> value;

      uint128_t tested(9223372036854775808U);
      EXPECT_EQ(
        static_cast<std::uint64_t>(tested << 64 >> static_cast<int>(value + 64)),
        expected
      );
      EXPECT_EQ(tested, uint128_t(9223372036854775808U));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasRightShiftAssignmentOperator) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint128_t expected = std::uint64_t(9223372036854775808U) >> value;
      uint128_t tested(9223372036854775808U);
      EXPECT_EQ((tested >>= static_cast<int>(value)), expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasRightShiftAssignmentOperatorSupportingOver64Bits) {
    for(std::size_t value = 0; value < 64; ++value) {
      uint64_t expected = std::uint64_t(9223372036854775808U) >> value;

      uint128_t tested = uint128_t(9223372036854775808U) << 64;
      tested >>= static_cast<int>(value + 64);
      EXPECT_EQ(
        static_cast<std::uint64_t>(tested),
        expected
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
