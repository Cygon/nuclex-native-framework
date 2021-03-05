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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H

#include "Nuclex/Support/Config.h"

// Known implementations besides this one for reference:
//
// Libraries of Lock-Free data structures:
// https://github.com/mpoeter/xenium
// https://liblfds.org/ (<-- Public Domain!)
// https://github.com/khizmax/libcds
//
// Interesting implementations:
// https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++.htm
// https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++.htm
//
// Intel's implementation (curiously not that good in benchmarks):
// https://github.com/oneapi-src/oneTBB (Intel TBB under its new name)
// https://github.com/oneapi-src/oneTBB/blob/master/include/oneapi/tbb/concurrent_queue.h
//
// "Battle Tested" implementation:
// https://github.com/rigtorp/awesome-lockfree
// https://github.com/rigtorp/MPMCQueue
//

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>How a concurrent collection is being accessed</summary>
  /// <remarks>
  ///   There fewer threads need to access the collection, the faster an implementation
  ///   can be. This is used as a template parameter to decide implementation.
  /// </remarks>
  enum class ConcurrentAccessBehavior {

    /// <summary>
    ///   Only one thread is taking data and another, but only one, is producing it
    /// </summary>
    SingleProducerSingleConsumer,

    /// <summary>
    ///   Only one thread is taking data, but multiple threads are adding data
    /// </summary>
    MultipleProducersSingleConsumer,

    /// <summary>
    ///   Any number of threads is taking data and any number of threads is adding it
    /// </summary>
    MultipleProducersMultipleConsumers

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Collection that can safely be used from multiple threads</summary>
  template<
    typename TElement,
    ConcurrentAccessBehavior accessBehavior = (
      ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentCollection {

    /*
    /// <summary>Initializes a concurrent collection</summary>
    public: ConcurrentCollection() {}
    */

    /// <summary>Destroys the concurrent queue</summary>
    public: virtual ~ConcurrentCollection() = default;

    /// <summary>Tries to appends an element to the collection in a thread-safe manner</summary>
    /// <param name="element">Element that will be appended to the collection</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: virtual bool TryAppend(const TElement &element) = 0;

    /// <summary>Tries to take an element from the queue</summary>
    /// <param name="element">Will receive the element taken from the queue</param>
    /// <returns>
    ///   True if an element was taken from the collection, false if the collection was empty
    /// </returns>
    public: virtual bool TryPop(TElement &element) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H
