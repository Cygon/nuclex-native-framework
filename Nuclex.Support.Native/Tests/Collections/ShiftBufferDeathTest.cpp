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

#include "Nuclex/Support/Collections/ShiftBuffer.h"
#include <gtest/gtest.h>

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(ShiftBufferDeathTest, SkippingOnEmptyBufferTriggersAssertion) {
    ShiftBuffer<std::uint8_t> test;

    ASSERT_DEATH(
      test.Skip(1),
      ".*Amount of data skipped must be less or equal to the amount of data in the buffer.*"
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(ShiftBufferDeathTest, ReadingFromEmptyBufferTriggersAssertion) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t retrieved[1];

    ASSERT_DEATH(
      test.Read(retrieved, 1),
      ".*Amount of data read must be less or equal to the amount of data in the buffer.*"
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
