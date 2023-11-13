#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "./NumberFormatter.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307) // Integral constant overflow
#pragma warning(disable: 4702) // Unreachable code
#endif
#include "./DragonBox-1.1.2/dragonbox.h" // for the float-to-decimal algorithm
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "Nuclex/Support/BitTricks.h" // for BitTricks::GetLogBase10()

// Brings the next two digits of the prepeared number into the upper 32 bits
// so they can be extracted by the WRITE_ONE_DIGIT and WRITE_TWO_DIGITS macros
#define READY_NEXT_TWO_DIGITS() \
  temp = std::uint64_t(100) * static_cast<std::uint32_t>(temp)

// Appends the next two highest digits in the prepared number to the char buffer
// Also adjusts the number such that the next two digits are ready for extraction.
#define WRITE_TWO_DIGITS(bufferPointer) \
  *reinterpret_cast<TwoChars *>(bufferPointer) = ( \
    *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[(temp >> 31) & 0xFE]) \
  )

// Appends the next highest digit in the prepared number to the char buffer
// Thus doesn't adjust the number because it is always used on the very last digit.
#define WRITE_ONE_DIGIT(bufferPointer) \
  *reinterpret_cast<char *>(bufferPointer) = ( \
    u8'0' + static_cast<char>(std::uint64_t(10) * std::uint32_t(temp) >> 32) \
  )

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Structure with the size of two chars</summary>
  /// <remarks>
  ///   This is only used to assign two characters at once. Benchmarks (in release mode on
  ///   AMD64 with -O3 on GCC 11) revealed that std::memcpy() is not inlined/intrinsic'd as
  ///   much as one would hope and that this method resulted in faster code.
  /// </remarks>
  struct TwoChars { char t, o; };

  // ------------------------------------------------------------------------------------------- //

  // Table of jeaiii values up to 1e+14.
  //
  //  Magnitude |           Factor            | Shift  | Bias
  // ---------------------------------------------------------
  //     1e0    |              4'294'967'297  |    0   |  0
  //     1e1    |                429'496'730  |    0   |  0
  //     1e2    |                 42'949'673  |    0   |  0
  //     1e3    |                  4'294'968  |    0   |  0
  //     1e4    |                    429'497  |    0   |  0
  //     1e5    |              2'814'749'768  |   16   |  0
  //     1e6    |              2'251'799'815  |   19   |  4
  //     1e7    |              3'602'879'703  |   23   |  4
  //     1e8    |              2'882'303'762  |   26   |  4
  //     1e9    |              2'305'843'010  |   29   |  4
  //     1e10   |             17'179'869'189  |   66   |  4
  //     1e11   |          1'099'511'628'033  |   72   |  4
  //     1e12   |        140'737'488'388'098  |   79   |  8
  //     1e13   |     18'014'398'513'676'290  |   86   |  8
  //     1e14   |  1'152'921'504'875'282'434  |   92   |  8

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Factors the jeaiii algorithm uses to prepare a number for printing</summary>
  const std::uint64_t factors[] = {
                0, // magnitude 1e-1 (invalid)
    4'294'967'297, // magnitude 1e0
      429'496'730, // magnitude 1e1
       42'949'673, // magnitude 1e2
        4'294'968, // magnitude 1e3
          429'497, // magnitude 1e4
    2'814'749'768, // magnitude 1e5
    2'251'799'815, // magnitude 1e6
    3'602'879'703, // magnitude 1e7
    2'882'303'762, // magnitude 1d8
    2'305'843'010, // magnitude 1e9
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit shifts the jeaiii algorithm uses to prepare a number for printing</summary>
  const int shift[] = {
     0, // magnitude 1e-1 (invalid)
     0, // magnitude 1e0
     0, // magnitude 1e1
     0, // magnitude 1e2
     0, // magnitude 1e3
     0, // magnitude 1e4
    16, // magnitude 1e5
    19, // magnitude 1e6
    23, // magnitude 1e7
    26, // magnitude 1e8
    29, // magnitude 1e9
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bias added to numbers by jeaiii algorithm</summary>
  const std::uint32_t bias[] = {
    0, // magnitude 1e-1 (invalid)
    0, // magnitude 1e0
    0, // magnitude 1e1
    0, // magnitude 1e2
    0, // magnitude 1e3
    0, // magnitude 1e4
    0, // magnitude 1e5
    4, // magnitude 1e6
    4, // magnitude 1e7
    4, // magnitude 1e8
    4, // magnitude 1e9
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Formats an integral number without adding a decimal point</summary>
  /// <param name="buffer">Buffer into which the number will be written</param>
  /// <param name="temp">The integer that will be written to the buffer</param>
  /// <param name="magnitude">Magnitude of the number (digit count minus 1)</param>
  /// <returns>A pointer one past the last written character in the buffer</returns>
  char *formatInteger32(char *buffer /* [10] */, std::uint64_t temp, std::size_t magnitude) {
    temp *= factors[magnitude];
    temp >>= shift[magnitude];
    temp += bias[magnitude];

    // If it's just one digit, skip the two-digit-pull loop and just
    // output that lone digit
    if(magnitude == 0) {
      WRITE_ONE_DIGIT(buffer);
      return buffer + 1;
    }

    // If there are at least two digits, turn them into text in pairs until
    // less than two are left.
    for(;;) {
      WRITE_TWO_DIGITS(buffer);
      if(magnitude < 3) { // Are less than 2 remaining?
        if(magnitude >= 2) { // is even 1 remaining?
          WRITE_ONE_DIGIT(buffer + 2);
          return buffer + 3;
        } else {
          return buffer + 2;
        }
      }
      READY_NEXT_TWO_DIGITS();
      magnitude -= 2;
      buffer += 2;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Formats an integral number but adds a decimal point between two digits</summary>
  /// <param name="buffer">Buffer into which the number will be written</param>
  /// <param name="temp">Significand, aka the digits without a decimal point</param>
  /// <param name="magnitude">Magnitude of the number (digit count minus 1)</param>
  /// <param name="decimalPointPosition">
  ///   Position of the decimal point with 0 pointing to the first possible location,
  ///   which is between the first and second integral digit
  /// </param>
  /// <returns>A pointer one past the last written character in the buffer</returns>
  char *formatInteger32WithDecimalPoint(
    char *buffer /* [48] */, std::uint64_t temp,
    std::size_t magnitude, std::size_t decimalPointPosition
  ) {
    assert(static_cast<std::uint32_t>(temp) == temp); // Must fit in 32 bits integer!

    //     ###      The magnitude and decimalPointPosition inputs are offset by -1 and
    //    ## ##     incrementing them would just cost CPU cycles.
    //   ## | ##
    //  ##  '  ##   123.456    <-- magnitude = 5
    // ###########     ^-- decimalPointPosition = 2
    //
    temp *= factors[magnitude];
    temp >>= shift[magnitude];
    temp += bias[magnitude];

    // If this method is called, the decimal point is between two digits,
    // thus the number must have magnitude 1 (two digits) at least.
    assert((magnitude >= 1) && u8"At least two digits are present");

    // Calculate the remaining digits behind the decimal point
    magnitude -= decimalPointPosition;

    // Is there an odd number of digits before the decimal point? Logic is inverse
    // because of the -1 offset on the decimal point posiiton.
    if((decimalPointPosition & 1) == 0) {
      char pendingDigit;

      // Append the digits before the decimal point. We know it's an odd number,
      // so once we get within 2 chars of the decimal point, we have to keep one on hold.
      for(;;) {
        WRITE_TWO_DIGITS(buffer);
        if(decimalPointPosition < 2) { // Are less than 3 remaining?
          pendingDigit = buffer[1]; // Remember the digit that goes after the decimal point
          break;
        }
        READY_NEXT_TWO_DIGITS();
        decimalPointPosition -= 2;
        buffer += 2;
      }

      // Here comes the decimal point now
      buffer[1] = u8'.';
      buffer[2] = pendingDigit;

      //     ###      We subtracted the decimal point position from the magnitude to get
      //    ## ##     the remaining digits, but both are offset by -1, so now there's
      //   ## | ##    no offset anymore *but* we already wrote one digit above.
      //  ##  '  ##
      // ###########  [4] 56  <-- magnitude = 3
      //

      // Append the digits after the decimal point. This time we can use the ordinary
      // mixed double/single loop because we don't have to interrupt work in the middle.
      for(;;) {
        if(magnitude < 3) { // Are less than 2 remaining? (3 because pendingDigit)
          if(magnitude >= 2) { // is even 1 remaining? (2 because pendingDigit)
            WRITE_ONE_DIGIT(buffer + 3);
            return buffer + 4;
          } else {
            return buffer + 3;
          }
        }

        READY_NEXT_TWO_DIGITS();
        WRITE_TWO_DIGITS(buffer + 3);

        magnitude -= 2;
        buffer += 2;
      }

    } else { // Even number of digits before decimal point

      // Append all digits before the decimal point. We know it's an even number,
      // so we can skip the single digit check and don't need to store a half.
      for(;;) {
        WRITE_TWO_DIGITS(buffer);
        if(decimalPointPosition < 3) { // Are less than 2 following? (3 because pre-decrement)
          break;
        }
        READY_NEXT_TWO_DIGITS();
        decimalPointPosition -= 2;
        buffer += 2;
      }

      // Here comes the decimal point now
      buffer[2] = u8'.';

      //     ###      We subtracted the decimal point position from the magnitude to get
      //    ## ##     the remaining digits, but both are offset by -1, so now there's
      //   ## | ##    no offset anymore.
      //  ##  '  ##
      // ###########  456  <-- magnitude = 3
      //

      // The digits behind the decimal point are at least 1 (otherwise this method
      // would not be called), but they may also be exactly 1, so deal with this here.
      if(magnitude == 1) {
        WRITE_ONE_DIGIT(buffer + 3);
        return buffer + 4;
      }

      // Append the digits after the decimal point. This time we can use the ordinary
      // mixed double/single loop because we don't have to interrupt work in the middle.
      for(;;) {
        READY_NEXT_TWO_DIGITS();
        WRITE_TWO_DIGITS(buffer + 3);
        if(magnitude < 4) { // are less than 2 remaining? (4 because we didn't decrement yet)
          if(magnitude >= 3) { // is even 1 remaining? (3 because we didn't decrement yet)
            WRITE_ONE_DIGIT(buffer + 5);
            return buffer + 6;
          } else { // none are remaining
            return buffer + 5;
          }
        }
        magnitude -= 2;
        buffer += 2;
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Formats an integral number but adds a decimal point between two digits</summary>
  /// <param name="buffer">Buffer into which the number will be written</param>
  /// <param name="number">Significand, aka the digits without a decimal point</param>
  /// <param name="magnitude">Magnitude of the number (digit count minus 1)</param>
  /// <param name="decimalPointPosition">
  ///   Position of the decimal point with 0 pointing to the first possible location,
  ///   which is between the first and second integral digit
  /// </param>
  /// <returns>A pointer one past the last written character in the buffer</returns>
  char *formatInteger64WithDecimalPoint(
    char *buffer /* [325] */, std::uint64_t number,
    std::size_t magnitude, std::size_t decimalPointPosition
  ) {
    // float64 has 53 bits precision for the significand, thus the largest value we can
    // expect in 'number' is 9'007'199'254'740'991.
    //
    // 18'446'744'073'709'551'615     Maximum 64 bit integer
    //      9'007'199'254'740'991     Maximum 53 bit integer (float64 significand)
    //              4'294'967'295     Maximum 32 bit integer
    //
    // This fits beautifully into two calls to the 32 bit integer formatting method!

    //     ###      The magnitude and decimalPointPosition inputs are offset by -1 and
    //    ## ##     incrementing them would just cost CPU cycles.
    //   ## | ##
    //  ##  '  ##   123456789.123456789    <-- magnitude = 17 (with 18 digits)
    // ###########           ^-- decimalPointPosition = 8 (after 9th digit)
    //

    if(magnitude < 10) {
      return formatInteger32WithDecimalPoint(buffer, number, magnitude, decimalPointPosition);
    } else if(decimalPointPosition < 9) {

      buffer = formatInteger32WithDecimalPoint(
        buffer, number / 1'000'000'000, magnitude - 9, decimalPointPosition
      );
      return formatInteger32(buffer, number % 1'000'000'000, 8);

    } else {

      buffer = formatInteger32(buffer, number / 1'000'000'000, magnitude - 9);
      return formatInteger32WithDecimalPoint(
        buffer, number % 1'000'000'000, 8, decimalPointPosition - (magnitude - 8)
      );

    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [46] */, float value) {
    jkj::dragonbox::float_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<float>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          float, jkj::dragonbox::default_float_traits<float>
        >(significandBits, exponentBits, jkj::dragonbox::policy::trailing_zero::remove);

        // If the exponent is negative, the decimal point lies within or before the number
        if(result.exponent < 0) {
          std::size_t digitCountMinusOne = (
            Nuclex::Support::BitTricks::GetLogBase10(result.significand)
          );
          int decimalPointPosition = result.exponent + static_cast<int>(digitCountMinusOne);

          // Does the decimal point lie before all the significand's digits?
          if(decimalPointPosition < 0) {
            buffer[0] = u8'0';
            buffer[1] = u8'.';
            buffer += 2;
            while(decimalPointPosition < -1) {
              *buffer++ = u8'0';
              ++decimalPointPosition;
            }
            return FormatInteger(buffer, result.significand);
          } else { // Nope, the decimal point is within the significand's digits!
            return formatInteger32WithDecimalPoint(
              buffer, result.significand, digitCountMinusOne, decimalPointPosition
            );
          }
        } else { // Exponent is zero or positive, number has no decimal places
          buffer = FormatInteger(buffer, result.significand);
          while(result.exponent > 0) {
            *buffer++ = u8'0';
            --result.exponent;
          }

          // Append a ".0" to indicate that this is a floating point number
          buffer[0] = u8'.';
          buffer[1] = u8'0';
          return buffer + 2;
        }
      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }
    } else if(significandBits.has_all_zero_significand_bits()) { // indicates infinity
      if(significandBits.is_negative()) {
        std::memcpy(buffer, "-Infinity", 9);
        return buffer + 9;
      } else {
        std::memcpy(buffer, "Infinity", 8);
        return buffer + 8;
      }
    } else { // infinite and non-empty signifiand -> not a number
      std::memcpy(buffer, "NaN", 3);
      return buffer + 3;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [325] */, double value) {
    jkj::dragonbox::float_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<double>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          double, jkj::dragonbox::default_float_traits<double>
        >(significandBits, exponentBits, jkj::dragonbox::policy::trailing_zero::remove);

        // If the exponent is negative, the decimal point lies within or before the number
        if(result.exponent < 0) {
          std::size_t digitCountMinusOne = (
            Nuclex::Support::BitTricks::GetLogBase10(result.significand)
          );
          int decimalPointPosition = result.exponent + static_cast<int>(digitCountMinusOne);

          // Does the decimal point lie before all the significand's digits?
          if(decimalPointPosition < 0) {
            buffer[0] = u8'0';
            buffer[1] = u8'.';
            buffer += 2;
            while(decimalPointPosition < -1) {
              *buffer++ = u8'0';
              ++decimalPointPosition;
            }
            return FormatInteger(buffer, result.significand);
          } else { // Nope, the decimal point is within the significand's digits!
            std::uint32_t number = static_cast<std::uint32_t>(result.significand);
            if(number == result.significand) {
              return formatInteger32WithDecimalPoint(
                buffer, number, digitCountMinusOne, decimalPointPosition
              );
            } else {
              return formatInteger64WithDecimalPoint(
                buffer, result.significand, digitCountMinusOne, decimalPointPosition
              );
            }
          }
        } else { // Exponent is zero or positive, number has no decimal places
          buffer = FormatInteger(buffer, result.significand);
          while(result.exponent > 0) {
            *buffer++ = u8'0';
            --result.exponent;
          }

          // Append a ".0" to indicate that this is a floating point number
          buffer[0] = u8'.';
          buffer[1] = u8'0';
          return buffer + 2;
        }
      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }
    } else if(significandBits.has_all_zero_significand_bits()) { // indicates infinity
      if(significandBits.is_negative()) {
        std::memcpy(buffer, "-Infinity", 9);
        return buffer + 9;
      } else {
        std::memcpy(buffer, "Infinity", 8);
        return buffer + 8;
      }
    } else { // infinite and non-empty signifiand -> not a number
      std::memcpy(buffer, "NaN", 3);
      return buffer + 3;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#undef WRITE_TWO_DIGITS
#undef WRITE_ONE_DIGIT
#undef READY_NEXT_TWO_DIGITS
