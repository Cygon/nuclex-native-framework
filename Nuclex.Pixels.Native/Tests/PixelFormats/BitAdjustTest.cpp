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

#include "../../Source/PixelFormats/BitAdjust.h"
#include <gtest/gtest.h>

//#define NUCLEX_PIXELS_BITADJUST_DEBUGOUTPUT 1

#if defined(NUCLEX_PIXELS_BITADJUST_DEBUGOUTPUT)
#include <functional>
#include <algorithm>
#include <iostream>
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_BITADJUST_DEBUGOUTPUT)
  /// <summary>Prints the conversion results of a bit adjuster as a table</summary>
  /// <param name="fromBits">Number of bits in the original color channel</param>
  /// <param name="toBits">Target number of bits for the conversion</param>
  /// <param name="adjuster">Function that performs the adjustment</param>
  void printAsTable(
    std::size_t fromBits, std::size_t toBits, std::function<std::size_t(std::size_t)> adjuster
  ) {
    std::size_t lastOptimal = 0;
    std::size_t lastActual = 0;

    std::size_t maxFrom = (1 << fromBits);
    std::size_t maxTo = (1 << toBits);

    for(std::size_t value = 0; value < maxFrom; ++value) {
      std::size_t actual = adjuster(value);
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) /
        static_cast<double>(maxFrom - 1) *
        (
          static_cast<double>(maxTo) - (1.0 / static_cast<double>(maxTo))
        )
      );

      std::cout <<
        "From: " << value <<
        " | Actual: " << actual << " (" << (actual - lastActual) << ")" <<
        " | Optimal: " << optimal << " (" << (optimal - lastOptimal) << ")" <<
        std::endl;

      lastOptimal = optimal;
      lastActual = actual;
    }
  }
#endif // defined(NUCLEX_PIXELS_BITADJUST_DEBUGOUTPUT)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom4To5) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 15.0 * 32.0 - (1.0 / 32.0)
      );
      std::size_t actual = BitAdjuster<4, 5>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom4To6) {
    for(std::size_t value = 0; value < 16; ++value) {
      // What's this crazy formula? This rounding point most evenly distributes
      // the major steps along the numeric range. It happens to agree with the most
      // simple bit adjustment method, too.
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 15.0 * 64.0 - (1.0 / 64.0)
      );
      std::size_t actual = BitAdjuster<4, 6>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom4To8) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 15.0 * 256.0 - (1.0 / 256.0)
      );
      std::size_t actual = BitAdjuster<4, 8>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom4To10) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 15.0 * 1024.0 - (1.0 / 1024.0)
      );
      std::size_t actual = BitAdjuster<4, 10>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom4To16) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 15.0 * 65536.0 - (1.0 / 65536.0)
      );
      std::size_t actual = BitAdjuster<4, 16>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom5To4) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 32.0 * 16.0
      );
      std::size_t actual = BitAdjuster<5, 4>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom5To6) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 31.0 * 64.0 - (1.0 / 64.0)
      );
      std::size_t actual = BitAdjuster<5, 6>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom5To8) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 31.0 * 256.0 - (1.0 / 256.0)
      );
      std::size_t actual = BitAdjuster<5, 8>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom5To10) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 31.0 * 1024.0 - (1.0 / 1024.0)
      );
      std::size_t actual = BitAdjuster<5, 10>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom5To16) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 31.0 * 65536.0 - (1.0 / 65536.0)
      );
      std::size_t actual = BitAdjuster<5, 16>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom6To4) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 64.0 * 16.0
      );
      std::size_t actual = BitAdjuster<6, 4>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom6To5) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 64.0 * 32.0
      );
      std::size_t actual = BitAdjuster<6, 5>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom6To8) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 63.0 * 256.0 - (1.0 / 256.0)
      );
      std::size_t actual = BitAdjuster<6, 8>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom6To10) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 63.0 * 1024.0 - (1.0 / 1024.0)
      );
      std::size_t actual = BitAdjuster<6, 10>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom6To16) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 63.0 * 65536.0 - (1.0 / 65536.0)
      );
      std::size_t actual = BitAdjuster<6, 16>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom8To4) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 256.0 * 16.0
      );
      std::size_t actual = BitAdjuster<8, 4>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom8To5) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 256.0 * 32.0
      );
      std::size_t actual = BitAdjuster<8, 5>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom8To6) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 256.0 * 64.0
      );
      std::size_t actual = BitAdjuster<8, 6>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom8To10) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 255.0 * 1024.0 - (1.0 / 1024.0)
      );
      std::size_t actual = BitAdjuster<8, 10>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom8To16) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 255.0 * 65536.0 - (1.0 / 65536.0)
      );
      std::size_t actual = BitAdjuster<8, 16>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom10To4) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 1024.0 * 16.0
      );
      std::size_t actual = BitAdjuster<10, 4>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom10To5) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 1024.0 * 32.0
      );
      std::size_t actual = BitAdjuster<10, 5>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom10To6) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 1024.0 * 64.0
      );
      std::size_t actual = BitAdjuster<10, 6>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom10To8) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 1024.0 * 256.0
      );
      std::size_t actual = BitAdjuster<10, 8>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeWidenedFrom10To16) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 1023.0 * 65536.0 - (1.0 / 65536.0)
      );
      std::size_t actual = BitAdjuster<10, 16>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom16To4) {
    for(std::size_t value = 0; value < 65536; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 65536.0 * 16.0
      );
      std::size_t actual = BitAdjuster<16, 4>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom16To5) {
    for(std::size_t value = 0; value < 65536; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 65536.0 * 32.0
      );
      std::size_t actual = BitAdjuster<16, 5>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom16To6) {
    for(std::size_t value = 0; value < 65536; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 65536.0 * 64.0
      );
      std::size_t actual = BitAdjuster<16, 6>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom16To8) {
    for(std::size_t value = 0; value < 65536; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 65536.0 * 256.0
      );
      std::size_t actual = BitAdjuster<16, 8>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeNarrowedFrom16To10) {
    for(std::size_t value = 0; value < 65536; ++value) {
      std::size_t optimal = static_cast<std::size_t>(
        static_cast<double>(value) / 65536.0 * 1024.0
      );
      std::size_t actual = BitAdjuster<16, 10>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitAdjustTest, BitsCanBeKeptTheSame) {
    for(std::size_t value = 0; value < 512; ++value) {
      std::size_t optimal = value;
      std::size_t actual = BitAdjuster<9, 9>::Adjust(value);

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
