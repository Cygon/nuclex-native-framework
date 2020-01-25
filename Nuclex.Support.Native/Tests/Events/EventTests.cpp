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

#include "Nuclex/Support/Events/Event.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Free function used to test event subscriptions</summary>
  void freeFunction(int) { }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Free function that returns an integral value for testing</summary>
  int getSenseOfLife() { return 42; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy class used to test event subscriptions</summary>
  class Mock {

    /// <summary>Initializes a new mocked subscriber</summary>
    public: Mock() :
      ReceivedNotificationCount(0),
      LastSomethingParameterValue(0),
      ToUnsubscribe(nullptr),
      ToSubscribe(nullptr) {}

    /// <summary>Method that can be subscribed to an event for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void Notify(int something) {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;

      if(this->ToUnsubscribe != nullptr) {
        this->ToUnsubscribe->Unsubscribe<Mock, &Mock::Notify>(this);
        this->ToUnsubscribe = nullptr;
      }
      if(this->ToSubscribe != nullptr) {
        this->ToSubscribe->Subscribe<Mock, &Mock::Notify>(this);
        this->ToSubscribe = nullptr;
      }
    }

    /// <summary>Method that can be subscribed to an event for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void ConstNotify(int something) const {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
    }

    /// <summary>Number of calls to Notify() the instance has observed</summary>
    public: mutable std::size_t ReceivedNotificationCount;
    /// <summary>Value that was last passed to the Notify() method</summary>
    public: mutable int LastSomethingParameterValue;

    /// <summary>When set, unsubscribes the Notify() method inside the event call</summary>
    /// <remarks>
    ///   Event subscribers are allowd to unsubscribe themselves from within the
    ///   notification callback. This is used to test that scenario.
    /// </remarks>
    public: Nuclex::Support::Events::Event<void(int something)> *ToUnsubscribe;

    /// <summary>When set, subscribes the Notify() method inside the event call</summary>
    /// <remarks>
    ///   Event subscribers are allowd to subscribe themselves or others from within
    ///   the notification callback. This is used to test that scenario.
    /// </remarks>
    public: Nuclex::Support::Events::Event<void(int something)> *ToSubscribe;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EventsCanBeCreated) {
    EXPECT_NO_THROW(
      Event<void(int something)> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, FreeFunctionsCanBeSubscribed) {
    Event<void(int something)> test;
    test.Subscribe<freeFunction>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EventCanHandleManySubscriptions) {
    Event<void(int something)> test;
    for(std::size_t index = 0; index < 32; ++index) {
      test.Subscribe<freeFunction>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, FreeFunctionsCanBeUnsubscribed) {
    Event<void(int something)> test;
    test.Subscribe<freeFunction>();
    
    bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EachSubscriptionRequiresOneUnsubscription) {
    Event<void(int something)> test;

    for(std::size_t index = 0; index < 32; ++index) {
      test.Subscribe<freeFunction>();
    }

    for(std::size_t index = 0; index < 32; ++index) {
      bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
      EXPECT_TRUE(wasUnsubscribed);
    }

    bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ObjectMethodsCanBeSubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ObjectMethodsCanBeUnsubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeSubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeUnsubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }


  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeSubscribedOnConstInstance) {
    Event<void(int something)> test;

    const Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeUnsubscribedOnConstInstance) {
    Event<void(int something)> test;

    const Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, NotificationsAreSentToSubscribers) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);

    EXPECT_EQ(mock.ReceivedNotificationCount, 0U);
    EXPECT_EQ(mock.LastSomethingParameterValue, 0);

    test(135);

    EXPECT_EQ(mock.ReceivedNotificationCount, 1U);
    EXPECT_EQ(mock.LastSomethingParameterValue, 135);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);

    test(135);

    EXPECT_EQ(mock.ReceivedNotificationCount, 1U);
    EXPECT_EQ(mock.LastSomethingParameterValue, 135);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, SubscribersCanUnsubscribeInsideEventCall) {
    const static std::size_t MockCount = 32;

    // This is a somewhat complex test. We create 32 subscribers, fire the event
    // 32 times and each time it fires, one subscriber will unregister itself from
    // inside the event call (this is supported). This guarantees the event will
    // have to switch from stack to heap storage and - most importantly - back,
    // while it is being fired!

    Event<void(int something)> test;
    Mock mocks[MockCount];

    // Subscribe all mocks to the event
    for(std::size_t index = 0; index < MockCount; ++index) {
      test.Subscribe<Mock, &Mock::Notify>(&mocks[index]);
    }
    for(std::size_t index = 0; index < MockCount; ++index) {
      EXPECT_EQ(mocks[index].ReceivedNotificationCount, 0U);
      EXPECT_EQ(mocks[index].LastSomethingParameterValue, 0);
    }

    // Send out notifications, each time telling one mock to unsubscribe itself
    // during the event call
    for(std::size_t repetition = 0; repetition < MockCount; ++repetition) {
      mocks[repetition].ToUnsubscribe = &test;
      test(static_cast<int>(repetition + 99));

      // Check that the outcome is as expected
      for(std::size_t index = 0; index < MockCount; ++index) {
        if(index < repetition) { // If this is an unsubscribed mock
          EXPECT_EQ(mocks[index].ReceivedNotificationCount, index + 1);
          EXPECT_EQ(
            mocks[index].LastSomethingParameterValue,
            static_cast<int>(index + 99)
          );
        } else { // It's still subscribed
          EXPECT_EQ(mocks[index].ReceivedNotificationCount, repetition + 1);
          EXPECT_EQ(mocks[index].LastSomethingParameterValue, static_cast<int>(repetition + 99));
        }
      }

      // The relevant mock should have been unsubscribed by now
      bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mocks[repetition]);
      EXPECT_FALSE(wasUnsubscribed);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, SubscribersCanSubscribeInsideEventCall) {
    const static std::size_t MockCount = 32;

    // Another somewhat complex test. We create 32 subscribers, fire the event
    // 32 times and each time it fires, one subscriber will register itself an
    // additional time from inside the event call (this is supported).
    // This guarantees the event will have to switch from stack to heap storage,
    // while it is being fired!

    Event<void(int something)> test;
    Mock mocks[MockCount];

    // Subscribe all mocks to the event
    for(std::size_t index = 0; index < MockCount; ++index) {
      test.Subscribe<Mock, &Mock::Notify>(&mocks[index]);
    }
    for(std::size_t index = 0; index < MockCount; ++index) {
      EXPECT_EQ(mocks[index].ReceivedNotificationCount, 0U);
      EXPECT_EQ(mocks[index].LastSomethingParameterValue, 0);
    }

    // Send out notifications, each time telling one mock to subscribe itself
    // an additional time during the event call
    for(std::size_t repetition = 0; repetition < MockCount; ++repetition) {
      mocks[repetition].ToSubscribe = &test;
      test(234);

      // Check that the outcome is as expected
      for(std::size_t index = 0; index < MockCount; ++index) {
        if(index <= repetition) { // If this is an unsubscribed mock
          // Can be this or this + 1, even may or may not invoke subscribers that
          // are added during event firing in the same firing cycle.
          std::size_t expectedCallCount = (repetition - index) + repetition + 1;
          EXPECT_GT(mocks[index].ReceivedNotificationCount, expectedCallCount);
          EXPECT_LE(mocks[index].ReceivedNotificationCount, expectedCallCount + 1);
        } else { // It's still subscribed
          EXPECT_EQ(mocks[index].ReceivedNotificationCount, repetition + 1);
        }
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ValuesReturnedBySubscribersAreCollected) {
    const static std::size_t SubscriberCount = 16;

    Event<int()> test;

    // Subscribe a bunch of callbacks to the event
    for(std::size_t index = 0; index < SubscriberCount; ++index) {
      test.Subscribe<getSenseOfLife>();
    }

    // Fire the event and collect the results
    std::vector<int> results = test();

    ASSERT_EQ(results.size(), SubscriberCount);
    for(std::size_t index = 0; index < SubscriberCount; ++index) {
      EXPECT_EQ(results[index], getSenseOfLife());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, SubscriberReturnValuesCanBeDiscarded) {
    const static std::size_t SubscriberCount = 16;

    Event<int()> test;

    // Subscribe a bunch of callbacks to the event
    for(std::size_t index = 0; index < SubscriberCount; ++index) {
      test.Subscribe<getSenseOfLife>();
    }

    // EmitAndCollect() stores return values, Emit() directly throws them away
    test.Emit();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ReturnValueListIsEmptyWithoutSubscribers) {
    Event<int()> test;
    std::vector<int> results = test();

    EXPECT_EQ(results.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, SubscriberReturnValuesCanBeCollectedWithoutAllocating) {
    const static std::size_t SubscriberCount = 16;

    // Vector into which the return values are written. This could be reused
    // from call to call, thus eliminating a heap allocation to provide the results.
    std::vector<int> results;
    results.reserve(SubscriberCount);

    Event<int()> test;

    // Subscribe a bunch of callbacks to the event
    for(std::size_t index = 0; index < SubscriberCount; ++index) {
      test.Subscribe<getSenseOfLife>();
    }

    // First argument to EmitAndCollect() is an output iterator
    test.EmitAndCollect(std::back_inserter(results));

    ASSERT_EQ(results.size(), SubscriberCount);
    for(std::size_t index = 0; index < SubscriberCount; ++index) {
      EXPECT_EQ(results[index], getSenseOfLife());
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
