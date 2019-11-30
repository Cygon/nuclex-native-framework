#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

  /// <summary>Returns the upper 64 bits of a 128 bit integer</summary>
  /// <param name="integer">128 bit integer of which the upper 64 bits are returned</param>
  /// <returns>The upper 64 bits of the specified 128 bit integer</returns>
  std::uint64_t upper64(Nuclex::Pixels::uint128_t integer) {
    return static_cast<std::uint64_t>(Nuclex::Pixels::BitShift<64>(integer));
  }

  /// <summary>Returns the lower 64 bits of a 128 bit integer</summary>
  /// <param name="integer">128 bit integer of which the lower 64 bits are returned</param>
  /// <returns>The lower 64 bits of the specified 128 bit integer</returns>
  std::uint64_t lower64(Nuclex::Pixels::uint128_t integer) {
    return static_cast<std::uint64_t>(integer);
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
    uint128_t zero(0U);
    uint128_t evenBits(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(zero, uint128_t(0U));
    EXPECT_EQ(zero | evenBits, evenBits);
    EXPECT_EQ(zero | oddBits, oddBits);
    EXPECT_EQ(zero | evenBits | oddBits, allBits);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseOrAssignmentOperator) {
    uint128_t test(0U);
    uint128_t evenBits(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(test, uint128_t(0U));
    test |= evenBits;
    EXPECT_EQ(test, evenBits);
    test |= oddBits;
    EXPECT_EQ(test, allBits);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseAndOperator) {
    uint128_t zero(0U);
    uint128_t evenBits(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    EXPECT_EQ(allBits & evenBits, evenBits);
    EXPECT_EQ(allBits & oddBits, oddBits);
    EXPECT_EQ(allBits & zero, zero);
    EXPECT_EQ(zero & evenBits & evenBits, zero);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, HasBitwiseAndAssignmentOperator) {
    uint128_t test(0U);
    uint128_t evenBits(0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa);
    uint128_t oddBits(0x5555555555555555, 0x5555555555555555);
    uint128_t allBits(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

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

  TEST(UInt128Test, CanShiftUpTo64BitsLeft) {
    uint128_t test = uint128_t(1U);

    EXPECT_EQ(BitShift<0>(test), uint128_t(1U));
    EXPECT_EQ(BitShift<-1>(test), uint128_t(2U));
    EXPECT_EQ(BitShift<-2>(test), uint128_t(4U));
    EXPECT_EQ(BitShift<-3>(test), uint128_t(8U));
    EXPECT_EQ(BitShift<-4>(test), uint128_t(16U));
    EXPECT_EQ(BitShift<-5>(test), uint128_t(32U));
    EXPECT_EQ(BitShift<-6>(test), uint128_t(64U));
    EXPECT_EQ(BitShift<-7>(test), uint128_t(128U));
    EXPECT_EQ(BitShift<-8>(test), uint128_t(256U));
    EXPECT_EQ(BitShift<-9>(test), uint128_t(512U));
    EXPECT_EQ(BitShift<-10>(test), uint128_t(1024U));
    EXPECT_EQ(BitShift<-11>(test), uint128_t(2048U));
    EXPECT_EQ(BitShift<-12>(test), uint128_t(4096U));
    EXPECT_EQ(BitShift<-13>(test), uint128_t(8192U));
    EXPECT_EQ(BitShift<-14>(test), uint128_t(16384U));
    EXPECT_EQ(BitShift<-15>(test), uint128_t(32768U));
    EXPECT_EQ(BitShift<-16>(test), uint128_t(65536U));
    EXPECT_EQ(BitShift<-17>(test), uint128_t(131072U));
    EXPECT_EQ(BitShift<-18>(test), uint128_t(262144U));
    EXPECT_EQ(BitShift<-19>(test), uint128_t(524288U));
    EXPECT_EQ(BitShift<-20>(test), uint128_t(1048576U));
    EXPECT_EQ(BitShift<-21>(test), uint128_t(2097152U));
    EXPECT_EQ(BitShift<-22>(test), uint128_t(4194304U));
    EXPECT_EQ(BitShift<-23>(test), uint128_t(8388608U));
    EXPECT_EQ(BitShift<-24>(test), uint128_t(16777216U));
    EXPECT_EQ(BitShift<-25>(test), uint128_t(33554432U));
    EXPECT_EQ(BitShift<-26>(test), uint128_t(67108864U));
    EXPECT_EQ(BitShift<-27>(test), uint128_t(134217728U));
    EXPECT_EQ(BitShift<-28>(test), uint128_t(268435456U));
    EXPECT_EQ(BitShift<-29>(test), uint128_t(536870912U));
    EXPECT_EQ(BitShift<-30>(test), uint128_t(1073741824U));
    EXPECT_EQ(BitShift<-31>(test), uint128_t(2147483648U));
    EXPECT_EQ(BitShift<-32>(test), uint128_t(4294967296U));
    EXPECT_EQ(BitShift<-33>(test), uint128_t(8589934592U));
    EXPECT_EQ(BitShift<-34>(test), uint128_t(17179869184U));
    EXPECT_EQ(BitShift<-35>(test), uint128_t(34359738368U));
    EXPECT_EQ(BitShift<-36>(test), uint128_t(68719476736U));
    EXPECT_EQ(BitShift<-37>(test), uint128_t(137438953472U));
    EXPECT_EQ(BitShift<-38>(test), uint128_t(274877906944U));
    EXPECT_EQ(BitShift<-39>(test), uint128_t(549755813888U));
    EXPECT_EQ(BitShift<-40>(test), uint128_t(1099511627776U));
    EXPECT_EQ(BitShift<-41>(test), uint128_t(2199023255552U));
    EXPECT_EQ(BitShift<-42>(test), uint128_t(4398046511104U));
    EXPECT_EQ(BitShift<-43>(test), uint128_t(8796093022208U));
    EXPECT_EQ(BitShift<-44>(test), uint128_t(17592186044416U));
    EXPECT_EQ(BitShift<-45>(test), uint128_t(35184372088832U));
    EXPECT_EQ(BitShift<-46>(test), uint128_t(70368744177664U));
    EXPECT_EQ(BitShift<-47>(test), uint128_t(140737488355328U));
    EXPECT_EQ(BitShift<-48>(test), uint128_t(281474976710656U));
    EXPECT_EQ(BitShift<-49>(test), uint128_t(562949953421312U));
    EXPECT_EQ(BitShift<-50>(test), uint128_t(1125899906842624U));
    EXPECT_EQ(BitShift<-51>(test), uint128_t(2251799813685248U));
    EXPECT_EQ(BitShift<-52>(test), uint128_t(4503599627370496U));
    EXPECT_EQ(BitShift<-53>(test), uint128_t(9007199254740992U));
    EXPECT_EQ(BitShift<-54>(test), uint128_t(18014398509481984U));
    EXPECT_EQ(BitShift<-55>(test), uint128_t(36028797018963968U));
    EXPECT_EQ(BitShift<-56>(test), uint128_t(72057594037927936U));
    EXPECT_EQ(BitShift<-57>(test), uint128_t(144115188075855872U));
    EXPECT_EQ(BitShift<-58>(test), uint128_t(288230376151711744U));
    EXPECT_EQ(BitShift<-59>(test), uint128_t(576460752303423488U));
    EXPECT_EQ(BitShift<-60>(test), uint128_t(1152921504606846976U));
    EXPECT_EQ(BitShift<-61>(test), uint128_t(2305843009213693952U));
    EXPECT_EQ(BitShift<-62>(test), uint128_t(4611686018427387904U));
    EXPECT_EQ(BitShift<-63>(test), uint128_t(9223372036854775808U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanShiftMoreThan64BitsLeft) {
    uint128_t test = uint128_t(1U);

    EXPECT_EQ(upper64(BitShift<-64>(test)), uint64_t(1U));
    EXPECT_EQ(upper64(BitShift<-65>(test)), uint64_t(2U));
    EXPECT_EQ(upper64(BitShift<-66>(test)), uint64_t(4U));
    EXPECT_EQ(upper64(BitShift<-67>(test)), uint64_t(8U));
    EXPECT_EQ(upper64(BitShift<-68>(test)), uint64_t(16U));
    EXPECT_EQ(upper64(BitShift<-69>(test)), uint64_t(32U));
    EXPECT_EQ(upper64(BitShift<-70>(test)), uint64_t(64U));
    EXPECT_EQ(upper64(BitShift<-71>(test)), uint64_t(128U));
    EXPECT_EQ(upper64(BitShift<-72>(test)), uint64_t(256U));
    EXPECT_EQ(upper64(BitShift<-73>(test)), uint64_t(512U));
    EXPECT_EQ(upper64(BitShift<-74>(test)), uint64_t(1024U));
    EXPECT_EQ(upper64(BitShift<-75>(test)), uint64_t(2048U));
    EXPECT_EQ(upper64(BitShift<-76>(test)), uint64_t(4096U));
    EXPECT_EQ(upper64(BitShift<-77>(test)), uint64_t(8192U));
    EXPECT_EQ(upper64(BitShift<-78>(test)), uint64_t(16384U));
    EXPECT_EQ(upper64(BitShift<-79>(test)), uint64_t(32768U));
    EXPECT_EQ(upper64(BitShift<-80>(test)), uint64_t(65536U));
    EXPECT_EQ(upper64(BitShift<-81>(test)), uint64_t(131072U));
    EXPECT_EQ(upper64(BitShift<-82>(test)), uint64_t(262144U));
    EXPECT_EQ(upper64(BitShift<-83>(test)), uint64_t(524288U));
    EXPECT_EQ(upper64(BitShift<-84>(test)), uint64_t(1048576U));
    EXPECT_EQ(upper64(BitShift<-85>(test)), uint64_t(2097152U));
    EXPECT_EQ(upper64(BitShift<-86>(test)), uint64_t(4194304U));
    EXPECT_EQ(upper64(BitShift<-87>(test)), uint64_t(8388608U));
    EXPECT_EQ(upper64(BitShift<-88>(test)), uint64_t(16777216U));
    EXPECT_EQ(upper64(BitShift<-89>(test)), uint64_t(33554432U));
    EXPECT_EQ(upper64(BitShift<-90>(test)), uint64_t(67108864U));
    EXPECT_EQ(upper64(BitShift<-91>(test)), uint64_t(134217728U));
    EXPECT_EQ(upper64(BitShift<-92>(test)), uint64_t(268435456U));
    EXPECT_EQ(upper64(BitShift<-93>(test)), uint64_t(536870912U));
    EXPECT_EQ(upper64(BitShift<-94>(test)), uint64_t(1073741824U));
    EXPECT_EQ(upper64(BitShift<-95>(test)), uint64_t(2147483648U));
    EXPECT_EQ(upper64(BitShift<-96>(test)), uint64_t(4294967296U));
    EXPECT_EQ(upper64(BitShift<-97>(test)), uint64_t(8589934592U));
    EXPECT_EQ(upper64(BitShift<-98>(test)), uint64_t(17179869184U));
    EXPECT_EQ(upper64(BitShift<-99>(test)), uint64_t(34359738368U));
    EXPECT_EQ(upper64(BitShift<-100>(test)), uint64_t(68719476736U));
    EXPECT_EQ(upper64(BitShift<-101>(test)), uint64_t(137438953472U));
    EXPECT_EQ(upper64(BitShift<-102>(test)), uint64_t(274877906944U));
    EXPECT_EQ(upper64(BitShift<-103>(test)), uint64_t(549755813888U));
    EXPECT_EQ(upper64(BitShift<-104>(test)), uint64_t(1099511627776U));
    EXPECT_EQ(upper64(BitShift<-105>(test)), uint64_t(2199023255552U));
    EXPECT_EQ(upper64(BitShift<-106>(test)), uint64_t(4398046511104U));
    EXPECT_EQ(upper64(BitShift<-107>(test)), uint64_t(8796093022208U));
    EXPECT_EQ(upper64(BitShift<-108>(test)), uint64_t(17592186044416U));
    EXPECT_EQ(upper64(BitShift<-109>(test)), uint64_t(35184372088832U));
    EXPECT_EQ(upper64(BitShift<-110>(test)), uint64_t(70368744177664U));
    EXPECT_EQ(upper64(BitShift<-111>(test)), uint64_t(140737488355328U));
    EXPECT_EQ(upper64(BitShift<-112>(test)), uint64_t(281474976710656U));
    EXPECT_EQ(upper64(BitShift<-113>(test)), uint64_t(562949953421312U));
    EXPECT_EQ(upper64(BitShift<-114>(test)), uint64_t(1125899906842624U));
    EXPECT_EQ(upper64(BitShift<-115>(test)), uint64_t(2251799813685248U));
    EXPECT_EQ(upper64(BitShift<-116>(test)), uint64_t(4503599627370496U));
    EXPECT_EQ(upper64(BitShift<-117>(test)), uint64_t(9007199254740992U));
    EXPECT_EQ(upper64(BitShift<-118>(test)), uint64_t(18014398509481984U));
    EXPECT_EQ(upper64(BitShift<-119>(test)), uint64_t(36028797018963968U));
    EXPECT_EQ(upper64(BitShift<-120>(test)), uint64_t(72057594037927936U));
    EXPECT_EQ(upper64(BitShift<-121>(test)), uint64_t(144115188075855872U));
    EXPECT_EQ(upper64(BitShift<-122>(test)), uint64_t(288230376151711744U));
    EXPECT_EQ(upper64(BitShift<-123>(test)), uint64_t(576460752303423488U));
    EXPECT_EQ(upper64(BitShift<-124>(test)), uint64_t(1152921504606846976U));
    EXPECT_EQ(upper64(BitShift<-125>(test)), uint64_t(2305843009213693952U));
    EXPECT_EQ(upper64(BitShift<-126>(test)), uint64_t(4611686018427387904U));
    EXPECT_EQ(upper64(BitShift<-127>(test)), uint64_t(9223372036854775808U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanShiftUpTo64BitsRight) {
    uint128_t test = uint128_t(9223372036854775808U);

    EXPECT_EQ(BitShift<63>(test), uint128_t(1U));
    EXPECT_EQ(BitShift<62>(test), uint128_t(2U));
    EXPECT_EQ(BitShift<61>(test), uint128_t(4U));
    EXPECT_EQ(BitShift<60>(test), uint128_t(8U));
    EXPECT_EQ(BitShift<59>(test), uint128_t(16U));
    EXPECT_EQ(BitShift<58>(test), uint128_t(32U));
    EXPECT_EQ(BitShift<57>(test), uint128_t(64U));
    EXPECT_EQ(BitShift<56>(test), uint128_t(128U));
    EXPECT_EQ(BitShift<55>(test), uint128_t(256U));
    EXPECT_EQ(BitShift<54>(test), uint128_t(512U));
    EXPECT_EQ(BitShift<53>(test), uint128_t(1024U));
    EXPECT_EQ(BitShift<52>(test), uint128_t(2048U));
    EXPECT_EQ(BitShift<51>(test), uint128_t(4096U));
    EXPECT_EQ(BitShift<50>(test), uint128_t(8192U));
    EXPECT_EQ(BitShift<49>(test), uint128_t(16384U));
    EXPECT_EQ(BitShift<48>(test), uint128_t(32768U));
    EXPECT_EQ(BitShift<47>(test), uint128_t(65536U));
    EXPECT_EQ(BitShift<46>(test), uint128_t(131072U));
    EXPECT_EQ(BitShift<45>(test), uint128_t(262144U));
    EXPECT_EQ(BitShift<44>(test), uint128_t(524288U));
    EXPECT_EQ(BitShift<43>(test), uint128_t(1048576U));
    EXPECT_EQ(BitShift<42>(test), uint128_t(2097152U));
    EXPECT_EQ(BitShift<41>(test), uint128_t(4194304U));
    EXPECT_EQ(BitShift<40>(test), uint128_t(8388608U));
    EXPECT_EQ(BitShift<39>(test), uint128_t(16777216U));
    EXPECT_EQ(BitShift<38>(test), uint128_t(33554432U));
    EXPECT_EQ(BitShift<37>(test), uint128_t(67108864U));
    EXPECT_EQ(BitShift<36>(test), uint128_t(134217728U));
    EXPECT_EQ(BitShift<35>(test), uint128_t(268435456U));
    EXPECT_EQ(BitShift<34>(test), uint128_t(536870912U));
    EXPECT_EQ(BitShift<33>(test), uint128_t(1073741824U));
    EXPECT_EQ(BitShift<32>(test), uint128_t(2147483648U));
    EXPECT_EQ(BitShift<31>(test), uint128_t(4294967296U));
    EXPECT_EQ(BitShift<30>(test), uint128_t(8589934592U));
    EXPECT_EQ(BitShift<29>(test), uint128_t(17179869184U));
    EXPECT_EQ(BitShift<28>(test), uint128_t(34359738368U));
    EXPECT_EQ(BitShift<27>(test), uint128_t(68719476736U));
    EXPECT_EQ(BitShift<26>(test), uint128_t(137438953472U));
    EXPECT_EQ(BitShift<25>(test), uint128_t(274877906944U));
    EXPECT_EQ(BitShift<24>(test), uint128_t(549755813888U));
    EXPECT_EQ(BitShift<23>(test), uint128_t(1099511627776U));
    EXPECT_EQ(BitShift<22>(test), uint128_t(2199023255552U));
    EXPECT_EQ(BitShift<21>(test), uint128_t(4398046511104U));
    EXPECT_EQ(BitShift<20>(test), uint128_t(8796093022208U));
    EXPECT_EQ(BitShift<19>(test), uint128_t(17592186044416U));
    EXPECT_EQ(BitShift<18>(test), uint128_t(35184372088832U));
    EXPECT_EQ(BitShift<17>(test), uint128_t(70368744177664U));
    EXPECT_EQ(BitShift<16>(test), uint128_t(140737488355328U));
    EXPECT_EQ(BitShift<15>(test), uint128_t(281474976710656U));
    EXPECT_EQ(BitShift<14>(test), uint128_t(562949953421312U));
    EXPECT_EQ(BitShift<13>(test), uint128_t(1125899906842624U));
    EXPECT_EQ(BitShift<12>(test), uint128_t(2251799813685248U));
    EXPECT_EQ(BitShift<11>(test), uint128_t(4503599627370496U));
    EXPECT_EQ(BitShift<10>(test), uint128_t(9007199254740992U));
    EXPECT_EQ(BitShift<9>(test), uint128_t(18014398509481984U));
    EXPECT_EQ(BitShift<8>(test), uint128_t(36028797018963968U));
    EXPECT_EQ(BitShift<7>(test), uint128_t(72057594037927936U));
    EXPECT_EQ(BitShift<6>(test), uint128_t(144115188075855872U));
    EXPECT_EQ(BitShift<5>(test), uint128_t(288230376151711744U));
    EXPECT_EQ(BitShift<4>(test), uint128_t(576460752303423488U));
    EXPECT_EQ(BitShift<3>(test), uint128_t(1152921504606846976U));
    EXPECT_EQ(BitShift<2>(test), uint128_t(2305843009213693952U));
    EXPECT_EQ(BitShift<1>(test), uint128_t(4611686018427387904U));
    EXPECT_EQ(BitShift<0>(test), uint128_t(9223372036854775808U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UInt128Test, CanShiftMoreThan64BitsRight) {
    uint128_t test = BitShift<-64>(uint128_t(9223372036854775808U));
    
    EXPECT_EQ(lower64(BitShift<127>(test)), uint64_t(1U));
    EXPECT_EQ(lower64(BitShift<126>(test)), uint64_t(2U));
    EXPECT_EQ(lower64(BitShift<125>(test)), uint64_t(4U));
    EXPECT_EQ(lower64(BitShift<124>(test)), uint64_t(8U));
    EXPECT_EQ(lower64(BitShift<123>(test)), uint64_t(16U));
    EXPECT_EQ(lower64(BitShift<122>(test)), uint64_t(32U));
    EXPECT_EQ(lower64(BitShift<121>(test)), uint64_t(64U));
    EXPECT_EQ(lower64(BitShift<120>(test)), uint64_t(128U));
    EXPECT_EQ(lower64(BitShift<119>(test)), uint64_t(256U));
    EXPECT_EQ(lower64(BitShift<118>(test)), uint64_t(512U));
    EXPECT_EQ(lower64(BitShift<117>(test)), uint64_t(1024U));
    EXPECT_EQ(lower64(BitShift<116>(test)), uint64_t(2048U));
    EXPECT_EQ(lower64(BitShift<115>(test)), uint64_t(4096U));
    EXPECT_EQ(lower64(BitShift<114>(test)), uint64_t(8192U));
    EXPECT_EQ(lower64(BitShift<113>(test)), uint64_t(16384U));
    EXPECT_EQ(lower64(BitShift<112>(test)), uint64_t(32768U));
    EXPECT_EQ(lower64(BitShift<111>(test)), uint64_t(65536U));
    EXPECT_EQ(lower64(BitShift<110>(test)), uint64_t(131072U));
    EXPECT_EQ(lower64(BitShift<109>(test)), uint64_t(262144U));
    EXPECT_EQ(lower64(BitShift<108>(test)), uint64_t(524288U));
    EXPECT_EQ(lower64(BitShift<107>(test)), uint64_t(1048576U));
    EXPECT_EQ(lower64(BitShift<106>(test)), uint64_t(2097152U));
    EXPECT_EQ(lower64(BitShift<105>(test)), uint64_t(4194304U));
    EXPECT_EQ(lower64(BitShift<104>(test)), uint64_t(8388608U));
    EXPECT_EQ(lower64(BitShift<103>(test)), uint64_t(16777216U));
    EXPECT_EQ(lower64(BitShift<102>(test)), uint64_t(33554432U));
    EXPECT_EQ(lower64(BitShift<101>(test)), uint64_t(67108864U));
    EXPECT_EQ(lower64(BitShift<100>(test)), uint64_t(134217728U));
    EXPECT_EQ(lower64(BitShift<99>(test)), uint64_t(268435456U));
    EXPECT_EQ(lower64(BitShift<98>(test)), uint64_t(536870912U));
    EXPECT_EQ(lower64(BitShift<97>(test)), uint64_t(1073741824U));
    EXPECT_EQ(lower64(BitShift<96>(test)), uint64_t(2147483648U));
    EXPECT_EQ(lower64(BitShift<95>(test)), uint64_t(4294967296U));
    EXPECT_EQ(lower64(BitShift<94>(test)), uint64_t(8589934592U));
    EXPECT_EQ(lower64(BitShift<93>(test)), uint64_t(17179869184U));
    EXPECT_EQ(lower64(BitShift<92>(test)), uint64_t(34359738368U));
    EXPECT_EQ(lower64(BitShift<91>(test)), uint64_t(68719476736U));
    EXPECT_EQ(lower64(BitShift<90>(test)), uint64_t(137438953472U));
    EXPECT_EQ(lower64(BitShift<89>(test)), uint64_t(274877906944U));
    EXPECT_EQ(lower64(BitShift<88>(test)), uint64_t(549755813888U));
    EXPECT_EQ(lower64(BitShift<87>(test)), uint64_t(1099511627776U));
    EXPECT_EQ(lower64(BitShift<86>(test)), uint64_t(2199023255552U));
    EXPECT_EQ(lower64(BitShift<85>(test)), uint64_t(4398046511104U));
    EXPECT_EQ(lower64(BitShift<84>(test)), uint64_t(8796093022208U));
    EXPECT_EQ(lower64(BitShift<83>(test)), uint64_t(17592186044416U));
    EXPECT_EQ(lower64(BitShift<82>(test)), uint64_t(35184372088832U));
    EXPECT_EQ(lower64(BitShift<81>(test)), uint64_t(70368744177664U));
    EXPECT_EQ(lower64(BitShift<80>(test)), uint64_t(140737488355328U));
    EXPECT_EQ(lower64(BitShift<79>(test)), uint64_t(281474976710656U));
    EXPECT_EQ(lower64(BitShift<78>(test)), uint64_t(562949953421312U));
    EXPECT_EQ(lower64(BitShift<77>(test)), uint64_t(1125899906842624U));
    EXPECT_EQ(lower64(BitShift<76>(test)), uint64_t(2251799813685248U));
    EXPECT_EQ(lower64(BitShift<75>(test)), uint64_t(4503599627370496U));
    EXPECT_EQ(lower64(BitShift<74>(test)), uint64_t(9007199254740992U));
    EXPECT_EQ(lower64(BitShift<73>(test)), uint64_t(18014398509481984U));
    EXPECT_EQ(lower64(BitShift<72>(test)), uint64_t(36028797018963968U));
    EXPECT_EQ(lower64(BitShift<71>(test)), uint64_t(72057594037927936U));
    EXPECT_EQ(lower64(BitShift<70>(test)), uint64_t(144115188075855872U));
    EXPECT_EQ(lower64(BitShift<69>(test)), uint64_t(288230376151711744U));
    EXPECT_EQ(lower64(BitShift<68>(test)), uint64_t(576460752303423488U));
    EXPECT_EQ(lower64(BitShift<67>(test)), uint64_t(1152921504606846976U));
    EXPECT_EQ(lower64(BitShift<66>(test)), uint64_t(2305843009213693952U));
    EXPECT_EQ(lower64(BitShift<65>(test)), uint64_t(4611686018427387904U));
    EXPECT_EQ(lower64(BitShift<64>(test)), uint64_t(9223372036854775808U));
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
