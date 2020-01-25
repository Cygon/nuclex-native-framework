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

#include "Nuclex/Support/Services/LazyServiceInjector.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Example service providing a few simple math methods</summary>
  class CalculatorService {

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

    /// <summary>Factory method that creates an instance of the broken calculator</summary>
    /// <returns>The new broken calculator instance</returns>
    public: static std::shared_ptr<BrokenCalculator> CreateInstance(
      const Nuclex::Support::Services::ServiceProvider &
    ) {
      return std::make_shared<BrokenCalculator>();
    }

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

  /// <summary>Example class that consumes the calculator service</summary>
  class CalculatorUser {

    /// <summary>Initializes the calculator user example</summary>
    /// <param name="calculator">Calculator service the example will be working with</param>
    public: CalculatorUser(const std::shared_ptr<CalculatorService> &calculator) :
      calculator(calculator) {}

    /// <summary>Performs a calculation using the calculator service</summary>
    /// <returns>The result of the calculation</returns>
    public: int CalculateSomething() {
      return this->calculator->Add(1, 2) + this->calculator->Multiply(2, 2);
    }

    /// <summary>Calculator service the example has been provided with</summary>
    private: std::shared_ptr<CalculatorService> calculator;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, StandardIsConstructibleWorks) {
    constexpr bool withoutArguments = std::is_constructible<CalculatorUser>::value;
    EXPECT_FALSE(withoutArguments);

    constexpr bool withWrongArgument = std::is_constructible<CalculatorUser, int>::value;
    EXPECT_FALSE(withWrongArgument);

    constexpr bool withTooManyArguments = std::is_constructible<
      CalculatorUser, std::shared_ptr<CalculatorService>, int
    >::value;
    EXPECT_FALSE(withTooManyArguments);

    constexpr bool withMatchingArguments = std::is_constructible<
      CalculatorUser, std::shared_ptr<CalculatorService>
    >::value;
    EXPECT_TRUE(withMatchingArguments);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanCreateIntegerSequence) {
    EXPECT_NO_THROW(
      typename Nuclex::Support::Services::Private::BuildIntegerSequence<4> test;
      test = test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanDetectSharedPtrs) {
    constexpr bool isIntASharedPtr = (
      Nuclex::Support::Services::Private::IsSharedPtr<int>::value
    );
    EXPECT_FALSE(isIntASharedPtr);

    constexpr bool isSharedPtrASharedPtr = (
      Nuclex::Support::Services::Private::IsSharedPtr<std::shared_ptr<int>>::value
    );
    EXPECT_TRUE(isSharedPtrASharedPtr);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanDetectInjectableTypes) {
    class Evil : public std::shared_ptr<int> {
      public: virtual void MakeItAbstract() = 0;
    };

    constexpr bool isIntInjectable = (
      Nuclex::Support::Services::Private::IsInjectableArgument<int>::value
    );
    EXPECT_FALSE(isIntInjectable); // It's not a shared_ptr

    constexpr bool isAbstractSharedPtrInjectable = (
      Nuclex::Support::Services::Private::IsInjectableArgument<Evil>::value
    );
    EXPECT_FALSE(isAbstractSharedPtrInjectable); // It's abstract

    constexpr bool isSharedPtrIntInjectable = (
      Nuclex::Support::Services::Private::IsInjectableArgument<std::shared_ptr<int>>::value
    );
    EXPECT_TRUE(isSharedPtrIntInjectable); // Silly but okay

    constexpr bool isSharedPtrClassInjectable = (
      Nuclex::Support::Services::Private::IsInjectableArgument<
        std::shared_ptr<CalculatorService>
      >::value
    );
    EXPECT_TRUE(isSharedPtrClassInjectable); // Alright!
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanDetectConstructorSignature) {
    class DefaultConstructible {};
    constexpr bool defaultConstructibleIsDetected = !std::is_base_of<
      Nuclex::Support::Services::Private::InvalidConstructorSignature,
      Nuclex::Support::Services::Private::DetectConstructorSignature<DefaultConstructible>
    >::value;
    EXPECT_TRUE(defaultConstructibleIsDetected);

    class OneArgumentConstructible {
      public: OneArgumentConstructible(const std::shared_ptr<int> &) {}
    };
    constexpr bool singleArgumentIsDetected = !std::is_base_of<
      Nuclex::Support::Services::Private::InvalidConstructorSignature,
      Nuclex::Support::Services::Private::DetectConstructorSignature<OneArgumentConstructible>
    >::value;
    EXPECT_TRUE(singleArgumentIsDetected);

    class TwoArgumentConstructible {
      public: TwoArgumentConstructible(
        const std::shared_ptr<int> &, const std::shared_ptr<int> &
      ) {}
    };
    constexpr bool twoArgumentsAreDetected = !std::is_base_of<
      Nuclex::Support::Services::Private::InvalidConstructorSignature,
      Nuclex::Support::Services::Private::DetectConstructorSignature<TwoArgumentConstructible>
    >::value;
    EXPECT_TRUE(twoArgumentsAreDetected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, RejectsConstructorWithUninjectableArguments) {
    class UninjectableConstructor {
      public: UninjectableConstructor(float) {}
    };
    constexpr bool uninjectableConstructorIsRejected = std::is_base_of<
      Nuclex::Support::Services::Private::InvalidConstructorSignature,
      Nuclex::Support::Services::Private::DetectConstructorSignature<UninjectableConstructor>
    >::value;
    EXPECT_TRUE(uninjectableConstructorIsRejected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, RejectsConstructorWithTooManyArguments) {
    class NineArgumentConstructible {
      public: NineArgumentConstructible(
        const std::shared_ptr<int> &, const std::shared_ptr<int> &,
        const std::shared_ptr<int> &, const std::shared_ptr<int> &,
        const std::shared_ptr<int> &, const std::shared_ptr<int> &,
        const std::shared_ptr<int> &, const std::shared_ptr<int> &,
        const std::shared_ptr<int> &
      ) {}
    };

    // This test will obviously break if you increase the argument limit beyond eight
    constexpr bool nineArgumentsAreAccepted = !std::is_base_of<
      Nuclex::Support::Services::Private::InvalidConstructorSignature,
      Nuclex::Support::Services::Private::DetectConstructorSignature<NineArgumentConstructible>
    >::value;
    EXPECT_FALSE(nineArgumentsAreAccepted);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanBindServiceToImplementation) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<CalculatorService>().To<BrokenCalculator>();
    
    std::shared_ptr<CalculatorService> service = serviceInjector.Get<CalculatorService>();
    ASSERT_TRUE(!!service);

    EXPECT_NO_THROW(service->Add(1, 2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, ServiceCanSelfImplement) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<BrokenCalculator>().ToSelf();

    std::shared_ptr<BrokenCalculator> service = serviceInjector.Get<BrokenCalculator>();
    ASSERT_TRUE(!!service);

    EXPECT_NO_THROW(service->Add(1, 2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanBindServiceToFactoryMethod) {
    LazyServiceInjector serviceInjector;

    // Simple form of .ToFactoryMethod() that expects the factory method to
    // return the service type
    serviceInjector.Bind<BrokenCalculator>().ToFactoryMethod<
      &BrokenCalculator::CreateInstance
    >();

    std::shared_ptr<BrokenCalculator> service = serviceInjector.Get<BrokenCalculator>();
    ASSERT_TRUE(!!service);

    EXPECT_NO_THROW(service->Add(1, 2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanBindServiceToFactoryMethodReturningImplementation) {
    LazyServiceInjector serviceInjector;

    // More elaborate form of .ToFactoryMethod() where the factory methood
    // can return any type that inherits from the service type
    serviceInjector.Bind<CalculatorService>().ToFactoryMethod<
      BrokenCalculator, &BrokenCalculator::CreateInstance
    >();

    std::shared_ptr<CalculatorService> service = serviceInjector.Get<CalculatorService>();
    ASSERT_TRUE(!!service);

    EXPECT_NO_THROW(service->Add(1, 2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanBindServiceToInstance) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<CalculatorService>().ToInstance(std::make_shared<BrokenCalculator>());

    std::shared_ptr<CalculatorService> service = serviceInjector.Get<CalculatorService>();
    ASSERT_TRUE(!!service);

    EXPECT_NO_THROW(service->Add(1, 2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanResolveServiceImplementationDependencies) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<CalculatorService>().To<BrokenCalculator>();
    serviceInjector.Bind<CalculatorUser>().ToSelf();

    std::shared_ptr<CalculatorUser> user = serviceInjector.Get<CalculatorUser>();
    ASSERT_TRUE(!!user);

    EXPECT_NO_THROW(user->CalculateSomething());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, ServiceInstancesAreShared) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<BrokenCalculator>().ToSelf();

    std::shared_ptr<BrokenCalculator> first = serviceInjector.Get<BrokenCalculator>();
    std::shared_ptr<BrokenCalculator> second = serviceInjector.Get<BrokenCalculator>();
    ASSERT_TRUE(!!first);
    ASSERT_TRUE(!!second);

    // The service injector should have delivered the same instance both times
    EXPECT_EQ(first.get(), second.get());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LazyServiceInjectorTest, CanProvideServiceFactoryFunction) {
    LazyServiceInjector serviceInjector;

    serviceInjector.Bind<BrokenCalculator>().ToSelf();

    /*
    std::shared_ptr<BrokenCalculator> (*factory)() = (
      serviceInjector.GetServiceFactory<BrokenCalculator>()
    );
    */

    std::shared_ptr<BrokenCalculator> shared = serviceInjector.Get<BrokenCalculator>();
    ASSERT_TRUE(!!shared);

    std::shared_ptr<BrokenCalculator> first = serviceInjector.Create<BrokenCalculator>();
    std::shared_ptr<BrokenCalculator> second = serviceInjector.Create<BrokenCalculator>();
    ASSERT_TRUE(!!first);
    ASSERT_TRUE(!!second);

    // The service injector should have created a new instance both times
    EXPECT_NE(first.get(), second.get());
    EXPECT_NE(shared.get(), first.get());
    EXPECT_NE(shared.get(), second.get());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
