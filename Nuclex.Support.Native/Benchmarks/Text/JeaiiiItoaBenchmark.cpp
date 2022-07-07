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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Config.h"

#if defined(HAVE_JEAIII_ITOA)

#include <celero/Celero.h>

// James Edward Anhalt III.'s itoa() implementation
// https://github.com/jeaiii/itoa
#include "./Jeaiii-2020/int_to_chars_jeaiii.h"

#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <type_traits> // for std::integral_constant

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Table of the numbers 00 .. 99 as a flat array</summary>
  /// <remarks>
  ///   Used for James Edward Anhalt III.'s integer formatting technique where two digits
  ///   are converted at once, among other tricks.
  /// </remarks>
  constexpr char Radix100[] = {
    u8'0', u8'0',   u8'0', u8'1',   u8'0', u8'2',   u8'0', u8'3',   u8'0', u8'4',
    u8'0', u8'5',   u8'0', u8'6',   u8'0', u8'7',   u8'0', u8'8',   u8'0', u8'9',
    u8'1', u8'0',   u8'1', u8'1',   u8'1', u8'2',   u8'1', u8'3',   u8'1', u8'4',
    u8'1', u8'5',   u8'1', u8'6',   u8'1', u8'7',   u8'1', u8'8',   u8'1', u8'9',
    u8'2', u8'0',   u8'2', u8'1',   u8'2', u8'2',   u8'2', u8'3',   u8'2', u8'4',
    u8'2', u8'5',   u8'2', u8'6',   u8'2', u8'7',   u8'2', u8'8',   u8'2', u8'9',
    u8'3', u8'0',   u8'3', u8'1',   u8'3', u8'2',   u8'3', u8'3',   u8'3', u8'4',
    u8'3', u8'5',   u8'3', u8'6',   u8'3', u8'7',   u8'3', u8'8',   u8'3', u8'9',
    u8'4', u8'0',   u8'4', u8'1',   u8'4', u8'2',   u8'4', u8'3',   u8'4', u8'4',
    u8'4', u8'5',   u8'4', u8'6',   u8'4', u8'7',   u8'4', u8'8',   u8'4', u8'9',
    u8'5', u8'0',   u8'5', u8'1',   u8'5', u8'2',   u8'5', u8'3',   u8'5', u8'4',
    u8'5', u8'5',   u8'5', u8'6',   u8'5', u8'7',   u8'5', u8'8',   u8'5', u8'9',
    u8'6', u8'0',   u8'6', u8'1',   u8'6', u8'2',   u8'6', u8'3',   u8'6', u8'4',
    u8'6', u8'5',   u8'6', u8'6',   u8'6', u8'7',   u8'6', u8'8',   u8'6', u8'9',
    u8'7', u8'0',   u8'7', u8'1',   u8'7', u8'2',   u8'7', u8'3',   u8'7', u8'4',
    u8'7', u8'5',   u8'7', u8'6',   u8'7', u8'7',   u8'7', u8'8',   u8'7', u8'9',
    u8'8', u8'0',   u8'8', u8'1',   u8'8', u8'2',   u8'8', u8'3',   u8'8', u8'4',
    u8'8', u8'5',   u8'8', u8'6',   u8'8', u8'7',   u8'8', u8'8',   u8'8', u8'9',
    u8'9', u8'0',   u8'9', u8'1',   u8'9', u8'2',   u8'9', u8'3',   u8'9', u8'4',
    u8'9', u8'5',   u8'9', u8'6',   u8'9', u8'7',   u8'9', u8'8',   u8'9', u8'9'
  };

  // ------------------------------------------------------------------------------------------- //

  // Remade Jeaiii code without macros from
  // https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/
  char *itoa_better_y(std::uint32_t n, char *buffer) {
    std::uint64_t prod;

    auto get_next_two_digits = [&]() {
      prod = std::uint32_t(prod) * std::uint64_t(100);
      return int(prod >> 32);
    };
    auto print_1 = [&](int digit) {
      buffer[0] = char(digit + '0');
      buffer += 1;
    };
    auto print_2 = [&] (int two_digits) {
      std::memcpy(buffer, Radix100 + two_digits * 2, 2);
      buffer += 2;
    };
    auto print = [&](std::uint64_t magic_number, int extra_shift, auto remaining_count) {
      prod = n * magic_number;
      prod >>= extra_shift;
      auto two_digits = int(prod >> 32);

      if (two_digits < 10) {
        print_1(two_digits);
        for (int i = 0; i < remaining_count; ++i) {
          print_2(get_next_two_digits());
        }
      }
      else {
        print_2(two_digits);
        for (int i = 0; i < remaining_count; ++i) {
          print_2(get_next_two_digits());
        }
      }
    };

    if (n < 100) {
      if (n < 10) {
        // 1 digit.
        print_1(n);
      }
      else {
        // 2 digit.
        print_2(n);
      }
    }
    else {
      if (n < 1'000'000) {
        if (n < 10'000) {
          // 3 or 4 digits.
          // 42949673 = ceil(2^32 / 10^2)
          print(42'949'673, 0, std::integral_constant<int, 1>{});
        }
        else {
          // 5 or 6 digits.
          // 429497 = ceil(2^32 / 10^4)
          print(429'497, 0, std::integral_constant<int, 2>{});
        }
      }
      else {
        if (n < 100'000'000) {
          // 7 or 8 digits.
          // 281474978 = ceil(2^48 / 10^6) + 1
          print(281'474'978, 16, std::integral_constant<int, 3>{});
        }
        else {
          if (n < 1'000'000'000) {
            // 9 digits.
            // 1441151882 = ceil(2^57 / 10^8) + 1
            prod = n * std::uint64_t(1'441'151'882);
            prod >>= 25;
            print_1(int(prod >> 32));
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
          }
          else {
            // 10 digits.
            // 1441151881 = ceil(2^57 / 10^8)
            prod = n * std::uint64_t(1'441'151'881);
            prod >>= 25;
            print_2(int(prod >> 32));
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
          }
        }
      }
    }
    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937 randomNumberGenerator32;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator64;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint64_t> randomNumberDistribution64;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, JeaiiiOriginal, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      int_to_chars_jeaiii(
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer64Itoa, JeaiiiOriginal, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      int_to_chars_jeaiii(
        static_cast<std::uint64_t>(randomNumberDistribution64(randomNumberGenerator64)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, JeaiiiImproved, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      itoa_better_y(
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // defined(HAVE_JEAIII_ITOA)
