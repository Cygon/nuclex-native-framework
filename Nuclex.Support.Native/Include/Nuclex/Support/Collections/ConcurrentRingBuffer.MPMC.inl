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

#if !defined(NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H)
#error This file must be included through via ConcurrentRingBuffer.h
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1920)
// https://github.com/microsoft/STL/issues/1673
// https://developercommunity2.visualstudio.com/t/compiler/1346618
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message ( \
  __FILE__ "(" STRING(__LINE__) "): " \
  "warning: compiling the Nuclex MPMC Queue on VS2019 will trigger a compiler bug " \
  "at the point of usage (see https://github.com/microsoft/STL/issues/1673)" \
)
// Unfortunately, there seems to be no usable workaround for this compiler.
// I don't know when or if it will be fixed, so I'm leaving this warning open-ended.
// If I spot a version of Microsoft's C++ compiler that works, I'll limit the range.
#endif

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size circular buffer for multiple consumers and producers</summary>
  /// <remarks>
  ///   <para>
  ///     This multi-producer, multi-consumer variant of the concurrent buffer can be
  ///     freely used from any number of threads. Any thread can append items to the buffer
  ///     and any thread can take items from the buffer without any restrictions.
  ///   </para>
  ///   <para>
  ///     This implementation is lock-free and also wait-free (i.e. no compare-and-swap loops).
  ///     Batch operations are supported and this variant gives a strong-ish exception
  ///     guarantee: if an operation fails, the buffer's state remains as if it never happened,
  ///     but the buffer's capacity will be temporarily reduced.
  ///   </para>
  ///   <para>
  ///     <strong>Container type</strong>: bounded ring buffer
  ///   </para>
  ///   <para>
  ///     <strong>Thread safety</strong>: any number of consumers, any numbers of producers
  ///   </para>
  ///   <para>
  ///     <strong>Exception guarantee</strong>: strong-ish (exception = buffer unchanged)
  ///   </para>
  /// </remarks>
  template<typename TElement>
  class ConcurrentRingBuffer<
    TElement, ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
  > {

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity),
      itemMemory(nullptr),
      itemStatus(nullptr),
      readIndex(0),
      writeIndex(0),
      occupiedCount(0),
      availableCount(0) {

      std::uint8_t *buffer = new std::uint8_t[sizeof(TElement[2]) * capacity / 2U];
      {
        auto itemMemoryDeleter = ON_SCOPE_EXIT_TRANSACTION {
          delete[] buffer;
        };
        this->itemStatus = new std::atomic<std::uint8_t>[capacity];
        itemMemoryDeleter.Commit(); // disarm the item memory deleter
      }
      this->itemMemory = reinterpret_cast<TElement *>(buffer);

      // Initialize the status of all items
      for(std::size_t index = 0; index < capacity; ++index) {
        this->itemStatus[index].store(0, std::memory_order_relaxed);
      }

      std::atomic_thread_fence(std::memory_order_release);
    }
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentRingBuffer() {
      if constexpr(!std::is_trivially_destructible<TElement>::value) {
        std::size_t safeCount = this->occupiedCount.load(
          std::memory_order::memory_order_consume // consume: if() below carries dependency
        );
        if(safeCount >= 1) {
          std::size_t safeReadIndex = this->readIndex.load(
            std::memory_order::memory_order_consume // consume: while() below carries dependency
          );
          while(safeCount >= 1) {
            this->itemMemory[safeReadIndex].~TElement();
            safeReadIndex = (safeReadIndex + 1) % this->capacity;
            --safeCount;
            // Don't update free slot occupiedCount, read index, item status because it's the d'tor
          }
        }
      }

      delete[] this->itemStatus;
#if !defined(NDEBUG)
      this->itemStatus = nullptr;
#endif
      delete[] reinterpret_cast<std::uint8_t *>(this->itemMemory);
#if !defined(NDEBUG)
      this->itemMemory = nullptr;
#endif
    }

    /// <summary>Estimates the number of items stored in the queue</summary>
    /// <returns>The probable number of items the queue held at the time of the call</returns>
    /// <remarks>
    ///   This method can be called from any thread and will have just about the same
    ///   accuracy as when it is called from the consumer thread or one of the producers.
    ///   If an item constructor throws an exception while the item is being copied/moved
    ///   into the buffer's memory, this will still increase the occupiedCount (until )
    /// </remarks>
    public: std::size_t Count() const {

      // If many producers add at the same time, the item count may for a moment jump above
      // 'capacity' (the producer that incremented it above capacity silently decrements it
      // again and reports to its caller that the queue was full).
      return std::min(this->occupiedCount.load(std::memory_order_relaxed), this->capacity);

    }

    /// <summary>Tries to append the specified element to the queue</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(const TElement &element) {

      // Try to reserve a slot. If the queue is full, the value will hit the capacity (or even
      // exceed it if highly contested), in which case we just hand the unusable slot back.
      {
        std::size_t safeCount = this->occupiedCount.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );
        if(safeCount >= this->capacity) { // can happen under high contention of this code spot
          this->occupiedCount.fetch_sub(1, std::memory_order_release);
          return false;
        }
      }

      // If we reach this spot, we know there was at least 1 slot free in the queue and we
      // just captured it (i.e. no other thread will cause less than 1 slot to remain free).
      // So we just need to 'take' a slot index from the write index list
      std::size_t targetSlotIndex;
      {
        int safeOccupiedIndex = this->writeIndex.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );

        // If the write index goes past 'capacity', do a wrap-around (ring buffer).
        // Multiple threads may simultaneously hit this spot, moving write index
        // into the negative. That is fine (we do a positive modulo on the index).
        if(safeOccupiedIndex > 0) {
          if(static_cast<std::size_t>(safeOccupiedIndex) >= this->capacity) {
            this->writeIndex.fetch_sub(
              static_cast<int>(this->capacity), std::memory_order_relaxed
            );
          }
        }

        targetSlotIndex = positiveModulo(safeOccupiedIndex, static_cast<int>(this->capacity));
      }

      // Mark the slot as under construction for the reading thread
#if !defined(NDEBUG) // not really needed, empty and under construction are treated the same
      this->itemStatus[targetSlotIndex].store(1, std::memory_order_release);
#endif

      // Copy the item into the slot. If its copy constructor throws, the slot must be
      // marked as broken so the reading thread will skip it.
      {
        auto brokenSlotScope = ON_SCOPE_EXIT_TRANSACTION {
          this->itemStatus[targetSlotIndex].store(3, std::memory_order_release);
          this->availableCount.fetch_add(1, std::memory_order_release);
        };
        new(this->itemMemory + targetSlotIndex) TElement(element);
        brokenSlotScope.Commit();
      }

      // Mark the slot as available for the reading thread
      this->itemStatus[targetSlotIndex].store(2, std::memory_order_release);
      this->availableCount.fetch_add(1, std::memory_order_release);

      // Item was appended!
      return true;

    }

    /// <summary>Tries to remove an element from the queue</summary>
    /// <param name="element">Element into which the queue's element will be placed</param>
    /// <returns>True if an item was available and return, false otherwise</returns>
    /// <remarks>
    ///   This method always attempts to use move semantics because item in the buffer is
    ///   rendered inaccessible and eventually destroyed anyway.
    /// </remarks>
    public: bool TryTake(TElement &element) {

      int safeAvailableCount = this->availableCount.fetch_sub(1, std::memory_order_release);
      if(safeAvailableCount < 1) {
        this->availableCount.fetch_add(1, std::memory_order_relaxed);
        return false;
      }

      // At this point, we know there's at least one item in the queue and no other thread
      // is going to take it (because we reserved it through the availableCount).
      std::size_t sourceSlotIndex;
      {
        int safeReadIndex = this->readIndex.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );

        // If the write index goes past 'capacity', do a wrap-around (ring buffer).
        // Multiple threads may simultaneously hit this spot, moving write index
        // into the negative. That is fine (we do a positive modulo on the index).
        if(safeReadIndex > static_cast<int>(this->capacity)) {
          this->readIndex.fetch_sub(
            static_cast<int>(this->capacity), std::memory_order_relaxed
          );
        }

        sourceSlotIndex = positiveModulo(safeReadIndex, static_cast<int>(this->capacity));
      }

      // Move the item to the caller-provided memory. This may throw.
      TElement *readAddress = this->itemMemory + sourceSlotIndex;
      {
        ON_SCOPE_EXIT {
          if constexpr(!std::is_trivially_destructible<TElement>::value) {
            readAddress->~TElement();
          }
          this->itemStatus[sourceSlotIndex].store(0, std::memory_order_release);
          this->occupiedCount.fetch_sub(1, std::memory_order_relaxed);
        };

        element = std::move(*readAddress);
      }

      return true; // Item was read
    }

    /// <summary>Returns the maximum number of items the queue can hold</summary>
    /// <returns>The maximum number of items the queue can hold</returns>
    public: std::size_t GetCapacity() const { return this->capacity; }

    /// <summary>Performs the modulo operation, but returns 0..divisor-1</summary>
    /// <param name="value">Value for which the positive modulo will be calculated</param>
    /// <param name="divisor">Divisor of which the remainder will be calculated</param>
    /// <returns>The positive division remainder of the specified value</returns>
    /// <remarks>
    ///   There are various tricks to achieve this without branching, but they're all slower.
    ///   Reason: x86, amd64 and ARM CPUs have conditional move instructions, allowing cases
    ///   like this one to execute without branching at the machine code level.
    /// </remarks>
    private: static std::size_t positiveModulo(int value, int divisor) {
      value %= divisor;
      if(value < 0) {
        return static_cast<std::size_t>(value + divisor);
      } else {
        return static_cast<std::size_t>(value);
      }
    }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: const std::size_t capacity;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    /// <remarks>
    ///   This is allocated as a buffer of unsigned characters, thus it points to
    ///   uninitialized memory, except for the items which have been placed into it.
    /// </remarks>
    private: TElement *itemMemory;
    /// <summary>Status of items in buffer, 0: empty, 1: filling, 2: present, 3: gap</summary>
    private: std::atomic<std::uint8_t> *itemStatus;

    /// <summary>Index from which the next item will be read</summary>
    /// <remarks>
    ///   Once a thread knows that an item is available and has reserved it through
    ///   <see cref="availableCount" />, it will blindly increment this value. If
    ///   the incrementing thread sees that the read index is past the capacity, it will
    ///   just as blindly decrement it by the capacity to force a wrap-around. In turn,
    ///   readIndex can be both less than 0 and more than capacity, but when wrapped into
    ///   the valid range, it will point to the correct item.
    /// </remarks>
    private: std::atomic<int> readIndex;
    /// <summary>Index at which the most recently written item is stored</summary>
    /// <remarks>
    ///   <para>
    ///     Notice that contrary to normal practice, this does not point one past the last
    ///     item (i.e. to the position of the next write), but is the index of the last item
    ///     that has been stored in the buffer. Lock-free synchronization is easier this way.
    ///   </para>
    ///   <para>
    ///     The write index follows the same behavior documented for the read index, it may
    ///     go beyond capacity or be less than 0 if multiple threads see it there and subtract
    ///     the buffer's capacity. It, too, will point to the correct item with wrapp-around.
    ///   </para>
    /// </remarks>
    private: std::atomic<int> writeIndex;

    /// <summary>Number of free slots the queue can store elements in</summary>
    /// <remarks>
    ///   <para>
    ///     This allows the <see cref="TryAppend" /> method to know whether a slot will be free
    ///     after the current write index, eliminating the whole C-A-S loop. While reserving,
    ///     the value will be blindly incremented, checked and - if beyond capacity - decremented
    ///     back down.
    ///   </para>
    ///   <para>
    ///     Also important is that this counts slots, not items. If a constructor throws during
    ///     an append operation, the slot will remain occupied (because it can't be safely
    ///     returned due to other threads being able to grab slots after it) but marked as a gap.
    ///   </para>
    /// </remarks>
    private: std::atomic<std::size_t> occupiedCount;

    /// <summary>Number of guaranteed available slots the queue can read elements from</summary>
    /// <remarks>
    ///   <para>
    ///     As the counterpart to <see cref="occupiedCount" />, this will be blindly decremented
    ///     when a thread is attempting to take an element from the queue. If it ends up less
    ///     than 0 (meaning no elements were available), it is immediately incremented back up.
    ///   </para>
    ///   <para>
    ///     Similarly to its counterpart, an available slot does not guarantee an available item.
    ///     Slots may contain gap items (this happens when the copy or move constructur of
    ///     an item throws an exception while it's being added).
    ///   </para>
    /// </remarks>
    private: std::atomic<int> availableCount;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
