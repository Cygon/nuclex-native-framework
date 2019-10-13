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

#if !defined(NDEBUG) // assert() is documented to toggle in response to this constant

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

  TEST(PixelIteratorDeathTest, SinglePixelRetreatBeforeBeginningTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);
    ASSERT_DEATH(
      --accessor,
      ".*Iterator must be at a position that has a preceding position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, SinglePixelAdvancePastEndTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    bitmapMemory.Width = 1;
    bitmapMemory.Height = 1;

    PixelIterator accessor(bitmapMemory);
    ++accessor; // Moves to the special end position any iterator must allow
    ASSERT_DEATH(
      ++accessor,
      ".*Iterator must be at the end position or have a succeeding position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, MultiPixelRetreatBeforeBeginningTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();

    PixelIterator accessor(bitmapMemory);
    accessor += 15 * 100 + 50;
    ASSERT_DEATH(
      accessor -= 15 * 100 + 51,
      ".*Cannot move iterator before the first pixel.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, MultiPixelAdvancePastEndTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();

    PixelIterator accessor(bitmapMemory);
    accessor += 90 * 100 + 50;
    ASSERT_DEATH(
      accessor += 10 * 100 + 25,
      ".*Cannot move iterator beyond the end position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, MovingBeyondEndTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor(bitmapMemory);

    ASSERT_DEATH(
      accessor.MoveTo(102, 100),
      ".*Cannot move iterator beyond the end position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, DereferencingEndIteratorTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor = PixelIterator::GetEnd(bitmapMemory);

    ASSERT_DEATH(
      *accessor,
      ".*Iterator must lie on a valid position and before the end position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelIteratorDeathTest, QueryingPositionOfEndIteratorTriggersAssertion) {
    BitmapMemory bitmapMemory = makeDummyBitmapMemory();
    PixelIterator accessor = PixelIterator::GetEnd(bitmapMemory);

    ASSERT_DEATH(
      *accessor,
      ".*Iterator must lie on a valid position and before the end position.*"
    );
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // !defined(NDEBUG)
