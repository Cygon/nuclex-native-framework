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

#include "Nuclex/Support/Variant.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanBeCreated) {
    EXPECT_NO_THROW(
      Variant test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, HasCopyConstructor) {
    Variant original = 123;
    Variant copy(original);

    EXPECT_EQ(copy.ToInt32(), original.ToInt32());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, HasMoveConstructor) {
    Variant original = 123;
    Variant moved(std::move(original));

    EXPECT_EQ(moved.ToInt32(), std::int32_t(123));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanHoldDifferentTypes) {
    { Variant boolVariant(true); }
    { Variant uint8Variant(std::uint8_t(234)); }
    { Variant int8Variant(std::int8_t(123)); }
    { Variant uint16Variant(std::uint16_t(54321)); }
    { Variant int16Variant(std::int16_t(12345)); }
    { Variant uint32Variant(std::uint32_t(987654321)); }
    { Variant int32Variant(std::int32_t(123456789)); }
    { Variant uint64Variant(std::uint64_t(9876543210000ULL)); }
    { Variant int64Variant(std::int64_t(1234567890000LL)); }
    { Variant floatVariant(float(12.34f)); }
    { Variant doubleVariant(double(1234.5678)); }
    { Variant stringVariant(std::string("Hello World")); }
    { Variant wstringVariant(std::wstring(L"Hello World")); }
    { Variant anyVariant(Any(12345)); }
    { Variant voidPointerVariant(nullptr); }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanCheckForEmptiness) {
    EXPECT_TRUE(Variant().IsEmpty());
    EXPECT_FALSE(Variant(false).IsEmpty());
    EXPECT_FALSE(Variant(true).IsEmpty());
    EXPECT_FALSE(Variant(std::uint8_t(234)).IsEmpty());
    EXPECT_FALSE(Variant(std::int8_t(123)).IsEmpty());
    EXPECT_FALSE(Variant(std::uint16_t(54321)).IsEmpty());
    EXPECT_FALSE(Variant(std::int16_t(12345)).IsEmpty());
    EXPECT_FALSE(Variant(std::uint32_t(987654321)).IsEmpty());
    EXPECT_FALSE(Variant(std::int32_t(123456789)).IsEmpty());
    EXPECT_FALSE(Variant(std::uint64_t(9876543210000ULL)).IsEmpty());
    EXPECT_FALSE(Variant(std::int64_t(1234567890000LL)).IsEmpty());
    EXPECT_FALSE(Variant(float(12.34f)).IsEmpty());
    EXPECT_FALSE(Variant(double(1234.5678)).IsEmpty());
    EXPECT_FALSE(Variant(std::string(u8"Hello World")).IsEmpty());
    EXPECT_FALSE(Variant(std::wstring(L"Hello World")).IsEmpty());
    EXPECT_FALSE(Variant(Any(12345)).IsEmpty());
    EXPECT_FALSE(Variant(nullptr).IsEmpty());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, BooleanVariantCanBeConvertedToDifferentTypes) {
    Variant trueVariant(true), falseVariant(false);
    
    EXPECT_EQ(false, falseVariant.ToBoolean());
    EXPECT_EQ(true, trueVariant.ToBoolean());
    EXPECT_EQ(std::uint8_t(0), falseVariant.ToUint8());
    EXPECT_EQ(std::uint8_t(1), trueVariant.ToUint8());
    EXPECT_EQ(std::int8_t(0), falseVariant.ToInt8());
    EXPECT_EQ(std::int8_t(1), trueVariant.ToInt8());
    EXPECT_EQ(std::uint16_t(0), falseVariant.ToUint16());
    EXPECT_EQ(std::uint16_t(1), trueVariant.ToUint16());
    EXPECT_EQ(std::int16_t(0), falseVariant.ToInt16());
    EXPECT_EQ(std::int16_t(1), trueVariant.ToInt16());
    EXPECT_EQ(std::uint32_t(0), falseVariant.ToUint32());
    EXPECT_EQ(std::uint32_t(1), trueVariant.ToUint32());
    EXPECT_EQ(std::int32_t(0), falseVariant.ToInt32());
    EXPECT_EQ(std::int32_t(1), trueVariant.ToInt32());
    EXPECT_EQ(std::uint64_t(0), falseVariant.ToUint64());
    EXPECT_EQ(std::uint64_t(1), trueVariant.ToUint64());
    EXPECT_EQ(std::int64_t(0), falseVariant.ToInt64());
    EXPECT_EQ(std::int64_t(1), trueVariant.ToInt64());
    EXPECT_EQ(float(0.0f), falseVariant.ToFloat());
    EXPECT_EQ(float(1.0f), trueVariant.ToFloat());
    EXPECT_EQ(double(0.0), falseVariant.ToDouble());
    EXPECT_EQ(double(1.0), trueVariant.ToDouble());
    EXPECT_EQ(std::string(u8"0"), falseVariant.ToString());
    EXPECT_EQ(std::string(u8"1"), trueVariant.ToString());
    EXPECT_EQ(std::wstring(L"0"), falseVariant.ToWString());
    EXPECT_EQ(std::wstring(L"1"), trueVariant.ToWString());
    EXPECT_EQ(false, falseVariant.ToAny().Get<bool>());
    EXPECT_EQ(true, trueVariant.ToAny().Get<bool>());
    EXPECT_EQ(reinterpret_cast<void *>(std::uintptr_t(0)), falseVariant.ToVoidPointer());
    EXPECT_EQ(reinterpret_cast<void *>(std::uintptr_t(1)), trueVariant.ToVoidPointer());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Uint8VariantCanBeConvertedToDifferentTypes) {
    Variant uint8Variant(std::uint8_t(222));

    EXPECT_EQ(false, Variant(std::uint8_t(0)).ToBoolean());
    EXPECT_EQ(true, uint8Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(222), uint8Variant.ToUint8());
    EXPECT_EQ(std::int8_t(-34), uint8Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(222), uint8Variant.ToUint16());
    EXPECT_EQ(std::int16_t(222), uint8Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(222), uint8Variant.ToUint32());
    EXPECT_EQ(std::int32_t(222), uint8Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(222), uint8Variant.ToUint64());
    EXPECT_EQ(std::int64_t(222), uint8Variant.ToInt64());
    EXPECT_EQ(float(222.0f), uint8Variant.ToFloat());
    EXPECT_EQ(double(222.0), uint8Variant.ToDouble());
    EXPECT_EQ(std::string(u8"222"), uint8Variant.ToString());
    EXPECT_EQ(std::wstring(L"222"), uint8Variant.ToWString());
    EXPECT_EQ(std::uint8_t(222), uint8Variant.ToAny().Get<std::uint8_t>());
    EXPECT_EQ(reinterpret_cast<void *>(std::uintptr_t(222)), uint8Variant.ToVoidPointer());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Int8VariantCanBeConvertedToDifferentTypes) {
    Variant int8Variant(std::int8_t(-123));

    EXPECT_EQ(false, Variant(std::int8_t(0)).ToBoolean());
    EXPECT_EQ(true, int8Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(133), int8Variant.ToUint8());
    EXPECT_EQ(std::int8_t(-123), int8Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(65413), int8Variant.ToUint16());
    EXPECT_EQ(std::int16_t(-123), int8Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(4294967173), int8Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-123), int8Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(18446744073709551493ULL), int8Variant.ToUint64());
    EXPECT_EQ(std::int64_t(-123), int8Variant.ToInt64());
    EXPECT_EQ(float(-123.0f), int8Variant.ToFloat());
    EXPECT_EQ(double(-123.0), int8Variant.ToDouble());
    EXPECT_EQ(std::string(u8"-123"), int8Variant.ToString());
    EXPECT_EQ(std::wstring(L"-123"), int8Variant.ToWString());
    EXPECT_EQ(std::int8_t(-123), int8Variant.ToAny().Get<std::int8_t>());
    EXPECT_EQ(reinterpret_cast<void *>(std::intptr_t(-123)), int8Variant.ToVoidPointer());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Uint16VariantCanBeConvertedToDifferentTypes) {
    Variant uint16Variant(std::uint16_t(55555));

    EXPECT_EQ(false, Variant(std::uint16_t(0)).ToBoolean());
    EXPECT_EQ(true, uint16Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(3), uint16Variant.ToUint8());
    EXPECT_EQ(std::int8_t(3), uint16Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(55555), uint16Variant.ToUint16());
    EXPECT_EQ(std::int16_t(-9981), uint16Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(55555), uint16Variant.ToUint32());
    EXPECT_EQ(std::int32_t(55555), uint16Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(55555), uint16Variant.ToUint64());
    EXPECT_EQ(std::int64_t(55555), uint16Variant.ToInt64());
    EXPECT_EQ(float(55555.0f), uint16Variant.ToFloat());
    EXPECT_EQ(double(55555.0), uint16Variant.ToDouble());
    EXPECT_EQ(std::string(u8"55555"), uint16Variant.ToString());
    EXPECT_EQ(std::wstring(L"55555"), uint16Variant.ToWString());
    EXPECT_EQ(std::uint16_t(55555), uint16Variant.ToAny().Get<std::uint16_t>());
    EXPECT_EQ(reinterpret_cast<void *>(std::uintptr_t(55555)), uint16Variant.ToVoidPointer());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Int16VariantCanBeConvertedToDifferentTypes) {
    Variant int16Variant(std::int16_t(-22222));

    EXPECT_EQ(false, Variant(std::int16_t(0)).ToBoolean());
    EXPECT_EQ(true, int16Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(50), int16Variant.ToUint8());
    EXPECT_EQ(std::int8_t(50), int16Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(43314), int16Variant.ToUint16());
    EXPECT_EQ(std::int16_t(-22222), int16Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(4294945074), int16Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-22222), int16Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(18446744073709529394ULL), int16Variant.ToUint64());
    EXPECT_EQ(std::int64_t(-22222), int16Variant.ToInt64());
    EXPECT_EQ(float(-22222.0f), int16Variant.ToFloat());
    EXPECT_EQ(double(-22222.0), int16Variant.ToDouble());
    EXPECT_EQ(std::string(u8"-22222"), int16Variant.ToString());
    EXPECT_EQ(std::wstring(L"-22222"), int16Variant.ToWString());
    EXPECT_EQ(std::int16_t(-22222), int16Variant.ToAny().Get<std::int16_t>());
    EXPECT_EQ(reinterpret_cast<void *>(std::intptr_t(-22222)), int16Variant.ToVoidPointer());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Uint32VariantCanBeConvertedToDifferentTypes) {
    Variant uint32Variant(std::uint32_t(3333333333));

    EXPECT_EQ(false, Variant(std::uint32_t(0)).ToBoolean());
    EXPECT_EQ(true, uint32Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(85), uint32Variant.ToUint8());
    EXPECT_EQ(std::int8_t(85), uint32Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(41301), uint32Variant.ToUint16());
    EXPECT_EQ(std::int16_t(-24235), uint32Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(3333333333), uint32Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-961633963), uint32Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(3333333333), uint32Variant.ToUint64());
    EXPECT_EQ(std::int64_t(3333333333), uint32Variant.ToInt64());
    EXPECT_EQ(float(3333333333), uint32Variant.ToFloat());
    EXPECT_EQ(double(3333333333), uint32Variant.ToDouble());
    EXPECT_EQ(std::string(u8"3333333333"), uint32Variant.ToString());
    EXPECT_EQ(std::wstring(L"3333333333"), uint32Variant.ToWString());
    EXPECT_EQ(std::uint32_t(3333333333), uint32Variant.ToAny().Get<std::uint32_t>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::uintptr_t(3333333333)),
      uint32Variant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Int32VariantCanBeConvertedToDifferentTypes) {
    Variant int32Variant(std::int32_t(-1111111111));

    EXPECT_EQ(false, Variant(std::int32_t(0)).ToBoolean());
    EXPECT_EQ(true, int32Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(57), int32Variant.ToUint8());
    EXPECT_EQ(std::int8_t(57), int32Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(51769), int32Variant.ToUint16());
    EXPECT_EQ(std::int16_t(-13767), int32Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(3183856185), int32Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-1111111111), int32Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(18446744072598440505ULL), int32Variant.ToUint64());
    EXPECT_EQ(std::int64_t(-1111111111), int32Variant.ToInt64());
    EXPECT_EQ(float(-1111111111), int32Variant.ToFloat());
    EXPECT_EQ(double(-1111111111), int32Variant.ToDouble());
    EXPECT_EQ(std::string(u8"-1111111111"), int32Variant.ToString());
    EXPECT_EQ(std::wstring(L"-1111111111"), int32Variant.ToWString());
    EXPECT_EQ(std::int32_t(-1111111111), int32Variant.ToAny().Get<std::int32_t>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::intptr_t(-1111111111)),
      int32Variant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Uint64VariantCanBeConvertedToDifferentTypes) {
    Variant uint64Variant(std::uint64_t(11111111111111111111ULL));

    EXPECT_EQ(false, Variant(std::uint64_t(0)).ToBoolean());
    EXPECT_EQ(true, uint64Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(199), uint64Variant.ToUint8());
    EXPECT_EQ(std::int8_t(-57), uint64Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(29127), uint64Variant.ToUint16());
    EXPECT_EQ(std::int16_t(29127), uint64Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(3047977415), uint64Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-1246989881), uint64Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(11111111111111111111ULL), uint64Variant.ToUint64());
    EXPECT_EQ(std::int64_t(-7335632962598440505LL), uint64Variant.ToInt64());
#if defined(NUCLEX_SUPPORT_WIN32) && defined(_M_X64) && defined(_MSC_VER) && (_MSC_VER < 1920) // VS2017 bug-thing
    EXPECT_EQ(float(1.11111104e+19f), uint64Variant.ToFloat());
#else
    EXPECT_EQ(float(1.11111115e+19f), uint64Variant.ToFloat());
#endif
    EXPECT_EQ(double(11111111111111111111.0), uint64Variant.ToDouble());
    EXPECT_EQ(std::string(u8"11111111111111111111"), uint64Variant.ToString());
    EXPECT_EQ(std::wstring(L"11111111111111111111"), uint64Variant.ToWString());
    EXPECT_EQ(std::uint64_t(11111111111111111111ULL), uint64Variant.ToAny().Get<std::uint64_t>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::uintptr_t(11111111111111111111ULL)),
      uint64Variant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, Int64VariantCanBeConvertedToDifferentTypes) {
    Variant int64Variant(std::int64_t(-8888888888888888888LL));

    EXPECT_EQ(false, Variant(std::int64_t(0)).ToBoolean());
    EXPECT_EQ(true, int64Variant.ToBoolean());
    EXPECT_EQ(std::uint8_t(200), int64Variant.ToUint8());
    EXPECT_EQ(std::int8_t(-56), int64Variant.ToInt8());
    EXPECT_EQ(std::uint16_t(29128), int64Variant.ToUint16());
    EXPECT_EQ(std::int16_t(29128), int64Variant.ToInt16());
    EXPECT_EQ(std::uint32_t(2715578824), int64Variant.ToUint32());
    EXPECT_EQ(std::int32_t(-1579388472), int64Variant.ToInt32());
    EXPECT_EQ(std::uint64_t(9557855184820662728ULL), int64Variant.ToUint64());
    EXPECT_EQ(std::int64_t(-8888888888888888888LL), int64Variant.ToInt64());
    EXPECT_EQ(float(-8.88888905e+18f), int64Variant.ToFloat());
    EXPECT_EQ(double(-8888888888888888888.0), int64Variant.ToDouble());
    EXPECT_EQ(std::string(u8"-8888888888888888888"), int64Variant.ToString());
    EXPECT_EQ(std::wstring(L"-8888888888888888888"), int64Variant.ToWString());
    EXPECT_EQ(std::int64_t(-8888888888888888888LL), int64Variant.ToAny().Get<std::int64_t>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::intptr_t(-8888888888888888888LL)),
      int64Variant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, FloatVariantCanBeConvertedToDifferentTypes) {
    Variant floatVariant(float(123.75f));

    EXPECT_EQ(false, Variant(float(0.0f)).ToBoolean());
    EXPECT_EQ(true, floatVariant.ToBoolean());
    EXPECT_EQ(std::uint8_t(123), floatVariant.ToUint8());
    EXPECT_EQ(std::int8_t(123), floatVariant.ToInt8());
    EXPECT_EQ(std::uint16_t(123), floatVariant.ToUint16());
    EXPECT_EQ(std::int16_t(123), floatVariant.ToInt16());
    EXPECT_EQ(std::uint32_t(123), floatVariant.ToUint32());
    EXPECT_EQ(std::int32_t(123), floatVariant.ToInt32());
    EXPECT_EQ(std::uint64_t(123ULL), floatVariant.ToUint64());
    EXPECT_EQ(std::int64_t(123LL), floatVariant.ToInt64());
    EXPECT_EQ(float(123.75f), floatVariant.ToFloat());
    EXPECT_EQ(double(123.75), floatVariant.ToDouble());
    EXPECT_EQ(std::string(u8"123.75"), floatVariant.ToString());
    EXPECT_EQ(std::wstring(L"123.75"), floatVariant.ToWString());
    EXPECT_EQ(float(123.75f), floatVariant.ToAny().Get<float>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::uintptr_t(123)),
      floatVariant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, DoubleVariantCanBeConvertedToDifferentTypes) {
    Variant doubleVariant(double(12345.84375));

    EXPECT_EQ(false, Variant(double(0.0)).ToBoolean());
    EXPECT_EQ(true, doubleVariant.ToBoolean());
    EXPECT_EQ(std::uint8_t(57), doubleVariant.ToUint8());
    EXPECT_EQ(std::int8_t(57), doubleVariant.ToInt8());
    EXPECT_EQ(std::uint16_t(12345), doubleVariant.ToUint16());
    EXPECT_EQ(std::int16_t(12345), doubleVariant.ToInt16());
    EXPECT_EQ(std::uint32_t(12345), doubleVariant.ToUint32());
    EXPECT_EQ(std::int32_t(12345), doubleVariant.ToInt32());
    EXPECT_EQ(std::uint64_t(12345ULL), doubleVariant.ToUint64());
    EXPECT_EQ(std::int64_t(12345LL), doubleVariant.ToInt64());
    EXPECT_EQ(float(12345.84375f), doubleVariant.ToFloat());
    EXPECT_EQ(double(12345.84375), doubleVariant.ToDouble());
    EXPECT_EQ(std::string(u8"12345.84375"), doubleVariant.ToString());
    EXPECT_EQ(std::wstring(L"12345.84375"), doubleVariant.ToWString());
    EXPECT_EQ(double(12345.84375), doubleVariant.ToAny().Get<double>());
    EXPECT_EQ(
      reinterpret_cast<void *>(std::uintptr_t(12345.84375)),
      doubleVariant.ToVoidPointer()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanTellIfContainsNumber) {
    EXPECT_FALSE(Variant().IsNumber());
    EXPECT_FALSE(Variant(false).IsNumber());
    EXPECT_FALSE(Variant(true).IsNumber());
    EXPECT_TRUE(Variant(std::uint8_t(234)).IsNumber());
    EXPECT_TRUE(Variant(std::int8_t(123)).IsNumber());
    EXPECT_TRUE(Variant(std::uint16_t(54321)).IsNumber());
    EXPECT_TRUE(Variant(std::int16_t(12345)).IsNumber());
    EXPECT_TRUE(Variant(std::uint32_t(987654321)).IsNumber());
    EXPECT_TRUE(Variant(std::int32_t(123456789)).IsNumber());
    EXPECT_TRUE(Variant(std::uint64_t(9876543210000ULL)).IsNumber());
    EXPECT_TRUE(Variant(std::int64_t(1234567890000LL)).IsNumber());
    EXPECT_TRUE(Variant(float(12.34f)).IsNumber());
    EXPECT_TRUE(Variant(double(1234.5678)).IsNumber());
    EXPECT_FALSE(Variant(std::string(u8"Hello World")).IsNumber());
    EXPECT_FALSE(Variant(std::wstring(L"Hello World")).IsNumber());
    EXPECT_FALSE(Variant(Any(12345)).IsNumber());
    EXPECT_FALSE(Variant(nullptr).IsNumber());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanTellIfContainsString) {
    EXPECT_FALSE(Variant().IsString());
    EXPECT_FALSE(Variant(false).IsString());
    EXPECT_FALSE(Variant(true).IsString());
    EXPECT_FALSE(Variant(std::uint8_t(234)).IsString());
    EXPECT_FALSE(Variant(std::int8_t(123)).IsString());
    EXPECT_FALSE(Variant(std::uint16_t(54321)).IsString());
    EXPECT_FALSE(Variant(std::int16_t(12345)).IsString());
    EXPECT_FALSE(Variant(std::uint32_t(987654321)).IsString());
    EXPECT_FALSE(Variant(std::int32_t(123456789)).IsString());
    EXPECT_FALSE(Variant(std::uint64_t(9876543210000ULL)).IsString());
    EXPECT_FALSE(Variant(std::int64_t(1234567890000LL)).IsString());
    EXPECT_FALSE(Variant(float(12.34f)).IsString());
    EXPECT_FALSE(Variant(double(1234.5678)).IsString());
    EXPECT_TRUE(Variant(std::string(u8"Hello World")).IsString());
    EXPECT_TRUE(Variant(std::wstring(L"Hello World")).IsString());
    EXPECT_FALSE(Variant(Any(12345)).IsString());
    EXPECT_FALSE(Variant(nullptr).IsString());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, CanTellTypeOfContainedValue) {
    EXPECT_EQ(VariantType::Empty, Variant().GetType());
    EXPECT_EQ(VariantType::Boolean, Variant(false).GetType());
    EXPECT_EQ(VariantType::Boolean, Variant(true).GetType());
    EXPECT_EQ(VariantType::Uint8, Variant(std::uint8_t(234)).GetType());
    EXPECT_EQ(VariantType::Int8, Variant(std::int8_t(123)).GetType());
    EXPECT_EQ(VariantType::Uint16, Variant(std::uint16_t(54321)).GetType());
    EXPECT_EQ(VariantType::Int16, Variant(std::int16_t(12345)).GetType());
    EXPECT_EQ(VariantType::Uint32, Variant(std::uint32_t(987654321)).GetType());
    EXPECT_EQ(VariantType::Int32, Variant(std::int32_t(123456789)).GetType());
    EXPECT_EQ(VariantType::Uint64, Variant(std::uint64_t(9876543210000ULL)).GetType());
    EXPECT_EQ(VariantType::Int64, Variant(std::int64_t(1234567890000LL)).GetType());
    EXPECT_EQ(VariantType::Float, Variant(float(12.34f)).GetType());
    EXPECT_EQ(VariantType::Double, Variant(double(1234.5678)).GetType());
    EXPECT_EQ(VariantType::String, Variant(std::string(u8"Hello World")).GetType());
    EXPECT_EQ(VariantType::WString, Variant(std::wstring(L"Hello World")).GetType());
    EXPECT_EQ(VariantType::Any, Variant(Any(12345)).GetType());
    EXPECT_EQ(VariantType::VoidPointer, Variant(nullptr).GetType());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VariantTest, SupportsMoveAssignment) {
    Variant source(std::string(u8"Hello World"));

    Variant target(123);
    target = std::move(source);

    EXPECT_EQ(target.GetType(), VariantType::String);
    EXPECT_EQ(target.ToString(), std::string(u8"Hello World"));
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
