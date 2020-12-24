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

#include "Nuclex/Support/Events/Delegate.h"

#include <gtest/gtest.h>

using namespace std;

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Free function used to test delegate calls</summary>
  void freeFunction(int) { }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes a value into the integer at the provided address</summary>
  /// <param name="integerToSet">Address of an integer that will be set</param>
  void setIntegerFunction(int *integerToSet) { *integerToSet = 123; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes a different value into the integer at the provided address</summary>
  /// <param name="integerToSet">Address of an integer that will be set</param>
  void setOtherIntegerFunction(int *integerToSet) { *integerToSet = 987; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy class used to test delegate calls to class methods</summary>
  class StaticMock {

    /// <summary>Method that can be called via delegate for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: static void Notify(int something) {
      LastSomethingParameterValue = something;
      ++ReceivedNotificationCount;
    }

    /// <summary>Method that does nothing used for delegate comparison testing</summary>
    public: static void Dummy(int) {}

    /// <summary>Number of calls to Notify() the instance has observed</summary>
    public: static std::size_t ReceivedNotificationCount;
    /// <summary>Value that was last passed to the Notify() method</summary>
    public: static int LastSomethingParameterValue;

  };

  /// <summary>Number of calls to Notify() the instance has observed</summary>
  std::size_t StaticMock::ReceivedNotificationCount = 0;
  /// <summary>Value that was last passed to the Notify() method</summary>
  int StaticMock::LastSomethingParameterValue = 0;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy class used to test delegate calls to object methods</summary>
  class Mock {

    /// <summary>Initializes a new mocked subscriber</summary>
    public: Mock() :
      ReceivedNotificationCount(0),
      LastSomethingParameterValue(0),
      ConstNotifyCalled(false) {}

    /// <summary>Method that can be called via delegate for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void Notify(int something) {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
    }

    /// <summary>Method that can be called via delegate for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void ConstNotify(int something) const{
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
      this->ConstNotifyCalled = true;
    }

    /// <summary>Number of calls to Notify() the instance has observed</summary>
    public: mutable std::size_t ReceivedNotificationCount;
    /// <summary>Value that was last passed to the Notify() method</summary>
    public: mutable int LastSomethingParameterValue;
    /// <summary>Whether the ConstNotify() method was called</summary>
    public: mutable bool ConstNotifyCalled;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      Delegate<void(int something)> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CallingEmptyEventThrowsException) {
    Delegate<void(int something)> test;
    EXPECT_THROW(
      test(123),
      Errors::EmptyDelegateCallError
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanCallFreeFunctions) {
    Delegate<void(int *something)> test = (
      Delegate<void(int *something)>::Create<setIntegerFunction>()
    );

    int value = 456;
    EXPECT_EQ(value, 456);

    test(&value);
    EXPECT_EQ(value, 123);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanCallStaticClassMethods) {
    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<&StaticMock::Notify>()
    );

    EXPECT_EQ(StaticMock::LastSomethingParameterValue, 0);
    EXPECT_EQ(StaticMock::ReceivedNotificationCount, 0U);

    test(666);

    EXPECT_EQ(StaticMock::LastSomethingParameterValue, 666);
    EXPECT_EQ(StaticMock::ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanCallObjectMethods) {
    Mock callTarget;

    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<Mock, &Mock::Notify>(&callTarget)
    );

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(42);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 42);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanCallConstObjectMethodsOnNonConstObject) {
    Mock callTarget;

    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&callTarget)
    );

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(0x4472676E);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0x4472676E);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanCallConstObjectMethodsOnConstObject) {
    const Mock callTarget;

    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&callTarget)
    );

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(0x4472676E);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0x4472676E);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToEmpty) {
    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<freeFunction>()
    );

    test.Reset();

    EXPECT_THROW(
      test(654),
      Errors::EmptyDelegateCallError
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToDifferentFunction) {
    Delegate<void(int *something)> test = (
      Delegate<void(int *something)>::Create<setIntegerFunction>()
    );
    test.Reset<setOtherIntegerFunction>();

    int value = 111;
    EXPECT_EQ(value, 111);

    test(&value);
    EXPECT_EQ(value, 987);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToDifferentStaticMethod) {
    Delegate<void(int *something)> test = (
      Delegate<void(int *something)>::Create<setIntegerFunction>()
    );
    test.Reset<setOtherIntegerFunction>();
    Delegate<void(int something)>::Create<&StaticMock::Notify>();

    int value = 111;
    EXPECT_EQ(value, 111);

    test(&value);
    EXPECT_EQ(value, 987);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToDifferentObjectMethod) {
    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<freeFunction>()
    );

    Mock callTarget;
    test.Reset<Mock, &Mock::Notify>(&callTarget);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(555);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 555);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToDifferentConstObjectMethodOnNonConstObject) {
    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<freeFunction>()
    );

    Mock callTarget;
    test.Reset<Mock, &Mock::ConstNotify>(&callTarget);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(555);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 555);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, CanResetDelegateToDifferentConstObjectMethodOnConstObject) {
    Delegate<void(int something)> test = (
      Delegate<void(int something)>::Create<freeFunction>()
    );

    const Mock callTarget;
    test.Reset<Mock, &Mock::ConstNotify>(&callTarget);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 0);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 0U);

    test(555);

    EXPECT_EQ(callTarget.LastSomethingParameterValue, 555);
    EXPECT_EQ(callTarget.ReceivedNotificationCount, 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, EmptyDelegateEqualsEmptyDelegate) {
    Delegate<void(int something)> a, b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, FreeFunctionDelegatesCanBeCompared) {
    Delegate<void(int *something)> a = (
      Delegate<void(int *something)>::Create<setIntegerFunction>()
    );
    Delegate<void(int *something)> b = (
      Delegate<void(int *something)>::Create<setIntegerFunction>()
    );
    Delegate<void(int *something)> x = (
      Delegate<void(int *something)>::Create<setOtherIntegerFunction>()
    );
    Delegate<void(int *something)> empty;

    // Both delegates invoking the same free function are equal
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(b == a);
    EXPECT_FALSE(b != a);

    // Delegates invoking different free functions are not equal
    EXPECT_FALSE(b == x);
    EXPECT_TRUE(b != x);

    // Delegates invoking a free function aren't equal to empty delegates
    EXPECT_FALSE(b == empty);
    EXPECT_TRUE(b != empty);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, StaticClassMethodDelegatesCanBeCompared) {
    Delegate<void(int something)> a = (
      Delegate<void(int something)>::Create<&StaticMock::Notify>()
    );
    Delegate<void(int something)> b = (
      Delegate<void(int something)>::Create<&StaticMock::Notify>()
    );
    Delegate<void(int something)> x = (
      Delegate<void(int something)>::Create<&StaticMock::Dummy>()
    );
    Delegate<void(int something)> empty;

    // Both delegates invoking the same free function are equal
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(b == a);
    EXPECT_FALSE(b != a);

    // Delegates invoking different free functions are not equal
    EXPECT_FALSE(b == x);
    EXPECT_TRUE(b != x);

    // Delegates invoking a free function aren't equal to empty delegates
    EXPECT_FALSE(b == empty);
    EXPECT_TRUE(b != empty);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, ObjectMethodDelegatesCanBeCompared) {
    Mock firstTarget, secondTarget;

    Delegate<void(int something)> a = (
      Delegate<void(int something)>::Create<Mock, &Mock::Notify>(&firstTarget)
    );
    Delegate<void(int something)> b = (
      Delegate<void(int something)>::Create<Mock, &Mock::Notify>(&firstTarget)
    );
    Delegate<void(int something)> x = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&firstTarget)
    );
    Delegate<void(int something)> y = (
      Delegate<void(int something)>::Create<Mock, &Mock::Notify>(&secondTarget)
    );
    Delegate<void(int something)> empty;

    // Both delegates invoking the same object method are equal
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(b == a);
    EXPECT_FALSE(b != a);

    // Delegates calling a different method on the same object are not equal
    EXPECT_FALSE(a == x);
    EXPECT_TRUE(a != x);

    // Delegates calling the same method on a different object are not equal
    EXPECT_FALSE(a == y);
    EXPECT_TRUE(a != y);

    // Delegates invoking an object method aren't the same as empty delegates
    EXPECT_FALSE(a == empty);
    EXPECT_TRUE(a != empty);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, ConstObjectMethodDelegatesCanBeCompared) {
    Mock firstTarget, secondTarget;

    Delegate<void(int something)> a = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&firstTarget)
    );
    Delegate<void(int something)> b = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&firstTarget)
    );
    Delegate<void(int something)> x = (
      Delegate<void(int something)>::Create<Mock, &Mock::Notify>(&firstTarget)
    );
    Delegate<void(int something)> y = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&secondTarget)
    );
    Delegate<void(int something)> empty;

    // Both delegates invoking the same object method are equal
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(b == a);
    EXPECT_FALSE(b != a);

    // Delegates calling a different method on the same object are not equal
    EXPECT_FALSE(a == x);
    EXPECT_TRUE(a != x);

    // Delegates calling the same method on a different object are not equal
    EXPECT_FALSE(a == y);
    EXPECT_TRUE(a != y);

    // Delegates invoking an object method aren't the same as empty delegates
    EXPECT_FALSE(a == empty);
    EXPECT_TRUE(a != empty);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, ConstMethodDelegateMatchesBetweenConstAndNonConstInstances) {
    Mock callTarget;

    Mock &nonConstTarget = callTarget;
    const Mock &constTarget = callTarget;

    Delegate<void(int something)> a = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&nonConstTarget)
    );
    Delegate<void(int something)> b = (
      Delegate<void(int something)>::Create<Mock, &Mock::ConstNotify>(&constTarget)
    );

    // Both delegates invoking the same object method are equal, even if once is
    // subscribes to a const object and the other to the same object, but non-const
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(b == a);
    EXPECT_FALSE(b != a);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DelegateTest, OnlyEmptyDelegateHasNoValue) {
    Mock callTarget;
    Delegate<void(int something)> test;

    EXPECT_TRUE(!test);
    EXPECT_FALSE(test.HasTarget());

    test.Reset<Mock, &Mock::Notify>(&callTarget);
    EXPECT_FALSE(!test);
    EXPECT_TRUE(test.HasTarget());

    test.Reset<&StaticMock::Dummy>();
    EXPECT_FALSE(!test);
    EXPECT_TRUE(test.HasTarget());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
