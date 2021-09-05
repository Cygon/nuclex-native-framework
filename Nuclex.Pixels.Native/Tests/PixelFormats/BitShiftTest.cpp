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

#include "../../Source/PixelFormats/BitShift.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a 128 bit integer using the specified 64 bits as high bits</summary>
  /// <param name="highBits">Bits that will serveras the upper 64 bit of the integer</param>
  /// <returns>The 128 bit integer built using the specified high bits</returns>
  constexpr Nuclex::Pixels::uint128_t makeUInt128FromHighBits(std::uint64_t highBits) {
#if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)
    return static_cast<Nuclex::Pixels::uint128_t>(highBits) << 64;
#else
    return Nuclex::Pixels::uint128_t(highBits, 0);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift8BitIntegerLeft) {
    std::uint8_t test = std::uint8_t(1U);

    EXPECT_EQ(BitShift<-0>(test), std::uint8_t(1U));
    EXPECT_EQ(BitShift<-1>(test), std::uint8_t(2U));
    EXPECT_EQ(BitShift<-2>(test), std::uint8_t(4U));
    EXPECT_EQ(BitShift<-3>(test), std::uint8_t(8U));
    EXPECT_EQ(BitShift<-4>(test), std::uint8_t(16U));
    EXPECT_EQ(BitShift<-5>(test), std::uint8_t(32U));
    EXPECT_EQ(BitShift<-6>(test), std::uint8_t(64U));
    EXPECT_EQ(BitShift<-7>(test), std::uint8_t(128U));
    EXPECT_EQ(BitShift<-8>(test), std::uint8_t(0U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift8BitIntegerRight) {
    std::uint8_t test = std::uint8_t(128U);

    EXPECT_EQ(BitShift<8>(test), std::uint8_t(0U));
    EXPECT_EQ(BitShift<7>(test), std::uint8_t(1U));
    EXPECT_EQ(BitShift<6>(test), std::uint8_t(2U));
    EXPECT_EQ(BitShift<5>(test), std::uint8_t(4U));
    EXPECT_EQ(BitShift<4>(test), std::uint8_t(8U));
    EXPECT_EQ(BitShift<3>(test), std::uint8_t(16U));
    EXPECT_EQ(BitShift<2>(test), std::uint8_t(32U));
    EXPECT_EQ(BitShift<1>(test), std::uint8_t(64U));
    EXPECT_EQ(BitShift<0>(test), std::uint8_t(128U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift16BitIntegerLeft) {
    std::uint16_t test = std::uint16_t(1U);

    EXPECT_EQ(BitShift<-0>(test), std::uint16_t(1U));
    EXPECT_EQ(BitShift<-1>(test), std::uint16_t(2U));
    EXPECT_EQ(BitShift<-2>(test), std::uint16_t(4U));
    EXPECT_EQ(BitShift<-3>(test), std::uint16_t(8U));
    EXPECT_EQ(BitShift<-4>(test), std::uint16_t(16U));
    EXPECT_EQ(BitShift<-5>(test), std::uint16_t(32U));
    EXPECT_EQ(BitShift<-6>(test), std::uint16_t(64U));
    EXPECT_EQ(BitShift<-7>(test), std::uint16_t(128U));
    EXPECT_EQ(BitShift<-8>(test), std::uint16_t(256U));
    EXPECT_EQ(BitShift<-9>(test), std::uint16_t(512U));
    EXPECT_EQ(BitShift<-10>(test), std::uint16_t(1024U));
    EXPECT_EQ(BitShift<-11>(test), std::uint16_t(2048U));
    EXPECT_EQ(BitShift<-12>(test), std::uint16_t(4096U));
    EXPECT_EQ(BitShift<-13>(test), std::uint16_t(8192U));
    EXPECT_EQ(BitShift<-14>(test), std::uint16_t(16384U));
    EXPECT_EQ(BitShift<-15>(test), std::uint16_t(32768U));
    EXPECT_EQ(BitShift<-16>(test), std::uint16_t(0U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift16BitIntegerRight) {
    std::uint16_t test = std::uint16_t(32768U);

    EXPECT_EQ(BitShift<16>(test), std::uint16_t(0U));
    EXPECT_EQ(BitShift<15>(test), std::uint16_t(1U));
    EXPECT_EQ(BitShift<14>(test), std::uint16_t(2U));
    EXPECT_EQ(BitShift<13>(test), std::uint16_t(4U));
    EXPECT_EQ(BitShift<12>(test), std::uint16_t(8U));
    EXPECT_EQ(BitShift<11>(test), std::uint16_t(16U));
    EXPECT_EQ(BitShift<10>(test), std::uint16_t(32U));
    EXPECT_EQ(BitShift<9>(test), std::uint16_t(64U));
    EXPECT_EQ(BitShift<8>(test), std::uint16_t(128U));
    EXPECT_EQ(BitShift<7>(test), std::uint16_t(256U));
    EXPECT_EQ(BitShift<6>(test), std::uint16_t(512U));
    EXPECT_EQ(BitShift<5>(test), std::uint16_t(1024U));
    EXPECT_EQ(BitShift<4>(test), std::uint16_t(2048U));
    EXPECT_EQ(BitShift<3>(test), std::uint16_t(4096U));
    EXPECT_EQ(BitShift<2>(test), std::uint16_t(8192U));
    EXPECT_EQ(BitShift<1>(test), std::uint16_t(16384U));
    EXPECT_EQ(BitShift<0>(test), std::uint16_t(32768U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift32BitIntegerLeft) {
    std::uint32_t test = std::uint32_t(1U);

    EXPECT_EQ(BitShift<-0>(test), std::uint32_t(1U));
    EXPECT_EQ(BitShift<-1>(test), std::uint32_t(2U));
    EXPECT_EQ(BitShift<-2>(test), std::uint32_t(4U));
    EXPECT_EQ(BitShift<-3>(test), std::uint32_t(8U));
    EXPECT_EQ(BitShift<-4>(test), std::uint32_t(16U));
    EXPECT_EQ(BitShift<-5>(test), std::uint32_t(32U));
    EXPECT_EQ(BitShift<-6>(test), std::uint32_t(64U));
    EXPECT_EQ(BitShift<-7>(test), std::uint32_t(128U));
    EXPECT_EQ(BitShift<-8>(test), std::uint32_t(256U));
    EXPECT_EQ(BitShift<-9>(test), std::uint32_t(512U));
    EXPECT_EQ(BitShift<-10>(test), std::uint32_t(1024U));
    EXPECT_EQ(BitShift<-11>(test), std::uint32_t(2048U));
    EXPECT_EQ(BitShift<-12>(test), std::uint32_t(4096U));
    EXPECT_EQ(BitShift<-13>(test), std::uint32_t(8192U));
    EXPECT_EQ(BitShift<-14>(test), std::uint32_t(16384U));
    EXPECT_EQ(BitShift<-15>(test), std::uint32_t(32768U));
    EXPECT_EQ(BitShift<-16>(test), std::uint32_t(65536U));
    EXPECT_EQ(BitShift<-17>(test), std::uint32_t(131072U));
    EXPECT_EQ(BitShift<-18>(test), std::uint32_t(262144U));
    EXPECT_EQ(BitShift<-19>(test), std::uint32_t(524288U));
    EXPECT_EQ(BitShift<-20>(test), std::uint32_t(1048576U));
    EXPECT_EQ(BitShift<-21>(test), std::uint32_t(2097152U));
    EXPECT_EQ(BitShift<-22>(test), std::uint32_t(4194304U));
    EXPECT_EQ(BitShift<-23>(test), std::uint32_t(8388608U));
    EXPECT_EQ(BitShift<-24>(test), std::uint32_t(16777216U));
    EXPECT_EQ(BitShift<-25>(test), std::uint32_t(33554432U));
    EXPECT_EQ(BitShift<-26>(test), std::uint32_t(67108864U));
    EXPECT_EQ(BitShift<-27>(test), std::uint32_t(134217728U));
    EXPECT_EQ(BitShift<-28>(test), std::uint32_t(268435456U));
    EXPECT_EQ(BitShift<-29>(test), std::uint32_t(536870912U));
    EXPECT_EQ(BitShift<-30>(test), std::uint32_t(1073741824U));
    EXPECT_EQ(BitShift<-31>(test), std::uint32_t(2147483648U));
    EXPECT_EQ(BitShift<-32>(test), std::uint32_t(0U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift32BitIntegerRight) {
    std::uint32_t test = std::uint32_t(2147483648U);

    EXPECT_EQ(BitShift<32>(test), std::uint32_t(0U));
    EXPECT_EQ(BitShift<31>(test), std::uint32_t(1U));
    EXPECT_EQ(BitShift<30>(test), std::uint32_t(2U));
    EXPECT_EQ(BitShift<29>(test), std::uint32_t(4U));
    EXPECT_EQ(BitShift<28>(test), std::uint32_t(8U));
    EXPECT_EQ(BitShift<27>(test), std::uint32_t(16U));
    EXPECT_EQ(BitShift<26>(test), std::uint32_t(32U));
    EXPECT_EQ(BitShift<25>(test), std::uint32_t(64U));
    EXPECT_EQ(BitShift<24>(test), std::uint32_t(128U));
    EXPECT_EQ(BitShift<23>(test), std::uint32_t(256U));
    EXPECT_EQ(BitShift<22>(test), std::uint32_t(512U));
    EXPECT_EQ(BitShift<21>(test), std::uint32_t(1024U));
    EXPECT_EQ(BitShift<20>(test), std::uint32_t(2048U));
    EXPECT_EQ(BitShift<19>(test), std::uint32_t(4096U));
    EXPECT_EQ(BitShift<18>(test), std::uint32_t(8192U));
    EXPECT_EQ(BitShift<17>(test), std::uint32_t(16384U));
    EXPECT_EQ(BitShift<16>(test), std::uint32_t(32768U));
    EXPECT_EQ(BitShift<15>(test), std::uint32_t(65536U));
    EXPECT_EQ(BitShift<14>(test), std::uint32_t(131072U));
    EXPECT_EQ(BitShift<13>(test), std::uint32_t(262144U));
    EXPECT_EQ(BitShift<12>(test), std::uint32_t(524288U));
    EXPECT_EQ(BitShift<11>(test), std::uint32_t(1048576U));
    EXPECT_EQ(BitShift<10>(test), std::uint32_t(2097152U));
    EXPECT_EQ(BitShift<9>(test), std::uint32_t(4194304U));
    EXPECT_EQ(BitShift<8>(test), std::uint32_t(8388608U));
    EXPECT_EQ(BitShift<7>(test), std::uint32_t(16777216U));
    EXPECT_EQ(BitShift<6>(test), std::uint32_t(33554432U));
    EXPECT_EQ(BitShift<5>(test), std::uint32_t(67108864U));
    EXPECT_EQ(BitShift<4>(test), std::uint32_t(134217728U));
    EXPECT_EQ(BitShift<3>(test), std::uint32_t(268435456U));
    EXPECT_EQ(BitShift<2>(test), std::uint32_t(536870912U));
    EXPECT_EQ(BitShift<1>(test), std::uint32_t(1073741824U));
    EXPECT_EQ(BitShift<0>(test), std::uint32_t(2147483648U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift64BitIntegerLeft) {
    std::uint64_t test = std::uint64_t(1U);

    EXPECT_EQ(BitShift<-0>(test), std::uint64_t(1U));
    EXPECT_EQ(BitShift<-1>(test), std::uint64_t(2U));
    EXPECT_EQ(BitShift<-2>(test), std::uint64_t(4U));
    EXPECT_EQ(BitShift<-3>(test), std::uint64_t(8U));
    EXPECT_EQ(BitShift<-4>(test), std::uint64_t(16U));
    EXPECT_EQ(BitShift<-5>(test), std::uint64_t(32U));
    EXPECT_EQ(BitShift<-6>(test), std::uint64_t(64U));
    EXPECT_EQ(BitShift<-7>(test), std::uint64_t(128U));
    EXPECT_EQ(BitShift<-8>(test), std::uint64_t(256U));
    EXPECT_EQ(BitShift<-9>(test), std::uint64_t(512U));
    EXPECT_EQ(BitShift<-10>(test), std::uint64_t(1024U));
    EXPECT_EQ(BitShift<-11>(test), std::uint64_t(2048U));
    EXPECT_EQ(BitShift<-12>(test), std::uint64_t(4096U));
    EXPECT_EQ(BitShift<-13>(test), std::uint64_t(8192U));
    EXPECT_EQ(BitShift<-14>(test), std::uint64_t(16384U));
    EXPECT_EQ(BitShift<-15>(test), std::uint64_t(32768U));
    EXPECT_EQ(BitShift<-16>(test), std::uint64_t(65536U));
    EXPECT_EQ(BitShift<-17>(test), std::uint64_t(131072U));
    EXPECT_EQ(BitShift<-18>(test), std::uint64_t(262144U));
    EXPECT_EQ(BitShift<-19>(test), std::uint64_t(524288U));
    EXPECT_EQ(BitShift<-20>(test), std::uint64_t(1048576U));
    EXPECT_EQ(BitShift<-21>(test), std::uint64_t(2097152U));
    EXPECT_EQ(BitShift<-22>(test), std::uint64_t(4194304U));
    EXPECT_EQ(BitShift<-23>(test), std::uint64_t(8388608U));
    EXPECT_EQ(BitShift<-24>(test), std::uint64_t(16777216U));
    EXPECT_EQ(BitShift<-25>(test), std::uint64_t(33554432U));
    EXPECT_EQ(BitShift<-26>(test), std::uint64_t(67108864U));
    EXPECT_EQ(BitShift<-27>(test), std::uint64_t(134217728U));
    EXPECT_EQ(BitShift<-28>(test), std::uint64_t(268435456U));
    EXPECT_EQ(BitShift<-29>(test), std::uint64_t(536870912U));
    EXPECT_EQ(BitShift<-30>(test), std::uint64_t(1073741824U));
    EXPECT_EQ(BitShift<-31>(test), std::uint64_t(2147483648U));
    EXPECT_EQ(BitShift<-32>(test), std::uint64_t(4294967296U));
    EXPECT_EQ(BitShift<-33>(test), std::uint64_t(8589934592U));
    EXPECT_EQ(BitShift<-34>(test), std::uint64_t(17179869184U));
    EXPECT_EQ(BitShift<-35>(test), std::uint64_t(34359738368U));
    EXPECT_EQ(BitShift<-36>(test), std::uint64_t(68719476736U));
    EXPECT_EQ(BitShift<-37>(test), std::uint64_t(137438953472U));
    EXPECT_EQ(BitShift<-38>(test), std::uint64_t(274877906944U));
    EXPECT_EQ(BitShift<-39>(test), std::uint64_t(549755813888U));
    EXPECT_EQ(BitShift<-40>(test), std::uint64_t(1099511627776U));
    EXPECT_EQ(BitShift<-41>(test), std::uint64_t(2199023255552U));
    EXPECT_EQ(BitShift<-42>(test), std::uint64_t(4398046511104U));
    EXPECT_EQ(BitShift<-43>(test), std::uint64_t(8796093022208U));
    EXPECT_EQ(BitShift<-44>(test), std::uint64_t(17592186044416U));
    EXPECT_EQ(BitShift<-45>(test), std::uint64_t(35184372088832U));
    EXPECT_EQ(BitShift<-46>(test), std::uint64_t(70368744177664U));
    EXPECT_EQ(BitShift<-47>(test), std::uint64_t(140737488355328U));
    EXPECT_EQ(BitShift<-48>(test), std::uint64_t(281474976710656U));
    EXPECT_EQ(BitShift<-49>(test), std::uint64_t(562949953421312U));
    EXPECT_EQ(BitShift<-50>(test), std::uint64_t(1125899906842624U));
    EXPECT_EQ(BitShift<-51>(test), std::uint64_t(2251799813685248U));
    EXPECT_EQ(BitShift<-52>(test), std::uint64_t(4503599627370496U));
    EXPECT_EQ(BitShift<-53>(test), std::uint64_t(9007199254740992U));
    EXPECT_EQ(BitShift<-54>(test), std::uint64_t(18014398509481984U));
    EXPECT_EQ(BitShift<-55>(test), std::uint64_t(36028797018963968U));
    EXPECT_EQ(BitShift<-56>(test), std::uint64_t(72057594037927936U));
    EXPECT_EQ(BitShift<-57>(test), std::uint64_t(144115188075855872U));
    EXPECT_EQ(BitShift<-58>(test), std::uint64_t(288230376151711744U));
    EXPECT_EQ(BitShift<-59>(test), std::uint64_t(576460752303423488U));
    EXPECT_EQ(BitShift<-60>(test), std::uint64_t(1152921504606846976U));
    EXPECT_EQ(BitShift<-61>(test), std::uint64_t(2305843009213693952U));
    EXPECT_EQ(BitShift<-62>(test), std::uint64_t(4611686018427387904U));
    EXPECT_EQ(BitShift<-63>(test), std::uint64_t(9223372036854775808U));
    EXPECT_EQ(BitShift<-64>(test), std::uint64_t(0U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift64BitIntegerRight) {
    std::uint64_t test = std::uint64_t(9223372036854775808U);

    EXPECT_EQ(BitShift<64>(test), std::uint64_t(0U));
    EXPECT_EQ(BitShift<63>(test), std::uint64_t(1U));
    EXPECT_EQ(BitShift<62>(test), std::uint64_t(2U));
    EXPECT_EQ(BitShift<61>(test), std::uint64_t(4U));
    EXPECT_EQ(BitShift<60>(test), std::uint64_t(8U));
    EXPECT_EQ(BitShift<59>(test), std::uint64_t(16U));
    EXPECT_EQ(BitShift<58>(test), std::uint64_t(32U));
    EXPECT_EQ(BitShift<57>(test), std::uint64_t(64U));
    EXPECT_EQ(BitShift<56>(test), std::uint64_t(128U));
    EXPECT_EQ(BitShift<55>(test), std::uint64_t(256U));
    EXPECT_EQ(BitShift<54>(test), std::uint64_t(512U));
    EXPECT_EQ(BitShift<53>(test), std::uint64_t(1024U));
    EXPECT_EQ(BitShift<52>(test), std::uint64_t(2048U));
    EXPECT_EQ(BitShift<51>(test), std::uint64_t(4096U));
    EXPECT_EQ(BitShift<50>(test), std::uint64_t(8192U));
    EXPECT_EQ(BitShift<49>(test), std::uint64_t(16384U));
    EXPECT_EQ(BitShift<48>(test), std::uint64_t(32768U));
    EXPECT_EQ(BitShift<47>(test), std::uint64_t(65536U));
    EXPECT_EQ(BitShift<46>(test), std::uint64_t(131072U));
    EXPECT_EQ(BitShift<45>(test), std::uint64_t(262144U));
    EXPECT_EQ(BitShift<44>(test), std::uint64_t(524288U));
    EXPECT_EQ(BitShift<43>(test), std::uint64_t(1048576U));
    EXPECT_EQ(BitShift<42>(test), std::uint64_t(2097152U));
    EXPECT_EQ(BitShift<41>(test), std::uint64_t(4194304U));
    EXPECT_EQ(BitShift<40>(test), std::uint64_t(8388608U));
    EXPECT_EQ(BitShift<39>(test), std::uint64_t(16777216U));
    EXPECT_EQ(BitShift<38>(test), std::uint64_t(33554432U));
    EXPECT_EQ(BitShift<37>(test), std::uint64_t(67108864U));
    EXPECT_EQ(BitShift<36>(test), std::uint64_t(134217728U));
    EXPECT_EQ(BitShift<35>(test), std::uint64_t(268435456U));
    EXPECT_EQ(BitShift<34>(test), std::uint64_t(536870912U));
    EXPECT_EQ(BitShift<33>(test), std::uint64_t(1073741824U));
    EXPECT_EQ(BitShift<32>(test), std::uint64_t(2147483648U));
    EXPECT_EQ(BitShift<31>(test), std::uint64_t(4294967296U));
    EXPECT_EQ(BitShift<30>(test), std::uint64_t(8589934592U));
    EXPECT_EQ(BitShift<29>(test), std::uint64_t(17179869184U));
    EXPECT_EQ(BitShift<28>(test), std::uint64_t(34359738368U));
    EXPECT_EQ(BitShift<27>(test), std::uint64_t(68719476736U));
    EXPECT_EQ(BitShift<26>(test), std::uint64_t(137438953472U));
    EXPECT_EQ(BitShift<25>(test), std::uint64_t(274877906944U));
    EXPECT_EQ(BitShift<24>(test), std::uint64_t(549755813888U));
    EXPECT_EQ(BitShift<23>(test), std::uint64_t(1099511627776U));
    EXPECT_EQ(BitShift<22>(test), std::uint64_t(2199023255552U));
    EXPECT_EQ(BitShift<21>(test), std::uint64_t(4398046511104U));
    EXPECT_EQ(BitShift<20>(test), std::uint64_t(8796093022208U));
    EXPECT_EQ(BitShift<19>(test), std::uint64_t(17592186044416U));
    EXPECT_EQ(BitShift<18>(test), std::uint64_t(35184372088832U));
    EXPECT_EQ(BitShift<17>(test), std::uint64_t(70368744177664U));
    EXPECT_EQ(BitShift<16>(test), std::uint64_t(140737488355328U));
    EXPECT_EQ(BitShift<15>(test), std::uint64_t(281474976710656U));
    EXPECT_EQ(BitShift<14>(test), std::uint64_t(562949953421312U));
    EXPECT_EQ(BitShift<13>(test), std::uint64_t(1125899906842624U));
    EXPECT_EQ(BitShift<12>(test), std::uint64_t(2251799813685248U));
    EXPECT_EQ(BitShift<11>(test), std::uint64_t(4503599627370496U));
    EXPECT_EQ(BitShift<10>(test), std::uint64_t(9007199254740992U));
    EXPECT_EQ(BitShift<9>(test), std::uint64_t(18014398509481984U));
    EXPECT_EQ(BitShift<8>(test), std::uint64_t(36028797018963968U));
    EXPECT_EQ(BitShift<7>(test), std::uint64_t(72057594037927936U));
    EXPECT_EQ(BitShift<6>(test), std::uint64_t(144115188075855872U));
    EXPECT_EQ(BitShift<5>(test), std::uint64_t(288230376151711744U));
    EXPECT_EQ(BitShift<4>(test), std::uint64_t(576460752303423488U));
    EXPECT_EQ(BitShift<3>(test), std::uint64_t(1152921504606846976U));
    EXPECT_EQ(BitShift<2>(test), std::uint64_t(2305843009213693952U));
    EXPECT_EQ(BitShift<1>(test), std::uint64_t(4611686018427387904U));
    EXPECT_EQ(BitShift<0>(test), std::uint64_t(9223372036854775808U));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift128BitIntegerLeft) {
    uint128_t test = uint128_t(1U);

    EXPECT_EQ(BitShift<-0>(test), uint128_t(1U));
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

    EXPECT_EQ(BitShift<-64>(test), makeUInt128FromHighBits(std::uint64_t(1U)));
    EXPECT_EQ(BitShift<-65>(test), makeUInt128FromHighBits(std::uint64_t(2U)));
    EXPECT_EQ(BitShift<-66>(test), makeUInt128FromHighBits(std::uint64_t(4U)));
    EXPECT_EQ(BitShift<-67>(test), makeUInt128FromHighBits(std::uint64_t(8U)));
    EXPECT_EQ(BitShift<-68>(test), makeUInt128FromHighBits(std::uint64_t(16U)));
    EXPECT_EQ(BitShift<-69>(test), makeUInt128FromHighBits(std::uint64_t(32U)));
    EXPECT_EQ(BitShift<-70>(test), makeUInt128FromHighBits(std::uint64_t(64U)));
    EXPECT_EQ(BitShift<-71>(test), makeUInt128FromHighBits(std::uint64_t(128U)));
    EXPECT_EQ(BitShift<-72>(test), makeUInt128FromHighBits(std::uint64_t(256U)));
    EXPECT_EQ(BitShift<-73>(test), makeUInt128FromHighBits(std::uint64_t(512U)));
    EXPECT_EQ(BitShift<-74>(test), makeUInt128FromHighBits(std::uint64_t(1024U)));
    EXPECT_EQ(BitShift<-75>(test), makeUInt128FromHighBits(std::uint64_t(2048U)));
    EXPECT_EQ(BitShift<-76>(test), makeUInt128FromHighBits(std::uint64_t(4096U)));
    EXPECT_EQ(BitShift<-77>(test), makeUInt128FromHighBits(std::uint64_t(8192U)));
    EXPECT_EQ(BitShift<-78>(test), makeUInt128FromHighBits(std::uint64_t(16384U)));
    EXPECT_EQ(BitShift<-79>(test), makeUInt128FromHighBits(std::uint64_t(32768U)));
    EXPECT_EQ(BitShift<-80>(test), makeUInt128FromHighBits(std::uint64_t(65536U)));
    EXPECT_EQ(BitShift<-81>(test), makeUInt128FromHighBits(std::uint64_t(131072U)));
    EXPECT_EQ(BitShift<-82>(test), makeUInt128FromHighBits(std::uint64_t(262144U)));
    EXPECT_EQ(BitShift<-83>(test), makeUInt128FromHighBits(std::uint64_t(524288U)));
    EXPECT_EQ(BitShift<-84>(test), makeUInt128FromHighBits(std::uint64_t(1048576U)));
    EXPECT_EQ(BitShift<-85>(test), makeUInt128FromHighBits(std::uint64_t(2097152U)));
    EXPECT_EQ(BitShift<-86>(test), makeUInt128FromHighBits(std::uint64_t(4194304U)));
    EXPECT_EQ(BitShift<-87>(test), makeUInt128FromHighBits(std::uint64_t(8388608U)));
    EXPECT_EQ(BitShift<-88>(test), makeUInt128FromHighBits(std::uint64_t(16777216U)));
    EXPECT_EQ(BitShift<-89>(test), makeUInt128FromHighBits(std::uint64_t(33554432U)));
    EXPECT_EQ(BitShift<-90>(test), makeUInt128FromHighBits(std::uint64_t(67108864U)));
    EXPECT_EQ(BitShift<-91>(test), makeUInt128FromHighBits(std::uint64_t(134217728U)));
    EXPECT_EQ(BitShift<-92>(test), makeUInt128FromHighBits(std::uint64_t(268435456U)));
    EXPECT_EQ(BitShift<-93>(test), makeUInt128FromHighBits(std::uint64_t(536870912U)));
    EXPECT_EQ(BitShift<-94>(test), makeUInt128FromHighBits(std::uint64_t(1073741824U)));
    EXPECT_EQ(BitShift<-95>(test), makeUInt128FromHighBits(std::uint64_t(2147483648U)));
    EXPECT_EQ(BitShift<-96>(test), makeUInt128FromHighBits(std::uint64_t(4294967296U)));
    EXPECT_EQ(BitShift<-97>(test), makeUInt128FromHighBits(std::uint64_t(8589934592U)));
    EXPECT_EQ(BitShift<-98>(test), makeUInt128FromHighBits(std::uint64_t(17179869184U)));
    EXPECT_EQ(BitShift<-99>(test), makeUInt128FromHighBits(std::uint64_t(34359738368U)));
    EXPECT_EQ(BitShift<-100>(test), makeUInt128FromHighBits(std::uint64_t(68719476736U)));
    EXPECT_EQ(BitShift<-101>(test), makeUInt128FromHighBits(std::uint64_t(137438953472U)));
    EXPECT_EQ(BitShift<-102>(test), makeUInt128FromHighBits(std::uint64_t(274877906944U)));
    EXPECT_EQ(BitShift<-103>(test), makeUInt128FromHighBits(std::uint64_t(549755813888U)));
    EXPECT_EQ(BitShift<-104>(test), makeUInt128FromHighBits(std::uint64_t(1099511627776U)));
    EXPECT_EQ(BitShift<-105>(test), makeUInt128FromHighBits(std::uint64_t(2199023255552U)));
    EXPECT_EQ(BitShift<-106>(test), makeUInt128FromHighBits(std::uint64_t(4398046511104U)));
    EXPECT_EQ(BitShift<-107>(test), makeUInt128FromHighBits(std::uint64_t(8796093022208U)));
    EXPECT_EQ(BitShift<-108>(test), makeUInt128FromHighBits(std::uint64_t(17592186044416U)));
    EXPECT_EQ(BitShift<-109>(test), makeUInt128FromHighBits(std::uint64_t(35184372088832U)));
    EXPECT_EQ(BitShift<-110>(test), makeUInt128FromHighBits(std::uint64_t(70368744177664U)));
    EXPECT_EQ(BitShift<-111>(test), makeUInt128FromHighBits(std::uint64_t(140737488355328U)));
    EXPECT_EQ(BitShift<-112>(test), makeUInt128FromHighBits(std::uint64_t(281474976710656U)));
    EXPECT_EQ(BitShift<-113>(test), makeUInt128FromHighBits(std::uint64_t(562949953421312U)));
    EXPECT_EQ(BitShift<-114>(test), makeUInt128FromHighBits(std::uint64_t(1125899906842624U)));
    EXPECT_EQ(BitShift<-115>(test), makeUInt128FromHighBits(std::uint64_t(2251799813685248U)));
    EXPECT_EQ(BitShift<-116>(test), makeUInt128FromHighBits(std::uint64_t(4503599627370496U)));
    EXPECT_EQ(BitShift<-117>(test), makeUInt128FromHighBits(std::uint64_t(9007199254740992U)));
    EXPECT_EQ(BitShift<-118>(test), makeUInt128FromHighBits(std::uint64_t(18014398509481984U)));
    EXPECT_EQ(BitShift<-119>(test), makeUInt128FromHighBits(std::uint64_t(36028797018963968U)));
    EXPECT_EQ(BitShift<-120>(test), makeUInt128FromHighBits(std::uint64_t(72057594037927936U)));
    EXPECT_EQ(BitShift<-121>(test), makeUInt128FromHighBits(std::uint64_t(144115188075855872U)));
    EXPECT_EQ(BitShift<-122>(test), makeUInt128FromHighBits(std::uint64_t(288230376151711744U)));
    EXPECT_EQ(BitShift<-123>(test), makeUInt128FromHighBits(std::uint64_t(576460752303423488U)));
    EXPECT_EQ(BitShift<-124>(test), makeUInt128FromHighBits(std::uint64_t(1152921504606846976U)));
    EXPECT_EQ(BitShift<-125>(test), makeUInt128FromHighBits(std::uint64_t(2305843009213693952U)));
    EXPECT_EQ(BitShift<-126>(test), makeUInt128FromHighBits(std::uint64_t(4611686018427387904U)));
    EXPECT_EQ(BitShift<-127>(test), makeUInt128FromHighBits(std::uint64_t(9223372036854775808U)));
    EXPECT_EQ(BitShift<-128>(test), makeUInt128FromHighBits(std::uint64_t(0U)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitShiftTest, CanShift128BitIntegerRight) {
    uint128_t test = makeUInt128FromHighBits(9223372036854775808U);

    EXPECT_EQ(BitShift<128>(test), uint128_t(0U));
    EXPECT_EQ(BitShift<127>(test), uint128_t(1U));
    EXPECT_EQ(BitShift<126>(test), uint128_t(2U));
    EXPECT_EQ(BitShift<125>(test), uint128_t(4U));
    EXPECT_EQ(BitShift<124>(test), uint128_t(8U));
    EXPECT_EQ(BitShift<123>(test), uint128_t(16U));
    EXPECT_EQ(BitShift<122>(test), uint128_t(32U));
    EXPECT_EQ(BitShift<121>(test), uint128_t(64U));
    EXPECT_EQ(BitShift<120>(test), uint128_t(128U));
    EXPECT_EQ(BitShift<119>(test), uint128_t(256U));
    EXPECT_EQ(BitShift<118>(test), uint128_t(512U));
    EXPECT_EQ(BitShift<117>(test), uint128_t(1024U));
    EXPECT_EQ(BitShift<116>(test), uint128_t(2048U));
    EXPECT_EQ(BitShift<115>(test), uint128_t(4096U));
    EXPECT_EQ(BitShift<114>(test), uint128_t(8192U));
    EXPECT_EQ(BitShift<113>(test), uint128_t(16384U));
    EXPECT_EQ(BitShift<112>(test), uint128_t(32768U));
    EXPECT_EQ(BitShift<111>(test), uint128_t(65536U));
    EXPECT_EQ(BitShift<110>(test), uint128_t(131072U));
    EXPECT_EQ(BitShift<109>(test), uint128_t(262144U));
    EXPECT_EQ(BitShift<108>(test), uint128_t(524288U));
    EXPECT_EQ(BitShift<107>(test), uint128_t(1048576U));
    EXPECT_EQ(BitShift<106>(test), uint128_t(2097152U));
    EXPECT_EQ(BitShift<105>(test), uint128_t(4194304U));
    EXPECT_EQ(BitShift<104>(test), uint128_t(8388608U));
    EXPECT_EQ(BitShift<103>(test), uint128_t(16777216U));
    EXPECT_EQ(BitShift<102>(test), uint128_t(33554432U));
    EXPECT_EQ(BitShift<101>(test), uint128_t(67108864U));
    EXPECT_EQ(BitShift<100>(test), uint128_t(134217728U));
    EXPECT_EQ(BitShift<99>(test), uint128_t(268435456U));
    EXPECT_EQ(BitShift<98>(test), uint128_t(536870912U));
    EXPECT_EQ(BitShift<97>(test), uint128_t(1073741824U));
    EXPECT_EQ(BitShift<96>(test), uint128_t(2147483648U));
    EXPECT_EQ(BitShift<95>(test), uint128_t(4294967296U));
    EXPECT_EQ(BitShift<94>(test), uint128_t(8589934592U));
    EXPECT_EQ(BitShift<93>(test), uint128_t(17179869184U));
    EXPECT_EQ(BitShift<92>(test), uint128_t(34359738368U));
    EXPECT_EQ(BitShift<91>(test), uint128_t(68719476736U));
    EXPECT_EQ(BitShift<90>(test), uint128_t(137438953472U));
    EXPECT_EQ(BitShift<89>(test), uint128_t(274877906944U));
    EXPECT_EQ(BitShift<88>(test), uint128_t(549755813888U));
    EXPECT_EQ(BitShift<87>(test), uint128_t(1099511627776U));
    EXPECT_EQ(BitShift<86>(test), uint128_t(2199023255552U));
    EXPECT_EQ(BitShift<85>(test), uint128_t(4398046511104U));
    EXPECT_EQ(BitShift<84>(test), uint128_t(8796093022208U));
    EXPECT_EQ(BitShift<83>(test), uint128_t(17592186044416U));
    EXPECT_EQ(BitShift<82>(test), uint128_t(35184372088832U));
    EXPECT_EQ(BitShift<81>(test), uint128_t(70368744177664U));
    EXPECT_EQ(BitShift<80>(test), uint128_t(140737488355328U));
    EXPECT_EQ(BitShift<79>(test), uint128_t(281474976710656U));
    EXPECT_EQ(BitShift<78>(test), uint128_t(562949953421312U));
    EXPECT_EQ(BitShift<77>(test), uint128_t(1125899906842624U));
    EXPECT_EQ(BitShift<76>(test), uint128_t(2251799813685248U));
    EXPECT_EQ(BitShift<75>(test), uint128_t(4503599627370496U));
    EXPECT_EQ(BitShift<74>(test), uint128_t(9007199254740992U));
    EXPECT_EQ(BitShift<73>(test), uint128_t(18014398509481984U));
    EXPECT_EQ(BitShift<72>(test), uint128_t(36028797018963968U));
    EXPECT_EQ(BitShift<71>(test), uint128_t(72057594037927936U));
    EXPECT_EQ(BitShift<70>(test), uint128_t(144115188075855872U));
    EXPECT_EQ(BitShift<69>(test), uint128_t(288230376151711744U));
    EXPECT_EQ(BitShift<68>(test), uint128_t(576460752303423488U));
    EXPECT_EQ(BitShift<67>(test), uint128_t(1152921504606846976U));
    EXPECT_EQ(BitShift<66>(test), uint128_t(2305843009213693952U));
    EXPECT_EQ(BitShift<65>(test), uint128_t(4611686018427387904U));
    EXPECT_EQ(BitShift<64>(test), uint128_t(9223372036854775808U));

    EXPECT_EQ(BitShift<63>(test), makeUInt128FromHighBits(1U));
    EXPECT_EQ(BitShift<62>(test), makeUInt128FromHighBits(2U));
    EXPECT_EQ(BitShift<61>(test), makeUInt128FromHighBits(4U));
    EXPECT_EQ(BitShift<60>(test), makeUInt128FromHighBits(8U));
    EXPECT_EQ(BitShift<59>(test), makeUInt128FromHighBits(16U));
    EXPECT_EQ(BitShift<58>(test), makeUInt128FromHighBits(32U));
    EXPECT_EQ(BitShift<57>(test), makeUInt128FromHighBits(64U));
    EXPECT_EQ(BitShift<56>(test), makeUInt128FromHighBits(128U));
    EXPECT_EQ(BitShift<55>(test), makeUInt128FromHighBits(256U));
    EXPECT_EQ(BitShift<54>(test), makeUInt128FromHighBits(512U));
    EXPECT_EQ(BitShift<53>(test), makeUInt128FromHighBits(1024U));
    EXPECT_EQ(BitShift<52>(test), makeUInt128FromHighBits(2048U));
    EXPECT_EQ(BitShift<51>(test), makeUInt128FromHighBits(4096U));
    EXPECT_EQ(BitShift<50>(test), makeUInt128FromHighBits(8192U));
    EXPECT_EQ(BitShift<49>(test), makeUInt128FromHighBits(16384U));
    EXPECT_EQ(BitShift<48>(test), makeUInt128FromHighBits(32768U));
    EXPECT_EQ(BitShift<47>(test), makeUInt128FromHighBits(65536U));
    EXPECT_EQ(BitShift<46>(test), makeUInt128FromHighBits(131072U));
    EXPECT_EQ(BitShift<45>(test), makeUInt128FromHighBits(262144U));
    EXPECT_EQ(BitShift<44>(test), makeUInt128FromHighBits(524288U));
    EXPECT_EQ(BitShift<43>(test), makeUInt128FromHighBits(1048576U));
    EXPECT_EQ(BitShift<42>(test), makeUInt128FromHighBits(2097152U));
    EXPECT_EQ(BitShift<41>(test), makeUInt128FromHighBits(4194304U));
    EXPECT_EQ(BitShift<40>(test), makeUInt128FromHighBits(8388608U));
    EXPECT_EQ(BitShift<39>(test), makeUInt128FromHighBits(16777216U));
    EXPECT_EQ(BitShift<38>(test), makeUInt128FromHighBits(33554432U));
    EXPECT_EQ(BitShift<37>(test), makeUInt128FromHighBits(67108864U));
    EXPECT_EQ(BitShift<36>(test), makeUInt128FromHighBits(134217728U));
    EXPECT_EQ(BitShift<35>(test), makeUInt128FromHighBits(268435456U));
    EXPECT_EQ(BitShift<34>(test), makeUInt128FromHighBits(536870912U));
    EXPECT_EQ(BitShift<33>(test), makeUInt128FromHighBits(1073741824U));
    EXPECT_EQ(BitShift<32>(test), makeUInt128FromHighBits(2147483648U));
    EXPECT_EQ(BitShift<31>(test), makeUInt128FromHighBits(4294967296U));
    EXPECT_EQ(BitShift<30>(test), makeUInt128FromHighBits(8589934592U));
    EXPECT_EQ(BitShift<29>(test), makeUInt128FromHighBits(17179869184U));
    EXPECT_EQ(BitShift<28>(test), makeUInt128FromHighBits(34359738368U));
    EXPECT_EQ(BitShift<27>(test), makeUInt128FromHighBits(68719476736U));
    EXPECT_EQ(BitShift<26>(test), makeUInt128FromHighBits(137438953472U));
    EXPECT_EQ(BitShift<25>(test), makeUInt128FromHighBits(274877906944U));
    EXPECT_EQ(BitShift<24>(test), makeUInt128FromHighBits(549755813888U));
    EXPECT_EQ(BitShift<23>(test), makeUInt128FromHighBits(1099511627776U));
    EXPECT_EQ(BitShift<22>(test), makeUInt128FromHighBits(2199023255552U));
    EXPECT_EQ(BitShift<21>(test), makeUInt128FromHighBits(4398046511104U));
    EXPECT_EQ(BitShift<20>(test), makeUInt128FromHighBits(8796093022208U));
    EXPECT_EQ(BitShift<19>(test), makeUInt128FromHighBits(17592186044416U));
    EXPECT_EQ(BitShift<18>(test), makeUInt128FromHighBits(35184372088832U));
    EXPECT_EQ(BitShift<17>(test), makeUInt128FromHighBits(70368744177664U));
    EXPECT_EQ(BitShift<16>(test), makeUInt128FromHighBits(140737488355328U));
    EXPECT_EQ(BitShift<15>(test), makeUInt128FromHighBits(281474976710656U));
    EXPECT_EQ(BitShift<14>(test), makeUInt128FromHighBits(562949953421312U));
    EXPECT_EQ(BitShift<13>(test), makeUInt128FromHighBits(1125899906842624U));
    EXPECT_EQ(BitShift<12>(test), makeUInt128FromHighBits(2251799813685248U));
    EXPECT_EQ(BitShift<11>(test), makeUInt128FromHighBits(4503599627370496U));
    EXPECT_EQ(BitShift<10>(test), makeUInt128FromHighBits(9007199254740992U));
    EXPECT_EQ(BitShift<9>(test), makeUInt128FromHighBits(18014398509481984U));
    EXPECT_EQ(BitShift<8>(test), makeUInt128FromHighBits(36028797018963968U));
    EXPECT_EQ(BitShift<7>(test), makeUInt128FromHighBits(72057594037927936U));
    EXPECT_EQ(BitShift<6>(test), makeUInt128FromHighBits(144115188075855872U));
    EXPECT_EQ(BitShift<5>(test), makeUInt128FromHighBits(288230376151711744U));
    EXPECT_EQ(BitShift<4>(test), makeUInt128FromHighBits(576460752303423488U));
    EXPECT_EQ(BitShift<3>(test), makeUInt128FromHighBits(1152921504606846976U));
    EXPECT_EQ(BitShift<2>(test), makeUInt128FromHighBits(2305843009213693952U));
    EXPECT_EQ(BitShift<1>(test), makeUInt128FromHighBits(4611686018427387904U));
    EXPECT_EQ(BitShift<0>(test), makeUInt128FromHighBits(9223372036854775808U));

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
