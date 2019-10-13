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
    bitmapMemory.Pixels = nullptr;

    return bitmapMemory;
  }

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceBySinglePixel) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    EXPECT_EQ((void *)0, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    ++accessor;
    EXPECT_EQ((void *)4, accessor.GetAddress());
    EXPECT_EQ(1, accessor.GetX());
    ++accessor;
    EXPECT_EQ((void *)8, accessor.GetAddress());
    EXPECT_EQ(2, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatBySinglePixel) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    ++accessor;
    ++accessor;

    EXPECT_EQ((void *)8, accessor.GetAddress());
    EXPECT_EQ(2, accessor.GetX());
    --accessor;
    EXPECT_EQ((void *)4, accessor.GetAddress());
    EXPECT_EQ(1, accessor.GetX());
    --accessor;
    EXPECT_EQ((void *)0, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, SinglePixelAdvanceCanMoveIntoNextLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    for(std::size_t index = 0; index < 99; ++index) {
      ++accessor;
    }

    // 100 pixels = 400 bytes
    EXPECT_EQ((void *)396, accessor.GetAddress());
    EXPECT_EQ(99, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());

    ++accessor;

    // 1000 bytes stride = start of second line at 1000 bytes
    EXPECT_EQ((void *)1000, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(1, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, SinglePixelRetreatCanMoveIntoPreviousLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    for(std::size_t index = 0; index < 100; ++index) {
      ++accessor;
    }

    EXPECT_EQ((void *)1000, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(1, accessor.GetY());

    --accessor;

    EXPECT_EQ((void *)396, accessor.GetAddress());
    EXPECT_EQ(99, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixels) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;

    EXPECT_EQ((void *)40, accessor.GetAddress());
    EXPECT_EQ(10, accessor.GetX());
  }

  // ------------------------------------------------------------------------------------------- //

  // Check for off-by-one errors
  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsToRightBorder) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 89;

    EXPECT_EQ((void *)396, accessor.GetAddress());
    EXPECT_EQ(99, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsToNextLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 90;

    EXPECT_EQ((void *)1000, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(1, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultiplePixelsAndMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += 220;

    EXPECT_EQ((void *)2120, accessor.GetAddress());
    EXPECT_EQ(30, accessor.GetX());
    EXPECT_EQ(2, accessor.GetY());
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

    EXPECT_EQ((void *)120, accessor.GetAddress());
    EXPECT_EQ(30, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToPreviousLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 250;
    accessor -= 80;

    EXPECT_EQ((void *)1280, accessor.GetAddress());
    EXPECT_EQ(70, accessor.GetX());
    EXPECT_EQ(1, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsAndMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 1520;
    accessor -= 740;

    EXPECT_EQ((void *)7320, accessor.GetAddress());
    EXPECT_EQ(80, accessor.GetX());
    EXPECT_EQ(7, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToLeftBorder) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 430;
    accessor -= 30;

    EXPECT_EQ((void *)4000, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(4, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultiplePixelsToBeginning) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 4321;
    accessor -= 4321;

    EXPECT_EQ((void *)0, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanMoveToArbitraryPosition) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor.MoveTo(12, 34);

    // y(34) * stride(1000) + ( x(12) * bytesPerPixel(4) ) = 34048
    EXPECT_EQ((void *)34048, accessor.GetAddress());
    EXPECT_EQ(12, accessor.GetX());
    EXPECT_EQ(34, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanMoveToBeginning) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor.MoveTo(0, 0);

    EXPECT_EQ((void *)0, accessor.GetAddress());
    EXPECT_EQ(0, accessor.GetX());
    EXPECT_EQ(0, accessor.GetY());
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

    EXPECT_EQ((void *)8200, accessor.GetAddress());
    EXPECT_EQ(50, accessor.GetX());
    EXPECT_EQ(8, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceSingleLine) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 10;
    accessor += Lines(1);

    EXPECT_EQ((void *)1040, accessor.GetAddress());
    EXPECT_EQ(10, accessor.GetX());
    EXPECT_EQ(1, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanRetreatMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 820;
    accessor -= Lines(3);

    EXPECT_EQ((void *)5080, accessor.GetAddress());
    EXPECT_EQ(20, accessor.GetX());
    EXPECT_EQ(5, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorTest, CanAdvanceMultipleLines) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    accessor += 470;
    accessor += Lines(9);

    EXPECT_EQ((void *)13280, accessor.GetAddress());
    EXPECT_EQ(70, accessor.GetX());
    EXPECT_EQ(13, accessor.GetY());
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
