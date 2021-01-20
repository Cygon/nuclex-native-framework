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

#include "Nuclex/Support/Services/ServiceContainer.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Example service providing a few simple math methods</summary>
  class CalculatorService {

    /// <summary>Frees all resources owned by a calculator instance</summary>
    public: virtual ~CalculatorService() = default;

    /// <summary>Calculates the sum of two integers</summary>
    /// <param name="first">First integer that will be part of the sum</param>
    /// <param name="second">Second integer that will be part of the sum</param>
    /// <returns>The sum of the two integers</returnd>
    public: virtual int Add(int first, int second) = 0;

    /// <summary>Multiplies two integers with each other</summary>
    /// <param name="first">First integer that will be multiplied with the other</param>
    /// <param name="second">Second integer that will be multiplied with the other</param>
    /// <returns>The sum of the two integers</returnd>
    public: virtual int Multiply(int first, int second) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Example implementation of the calculator service</summary>
  class BrokenCalculator : public virtual CalculatorService {

    /// <summary>Frees all resources owned by a calculator instance</summary>
    public: virtual ~BrokenCalculator() = default;

    /// <summary>Calculates the sum of two integers</summary>
    /// <param name="first">First integer that will be part of the sum</param>
    /// <param name="second">Second integer that will be part of the sum</param>
    /// <returns>The sum of the two integers</returnd>
    public: int Add(int first, int second) override { return first + second + 1; }

    /// <summary>Multiplies two integers with each other</summary>
    /// <param name="first">First integer that will be multiplied with the other</param>
    /// <param name="second">Second integer that will be multiplied with the other</param>
    /// <returns>The sum of the two integers</returnd>
    public: int Multiply(int first, int second) override { return first + first * second; };

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class used by the unit tests to track service destruction</summary>
  class DestructorTester {

    /// <summary>Initializes a new destructor tester using the specified flag</summary>
    /// <param name="destructionFlag">Will be set when the instance is destroyed</param>
    public: DestructorTester(bool *destructionFlag) :
      destructionFlag(destructionFlag) {}

    /// <summary>Sets the destruction flag (unless disarmeds)</summary>
    public: ~DestructorTester() {
      if(this->destructionFlag != nullptr) {
        *this->destructionFlag = true;
      }
    }

    /// <summary>Disarms the destructor tester, no longer letting it set the flag</summary>
    public: void Disarm() { this->destructionFlag = nullptr; }

    /// <summary>Address of a boolean that will be set when the destructor runs</summary>
    private: bool *destructionFlag;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      ServiceContainer test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, NewContainerHasNoServices) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ServicesCanBeAddedUnderOwnType) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0U);
    test.Add(std::make_shared<BrokenCalculator>());
    EXPECT_EQ(test.CountServices(), 1U);
    
    std::shared_ptr<CalculatorService> service;
    EXPECT_FALSE(test.TryGet<CalculatorService>(service));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ServicesCanBeAddedUnderServiceType) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0U);
    test.Add<CalculatorService>(std::make_shared<BrokenCalculator>());
    EXPECT_EQ(test.CountServices(), 1U);

    std::shared_ptr<CalculatorService> service;
    EXPECT_TRUE(test.TryGet<CalculatorService>(service));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ServicesCanBeRemoved) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0U);
    test.Add(std::make_shared<BrokenCalculator>());
    EXPECT_EQ(test.CountServices(), 1U);

    std::shared_ptr<BrokenCalculator> service;
    EXPECT_TRUE(test.TryGet<BrokenCalculator>(service));

    EXPECT_TRUE(test.Remove<BrokenCalculator>());
    EXPECT_FALSE(test.TryGet<BrokenCalculator>(service));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ContainerDestructorReleasesServices) {
    bool destructorCalled = false;
    std::weak_ptr<DestructorTester> weak;
    {
      std::shared_ptr<DestructorTester> tester = (
        std::make_shared<DestructorTester>(&destructorCalled)
      );
      weak = tester;

      ServiceContainer test;
      EXPECT_EQ(test.CountServices(), 0U);
      test.Add(tester);
      EXPECT_EQ(test.CountServices(), 1U);

      // Dropping our shared_ptr to the test object will not destroy it because
      // another shared_ptr to it is kept by the service container
      tester.reset();
      EXPECT_FALSE(destructorCalled);
    }

    // When the service container is destroyed, it should release all shared_ptrs
    // it is holding on to (in whatever manner), thus now the destructor should run
    EXPECT_TRUE(destructorCalled);
    if(!weak.expired()) {
      std::shared_ptr<DestructorTester> crap = weak.lock();
      if(!!crap) {
        crap->Disarm();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
