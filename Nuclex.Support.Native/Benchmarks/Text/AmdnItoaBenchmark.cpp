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

#if defined(HAVE_AMDN_ITOA)

#include <celero/Celero.h>

// Arturo Martin-de-Nicolas' quite readable itoa() implementation
// https://github.com/amdn/itoa_ljust
#include "AmdnLJust-2016/itoa_ljust.h"

// Arturo Martin-de-Nicolas' fastest itoa() implementation
// https://github.com/amdn/itoa
#include "AmdnFast-2016/itoa.h"

#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t

namespace {

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

  BENCHMARK(Integer32Itoa, NicolasLJust, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      itoa_ljust::itoa(
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer64Itoa, NicolasLJust, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      itoa_ljust::itoa(
        static_cast<std::uint64_t>(randomNumberDistribution64(randomNumberGenerator64)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, NicolasFast, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      itoa_fwd(
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer64Itoa, NicolasFast, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      itoa_fwd(
        static_cast<std::uint64_t>(randomNumberDistribution64(randomNumberGenerator64)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // defined(HAVE_AMDN_ITOA)
