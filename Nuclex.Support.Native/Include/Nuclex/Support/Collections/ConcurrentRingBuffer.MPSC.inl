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

#if !defined(NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H)
#error This file must be included through via ConcurrentRingBuffer.h
#endif

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size circular buffer for one consumer and multiple producers</summary>
  /// <remarks>
  ///   <para>
  ///     This multi-producer, single-consumer version of the concurrent buffer lets any
  ///     number of threads add items to the buffer. A single thread can take items from
  ///     the buffer.
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
  ///     <strong>Thread safety</strong>: unlimited producing threads + one consuming thread
  ///   </para>
  ///   <para>
  ///     <strong>Exception guarantee</strong>: strong-ish (exception = buffer unchanged)
  ///   </para>
  ///   <para>
  ///     Footprint (stack): 48 bytes.
  ///     Footprint (heap):  +1 extra byte per element
  ///   </para>
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::MultipleProducersSingleConsumer> {

    /// <summary>
    ///   Initializes a new concurrent queue for a multiple producers and a single consumer
    /// </summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity),
      itemMemory(nullptr),
      itemStatus(nullptr),
      count(0),
      readIndex(0),
      writeIndex(0) {

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
        std::size_t safeCount = this->count.load(
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
            // Don't update free slot count, read index, item status because it's the d'tor
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
    /// <returns>The probably number of itemsthe queue held at the time of the call</returns>
    /// <remarks>
    ///   This method can be called from any thread and will have just about the same
    ///   accuracy as when it is called from the consumer thread or one of the producers.
    ///   If an item constructor throws an exception while the item is being copied/moved
    ///   into the buffer's memory, this will still increase the count (until )
    /// </remarks>
    public: std::size_t Count() const {

      // If many producers add at the same time, the item count may for a moment jump above
      // 'capacity' (the producer that incremented it above capacity silently decrements it
      // again and reports to its caller that the queue was full).
      return std::min(this->count.load(std::memory_order_relaxed), this->capacity);

    }

    /// <summary>Tries to append the specified element to the queue</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(const TElement &element) {

      // Try to reserve a slot. If the queue is full, the value will hit the capacity (or even
      // exceed it if highly contested), in which case we just hand the unusable slot back.
      {
        std::size_t safeCount = this->count.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );
        if(safeCount >= this->capacity) { // can happen under high contention of this code spot
          this->count.fetch_sub(1, std::memory_order_release);
          return false;
        }
      }

      // If we reach this spot, we know there was at least 1 slot free in the queue and we
      // just captured it (i.e. no other thread will cause less than 1 slot to remain free
      // while the following code runs). So we can happily increment the write index here.
      std::size_t targetSlotIndex;
      {
        int safeOccupiedIndex = this->writeIndex.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );

        // If the write index goes past 'capacity', do a wrap-around (ring buffer).
        // Multiple threads may simultaneously hit this spot, moving write index
        // into the negative. That is fine (we do a positive modulo on the index).
        if(
          (safeOccupiedIndex > 0) && // To ensure static_cast below is safe
          (static_cast<std::size_t>(safeOccupiedIndex) >= this->capacity)
        ) {
          this->writeIndex.fetch_sub(
            static_cast<int>(this->capacity), std::memory_order_relaxed
          );
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
        };
        new(this->itemMemory + targetSlotIndex) TElement(element);
        brokenSlotScope.Commit();
      }

      // Mark the slot as available for the reading thread
      this->itemStatus[targetSlotIndex].store(2, std::memory_order_release);

      // Item was appended!
      return true;

    }

    /// <summary>Tries to move-append the specified element to the queue</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryShove(const TElement &&element) {

      // Try to reserve a slot. If the queue is full, the value will hit the capacity (or even
      // exceed it if highly contested), in which case we just hand the unusable slot back.
      {
        std::size_t safeCount = this->count.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );
        if(safeCount >= this->capacity) { // can happen under high contestion of this code spot
          this->count.fetch_sub(1, std::memory_order_release);
          return false;
        }
      }

      // If we reach this spot, we know there was at least 1 slot free in the queue and we
      // just captured it (i.e. no other thread will cause less than 1 slot to remain free
      // while the following code runs). So we can happily increment the write index here.
      std::size_t targetSlotIndex;
      {
        int safeOccupiedIndex = this->writeIndex.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );

        // If the write index goes past 'capacity', do a wrap-around (ring buffer).
        // Multiple threads may simultaneously hit this spot, moving write index
        // into the negative. That is fine (we do a positive modulo on the index).
        if(
          (safeOccupiedIndex > 0) && // To ensure static_cast below is safe
          (static_cast<std::size_t>(safeOccupiedIndex) >= this->capacity)
        ) {
          this->writeIndex.fetch_sub(this->capacity, std::memory_order_relaxed);
        }

        targetSlotIndex = positiveModulo(safeOccupiedIndex, this->capacity);
      }

      // Mark the slot as under construction for the reading thread
#if !defined(NDEBUG) // not really needed, empty and under construction are treated the same
      this->itemStatus[targetSlotIndex].store(1, std::memory_order_release);
#endif

      // Move the item into the slot. If its move constructor throws, the slot must be
      // marked as broken so the reading thread will skip it.
      {
        auto brokenSlotScope = ON_SCOPE_EXIT_TRANSACTION {
          this->itemStatus[targetSlotIndex].store(3, std::memory_order_release);
        };
        new(this->itemMemory + targetSlotIndex) TElement(std::move(element));
        brokenSlotScope.Commit();
      }

      // Mark the slot as available for the reading thread
      this->itemStatus[targetSlotIndex].store(2, std::memory_order_release);

      // Item was appended!
      return true;

    }

    /// <summary>Tries to remove an element from the queue</summary>
    /// <param name="element">Element into which the queue's element will be placed</param>
    /// <remarks>
    ///   This method always attempts to use move semantics because item in the buffer is
    ///   rendered inaccessible and eventually destroyed anyway.
    /// </remarks>
    public: bool TryTake(TElement &element) {
      std::size_t safeCount = this->count.load(
        std::memory_order::memory_order_consume // consume: if() below carries dependency
      );
      if(safeCount < 1) {
        return false; // No more potential items in queue
      }

      // If we reach this point, there is at least one taken slot (which may contain
      // a valid item or represent a gap due to a constructor exception while adding the item)
      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_consume // consume: access below carries dependency
      );
      for(;;) { // Typical case: loop runs once. Case w/gaps: multiple runs, but deterministic
        std::uint8_t safeItemStatus = this->itemStatus[safeReadIndex].load(
          std::memory_order::memory_order_consume // consume: if() below carries dependency
        );
        if(safeItemStatus < 2) { // 0: item is empty, 1: item is under construction
          return false; // If the item is missing, act as if the queue had no more items
        }

        // Item status 2 means there is an item present in the slot
        if(safeItemStatus == 2) {
          break;
        }

        // safeItemStatus was 3, so the current item is a gap and can be skipped
        // (this happens when an item constructor throws an exception)
        if constexpr(!std::is_trivially_destructible<TElement>::value) {
          this->itemMemory[safeReadIndex].~TElement();
        }
        this->itemStatus[safeReadIndex].store(0, std::memory_order_relaxed);

        // Why read again? Because 'count' may have been equal to or larger than our capacity
        // (if many threads try to append at the same time), so for those cases, we re-read
        // to make sure we re-enter accurate territory at (capacity - 1).
        // CHECK: A simple std::min() at the top of the method would suffice, too, wouldn't it?
        safeCount = (
          this->count.fetch_sub(1, std::memory_order_consume) - 1 // if() below = dependency
        );
        if(safeCount < 1) {
          return false; // No more potential items in queue (everything was a gap)
        }

        safeReadIndex = (safeReadIndex + 1) % this->capacity;
      }

      // Move the item to the caller-provided memory. This may throw.
      TElement *readAddress = this->itemMemory + safeReadIndex;
      element = std::move(*readAddress);

      if constexpr(!std::is_trivially_destructible<TElement>::value) {
        readAddress->~TElement();
      }
      this->itemStatus[safeReadIndex].store(0, std::memory_order_release);

      // For a single reader, the ordering here is not that important, i.e. another
      // reader thread can't come by, see the free slot and read the un-updated read index
      this->readIndex.store(
        (safeReadIndex + 1) % this->capacity, std::memory_order::memory_order_relaxed
      );
      this->count.fetch_sub(1, std::memory_order_release);

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
    private: TElement *itemMemory;
    /// <summary>Status of items in buffer, 0: empty, 1: filling, 2: present, 3: gap</summary>
    private: std::atomic<std::uint8_t> *itemStatus;

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
    private: std::atomic<std::size_t> count;
    /// <summary>Index from which the next item will be read</summary>
    private: std::atomic<std::size_t> readIndex;
    /// <summary>Index at which the most recently written item is stored</summary>
    /// <remarks>
    ///   Notice that contrary to normal practice, this does not point one past the last
    ///   item (i.e. to the position of the next write), but is the index of the last item
    ///   that has been stored in the buffer. The lock-free synchronization is easier this way.
    /// </remarks>
    private: std::atomic<int> writeIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
