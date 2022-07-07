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

#if defined(HAVE_LSIGNAL)

#include "./lsignal-2017-05/lsignal.h"

#include <celero/Celero.h>

#include <algorithm> // for std::copy_n()
#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <string> // for std::string
#include <type_traits> // for std::is_signed
#include <cmath> // for std::abs()
#include <optional> // because lsignal::connection cannot default-construct

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
      this->doNothingConnection = this->testEvent.connect(doNothingCallback);
      this->doMoreNothingConnection = this->testEvent.connect(doMoreNothingCallback);
    }

    public: void tearDown() override {
      this->testEvent.disconnect(this->doMoreNothingConnection.value());
      this->testEvent.disconnect(this->doNothingConnection.value());
    }

    protected: lsignal::signal<void(int)> testEvent;
    protected: std::optional<lsignal::connection> doNothingConnection;
    protected: std::optional<lsignal::connection> doMoreNothingConnection;

  };

  // ------------------------------------------------------------------------------------------- //

  class Event50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->connections.reserve(50);
      this->connections.clear();
      for(std::size_t index = 0; index < 50; ++index) {
        this->connections.push_back(this->testEvent.connect(doNothingCallback));
      }
    }

    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.disconnect(this->connections[49 - index]);
      }
    }

    protected: lsignal::signal<void(int)> testEvent;
    protected: std::vector<lsignal::connection> connections;

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

  BENCHMARK(Subscribe2, LSignal, 1000, 0) {
    lsignal::signal<void(int)> testEvent;
    testEvent.connect(doNothingCallback);
    testEvent.connect(doMoreNothingCallback);
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe50, LSignal, 1000, 0) {
    lsignal::signal<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.connect(doNothingCallback);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe2, LSignal, 1000, 0) {
    lsignal::signal<void(int)> testEvent;
    lsignal::connection doNothingConnection = testEvent.connect(doNothingCallback);
    lsignal::connection doMoreNothingConnection = testEvent.connect(doMoreNothingCallback);
    testEvent.disconnect(doMoreNothingConnection);
    testEvent.disconnect(doNothingConnection);
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe50, LSignal, 1000, 0) {
    lsignal::signal<void(int)> testEvent;
    std::vector<lsignal::connection> connections;
    connections.reserve(50);
    for(std::size_t index = 0; index < 50; ++index) {
      connections.push_back(testEvent.connect(doNothingCallback));
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.disconnect(connections[49 - index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke2_x100, LSignal, Event2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke50_x100, LSignal, Event50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // defined(HAVE_LSIGNAL)
