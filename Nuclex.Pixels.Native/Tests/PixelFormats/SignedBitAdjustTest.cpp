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

#include "../../Source/PixelFormats/SignedBitAdjust.h"
#include <gtest/gtest.h>

#include <type_traits>
#include <cmath>

#define NUCLEX_PIXELS_SIGNEDBITADJUST_DEBUGOUTPUT 1

#if defined(NUCLEX_PIXELS_SIGNEDBITADJUST_DEBUGOUTPUT)
#include <functional>
#include <algorithm>
#include <iostream>
#include <bitset>
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an n-bit signed integer into a floating point value</summary>
  /// <typeparam name="TLowestBitIndex">Index of the integer's lowest bit</typeparam>
  /// <typeparam name="TBitCount">Number of bits the integer is long</typeparam>
  /// <typeparam name="TPixel">Type that is holding the n-bit integer</typeparam>
  /// <param name="pixel">Value containing the n-bit integer to convert</param>
  /// <returns>The normalized floating point value of the n-bit integer</returns>
  template<std::size_t TLowestBitIndex, std::size_t TBitCount, typename TPixel>
  double doubleFromBits(TPixel pixel) {
    bool isNegative = pixel & (1 << (TLowestBitIndex + TBitCount - 1));
    if(isNegative) {
      pixel = Nuclex::Pixels::PixelFormats::BitShift<TLowestBitIndex>(pixel);

      std::uint64_t unsignedPixel = (
        static_cast<std::uint64_t>(pixel) |
        Nuclex::Pixels::PixelFormats::BitMask<std::uint64_t, TBitCount, 64 - TBitCount>
      );
      std::int64_t signedPixel = (
        *reinterpret_cast<const std::int64_t *>(&unsignedPixel)
      );

      double pixelAsDouble = static_cast<double>(signedPixel);
      double highestValue = static_cast<double>(
        Nuclex::Pixels::PixelFormats::BitMask<TPixel, 0, TBitCount - 1>
      );
      return std::max(-1.0, pixelAsDouble / highestValue);
    } else {
      pixel = Nuclex::Pixels::PixelFormats::BitShift<TLowestBitIndex>(pixel);
      double pixelAsDouble = static_cast<double>(pixel);
      double highestValue = static_cast<double>(
        Nuclex::Pixels::PixelFormats::BitMask<TPixel, 0, TBitCount - 1>
      );
      return pixelAsDouble / highestValue;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into an n-bit signed integer</summary>
  /// <typeparam name="TLowestBitIndex">Index of the integer's lowest bit</typeparam>
  /// <typeparam name="TBitCount">Number of bits the integer will be long</typeparam>
  /// <typeparam name="TPixel">Type the n-bit integer will be returned in</typeparam>
  /// <param name="value">Normalized floating point value that will be converted</param>
  /// <returns>An integer value in which the n-bit signed integer is contained</returns>
  template<std::size_t TLowestBitIndex, std::size_t TBitCount, typename TPixel>
  TPixel bitsFromDouble(double value) {
    double highestValue = static_cast<double>(
      Nuclex::Pixels::PixelFormats::BitMask<TPixel, 0, TBitCount - 1>
    );

    double pixelAsDouble = (
      std::abs(value) *
      (
          static_cast<double>(1 << (TBitCount - 1)) -
          (1.0 / static_cast<double>(1 << (TBitCount - 1)))
      )
    );

    if(value >= 0.0)  {
      //double pixelAsDouble = (std::abs(value) * highestValue) + 0.5;
      TPixel pixel = static_cast<TPixel>(pixelAsDouble);
      return Nuclex::Pixels::PixelFormats::BitShift<-TLowestBitIndex>(pixel);
    } else { // The value is negative and did *not* truncate to zero
      //double pixelAsDouble = (std::abs(value) * (highestValue)) + 0.5;
      TPixel pixel = static_cast<TPixel>(pixelAsDouble);
      pixel = ~pixel + 1;
      pixel &= Nuclex::Pixels::PixelFormats::BitMask<std::size_t, 0, TBitCount>;
      return Nuclex::Pixels::PixelFormats::BitShift<-TLowestBitIndex>(pixel);
    }
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_SIGNEDBITADJUST_DEBUGOUTPUT)
  /// <summary>Prints the conversion results of a bit adjuster as a table</summary>
  /// <param name="fromBits">Number of bits in the original color channel</param>
  /// <param name="toBits">Target number of bits for the conversion</param>
  /// <param name="adjuster">Function that performs the adjustment</param>
  template<std::size_t TFromBits, std::size_t TToBits>
  void printAsTable(std::function<std::size_t(std::size_t)> adjuster) {
    std::int64_t lastActual, lastOptimal;

    std::size_t maxFrom = (1 << TFromBits);
    //std::size_t maxTo = (1 << TToBits);

    std::vector<std::size_t> actualIntervals, optimalIntervals;

    for(std::size_t _value = 0; _value < maxFrom; ++_value) {
      std::size_t value;
      if(_value < (1 << (TFromBits - 1))) {
        value = _value + (1 << (TFromBits - 1));
      } else {
        value = _value - (1 << (TFromBits - 1));
      }

      std::size_t actual = adjuster(value);
      actual &= Nuclex::Pixels::PixelFormats::BitMask<std::size_t, 0, TToBits>;
      std::size_t optimal = bitsFromDouble<0, TToBits, std::size_t>(
        doubleFromBits<0, TFromBits>(value)
      );

      std::int64_t signedValue, signedActual, signedOptimal;
      {
        std::uint64_t temp = static_cast<std::uint64_t>(value);
        if((temp & (1 << (TFromBits - 1))) != 0) {
          temp |= Nuclex::Pixels::PixelFormats::BitMask<std::uint64_t, TFromBits, 64 - TFromBits>;
        }
        signedValue = *reinterpret_cast<const std::int64_t *>(&temp);

        temp = static_cast<std::uint64_t>(actual);
        if((temp & (1 << (TToBits - 1))) != 0) {
          temp |= Nuclex::Pixels::PixelFormats::BitMask<std::uint64_t, TToBits, 64 - TToBits>;
        }
        signedActual = *reinterpret_cast<const std::int64_t *>(&temp);

        temp = static_cast<std::uint64_t>(optimal);
        if((temp & (1 << (TToBits - 1))) != 0) {
          temp |= Nuclex::Pixels::PixelFormats::BitMask<std::uint64_t, TToBits, 64 - TToBits>;
        }
        signedOptimal = *reinterpret_cast<const std::int64_t *>(&temp);
      }

      if(_value == 0) {
        lastOptimal = signedOptimal;
        lastActual = signedActual;
      }

      if(optimal != lastOptimal) {}

      if((lastOptimal != lastActual) || (actual != optimal) || (value == (1 << (TFromBits - 1)))) {
        std::cout <<
          "From: " << signedValue << " (" << std::bitset<TFromBits>(value) << ")" <<
          " | Actual: " << signedActual << " ("  << std::bitset<TToBits>(actual) << ")" <<
          " +" << (signedActual - lastActual) <<
          " | Optimal " << signedOptimal << " (" << std::bitset<TToBits>(optimal) << ")" <<
          " +" << (signedOptimal - lastOptimal) <<
          std::endl;
      }

      lastOptimal = signedOptimal;
      lastActual = signedActual;
    }
  }
#endif // defined(NUCLEX_PIXELS_SIGNEDBITADJUST_DEBUGOUTPUT)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, SignedBitsToDoubleConversionWorks) {
    const float MaximumError = 0.000001f;

    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(0)), 0.0);

    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(1)), 1.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(2)), 2.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(3)), 3.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(4)), 4.0 / 7.0);
    EXPECT_NEAR((doubleFromBits<0, 4, std::size_t>(5)), 5.0 / 7.0, MaximumError);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(6)), 6.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(7)), 7.0 / 7.0);

    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(15)), -1.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(14)), -2.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(13)), -3.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(12)), -4.0 / 7.0);
    EXPECT_NEAR((doubleFromBits<0, 4, std::size_t>(11)), -5.0 / 7.0, MaximumError);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(10)), -6.0 / 7.0);
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(9)), -7.0 / 7.0);

    // GPU signed formats are symmetrical, so unlike CPU integer math,
    // negative reach doesn't go one further than positive and is clamped.
    EXPECT_EQ((doubleFromBits<0, 4, std::size_t>(8)), -7.0 / 7.0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, DoubleToSignedBitsConversionWorks) {
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.000)), 0b0000U);

    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.143)), 0b0001U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.286)), 0b0010U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.429)), 0b0011U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.571)), 0b0100U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.714)), 0b0101U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(0.857)), 0b0110U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(1.000)), 0b0111U);

    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.143)), 0b1111U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.286)), 0b1110U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.429)), 0b1101U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.571)), 0b1100U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.714)), 0b1011U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-0.857)), 0b1010U);
    EXPECT_EQ((bitsFromDouble<0, 4, std::size_t>(-1.000)), 0b1001U);

    // 0b1000 cannot be reached because GPU signed integers are symmetrical.
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_PIXELS_SIGNEDBITADJUST_DEBUGOUTPUT)
  TEST(SignedBitAdjustTest, DebugPrintResultTable) {
    printAsTable<10, 4>(
      [](std::size_t value) {
        return SignedBitAdjuster<10, 4>::Adjust<0, 0>(value);
      }
    );

    /* Problem case, quantization invervals don't match up with double precision math
    std::cout << std::string(78, u8'-') << std::endl;
    printAsTable<6, 4>(
      [](std::size_t value) {
        return SignedBitAdjuster<6, 4>::Adjust<0, 0>(value);
      }
    );
    */
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom4To5) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = bitsFromDouble<0, 5, std::size_t>(doubleFromBits<0, 4>(value));
      std::size_t actual = SignedBitAdjuster<4, 5>::Adjust<0, 0>(value);

      if(value == 8) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom4To6) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = bitsFromDouble<0, 6, std::size_t>(doubleFromBits<0, 4>(value));
      std::size_t actual = SignedBitAdjuster<4, 6>::Adjust<0, 0>(value);

      if(value == 8) { continue; } // may return both -31 or -32, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom4To8) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = bitsFromDouble<0, 8, std::size_t>(doubleFromBits<0, 4>(value));
      std::size_t actual = SignedBitAdjuster<4, 8>::Adjust<0, 0>(value);

      if(value == 8) { continue; } // may return both -127 or -128, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom4To10) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = bitsFromDouble<0, 10, std::size_t>(doubleFromBits<0, 4>(value));
      std::size_t actual = SignedBitAdjuster<4, 10>::Adjust<0, 0>(value);

      if(value == 8) { continue; } // may return both -511 or -512, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom4To16) {
    for(std::size_t value = 0; value < 16; ++value) {
      std::size_t optimal = bitsFromDouble<0, 16, std::size_t>(doubleFromBits<0, 4>(value));
      std::size_t actual = SignedBitAdjuster<4, 16>::Adjust<0, 0>(value);

      if(value == 8) { continue; } // may return both -32767 or -32768, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom5To4) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = bitsFromDouble<0, 4, std::size_t>(doubleFromBits<0, 5>(value));
      std::size_t actual = SignedBitAdjuster<5, 4>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 4>;

      if(value == 16) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom5To6) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = bitsFromDouble<0, 6, std::size_t>(doubleFromBits<0, 5>(value));
      std::size_t actual = SignedBitAdjuster<5, 6>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 6>;

      if(value == 16) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom5To8) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = bitsFromDouble<0, 8, std::size_t>(doubleFromBits<0, 5>(value));
      std::size_t actual = SignedBitAdjuster<5, 8>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 8>;

      if(value == 16) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom5To10) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = bitsFromDouble<0, 10, std::size_t>(doubleFromBits<0, 5>(value));
      std::size_t actual = SignedBitAdjuster<5, 10>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 8>;

      if(value == 16) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom5To16) {
    for(std::size_t value = 0; value < 32; ++value) {
      std::size_t optimal = bitsFromDouble<0, 16, std::size_t>(doubleFromBits<0, 5>(value));
      std::size_t actual = SignedBitAdjuster<5, 16>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 8>;

      if(value == 16) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom6To4) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = bitsFromDouble<0, 4, std::size_t>(doubleFromBits<0, 6>(value));
      std::size_t actual = SignedBitAdjuster<6, 4>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 4>;

      if(value == 32) { continue; } // may return both -7 or -8, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom6To5) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = bitsFromDouble<0, 5, std::size_t>(doubleFromBits<0, 6>(value));
      std::size_t actual = SignedBitAdjuster<6, 5>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 5>;

      if(value == 32) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom6To8) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = bitsFromDouble<0, 8, std::size_t>(doubleFromBits<0, 6>(value));
      std::size_t actual = SignedBitAdjuster<6, 8>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 5>;

      if(value == 32) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom6To10) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = bitsFromDouble<0, 10, std::size_t>(doubleFromBits<0, 6>(value));
      std::size_t actual = SignedBitAdjuster<6, 10>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 5>;

      if(value == 32) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom6To16) {
    for(std::size_t value = 0; value < 64; ++value) {
      std::size_t optimal = bitsFromDouble<0, 16, std::size_t>(doubleFromBits<0, 6>(value));
      std::size_t actual = SignedBitAdjuster<6, 16>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 5>;

      if(value == 32) { continue; } // may return both -15 or -16, so we don't test it

      //EXPECT_NEAR(actual, optimal, 1);
      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom8To4) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = bitsFromDouble<0, 4, std::size_t>(doubleFromBits<0, 8>(value));
      std::size_t actual = SignedBitAdjuster<8, 4>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 4>;

      if(value == 128) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom8To5) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = bitsFromDouble<0, 5, std::size_t>(doubleFromBits<0, 8>(value));
      std::size_t actual = SignedBitAdjuster<8, 5>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 5>;

      if(value == 128) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom8To6) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = bitsFromDouble<0, 6, std::size_t>(doubleFromBits<0, 8>(value));
      std::size_t actual = SignedBitAdjuster<8, 6>::Adjust<0, 0>(value);
      actual &= BitMask<std::size_t, 0, 6>;

      if(value == 128) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom8To10) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = bitsFromDouble<0, 10, std::size_t>(doubleFromBits<0, 8>(value));
      std::size_t actual = SignedBitAdjuster<8, 10>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 4>;

      if(value == 128) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeWidenedFrom8To16) {
    for(std::size_t value = 0; value < 256; ++value) {
      std::size_t optimal = bitsFromDouble<0, 16, std::size_t>(doubleFromBits<0, 8>(value));
      std::size_t actual = SignedBitAdjuster<8, 16>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 4>;

      if(value == 128) { continue; } // may return both -15 or -16, so we don't test it

      EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SignedBitAdjustTest, BitsCanBeNarrowedFrom10To4) {
    for(std::size_t value = 0; value < 1024; ++value) {
      std::size_t optimal = bitsFromDouble<0, 4, std::size_t>(doubleFromBits<0, 10>(value));
      std::size_t actual = SignedBitAdjuster<10, 4>::Adjust<0, 0>(value);
      //actual &= BitMask<std::size_t, 0, 4>;

      if(value == 512) { continue; } // may return both -15 or -16, so we don't test it

      //EXPECT_EQ(actual, optimal);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
