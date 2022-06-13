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

#ifndef NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
#define NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Delegate.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <atomic> // for std::atomic
#include <algorithm> // for std::copy_n()
#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  // Prototype, required for variable argument template
  template<typename> class ConcurrentEvent;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a list of subscribers that receive callbacks when the event fires</summary>
  /// <typeparam name="TResult">Type of results the callbacks will return</typeparam>
  /// <typeparam name="TArguments">Types of the arguments accepted by the callback</typeparam>
  /// <remarks>
  ///   <para>
  ///     This is a special variant of the <see cref="Nuclex.Support.Events.Event" /> class,
  ///     a very lean signal/slot implementation. Whereas the normal event attempts to achieve
  ///     maximum performance and minimum resource use in a single-threaded scenario,
  ///     the concurrent event attempts the same while allowing free-threaded use.
  ///   </para>
  ///   <para>
  ///     Like the single-threaded event, it assumes granular use, meaning you create many
  ///     individual events rather than one big multi-purpose notification. It also assumes that
  ///     events typically have only a small number of subscribers and that firing will happen
  ///     vastly more often than subscription/unsubscription.
  ///   </para>
  ///   <para>
  ///     This concurrent event implementation can be freely used from any thread, including
  ///     simultaneous firing, subscription and unsubscription without any synchronization on
  ///     the side the user of the event. Depending on your platform and C++ standard library,
  ///     firing could be wait-free, but likely will use a spinlock around a piece of code
  ///     covering just a few CPU cyles (two instructions ideally).
  ///   </para>
  ///   <para>
  ///     A concurrent event should be equivalent in size to 1 shared_ptr on its own.
  ///     It does not allocate any memory upon construction or firing, but will allocate
  ///     a single memory block each time callbacks are subscribed or unsubscribed. Said memory
  ///     block is the size of the std::shared_ptr reference count + two pointers + two more
  ///     pointers per subscriber (typically 64 bytes + 16 bytes per subscriber).
  ///   </para>
  ///   <para>
  ///     Usage example:
  ///   </para>
  ///   <para>
  ///     <code>
  ///       int Dummy(int first, std::string second) { return 123; }
  ///
  ///       class Mock {
  ///         public: int Dummy(int first, std::string second) { return 456; }
  ///       };
  ///
  ///       int main() {
  ///         typedef ConcurrentEvent&lt;int(int foo, std::string bar)&gt; FooBarEvent;
  ///
  ///         FooBarEvent test;
  ///
  ///         // Subscribe the dummy function
  ///         test.Subscribe&lt;Dummy&gt;();
  ///
  ///         // Subscribe an object method
  ///         Mock myMock;
  ///         test.Subscribe&lt;Mock, &Mock::Dummy&gt;(&amp;myMock);
  ///
  ///         // Fire the event
  ///         std::vector&lt;int&gt; returnedValues = test(123, u8"Hello");
  ///
  ///         // Fire the event again but don't collect returned values
  ///         test.Emit(123, u8"Hello");
  ///       }
  ///     </code>
  ///   </para>
  /// </remarks>
  template<typename TResult, typename... TArguments>
  class ConcurrentEvent<TResult(TArguments...)> {

    /// <summary>Type of value that will be returned by the delegate</summary>
    public: typedef TResult ResultType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    public: typedef TResult CallType(TArguments...);
    /// <summary>Type of delegate used to call the event's subscribers</summary>
    public: typedef Delegate<TResult(TArguments...)> DelegateType;

    /// <summary>List of results returned by subscribers</summary>
    /// <remarks>
    ///   Having an std::vector&lt;void&gt; anywhere, even in a SFINAE-disabled method,
    ///   will trigger deprecation compiler warnings on Microsoft compilers.
    ///   Consider this type to be an alias for std::vector&lt;TResult&gt; and nothing else.
    /// </remarks>
    private: typedef std::vector<
      typename std::conditional<std::is_void<TResult>::value, char, TResult>::type
    > ResultVectorType;

    #pragma region struct BroadcastQueue

    /// <summary>Queue of subscribers to which the event will be broadcast</summary>
    private: struct BroadcastQueue {

      /// <summary>
      ///   Initializes a new broadcast queue for the specified number of subscribers
      /// </summary>
      /// <param name="count">
      ///   Number of subscribers the broadcast queue will be initialized for
      /// </param>
      /// <remarks>
      ///   The reference count is initialized to one since it would be pointless to create
      ///   an instance and then have to always run an extra increment operation.
      /// </remarks>
      public: BroadcastQueue(std::size_t count) :
        SubscriberCount(count),
        Subscribers(/* leave undefined! */) {}

      /// <summary>Frees all memory owned by the broadcast queue</summary>
      public: ~BroadcastQueue() = default;

      // Ensure the queue isn't copied or moved with default semantics
      BroadcastQueue(const BroadcastQueue &other) = delete;
      BroadcastQueue(BroadcastQueue &&other) = delete;
      void operator =(const BroadcastQueue &other) = delete;
      void operator =(BroadcastQueue &&other) = delete;

      /// <summary>Number of subscribers stored in the array</summary>
      public: std::size_t SubscriberCount;
      /// <summary>Plain array of all subscribers to which the event is broadcast</summary>
      public: DelegateType *Subscribers;

    };

    #pragma endregion // struct BroadcastQueue

    #pragma region struct BroadcastQueueAllocator

    /// <summary>Custom alloctor that allocates a broadcast queue and subscriber list</summary>
    /// <typeparam name="TElement">
    ///   Type of element that will be allocated together with a subscriber list
    /// </typeparam>
    /// <remarks>
    ///   Normally, a non-templated implementation of this allocator would seem to suffice,
    ///   but <code>std::allocate_shared()</code> implementations will very likely
    ///   (via the type-changing copy constructor) allocate a type inherited from our
    ///   <see cref="BroadcastQueue" /> that packages the reference counter of
    ///   the <code>std::shared_ptr</code> together with the instance.
    /// </remarks>
    template<class TElement>
    class BroadcastQueueAllocator {

      /// <summary>Type of element the allocator is for, required by standard</summary>
      public: typedef TElement value_type;

      /// <summary>Initializes a new allocator using the specified appended list size</summary>
      /// <param name="subscriberCount">Number of subscribers to allocate extra space for</param>
      public: BroadcastQueueAllocator(std::size_t subscriberCount) :
        subscriberCount(subscriberCount) {}

      /// <summary>
      ///   Creates this allocator as a clone of an allocator for a different type
      /// </summary>
      /// <typeparam name="TOther">Type the existing allocator is allocating for</typeparam>
      /// <param name="other">Existing allocator whose attributes will be copied</param>
      public: template<class TOther> BroadcastQueueAllocator(
        const BroadcastQueueAllocator<TOther> &other
      ) : subscriberCount(other.subscriberCount) {}

      /// <summary>Allocates memory for the specified number of elements (must be 1)</summary>
      /// <param name="count">Number of elements to allocato memory for (must be 1)</param>
      /// <returns>The allocated (but not initialized) memory for the required type</returns>
      public: TElement *allocate(std::size_t count) {
        constexpr std::size_t subscriberStartOffset = (
          sizeof(TElement) +
          (
            ((sizeof(TElement) % alignof(DelegateType *)) == 0) ?
            0 : // size happened to fit needed alignment of subscriber list
            (alignof(DelegateType *) - (sizeof(TElement) % alignof(TElement *)))
          )
        );

        // Number of bytes needed to allocate the requested type and a susbcriber list
        std::size_t requiredByteCount = (
          subscriberStartOffset + // Broadcast queue and padding for aligned subscriber list
          (sizeof(TElement[2]) * this->subscriberCount / 2) // Subscriber list
        );

        assert(count == 1);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(count);
        return reinterpret_cast<TElement *>(new std::uint8_t[requiredByteCount]);
      }

      /// <summary>Frees memory for the specified element (count must be 1)</summary>
      /// <param name="instance">Instance for which memory will be freed</param>
      /// <param name="count">Number of instances that will be freed (must be 1)</param>
      public: void deallocate(TElement *instance, std::size_t count) {
        assert(count == 1);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(count);
        delete[] reinterpret_cast<std::uint8_t *>(instance);
      }

      /// <summary>Number of subscribers for which extra space will be allocated</summary>
      public: std::size_t subscriberCount;

    };

    // template <class T, class U>
    // static bool operator==(const BroadcastQueueAllocator<T>&, const BroadcastQueueAllocator<U>&);
    // template <class T, class U>
    // static bool operator!=(const BroadcastQueueAllocator<T>&, const BroadcastQueueAllocator<U>&);

    #pragma endregion // struct BroadcastQueueAllocator

    /// <summary>Initializes a new concurrent event</summary>
    public: ConcurrentEvent() = default;
    /// <summary>Frees all memory used by a concurrent event</summary>
    public: ~ConcurrentEvent() = default;

    // TODO: Implement copy and move constructors + assignment operators

    /// <summary>Returns the current number of subscribers to the event</summary>
    /// <returns>The number of current subscribers</returns>
    public: std::size_t CountSubscribers() const {
      std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
        this->subscribers, std::memory_order::memory_order_consume // if carries dependency
      );
      if(likely(static_cast<bool>(currentQueue))) {
        return currentQueue->SubscriberCount;
      } else {
        return 0;
      }
    }

    /// <summary>Calls all subscribers of the event and collects their return values</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    /// <returns>An list of the values returned by the event subscribers</returns>
    /// <remarks>
    ///   This overload is enabled if the event signature returns anything other than 'void'.
    ///   The returned value is an std::vector&lt;TResult&gt; in all cases.
    /// </remarks>
    public: template<typename T = TResult>
    typename std::enable_if<
      !std::is_void<T>::value, ResultVectorType
    >::type operator()(TArguments&&... arguments) const {
      ResultVectorType results; // ResultVectorType is an alias for std::vector<TResult>

      std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
        &this->subscribers, std::memory_order::memory_order_consume // if() is dependency
      );
      if(static_cast<bool>(currentQueue)) {
        std::size_t subscriberCount = currentQueue->SubscriberCount;
        results.reserve(subscriberCount);
        for(std::size_t index = 0; index < subscriberCount; ++index) {
          results.push_back(currentQueue->Subscribers[index](std::forward<TArguments>(arguments)...));
          // We don't need to worry about queue edits within the callsbacks because
          // it will result in a new broadcast queue being placed while we happily
          // continue working with the copy in our std::shared_ptr.
        }
      }

      return results;
    }

    /// <summary>Calls all subscribers of the event</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    /// <remarks>
    ///   This overload is enabled if the event signature has the return type 'void'
    /// </remarks>
    public: template<typename T = TResult>
    typename std::enable_if<
      std::is_void<T>::value, void
    >::type operator()(TArguments&&... arguments) const {
      Emit(std::forward<TArguments>(arguments)...);
    }

    /// <summary>Calls all subscribers of the event and collects their return values</summary>
    /// <param name="results">
    ///   Output iterator into which the subscribers' return values will be written
    /// </param>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    public: template<typename TOutputIterator>
    void EmitAndCollect(TOutputIterator results, TArguments&&... arguments) const {
      std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
        &this->subscribers, std::memory_order::memory_order_consume // if() is dependency
      );
      if(static_cast<bool>(currentQueue)) {
        std::size_t subscriberCount = currentQueue->SubscriberCount;
        for(std::size_t index = 0; index < subscriberCount; ++index) {
          *results = currentQueue->Subscribers[index](std::forward<TArguments>(arguments)...);
          // We don't need to worry about queue edits within the callsbacks because
          // it will result in a new broadcast queue being placed while we happily
          // continue working with the copy in our std::shared_ptr.
          ++results;
        }
      }
    }

    /// <summary>Calls all subscribers of the event and discards their return values</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    public: void Emit(TArguments... arguments) const {
      std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
        &this->subscribers, std::memory_order::memory_order_consume // if() is dependency
      );
      if(static_cast<bool>(currentQueue)) {
        std::size_t subscriberCount = currentQueue->SubscriberCount;
        for(std::size_t index = 0; index < subscriberCount; ++index) {
          currentQueue->Subscribers[index](std::forward<TArguments>(arguments)...);
          // We don't need to worry about queue edits within the callsbacks because
          // it will result in a new broadcast queue being placed while we happily
          // continue working with the copy in our std::shared_ptr.
        }
      }
    }

    /// <summary>Subscribes the specified free function to the event</summary>
    /// <typeparam name="TMethod">Free function that will be subscribed</typeparam>
    public: template<TResult(*TMethod)(TArguments...)>
    void Subscribe() {
      Subscribe(DelegateType::template Create<TMethod>());
    }

    /// <summary>Subscribes the specified object method to the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Object method that will be subscribed to the event</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    void Subscribe(TClass *instance) {
      Subscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Subscribes the specified const object method to the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Object method that will be subscribed to the event</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...) const>
    void Subscribe(const TClass *instance) {
      Subscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Subscribes the specified delegate to the event</summary>
    /// <param name="delegate">Delegate that will be subscribed</param>
    public: void Subscribe(const DelegateType &delegate) {

      // This is a C-A-S replacement attempt, so we may have to go through the whole opration
      // multiple times. We expect this to be the case only very rarely, as contention should
      // happen when events are fired, not by threads subscribing & unsubscribing rapidly.
      for(;;) {
        std::shared_ptr<const BroadcastQueue> newQueue;

        std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
          &this->subscribers, std::memory_order::memory_order_consume // if carries dependency
        );
        if(currentQueue == nullptr) { // There was no previous subscriber list
          newQueue = allocateBroadcastQueue(1);
          new(newQueue->Subscribers) DelegateType(delegate);
        } else { // Non-empty subscriber list present, create clone with an extra entry
          std::size_t currentSubscriberCount = currentQueue->SubscriberCount;
          newQueue = allocateBroadcastQueue(currentSubscriberCount + 1);
          std::copy_n(currentQueue->Subscribers, currentSubscriberCount, newQueue->Subscribers);
          new(newQueue->Subscribers + currentSubscriberCount) DelegateType(delegate);
        }

        // Try to replace the current (null pointer) queue with our new one
        bool wasReplaced = std::atomic_compare_exchange_strong(
          &this->subscribers, &currentQueue, newQueue
        );
        if(wasReplaced) {
          break;
        }
      } // C-A-S loop

    }

    /// <summary>Unsubscribes the specified free function from the event</summary>
    /// <typeparam name="TMethod">
    ///   Free function that will be unsubscribed from the event
    /// </typeparam>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<TResult(*TMethod)(TArguments...)>
    bool Unsubscribe() {
      return Unsubscribe(DelegateType::template Create<TMethod>());
    }

    /// <summary>Unsubscribes the specified object method from the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">
    ///   Object method that will be unsubscribes from the event
    /// </typeparam>
    /// <param name="instance">Instance on which the object method was subscribed</param>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    bool Unsubscribe(TClass *instance) {
      return Unsubscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Unsubscribes the specified object method from the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">
    ///   Object method that will be unsubscribes from the event
    /// </typeparam>
    /// <param name="instance">Instance on which the object method was subscribed</param>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...) const>
    bool Unsubscribe(const TClass *instance) {
      return Unsubscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Unsubscribes the specified delegate from the event</summary>
    /// <param name="delegate">Delegate that will be unsubscribed</param>
    /// <returns>True if the callback was found and unsubscribed, false otherwise</returns>
    public: bool Unsubscribe(const DelegateType &delegate) {

      // This is a C-A-S replacement attempt, so we may have to go through the whole opration
      // multiple times. We expect this to be the case only very rarely, as contention should
      // happen when events are fired, not by threads subscribing & unsubscribing rapidly.
      for(;;) {

        // Grab the current queue in order to create a modified clone of it
        std::shared_ptr<const BroadcastQueue> currentQueue = std::atomic_load_explicit(
          &this->subscribers, std::memory_order::memory_order_consume // if carries dependency
        );
        if(unlikely(currentQueue == nullptr)) {
          return false; // Nothing we can do, there were no subscribers at all...
        }

        // Hunt for the delegate the caller wishes to unsubscribe
        std::size_t currentSubscriberCount = currentQueue->SubscriberCount;
        std::size_t index = 0;
        for(;;) {
          if(currentQueue->Subscribers[index] == delegate) {
            std::shared_ptr<const BroadcastQueue> newQueue;
            if(currentSubscriberCount > 1) {
              --currentSubscriberCount;
              newQueue = allocateBroadcastQueue(currentSubscriberCount);
              std::copy_n(currentQueue->Subscribers, index, newQueue->Subscribers);
              std::copy_n(
                currentQueue->Subscribers + index + 1,
                currentSubscriberCount - index,
                newQueue->Subscribers + index
              );
            }

            // Try to replace the current queue with our modified clone
            bool wasReplaced = std::atomic_compare_exchange_strong(
              &this->subscribers, &currentQueue, newQueue
            );
            if(wasReplaced) {
              return true; // Edited version of broadcast queue is in place, we're done
            } else {
              break; // Someone else edited the broadcast queue, repeat outer C-A-S loop
            }
          } // if delegate matched

          ++index;
          if(index == currentSubscriberCount) {
            return false; // Loop completed without finding the delegate
          }
        } // delegate search loop
      } // C-A-S loop
    }

    /// <summary>
    ///   Allocates a new broadcast queue for the specified number of subscribers
    /// </summary>
    /// <param name="subscriberCount">Number of subscribers the queue should hold</param>
    /// <returns>A new broadcast queue with an uninitialized subscriber list</returns>
    private: static std::shared_ptr<const BroadcastQueue> allocateBroadcastQueue(
      std::size_t subscriberCount
    ) {
      constexpr std::size_t subscriberStartOffset = (
        sizeof(BroadcastQueue) +
        (
          ((sizeof(BroadcastQueue) % alignof(DelegateType *)) == 0) ?
          0 : // size happened to fit needed alignment of subscriber list
          (alignof(DelegateType *) - (sizeof(BroadcastQueue) % alignof(BroadcastQueue *)))
        )
      );

      BroadcastQueueAllocator<BroadcastQueue> queueAllocator(subscriberCount);
      std::shared_ptr<BroadcastQueue> newQueue = (
        std::allocate_shared<BroadcastQueue>(queueAllocator, subscriberCount)
      );

      newQueue->Subscribers = reinterpret_cast<DelegateType *>(
        reinterpret_cast<std::uint8_t *>(newQueue.get()) + subscriberStartOffset
      );

      return newQueue;
    }

    /// <summary>Stores the current subscribers to the event</summary>
    /// <remarks>
    ///   This shared_ptr is written to by potentially multiple threads, use atomic
    ///   operations to access it!
    /// </remarks>
    public: /* atomic */ std::shared_ptr<const BroadcastQueue> subscribers;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
