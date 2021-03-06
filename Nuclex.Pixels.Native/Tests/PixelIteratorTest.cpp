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

#include "Nuclex/Pixels/PixelIterator.h"
#include <gtest/gtest.h>

#include <cstddef>

namespace {

  /// <summary>Creates a dummy BitmapMemory instance</summary>
  /// <returns>The dummy BitmapMemory instance</returns>
  Nuclex::Pixels::BitmapMemory makeDummyBitmapMemory() {
    Nuclex::Pixels::BitmapMemory bitmapMemory;

    bitmapMemory.Width = 100;
    bitmapMemory.Height = 100;
    bitmapMemory.Stride = 1000;
    bitmapMemory.PixelFormat = Nuclex::Pixels::PixelFormat::R8_G8_B8_A8_Unsigned;
    bitmapMemory.Pixels = reinterpret_cast<void *>(10000);

    return bitmapMemory;
  }

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceBySinglePixel) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    EXPECT_EQ(reinterpret_cast<void *>(10000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    ++accessor;
    EXPECT_EQ(reinterpret_cast<void *>(10004), accessor.GetAddress());
    EXPECT_EQ(1U, accessor.GetX());
    ++accessor;
    EXPECT_EQ(reinterpret_cast<void *>(10008), accessor.GetAddress());
    EXPECT_EQ(2U, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatBySinglePixel) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    ++accessor;
    ++accessor;

    EXPECT_EQ(reinterpret_cast<void *>(10008), accessor.GetAddress());
    EXPECT_EQ(2U, accessor.GetX());
    --accessor;
    EXPECT_EQ(reinterpret_cast<void *>(10004), accessor.GetAddress());
    EXPECT_EQ(1U, accessor.GetX());
    --accessor;
    EXPECT_EQ(reinterpret_cast<void *>(10000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, SinglePixelAdvanceCanMoveIntoNextLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    for(std::size_t index = 0; index < 99; ++index) {
      ++accessor;
    }

    // 100 pixels = 400 bytes
    EXPECT_EQ(reinterpret_cast<void *>(10396), accessor.GetAddress());
    EXPECT_EQ(99U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());

    ++accessor;

    // 1000 bytes stride = start of second line at 1000 bytes
    EXPECT_EQ(reinterpret_cast<void *>(11000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(1U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, SinglePixelRetreatCanMoveIntoPreviousLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    for(std::size_t index = 0; index < 100; ++index) {
      ++accessor;
    }

    EXPECT_EQ(reinterpret_cast<void *>(11000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(1U, accessor.GetY());

    --accessor;

    EXPECT_EQ(reinterpret_cast<void *>(10396), accessor.GetAddress());
    EXPECT_EQ(99U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixels) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;

    EXPECT_EQ(reinterpret_cast<void *>(10040), accessor.GetAddress());
    EXPECT_EQ(10U, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  // Check for off-by-one errors
  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsToRightBorder) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 89;

    EXPECT_EQ(reinterpret_cast<void *>(10396), accessor.GetAddress());
    EXPECT_EQ(99U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsToNextLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 90;

    EXPECT_EQ(reinterpret_cast<void *>(11000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(1U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsAndMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 220;

    EXPECT_EQ(reinterpret_cast<void *>(12120), accessor.GetAddress());
    EXPECT_EQ(30U, accessor.GetX());
    EXPECT_EQ(2U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsToEnd) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 4500;
    accessor += 5500;

    EXPECT_EQ(accessor, PixelIterator::GetEnd(bitmapMemory));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixels) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 50;
    accessor -= 20;

    EXPECT_EQ(reinterpret_cast<void *>(10120), accessor.GetAddress());
    EXPECT_EQ(30U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToPreviousLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 250;
    accessor -= 80;

    EXPECT_EQ(reinterpret_cast<void *>(11280), accessor.GetAddress());
    EXPECT_EQ(70U, accessor.GetX());
    EXPECT_EQ(1U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsAndMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 1520;
    accessor -= 740;

    EXPECT_EQ(reinterpret_cast<void *>(17320), accessor.GetAddress());
    EXPECT_EQ(80U, accessor.GetX());
    EXPECT_EQ(7U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToLeftBorder) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 430;
    accessor -= 30;

    EXPECT_EQ(reinterpret_cast<void *>(14000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(4U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToBeginning) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 4321;
    accessor -= 4321;

    EXPECT_EQ(reinterpret_cast<void *>(10000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanMoveToArbitraryPosition) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor.MoveTo(12, 34);

    // y(34) * stride(1000) + ( x(12) * bytesPerPixel(4) ) = 34048
    EXPECT_EQ(reinterpret_cast<void *>(44048), accessor.GetAddress());
    EXPECT_EQ(12U, accessor.GetX());
    EXPECT_EQ(34U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanMoveToBeginning) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor.MoveTo(0, 0);

    EXPECT_EQ(reinterpret_cast<void *>(10000), accessor.GetAddress());
    EXPECT_EQ(0U, accessor.GetX());
    EXPECT_EQ(0U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanMoveToEnd) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor.MoveTo(100, 100);

    EXPECT_EQ(accessor, PixelIterator::GetEnd(bitmapMemory));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatSingleLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 950;
    accessor -= Lines(1);

    EXPECT_EQ(reinterpret_cast<void *>(18200), accessor.GetAddress());
    EXPECT_EQ(50U, accessor.GetX());
    EXPECT_EQ(8U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceSingleLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += Lines(1);

    EXPECT_EQ(reinterpret_cast<void *>(11040), accessor.GetAddress());
    EXPECT_EQ(10U, accessor.GetX());
    EXPECT_EQ(1U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 820;
    accessor -= Lines(3);

    EXPECT_EQ(reinterpret_cast<void *>(15080), accessor.GetAddress());
    EXPECT_EQ(20U, accessor.GetX());
    EXPECT_EQ(5U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 470;
    accessor += Lines(9);

    EXPECT_EQ(reinterpret_cast<void *>(23280), accessor.GetAddress());
    EXPECT_EQ(70U, accessor.GetX());
    EXPECT_EQ(13U, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
