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

#include "Nuclex/Support/Config.h"

#if defined(HAVE_NANO_SIGNALS)

#include "./nano-signal-slot-2.0.1/nano_signal_slot.hpp"

#include <celero/Celero.h>

#include <algorithm> // for std::copy_n()
#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <string> // for std::string
#include <type_traits> // for std::is_signed
#include <cmath> // for std::abs()

namespace {

  // ------------------------------------------------------------------------------------------- //

  void doNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void doMoreNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  class Event2Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.connect<&doNothingCallback>();
      this->testEvent.connect<&doMoreNothingCallback>();
    }

    public: void tearDown() override {
      this->testEvent.disconnect<&doMoreNothingCallback>();
      this->testEvent.disconnect<&doNothingCallback>();
    }

    protected: Nano::Signal<void(int), Nano::ST_Policy> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class Event50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.connect<&doNothingCallback>();
      }
    }

    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.disconnect<&doNothingCallback>();
      }
    }

    protected: Nano::Signal<void(int), Nano::ST_Policy> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ThreadSafeEvent2Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.connect<&doNothingCallback>();
      this->testEvent.connect<&doMoreNothingCallback>();
    }

    public: void tearDown() override {
      this->testEvent.disconnect<&doMoreNothingCallback>();
      this->testEvent.disconnect<&doNothingCallback>();
    }

    protected: Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ThreadSafeEvent50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.connect<&doNothingCallback>();
      }
    }

    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.disconnect<&doNothingCallback>();
      }
    }

    protected: Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe2, NanoSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::ST_Policy> testEvent;
    testEvent.connect<&doNothingCallback>();
    testEvent.connect<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe2, NanoThreadSafeSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;
    testEvent.connect<&doNothingCallback>();
    testEvent.connect<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe50, NanoSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::ST_Policy> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe50, NanoThreadSafeSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe2, NanoSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::ST_Policy> testEvent;
    testEvent.connect<&doNothingCallback>();
    testEvent.connect<&doMoreNothingCallback>();
    testEvent.disconnect<&doMoreNothingCallback>();
    testEvent.disconnect<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe2, NanoThreadSafeSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;
    testEvent.connect<&doNothingCallback>();
    testEvent.connect<&doMoreNothingCallback>();
    testEvent.disconnect<&doMoreNothingCallback>();
    testEvent.disconnect<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe50, NanoSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::ST_Policy> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe50, NanoThreadSafeSignal, 1000, 0) {
    Nano::Signal<void(int), Nano::TS_Policy_Safe<>> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke2_x100, NanoSignal, Event2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.fire(index);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke2_x100, NanoThreadSafeSignal, ThreadSafeEvent2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.fire(index);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke50_x100, NanoSignal, Event50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.fire(index);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke50_x100, NanoThreadSafeSignal, ThreadSafeEvent50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.fire(index);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // defined(HAVE_NANO_SIGNALS)
