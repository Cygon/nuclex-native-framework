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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H

#include "Nuclex/Support/Collections/ConcurrentCollection.h"

// We delegate this implementation to the Boost-Licensed MoodyCamel library
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Lock-free unbounded queue for 1:1, 1:n and n:n producer/consumer threads
  /// </summary>
  /// <typeparam name="TElement">
  ///   Type of elements that will be stored in the queue
  /// </typeparam>
  /// <typeparam name="accessBehavior">
  ///   How the queue will be accessed from different threads
  /// </typeparam>
  /// <remarks>
  ///   <para>
  ///     Currently, this implementation is just an adapter around MoodyCamel's lock-free
  ///     queue. Its performance is very good (if you build with NUCLEX_SUPPORT_BENCHMARKS,
  ///     this adapter will be included in the benchmarks, too), but its memory footprint
  ///     is probably an order of magnitude higher than you expect.
  ///   </para>
  ///   <para>
  ///     <strong>Container type</strong>: unbounded segmented array queue
  ///   </para>
  ///   <para>
  ///     <strong>Thread safety</strong>: any number of readers, any number of writers
  ///   </para>
  ///   <para>
  ///     <strong>Exception guarantee</strong>: unknown, probably strong
  ///   </para>
  ///   <para>
  ///     Footprint (stack): 348 bytes.
  ///     Footprint (heap):  ?
  ///   </para>
  template<
    typename TElement,
    ConcurrentAccessBehavior accessBehavior = (
      ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentQueue : public ConcurrentCollection<TElement> {

    /// <summary>Initializes a new lock-free queue</summary>
    public: ConcurrentQueue() : wrappedQueue() {}

    /// <summary>Initializes a new lock-free queue</summary>
    /// <param name="initialCapacity">Capacity in items to reserve up-front</param>
    public: explicit ConcurrentQueue(std::size_t initialCapacity) :
      wrappedQueue(initialCapacity) {}

    /// <summary>Destroys the lock-free queue and all items still in it</summary>
    public: ~ConcurrentQueue() override = default;

    /// <summary>Tries to append an element to the collection in a thread-safe manner</summary>
    /// <param name="newItem">Element that will be appended to the collection</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: bool TryAppend(const TElement &newItem) override {
      this->wrappedQueue.enqueue(newItem);
      return true;
    }

    /// <summary>Tries to append an element to the collection in a thread-safe manner</summary>
    /// <param name="newItem">Element that will be appended to the collection</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: bool TryAppend(TElement &&newItem) override {
      this->wrappedQueue.enqueue(std::move(newItem));
      return true;
    }

    /// <summary>Tries to take an element from the queue</summary>
    /// <param name="result">Will receive the element taken from the queue</param>
    /// <returns>
    ///   True if an element was taken from the collection, false if the collection was empty
    /// </returns>
    public: bool TryTake(TElement &result) override {
      return this->wrappedQueue.try_dequeue(result);
    }

    /// <summary>Counts the numebr of elements current in the collection</summary>
    /// <returns>
    ///   The approximate number of elements that have been in the collection during the call
    /// </returns>
    public: std::size_t Count() const override {
      return this->wrappedQueue.size_approx();
    }

    /// <summary>Checks if the collection is empty</summary>
    /// <returns>True if the collection was probably empty during the call</returns>
    public: bool IsEmpty() const override {
      return (this->wrappedQueue.size_approx() == 0);
    }

    /// <summary>Lock-free queue from the MoodyCamel library we're wrapping</summary>
    private: moodycamel::ConcurrentQueue<TElement> wrappedQueue;

  };

  // ------------------------------------------------------------------------------------------- //
/*
  /// <summary>
  ///   Multi-producer, multi-consumer version of the lock-free, unbounded queue
  /// </summary>
  /// <typeparam name="TElement">Type of elements the queue will store</typeparam>
  template<typename TElement>
  class ConcurrentQueue<
    TElement, ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
  > : ConcurrentCollection<TElement> {};
*/
}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
