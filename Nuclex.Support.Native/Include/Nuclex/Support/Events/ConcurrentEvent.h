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
#include "Nuclex/Support/ScopeGuard.h"
#include "Nuclex/Support/BitTricks.h"

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
  ///     pointers per subscriber (overall typically 64 bytes + 16 bytes per subscriber).
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

    /// <summary>Initializes a new concurrent event</summary>
    public: ConcurrentEvent() :
      spinLock(false),
      subscribers(nullptr),
      recyclableSubscribers(nullptr) {}

    /// <summary>Frees all memory used by a concurrent event</summary>
    public: ~ConcurrentEvent();

    // TODO: Implement copy and move constructors + assignment operators

    /// <summary>Returns the current number of subscribers to the event</summary>
    /// <returns>The number of current subscribers</returns>
    public: std::size_t CountSubscribers() const;

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
    >::type operator()(TArguments&&... arguments) const;

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
    void EmitAndCollect(TOutputIterator results, TArguments&&... arguments) const;

    /// <summary>Calls all subscribers of the event and discards their return values</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    public: void Emit(TArguments... arguments) const;

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
    public: void Subscribe(const DelegateType &delegate);

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
    public: bool Unsubscribe(const DelegateType &delegate);

    #pragma region struct BroadcastQueue

    /// <summary>Queue of subscribers to which the event will be broadcast</summary>
    private: struct BroadcastQueue {

      /// <summary>
      ///   Initializes a new broadcast queue for the specified number of subscribers
      /// </summary>
      /// <param name="capacity">
      ///   Number of subscribers the broadcast queue can contain at most
      /// </param>
      /// <param name="count">
      ///   Number of subscribers the broadcast queue will contain
      /// </param>
      /// <remarks>
      ///   The reference count is initialized to one since it would be pointless to create
      ///   an instance and then have to always run an extra increment operation.
      /// </remarks>
      public: BroadcastQueue(std::size_t capacity, std::size_t count) noexcept :
        ReferenceCount(1),
        Capacity(capacity),
        Count(count),
        Callbacks(/* leave undefined! */) {}

      /// <summary>Frees all memory owned by the broadcast queue</summary>
      public: ~BroadcastQueue() = default;

      /// <summary>Number of references that exist to the instance</summary>
      public: mutable std::atomic<std::size_t> ReferenceCount;
      /// <summary>Number of subscribers for which memory has been allocated</summary>
      public: std::size_t Capacity;
      /// <summary>Number of subscribers stored in the array</summary>
      public: std::size_t Count;
      /// <summary>Plain array of all subscribers to which the event is broadcast</summary>
      public: DelegateType *Callbacks;

    };

    #pragma endregion // struct BroadcastQueue

    /// <summary>
    ///   Allocates a new broadcast queue for the specified number of subscribers
    /// </summary>
    /// <param name="subscriberCount">Number of subscribers the queue should hold</param>
    /// <returns>A new broadcast queue with an uninitialized subscriber list</returns>
    private: BroadcastQueue *allocateBroadcastQueue(std::size_t subscriberCount) {
      constexpr std::size_t subscriberStartOffset = (
        sizeof(BroadcastQueue) +
        (
          ((sizeof(BroadcastQueue) % alignof(DelegateType)) == 0) ?
          0 : // size happened to fit needed alignment of subscriber list
          (alignof(DelegateType) - (sizeof(BroadcastQueue) % alignof(DelegateType)))
        )
      );

      std::size_t capacity;
      if(subscriberCount < 4) {
        capacity = 4;
      } else {
        capacity = BitTricks::GetUpperPowerOfTwo(subscriberCount);
      }

      std::uint8_t *memory = new std::uint8_t[
        subscriberStartOffset + (sizeof(DelegateType[2]) * capacity / 2)
      ];

      BroadcastQueue *newQueue = new(memory) BroadcastQueue(capacity, subscriberCount);
      newQueue->Callbacks = reinterpret_cast<DelegateType *>(memory + subscriberStartOffset);
      //static_assert(std::is_trivially_constructible<DelegateType>::value);

      return newQueue;
    }

    /// <summary>Frees all memory owned by a broadcast queue</summary>
    /// <param name="queue">Queue whose memory will be freed</param>
    private: static void freeBroadcastQueue(BroadcastQueue *queue) noexcept {
      queue->~BroadcastQueue();
      //static_assert(std::is_trivially_constructible<BroadcastQueue>::value);
      //static_assert(std::is_trivially_constructible<DelegateType>::value);
      delete[] reinterpret_cast<const std::uint8_t *>(queue);
    }


    /// <summary>Acquires the spinlock to access the subscriber queues</summary>
    /// <remarks>
    ///   See https://rigtorp.se/spinlock/
    /// </remarks>
    private: inline void acquireSpinLock() const noexcept {
      for(;;) {

        // Optimistically assume the lock is free on the first try
        if(!this->spinLock.exchange(true, std::memory_order::memory_order_acquire)) {
          return;
        }

        // Wait for lock to be released without generating cache misses
        while(this->spinLock.load(std::memory_order::memory_order_relaxed)) {
          // Issue X86 PAUSE or ARM YIELD instruction to reduce contention
          // between hyper-threads
#if defined(_MSC_VER) // MSVC
          _mm_pause();
#else // GCC and clang
          __builtin_ia32_pause();
#endif
        }

      } // for(;;)
    }

    /// <summary>Releases the spinlock again</summary>
    private: inline void releaseSpinLock() const noexcept {
      this->spinLock.store(false, std::memory_order::memory_order_release);
    }

    /// <summary>Micro-spinlock to synchronize access to the subscriber list + refcount</summary>
    /// <remarks>
    ///   Yes, this is used for a busy loop, just like modern mutex implementations (for
    ///   a few thousand cycles at least) and std::atomic&lt;std::shared_ptr&lt;&gt;&gt;.
    ///   We use it as a micro-spinlock only guarding 2-5 instructions, so contention should
    ///   be really low.
    /// </remarks>
    public: mutable std::atomic<bool> spinLock;
    /// <summary>Stores the current subscribers to the event</summary>
    public: std::atomic<BroadcastQueue *> subscribers;
    /// <summary>Stores the previous subscriber list to the event</summary>
    /// <remarks>
    ///   In order to avoid needless allocations, the queue keeps the previous subscriber list
    ///   around for reuse. This wastes a little bit of memory but speeds things up drastically
    ///   if an event has massive numbers of subscribers.
    /// </remarks>
    public: mutable std::atomic<BroadcastQueue *> recyclableSubscribers;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  ConcurrentEvent<TResult(TArguments...)>::~ConcurrentEvent() {

    // Don't care about the spinlock, if the event is being destroyed, nobody is accessing
    // it anymore (and if it was, it'll be a race between a thread and the destructor of
    // the class that owns the event, this humble destructor can do little about that anyway)

    BroadcastQueue *currentQueue = this->subscribers.load(
      std::memory_order::memory_order_relaxed
    );
    if(currentQueue != nullptr) {
      freeBroadcastQueue(currentQueue);
    }

    currentQueue = this->recyclableSubscribers.load(
      std::memory_order::memory_order_relaxed
    );
    if(currentQueue != nullptr) {
      freeBroadcastQueue(currentQueue);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  std::size_t ConcurrentEvent<TResult(TArguments...)>::CountSubscribers() const {
    acquireSpinLock();

    const BroadcastQueue *currentQueue = this->subscribers.load(
      std::memory_order::memory_order_consume // if carries dependency
    );
    if(unlikely(currentQueue == nullptr)) {
      releaseSpinLock();
      return 0;
    } else {
      std::size_t subscriberCount = currentQueue->Count;
      releaseSpinLock();
      return subscriberCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  template<typename T>
  typename std::enable_if<
    !std::is_void<T>::value, typename ConcurrentEvent<TResult(TArguments...)>::ResultVectorType
  >::type ConcurrentEvent<TResult(TArguments...)>::operator()(TArguments&&... arguments) const {
    ResultVectorType results; // ResultVectorType is an alias for std::vector<TResult>

    // Get a hold of the current queue. If there is no current queue, bail out
    // without touching anything else (anticipated zero-subscriber case)
    acquireSpinLock();
    BroadcastQueue *currentQueue = this->subscribers.load(
      std::memory_order::memory_order_consume
    );
    if(likely(currentQueue == nullptr)) {
      releaseSpinLock();
      return results;
    } else { // A queue is present, increment its reference count so it isn't deleted
      currentQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_release);
      releaseSpinLock();
    }

    // There are subscribers, so the event needs to be fired and we have incremented
    // the queue's reference counter, requiring us to decrement it again
    {
      // The spinlock does not need to be acquired here. If the queue is still assigned as
      // the active subscriber list, the reference counter will not reach 0. Otherwise,
      // it was already replaced by another thread, so we don't even need to check.
      ON_SCOPE_EXIT {
        std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
          1, std::memory_order::memory_order_release
        );
        if(unlikely(totalReferences == 1)) { // We just released the last reference
          currentQueue = this->recyclableSubscribers.exchange(currentQueue);
          if(likely(currentQueue != nullptr)) {
            freeBroadcastQueue(currentQueue);
          }
        }
      };

      // Actually fire the event by calling all the subscribers
      std::size_t subscriberCount = currentQueue->Count;
      results.reserve(subscriberCount);
      for(std::size_t index = 0; index < subscriberCount; ++index) {
        results.push_back(currentQueue->Callbacks[index](std::forward<TArguments>(arguments)...));
        // We don't need to worry about queue edits within the callsbacks because
        // it will result in a new broadcast queue being placed while we happily
        // continue working with the copy in our std::shared_ptr.
      }
      return results;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  template<typename TOutputIterator>
  void ConcurrentEvent<TResult(TArguments...)>::EmitAndCollect(
    TOutputIterator results, TArguments&&... arguments
  ) const {

    // Get a hold of the current queue. If there is no current queue, bail out
    // without touching anything else (anticipated zero-subscriber case)
    acquireSpinLock();
    BroadcastQueue *currentQueue = this->subscribers.load(
      std::memory_order::memory_order_consume
    );
    if(likely(currentQueue == nullptr)) {
      releaseSpinLock();
      return;
    } else { // A queue is present, increment its reference count so it isn't deleted
      currentQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_release);
      releaseSpinLock();
    }

    // There are subscribers, so the event needs to be fired and we have incremented
    // the queue's reference counter, requiring us to decrement it again
    {
      ON_SCOPE_EXIT {
        // The spinlock does not need to be acquired here. If the queue is still assigned as
        // the active subscriber list, the reference counter will not reach 0. Otherwise,
        // it was already replaced by another thread, so we don't even need to check.
        std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
          1, std::memory_order::memory_order_release
        );
        if(unlikely(totalReferences == 1)) { // We just released the last reference
          currentQueue = this->recyclableSubscribers.exchange(currentQueue);
          if(likely(currentQueue != nullptr)) {
            freeBroadcastQueue(currentQueue);
          }
        }
      };

      // Actually fire the event by calling all the subscribers
      std::size_t subscriberCount = currentQueue->Count;
      for(std::size_t index = 0; index < subscriberCount; ++index) {
        *results = currentQueue->Callbacks[index](std::forward<TArguments>(arguments)...);
        // We don't need to worry about queue edits within the callsbacks because
        // it will result in a new broadcast queue being placed while we happily
        // continue working with the copy in our std::shared_ptr.
        ++results;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  void ConcurrentEvent<TResult(TArguments...)>::Emit(TArguments... arguments) const {

    // Get a hold of the current queue.
    acquireSpinLock();
    BroadcastQueue *currentQueue = this->subscribers.load(
      std::memory_order::memory_order_consume
    );
    if(likely(currentQueue == nullptr)) {
      releaseSpinLock();
      return;
    } else { // A queue is present, increment its reference count so it isn't deleted
      currentQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_release);
      releaseSpinLock();
    }

    // There are subscribers, so the event needs to be fired and we have incremented
    // the queue's reference counter, requiring us to decrement it again
    {
      ON_SCOPE_EXIT {
        // The spinlock does not need to be acquired here. If the queue is still assigned as
        // the active subscriber list, the reference counter will not reach 0. Otherwise,
        // it was already replaced by another thread, so we don't even need to check.
        std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
          1, std::memory_order::memory_order_release
        );
        if(unlikely(totalReferences == 1)) { // We just released the last reference
          currentQueue = this->recyclableSubscribers.exchange(currentQueue);
          if(likely(currentQueue != nullptr)) {
            freeBroadcastQueue(currentQueue);
          }
        }
      };

      // Actually fire the event by calling all the subscribers
      std::size_t subscriberCount = currentQueue->Count;
      for(std::size_t index = 0; index < subscriberCount; ++index) {
        currentQueue->Callbacks[index](std::forward<TArguments>(arguments)...);
        // We don't need to worry about queue edits within the callsbacks because
        // it will result in a new broadcast queue being placed while we happily
        // continue working with the copy in our std::shared_ptr.
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  void ConcurrentEvent<TResult(TArguments...)>::Subscribe(const DelegateType &delegate) {

    // This is a C-A-S replacement attempt, so we may have to go through the whole opration
    // multiple times. We expect this to be the case only very rarely, as contention should
    // happen when events are fired, not by threads subscribing & unsubscribing rapidly.
    for(;;) {
      BroadcastQueue *newQueue;

      // Get a hold of the current queue.
      acquireSpinLock();
      BroadcastQueue *currentQueue = this->subscribers.load(
        std::memory_order::memory_order_consume
      );
      if(likely(currentQueue == nullptr)) {
        releaseSpinLock();

        // Try to recycle an earlier queue. If we get one, we don't need to check its
        // capacity because any queue created will have space for at least 1 subscriber.
        newQueue = this->recyclableSubscribers.exchange(nullptr);
        if(unlikely(newQueue == nullptr)) {
          newQueue = allocateBroadcastQueue(1);
        }

        newQueue->Callbacks[0] = delegate;
      } else { // A queue is present, increment its reference count so it isn't deleted
        currentQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_release);
        releaseSpinLock();

        ON_SCOPE_EXIT {
          // The spinlock does not need to be acquired here. If the queue is still assigned as
          // the active subscriber list, the reference counter will not reach 0. Otherwise,
          // it was already replaced by another thread, so we don't even need to check.
          std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
            1, std::memory_order::memory_order_release
          );
          if(unlikely(totalReferences == 1)) { // We just released the last reference
            currentQueue = this->recyclableSubscribers.exchange(currentQueue);
            if(likely(currentQueue != nullptr)) {
              freeBroadcastQueue(currentQueue);
            }
          }
        };

        // Obtain a new queue to fill the subscribers into, either by re-=using the event's
        // previous queue or by creating a new one
        std::size_t subscriberCount = currentQueue->Count;
        newQueue = this->recyclableSubscribers.exchange(nullptr);
        if(unlikely(newQueue == nullptr)) { // No previous queue available?
          newQueue = allocateBroadcastQueue(subscriberCount + 1);
        } else if(unlikely(subscriberCount >= newQueue->Capacity)) { // Not enough capacity?
          freeBroadcastQueue(newQueue);
          newQueue = allocateBroadcastQueue(subscriberCount + 1);
        } else { // Recycled queue can be reused, raise its reference count
          newQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_relaxed);
          newQueue->Count = subscriberCount + 1;
        }

        // This section is at-risk of leaking memory if it throws. I don't think a memcpy()
        // equivalent call can throw, but if I'm mistaking, we need another scope guard here.
        std::copy_n(currentQueue->Callbacks, subscriberCount, newQueue->Callbacks);
        newQueue->Callbacks[subscriberCount] = delegate;
      } // else block, also decrements active queue's reference count upon scope xit

      // Try to replace the active queue with our modified clone. Yes we're acquiring
      // the spinlock for an atomic operation. Why? The fetch operation (load + fetch_add)
      // must be atomic (thus we wrap the two operations in a spinlock) and those two
      // operations must not be interrupted by this single C-A-S instruction, so we hold
      // the spinlock here, thereby preventing any acquiring threads from grabbing
      // the reference before we decrement the reference counter to possibly 0.
      acquireSpinLock();
      bool wasReplaced = std::atomic_compare_exchange_strong(
        &this->subscribers, &currentQueue, newQueue
      );
      releaseSpinLock();

      if(likely(wasReplaced)) {
        if(likely(currentQueue != nullptr)) {
          std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
            1, std::memory_order::memory_order_release
          );
          if(unlikely(totalReferences == 1)) { // We just released the last reference
            currentQueue = this->recyclableSubscribers.exchange(currentQueue);
            if(likely(currentQueue != nullptr)) {
              freeBroadcastQueue(currentQueue);
            }
          }
        }
        return; // Edited version of broadcast queue is in place, we're done
      } else { // Put our queue back into the loop, hopefully we can still reuse it above
        newQueue->ReferenceCount.store(0, std::memory_order::memory_order_relaxed);
        newQueue = this->recyclableSubscribers.exchange(newQueue);
        if(unlikely(newQueue != nullptr)) {
          freeBroadcastQueue(newQueue);
        }
      }

    } // C-A-S loop

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  bool ConcurrentEvent<TResult(TArguments...)>::Unsubscribe(const DelegateType &delegate) {

    // This is a C-A-S replacement attempt, so we may have to go through the whole opration
    // multiple times. We expect this to be the case only very rarely, as contention should
    // happen when events are fired, not by threads subscribing & unsubscribing rapidly.
    for(;;) {

      // Get a hold of the current queue.
      acquireSpinLock();
      BroadcastQueue *currentQueue = this->subscribers.load(
        std::memory_order::memory_order_consume
      );
      if(unlikely(currentQueue == nullptr)) {
        releaseSpinLock();
        return false; // No queue -> no subscribers -> subscriber not found -> exit!
      }

      { // A queue is present, increment its reference count so it isn't deleted
        currentQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_release);
        releaseSpinLock();

        ON_SCOPE_EXIT {
          // The spinlock does not need to be acquired here. If the queue is still assigned as
          // the active subscriber list, the reference counter will not reach 0. Otherwise,
          // it was already replaced by another thread, so we don't even need to check.
          std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
            1, std::memory_order::memory_order_release
          );
          if(unlikely(totalReferences == 1)) { // We just released the last reference
            currentQueue = this->recyclableSubscribers.exchange(currentQueue);
            if(likely(currentQueue != nullptr)) {
              freeBroadcastQueue(currentQueue);
            }
          }
        };

        BroadcastQueue *newQueue;

        // Hunt for the delegate the caller wishes to unsubscribe
        std::size_t index = 0;
        std::size_t currentSubscriberCount = currentQueue->Count;
        for(;;) {
          if(unlikely(currentQueue->Callbacks[index] == delegate)) {
            if(currentSubscriberCount == 1) { // Last subscriber just left
              newQueue = nullptr;
            } else { // Obtain a new queue to put the subscribers into
              newQueue = this->recyclableSubscribers.exchange(nullptr);
              if(unlikely(newQueue == nullptr)) { // No previous queue available?
                newQueue = allocateBroadcastQueue(currentSubscriberCount - 1);
              } else if(unlikely(newQueue->Capacity < currentSubscriberCount)) {
                freeBroadcastQueue(newQueue);
                newQueue = allocateBroadcastQueue(currentSubscriberCount - 1);
              } else {
                newQueue->ReferenceCount.fetch_add(1, std::memory_order::memory_order_relaxed);
                newQueue->Count = currentSubscriberCount - 1;
              }

              std::copy_n(currentQueue->Callbacks, index, newQueue->Callbacks);
              std::copy_n(
                currentQueue->Callbacks + index + 1,
                currentSubscriberCount - index,
                newQueue->Callbacks + index
              );
            }

            // Try to replace the active queue with our modified clone. Yes we're acquiring
            // the spinlock for an atomic operation. Why? The fetch operation (load + fetch_add)
            // must be atomic (thus we wrap the two operations in a spinlock) and those two
            // operations must not be interrupted by this single C-A-S instruction, so we hold
            // the spinlock here, thereby preventing any acquiring threads from grabbing
            // the reference before we decrement the reference counter to possibly 0.
            acquireSpinLock();
            bool wasReplaced = std::atomic_compare_exchange_strong(
              &this->subscribers, &currentQueue, newQueue
            );
            releaseSpinLock();

            if(likely(wasReplaced)) {
              if(likely(currentQueue != nullptr)) {
                std::size_t totalReferences = currentQueue->ReferenceCount.fetch_sub(
                  1, std::memory_order::memory_order_release
                );
                if(unlikely(totalReferences == 1)) { // We just released the last reference
                  currentQueue = this->recyclableSubscribers.exchange(currentQueue);
                  if(likely(currentQueue != nullptr)) {
                    freeBroadcastQueue(currentQueue);
                  }
                }
              }
              return true; // Edited version of broadcast queue is in place, we're done
            } else { // Put our queue back into the loop, hopefully we can still reuse it above
              newQueue->ReferenceCount.store(0, std::memory_order::memory_order_relaxed);
              newQueue = this->recyclableSubscribers.exchange(newQueue);
              if(unlikely(newQueue != nullptr)) {
                freeBroadcastQueue(newQueue);
              }
              break; // C-A-S loop needs to run again
            }
          } // if subscriber to be removed found

          // Search loop still running, advance to next item until end reached
          ++index;
          if(likely(index == currentSubscriberCount)) {
            return false; // Loop completed without finding the delegate
          }
        } // delegate search loop
      }

    } // C-A-S loop
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
