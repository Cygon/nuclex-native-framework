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

// Prepares an integral number or the specified decimal megnitude for printing.
//
// This uses a magic formula to turn a 32 bit number into a specific 64 bit number.
//
// I think the main thing this formula accomplishes is that the actual number sits at
// the upper end of a 32 bit integer. Thus, when you cast it to a 64 bit integer and
// multiply it by 100, you end up with the next two digits in the high 32 bits of
// your 64 bit integer where they're easy to grab.
//
// Magnitude is in blocks of 2, so 1 means 100, 2 means 1'000, 3 means 10'000 and so on.
#define PREPARE_NUMBER_OF_MAGNITUDE(number, magnitude) \
  temp = ( \
    (std::uint64_t(1) << (32 + magnitude / 5 * magnitude * 53 / 16)) / \
    std::uint32_t(1e##magnitude) + 1 + magnitude / 6 - magnitude / 8 \
  ), \
  temp *= number, \
  temp >>= magnitude / 5 * magnitude * 53 / 16, \
  temp += magnitude / 6 * 4

// Brings the next two digits of a prepared number into the high 32 bits
// so they can be extracted by the WRITE_ONE_DIGIT and WRITE_TWO_DIGITS macros
#define READY_NEXT_TWO_DIGITS() \
  temp = std::uint64_t(100) * static_cast<std::uint32_t>(temp)

// Appends the next two highest digits in the prepared number to the char buffer
#define WRITE_TWO_DIGITS(bufferPointer) \
  *reinterpret_cast<TwoChars *>(bufferPointer) = ( \
    *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[(temp >> 31) & 0xFE]) \
  )

// Appends the next highest digit in the prepared number to the char buffer
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

  /// <summary>
  ///   Takes the absolute value of a signed 32 bit integer and returns it as unsigned
  /// </summary>
  /// <param name="value">Value whose absolute value will be returned as an unsigned type</param>
  /// <returns>The absolute value if the input integer as an unsigned integer</returns>
  /// <remarks>
  ///   This avoids the undefined result of std::abs() applied to the lowest possible integer.
  /// </remarks>
  inline constexpr std::uint32_t absToUnsigned(std::int32_t value) noexcept {
    return 0u - static_cast<std::uint32_t>(value);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Takes the absolute value of a signed 64 bit integer and returns it as unsigned
  /// </summary>
  /// <param name="value">Value whose absolute value will be returned as an unsigned type</param>
  /// <returns>The absolute value if the input integer as an unsigned integer</returns>
  /// <remarks>
  ///   This avoids the undefined result of std::abs() applied to the lowest possible integer.
  /// </remarks>
  inline constexpr std::uint64_t absToUnsigned(std::int64_t value) noexcept {
    return 0u - static_cast<std::uint64_t>(value);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [10] */, std::uint32_t number) {
    std::uint64_t temp;

    // I have a nice Nuclex::Support::BitTricks::GetLogBase10() method which uses
    // no branching, just the CLZ (count leading zeros) CPU instruction, but feeding
    // this into a switch statement turns out to be slower than the branching tree.
    //
    // I also tested building a manual jump table with functions for each digit count
    // that is indexed by GetLogBase10() and called - so just one indirection in place
    // of several branching instructions, but it was slower, too. Not predictable enough
    // for the CPU?
    //
    // So this bunch of branches is outperforming every trick I have...
    //
    if(number < 100) {
      if(number < 10) {
        *buffer = static_cast<char>(u8'0' + number);
        return buffer + 1;
      } else {
        *reinterpret_cast<TwoChars *>(buffer) = (
          *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[number * 2])
        );
        return buffer + 2;
      }
    } else if(number < 1'000'000) {
      if(number < 10'000) {
        if(number < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          return buffer + 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 2);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          return buffer + 4;
        }
      } else {
        if(number < 100'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 3);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          WRITE_ONE_DIGIT(buffer + 4);
          return buffer + 5;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 4);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          return buffer + 6;
        }
      }
    } else {
      if(number < 100'000'000) {
        if(number < 10'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 5);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          WRITE_ONE_DIGIT(buffer + 6);
          return buffer + 7;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 6);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          return buffer + 8;
        }
      } else {
        if(number < 1'000'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 7);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          WRITE_ONE_DIGIT(buffer + 8);
          return buffer + 9;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 8);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 8);
          return buffer + 10;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [11] */, std::int32_t value) {
    if(value >= 0) {
      return FormatInteger(buffer, static_cast<std::uint32_t>(value));
    } else {
      *buffer++ = u8'-';
      return FormatInteger(buffer, absToUnsigned(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [20] */, std::uint64_t number64) {

    // If this number fits into 32 bits, then don't bother with the extra processing
    std::uint32_t number = static_cast<std::uint32_t>(number64);
    if(number == number64) {
      return FormatInteger(buffer, number);
    }

    // Temporary value, the integer to be converted will be placed in the upper end
    // of its lower 32 bits and then converted by shifting 2 characters apiece into
    // the upper 32 bits of this 64 bit integer.
    std::uint64_t temp;

    std::uint64_t a = number64 / 100'000'000u;
    number = static_cast<std::uint32_t>(a);
    if(number == a) {
      buffer = FormatInteger(buffer, number);
    } else {
      number = static_cast<std::uint32_t>(a / 100'000'000u);

      if(number < 100) {
        if(number < 10) {
          *buffer++ = static_cast<char>(u8'0' + number);
        } else {
          *reinterpret_cast<TwoChars *>(buffer) = (
            *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[number * 2])
          );
          buffer += 2;
        }
      } else {
        if(number < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          buffer += 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(number, 2);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          buffer += 4;
        }
      }

      number = a % 100'000'000u;

      PREPARE_NUMBER_OF_MAGNITUDE(number, 6);
      WRITE_TWO_DIGITS(buffer);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 2);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 4);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 6);
      buffer += 8;
    }

    number = number64 % 100'000'000u;

    PREPARE_NUMBER_OF_MAGNITUDE(number, 6);
    WRITE_TWO_DIGITS(buffer);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 2);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 4);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 6);

    return buffer + 8;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [20] */, std::int64_t number64) {
    if(number64 >= 0) {
      return FormatInteger(buffer, static_cast<std::uint64_t>(number64));
    } else {
      *buffer++ = u8'-';
      return FormatInteger(buffer, absToUnsigned(number64));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#undef WRITE_TWO_DIGITS
#undef WRITE_ONE_DIGIT
#undef READY_NEXT_TWO_DIGITS
#undef PREPARE_NUMBER_OF_MAGNITUDE
