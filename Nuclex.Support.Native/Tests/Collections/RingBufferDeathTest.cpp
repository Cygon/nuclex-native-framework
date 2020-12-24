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

#include "Nuclex/Support/Collections/RingBuffer.h"
#include <gtest/gtest.h>

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(RingBufferDeathTest, DequeuingFromEmptyBufferTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    ASSERT_DEATH(
      test.Read(items, 1),
      u8""
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(RingBufferDeathTest, DequeuingTooManyItemsTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[100];
    test.Write(items, 99);

    ASSERT_DEATH(
      test.Read(items, 100),
      u8""
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(RingBufferDeathTest, DequeuingTooManyItemsInWrappedBufferTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    std::vector<std::uint8_t> retrieved(capacity);

    std::size_t oneThirdCapacity = capacity / 3;
    test.Write(&items[0], oneThirdCapacity * 2);
    test.Read(&retrieved[0], oneThirdCapacity);
    test.Write(&items[0], oneThirdCapacity * 2);
    test.Read(&retrieved[0], oneThirdCapacity);

    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    ASSERT_DEATH(
      test.Read(&items[0], oneThirdCapacity * 2 + 1),
      u8""
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
