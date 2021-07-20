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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H

#include "Nuclex/Support/Collections/ConcurrentCollection.h"

#include <atomic> // for std::atomic
#include <cstdint> // for std::uint8_t
#include <algorithm> // for std::copy_n()

#include "Nuclex/Support/ScopeGuard.h" // for ON_SCOPE_EXIT macros

// CHECK: Rename this to ConcurrentRingBuffer? or something else?
//
// I decided that "Buffers" are fixed-size (bounded) and "Queues" are not
//
// RingBuffer (single-threaded) currently is unbounded. People may expect a fixed-size
// buffer when seeing the name ring buffer (or anything ending in buffer).
//
// Queue does not indicate boundedness. So perhaps I have created:
//   - A RingQueue
//   - A ShiftQueue
//   - A ConcurrentRingBuffer
//
// Downside: many papers and libraries talk about "bounded queues" and mean exactly
// what I've implemented here...
//

// CHECK: Was the rename a shitty idea? Maybe Buffers = Batch operations, Queue = individual?
//
// In this case, rename everything back to:
//   - RingBuffer
//   - ShiftBuffer
//   - ConcurrentRingQueue
//   -

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Lock-free bounded ring buffer for 1:1, 1:n and n:n producer/consumer threads
  /// </summary>
  /// <typeparam name="TElement">
  ///   Type of elements that will be stored in the ring buffer
  /// </typeparam>
  /// <typeparam name="accessBehavior">
  ///   How the ring buffer will be accessed from different threads
  /// </typeparam>
  /// <remarks>
  ///   <para>
  ///     This is a triplet of ring buffer implementations that are designed to be used from
  ///     multiple threads and synchronize based purely on atomic variables. There are no
  ///     mutexes used and there is no spinning (spinlock or CaS) in any of these variants,
  ///     so they're fully wait-free.
  ///   </para>
  ///   <para>
  ///     Please ensure to select the correct variant (or err on the side of caution and use
  ///     the multiple producer, multiple consumer variant) because otherwise, all kinds of
  ///     hard-to-find synchronization issues will pop up, just as if you used a single-threaded
  ///     ring buffer from multiple threads.
  ///   </para>
  ///   <para>
  ///     <strong>Container type</strong>: bounded ring buffer
  ///   </para>
  ///   <para>
  ///     <strong>Thread safety</strong>: depends on chosen variant, up to free-threaded
  ///   </para>
  ///   <para>
  ///     <strong>Exception guarantee</strong>: strong (exception = buffer unchanged)
  ///   </para>
  /// </remarks>
  template<
    typename TElement,
    ConcurrentAccessBehavior accessBehavior = (
      ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentRingBuffer;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#include "ConcurrentRingBuffer.SPSC.inl"
#include "ConcurrentRingBuffer.MPSC.inl"
#include "ConcurrentRingBuffer.MPMC.inl"

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H
