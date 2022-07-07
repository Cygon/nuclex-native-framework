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
#include "Nuclex/Support/Events/Event.h"
#include "Nuclex/Support/Events/ConcurrentEvent.h"

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
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }

    public: void tearDown() override {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }

    protected: Nuclex::Support::Events::Event<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class Event50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }

    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Unsubscribe<&doNothingCallback>();
      }
    }

    protected: Nuclex::Support::Events::Event<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ConcurrentEvent2Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }

    public: void tearDown() override {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }

    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ConcurrentEvent50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }

    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Unsubscribe<&doNothingCallback>();
      }
    }

    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;

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

  BASELINE(Subscribe2, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe2, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Subscribe50, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe50, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Unsubscribe2, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe2, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Unsubscribe50, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Unsubscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe50, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Unsubscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Invoke2_x100, NuclexEvent, Event2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke2_x100, NuclexConcurrentEvent, ConcurrentEvent2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Invoke50_x100, NuclexEvent, Event50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke50_x100, NuclexConcurrentEvent, ConcurrentEvent50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
