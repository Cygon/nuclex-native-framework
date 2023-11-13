#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_RINGQUEUE_H
#define NUCLEX_SUPPORT_COLLECTIONS_RINGQUEUE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/BitTricks.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <memory> // for std::unique_ptr
#include <type_traits> // for std::enable_if<>
#include <cstring> // for std::memcpy()
#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A ring buffer that can grow and read/write in batches</summary>
  /// <remarks>
  ///   <para>
  ///     <strong>Thread safety:</strong> each instance should be accessed by a single thread
  ///   </para>
  ///   <para>
  ///     <strong>Container type:</strong> unbounded ring buffer with batch operations
  ///   </para>
  ///   <para>
  ///     This differs from std::queue in two ways: 1) it is optimized for a constant
  ///     ring buffer size (i.e. the capacity can grow, but is assumed to settle quickly)
  ///     and 2) it provides efficient batch operations.
  ///   </para>
  /// </remarks>
  template<typename TItem>
  class RingQueue {

    /// <summary>Constant used to indicate an invalid index</summary>
    private: static const std::size_t InvalidIndex = static_cast<std::size_t>(-1);

    /// <summary>Initializes a new ring buffer</summary>
    /// <param name="capacity">Storage space in the ring buffer at the beginning</param>
    public: explicit RingQueue(std::size_t capacity = 256) :
      itemMemory(
        new std::uint8_t[sizeof(TItem[2]) * BitTricks::GetUpperPowerOfTwo(capacity) / 2]
      ),
      capacity(BitTricks::GetUpperPowerOfTwo(capacity)),
      startIndex(InvalidIndex),
      endIndex(InvalidIndex) {}

    /// <summary>Initializes a ring buffer as a copy of another ring buffer</summary>
    /// <param name="other">Other ring buffer that will be copied</param>
    public: RingQueue(const RingQueue &other) :
      itemMemory(new std::uint8_t[sizeof(TItem[2]) * other.capacity / 2]),
      capacity(other.capacity),
      startIndex(InvalidIndex),
      endIndex(InvalidIndex) {

      if(other.startIndex == InvalidIndex) {
        // We're good to go
      } else if(other.startIndex < other.endIndex) {
        TItem *sourceAddress = reinterpret_cast<TItem *>(other.itemMemory.get());
        emplaceInEmpty(sourceAddress, other.endIndex - other.startIndex);
      } else {
        TItem *sourceAddress = reinterpret_cast<TItem *>(other.itemMemory.get());
        sourceAddress += other.startIndex;
        emplaceInEmpty(sourceAddress, other.capacity - other.startIndex);

        sourceAddress = reinterpret_cast<TItem *>(other.itemMemory.get());
        emplaceInLinear(sourceAddress, other.endIndex);
      }
    }

    /// <summary>Initializes a ring buffer taking over another ring buffer</summary>
    /// <param name="other">Other ring buffer that will be taken over</param>
    public: RingQueue(RingQueue &&other) :
      itemMemory(std::move(other.itemMemory)),
      capacity(other.capacity),
      startIndex(other.startIndex),
      endIndex(other.endIndex) {
      other.itemMemory = nullptr;
#if !defined(NDEBUG)
      other.startIndex = InvalidIndex;
#endif
    }

    /// <summary>Destroys the ring buffer and all items in it</summary>
    public: ~RingQueue() {
      if(this->itemMemory != nullptr) { // Can be NULL if container donated its guts

        // If the buffer contains items, they, too, need to be destroyed
        if(this->startIndex != InvalidIndex) {

          // If the ring buffer is linear, simply destroy the items from start to end
          if(this->startIndex < this->endIndex) {
            TItem *items = reinterpret_cast<TItem *>(this->itemMemory.get());
            items += this->startIndex;
            for(std::size_t index = this->startIndex; index < this->endIndex; ++index) {
              items->~TItem();
              ++items;
            }
          } else { // If the ring buffer is wrapped, both segments need to be destroyed
            std::size_t segmentItemCount = this->capacity - this->startIndex;

            // Destroy all items in the older segment
            TItem *address = reinterpret_cast<TItem *>(this->itemMemory.get()) + this->startIndex;
            for(std::size_t index = 0; index < segmentItemCount; ++index) {
              address->~TItem();
            }

            // Destroy all items in the younger segment
            address = reinterpret_cast<TItem *>(this->itemMemory.get());
            for(std::size_t index = 0; index < this->endIndex; ++index) {
              address->~TItem();
            }
          }

        }

      }
    }

    /// <summary>Looks up the number of items the ring buffer has allocated memory for</summary>
    /// <returns>The number of items the ring buffer has reserved space for</returns>
    /// <remarks>
    ///   Just like std::vector::capacity(), this is not a limit. If the capacity is
    ///   exceeded, the ring buffer will allocate a large memory block and use that one.
    /// </remarks>
    public: std::size_t GetCapacity() const {
      return this->capacity;
    }

    /// <summary>Counts the number of items currently stored in the ring buffer</summary>
    public: std::size_t Count() const {
      if(this->startIndex == InvalidIndex) { // Empty
        return 0;
      } else if(this->startIndex < this->endIndex) { // Items linear
        return this->endIndex - this->startIndex;
      } else { // Items wrapped around
        return this->endIndex + (this->capacity - this->startIndex);
      }
    }

    /// <summary>Appends items to the end of the ring buffer</summary>
    /// <param name="items">Items that will be added to the ring buffer</param>
    /// <param name="count">Number of items that will be added</param>
    public: void Write(const TItem *items, std::size_t count) {
      if(count == 0) {
        return;
      }

      if(this->startIndex == InvalidIndex) {
        if(unlikely(count > this->capacity)) {
          std::size_t newCapacity = BitTricks::GetUpperPowerOfTwo(count);
          std::unique_ptr<std::uint8_t[]> newItemMemory(
            new std::uint8_t[sizeof(TItem[2]) * capacity / 2]
          );
          this->itemMemory.swap(newItemMemory);
          this->capacity = newCapacity;
        }
        emplaceInEmpty(items, count);
      } else if(this->endIndex > this->startIndex) {
        emplaceInLinear(items, count);
      } else {
        emplaceInWrapped(items, count);
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    public: void Read(TItem *items, std::size_t count) {
      if(count == 0) {
        return;
      }
      if(this->startIndex == InvalidIndex) {
        assert(
          (this->startIndex != InvalidIndex) &&
          u8"Ring buffer must contain at least the requested number of items"
        );
      } else if(this->endIndex > this->startIndex) {
        extractFromLinear(items, count);
      } else {
        extractFromWrapped(items, count);
      }
    }

    /// <summary>Emplaces the specified items into an empty ring buffer</summary>
    /// <param name="sourceItems">Items that will be emplaced into the buffer</param>
    /// <param name="itemCount">Number of items that will be emplaced</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_copyable<T>::value>::type emplaceInEmpty(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());

      std::size_t count = itemCount;
      try {
        while(count > 0) {
          new(targetItems) TItem(*sourceItems);
          ++sourceItems;
          ++targetItems;
          --count;
        }
      }
      catch(...) {
        if(count == itemCount) {
          this->startIndex = InvalidIndex;
#if !defined(NDEBUG)
          this->endIndex = InvalidIndex;
#endif
        } else {
          this->startIndex = 0;
          this->endIndex = (itemCount - count);
        }
        throw;
      }

      this->startIndex = 0;
      this->endIndex = itemCount;
    }

    /// <summary>Emplaces the specified items into an empty ring buffer</summary>
    /// <param name="sourceItems">Items that will be emplaced into the buffer</param>
    /// <param name="itemCount">Number of items that will be emplaced</param>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type emplaceInEmpty(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));

      this->startIndex = 0;
      this->endIndex = itemCount;
    }

    /// <summary>Appends items to a ring buffer with items that have wrapped around</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="itemCount">Number of items that will be appended</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_copyable<T>::value>::type emplaceInWrapped(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      std::size_t remainingItemCount = this->startIndex - this->endIndex;
      if(likely(remainingItemCount >= itemCount)) { // New data fits, simplest case there is
        TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        targetItems += this->endIndex;
        std::size_t count = itemCount;
        try {
          while(count > 0) {
            new(targetItems) TItem(*sourceItems);
            ++sourceItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->endIndex += (itemCount - count);
          throw;
        }
        this->endIndex += itemCount;
      } else { // New data doesn't fit, ring buffer needs to be extended
        std::size_t totalItemCount = (this->capacity - remainingItemCount) + itemCount;
        TItem *targetItems = reallocateWhenWrapped(totalItemCount);
        this->startIndex = 0;
        std::size_t count = itemCount;
        try {
          while(count > 0) {
            new(targetItems) TItem(*sourceItems);
            ++sourceItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->endIndex = (totalItemCount - count);
          throw;
        }

        this->endIndex = totalItemCount;
      }
    }

    /// <summary>Appends items to a ring buffer with items that have wrapped around</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="itemCount">Number of items that will be appended</param>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type emplaceInWrapped(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      std::size_t remainingItemCount = this->startIndex - this->endIndex;
      if(likely(remainingItemCount >= itemCount)) { // New data fits, simplest case there is
        TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        targetItems += this->endIndex;
        std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));
        this->endIndex += itemCount;
      } else { // New data doesn't fit, ring buffer needs to be extended
        std::size_t totalItemCount = this->capacity - remainingItemCount + itemCount;
        TItem *targetItems = reallocateWhenWrapped(totalItemCount);
        std::memcpy(targetItems, sourceItems, itemCount);
        this->startIndex = 0;
        this->endIndex = totalItemCount;
      }
    }

    /// <summary>Appends items to a ring buffer with items stored linearly</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="count">Number of items that will be appended</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_copyable<T>::value>::type emplaceInLinear(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      std::size_t remainingSegmentItemCount = this->capacity - this->endIndex;
      if(likely(remainingSegmentItemCount >= itemCount)) { // New data fits
        TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        targetItems += this->endIndex;

        std::size_t count = itemCount;
        try {
          while(count > 0) {
            new(targetItems) TItem(*sourceItems);
            ++sourceItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->endIndex += (itemCount - count);
          throw;
        }

        this->endIndex += itemCount;
      } else { // New data must be wrapped or ring buffer needs to be extended
        std::size_t wrappedItemCount = itemCount - remainingSegmentItemCount;
        if(likely(wrappedItemCount < this->startIndex)) {
          if(remainingSegmentItemCount > 0) {
            TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
            targetItems += this->endIndex;

            std::size_t count = remainingSegmentItemCount;
            try {
              while(count > 0) {
                new(targetItems) TItem(*sourceItems);
                ++sourceItems;
                ++targetItems;
                --count;
              }
            }
            catch(...) {
              this->endIndex += (itemCount - count);
              throw;
            }

            this->endIndex += itemCount;
          }
          {
            TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());

            std::size_t count = wrappedItemCount;
            try {
              while(count > 0) {
                new(targetItems) TItem(*sourceItems);
                ++sourceItems;
                ++targetItems;
                --count;
              }
            }
            catch(...) {
              if(count < wrappedItemCount) {
                this->endIndex += (wrappedItemCount - count);
              }
              throw;
            }

            this->endIndex = wrappedItemCount;
          }

        } else { // New data doesn't fit
          std::size_t totalItemCount = (this->endIndex - this->startIndex) + itemCount;
          TItem *targetItems = reallocateWhenLinear(totalItemCount);
          this->startIndex = 0;
          std::size_t count = itemCount;
          try {
            while(count > 0) {
              new(targetItems) TItem(*sourceItems);
              ++sourceItems;
              ++targetItems;
              --count;
            }
          }
          catch(...) {
            this->endIndex = (totalItemCount - count);
            throw;
          }

          this->endIndex = totalItemCount;
        }
      }
    }

    /// <summary>Appends items to a ring buffer with items stored linearly</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="count">Number of items that will be appended</param>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type emplaceInLinear(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      std::size_t remainingSegmentItemCount = this->capacity - this->endIndex;
      if(likely(remainingSegmentItemCount >= itemCount)) { // New data fits
        TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        targetItems += this->endIndex;
        std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));
        this->endIndex += itemCount;
      } else { // New data must be wrapped or ring buffer needs to be extended
        std::size_t remainingItemCount = remainingSegmentItemCount + this->startIndex;
        if(likely(remainingItemCount >= itemCount)) {
          TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
          targetItems += this->endIndex;
          std::memcpy(targetItems, sourceItems, remainingSegmentItemCount * sizeof(TItem));
          this->endIndex = itemCount - remainingSegmentItemCount;
          sourceItems += remainingSegmentItemCount;
          targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
          std::memcpy(targetItems, sourceItems, this->endIndex * sizeof(TItem));
        } else { // New data doesn't fit, ring buffer needs to be extended
          std::size_t totalItemCount = this->endIndex - this->startIndex + itemCount;
          TItem *targetItems = reallocateWhenLinear(totalItemCount);
          std::memcpy(targetItems, sourceItems, itemCount);
          this->startIndex = 0;
          this->endIndex = totalItemCount;
        }
      }
    }

    /// <summary>Reallocates the ring buffer's memory to fit the required items</summary>
    /// <param name="requiredItemCount">Number of items the buffer needs to hold</param>
    /// <returns>The address at which the next item can be written</returns>
    /// <remarks>
    ///   Careful: this method does not update the startIndex and endIndex variables,
    ///   as this operation is always followed by writing additional items into the buffer.
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_destructible<T>::value || !std::is_trivially_copyable<T>::value, TItem *
    >::type reallocateWhenWrapped(std::size_t requiredItemCount) {
      std::size_t newCapacity = BitTricks::GetUpperPowerOfTwo(requiredItemCount);

      // Allocate new memory for the enlarged buffer
      std::unique_ptr<std::uint8_t[]> swappedItemMemory(
        new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2]
      );
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());

      this->capacity = newCapacity;

      // Copy the older segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(swappedItemMemory.get());
        existingItems += this->endIndex;

        std::size_t count = this->capacity - this->startIndex;
        try {
          while(count > 0) {
            new(targetItems) TItem(std::move(*existingItems));
            existingItems->~TItem();
            ++existingItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->endIndex = (this->capacity - this->startIndex) - count;

          // Delete remainder of current segment
          while(count > 0) {
            existingItems->~TItem();
            ++existingItems;
            --count;
          }

          count = this->startIndex;
          existingItems = reinterpret_cast<TItem *>(swappedItemMemory.get());

          // Delete older segment
          while(count > 0) {
            existingItems->~TItem();
            ++existingItems;
            --count;
          }

          this->startIndex = 0;

          throw;
        }
      }

      // Copy the newer segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(swappedItemMemory.get());

        std::size_t count = this->endIndex;
        try {
          while(count > 0) {
            new(targetItems) TItem(std::move(*existingItems));
            existingItems->~TItem();
            ++existingItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->startIndex = 0;
          this->endIndex = (this->endIndex - count) + (this->capacity - this->startIndex);

          while(count > 0) {
            existingItems->~TItem();
            ++existingItems;
            --count;
          }

          throw;
        }
      }

      return targetItems;
    }

    /// <summary>Reallocates the ring buffer's memory to fit the required items</summary>
    /// <param name="requiredItemCount">Number of items the buffer needs to hold</param>
    /// <returns>The address at which the next item can be written</returns>
    /// <remarks>
    ///   Careful: this method does not update the startIndex and endIndex variables,
    ///   as this operation is always followed by writing additional items into the buffer.
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value, TItem *
    >::type reallocateWhenWrapped(std::size_t requiredItemCount) {
      std::size_t newCapacity = BitTricks::GetUpperPowerOfTwo(requiredItemCount);

      // Allocate new memory for the enlarged buffer
      std::unique_ptr<std::uint8_t[]> newItemMemory(
        new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2]
      );
      TItem *targetItems = reinterpret_cast<TItem *>(newItemMemory.get());

      // Copy the older segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        existingItems += this->endIndex;

        std::size_t count = this->capacity - this->startIndex;
        std::memcpy(targetItems, existingItems, count);
        targetItems += count;
      }

      // Copy the newer segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        std::memcpy(targetItems, existingItems, this->endIndex);
        targetItems += this->endIndex;
      }

      // Apply the changes. Note that we do not update startIndex and endIndex here.
      this->itemMemory.swap(newItemMemory);
      this->capacity = newCapacity;

      return targetItems;
    }

    /// <summary>Reallocates the ring buffer's memory to fit the required items</summary>
    /// <param name="requiredItemCount">Number of items the buffer needs to hold</param>
    /// <returns>The address at which the next item can be written</returns>
    /// <remarks>
    ///   Careful: this method does not update the startIndex and endIndex variables,
    ///   as this operation is always followed by writing additional items into the buffer.
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_destructible<T>::value || !std::is_trivially_copyable<T>::value, TItem *
    >::type reallocateWhenLinear(std::size_t requiredItemCount) {
      std::size_t newCapacity = BitTricks::GetUpperPowerOfTwo(requiredItemCount);

      // Allocate new memory for the enlarged buffer
      std::unique_ptr<std::uint8_t[]> swappedItemMemory(
        new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2]
      );
      swappedItemMemory.swap(this->itemMemory);
      this->capacity = newCapacity;

      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());

      // Copy the older segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(swappedItemMemory.get());
        existingItems += this->startIndex;

        std::size_t count = this->endIndex - this->startIndex;
        try {
          while(count > 0) {
            new(targetItems) TItem(std::move(*existingItems));
            existingItems->~TItem();
            ++existingItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->endIndex = (this->endIndex - this->startIndex) - count;
          this->startIndex = 0;

          while(count > 0) {
            existingItems->~TItem();
            ++existingItems;
            --count;
          }

          throw;
        }
      }

      return targetItems;
    }

    /// <summary>Reallocates the ring buffer's memory to fit the required items</summary>
    /// <param name="requiredItemCount">Number of items the buffer needs to hold</param>
    /// <returns>The address at which the next item can be written</returns>
    /// <remarks>
    ///   Careful: this method does not update the startIndex and endIndex variables,
    ///   as this operation is always followed by writing additional items into the buffer.
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value, TItem *
    >::type reallocateWhenLinear(std::size_t requiredItemCount) {
      std::size_t newCapacity = BitTricks::GetUpperPowerOfTwo(requiredItemCount);

      // Allocate new memory for the enlarged buffer
      std::unique_ptr<std::uint8_t[]> newItemMemory(
        new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2]
      );
      TItem *targetItems = reinterpret_cast<TItem *>(newItemMemory.get());

      // Copy the older segment of the existing items into the new buffer
      {
        TItem *existingItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        existingItems += this->startIndex;

        std::size_t count = this->endIndex - this->startIndex;
        std::memcpy(targetItems, existingItems, count);
        targetItems += count;
      }

      // Apply the changes. Note that we do not update startIndex and endIndex here.
      this->itemMemory.swap(newItemMemory);
      this->capacity = newCapacity;

      return targetItems;
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="targetItems">Buffer in which the dequeued items will be stored</param>
    /// <param name="itemCount">Number of items that will be dequeued</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_copyable<T>::value || !std::is_trivially_destructible<T>::value
    >::type extractFromLinear(TItem *targetItems, std::size_t itemCount) {
      std::size_t availableItemCount = this->endIndex - this->startIndex;
      if(likely(availableItemCount >= itemCount)) {
        TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        sourceItems += this->startIndex;
        std::size_t count = itemCount;
        try {
          while(count > 0) {
            *targetItems = std::move(*sourceItems);
            sourceItems->~TItem();
            ++sourceItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->startIndex += (itemCount - count);
          throw;
        }
        if(itemCount == availableItemCount) {
          this->startIndex = InvalidIndex;
#if !defined(NDEBUG)
          this->endIndex = InvalidIndex;
#endif
        } else {
          this->startIndex += itemCount;
        }
      } else {
        assert(
          (availableItemCount >= itemCount) &&
          u8"Ring buffer must contain at least the requested number of items"
        );
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="targetItems">Buffer in which the dequeued items will be stored</param>
    /// <param name="itemCount">Number of items that will be dequeued</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_copyable<T>::value && std::is_trivially_destructible<T>::value
    >::type extractFromLinear(TItem *targetItems, std::size_t itemCount) {
      std::size_t availableItemCount = this->endIndex - this->startIndex;
      if(likely(availableItemCount >= itemCount)) {
        TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        sourceItems += this->startIndex;
        std::memcpy(targetItems, sourceItems, sizeof(TItem) * itemCount);
        if(itemCount == availableItemCount) {
          this->startIndex = InvalidIndex;
#if !defined(NDEBUG)
          this->endIndex = InvalidIndex;
#endif
        } else {
          this->startIndex += itemCount;
        }
      } else {
        assert(
          (availableItemCount >= itemCount) &&
          u8"Ring buffer must contain at least the requested number of items"
        );
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_copyable<T>::value || !std::is_trivially_destructible<T>::value
    >::type extractFromWrapped(TItem *targetItems, std::size_t itemCount) {
      std::size_t availableSegmentItemCount = this->capacity - this->startIndex;
      if(availableSegmentItemCount >= itemCount) { // Enough data in older segment
        TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        sourceItems += this->startIndex;

        std::size_t count = itemCount;
        try {
          while(count > 0) {
            *targetItems = std::move(*sourceItems);
            sourceItems->~TItem();
            ++sourceItems;
            ++targetItems;
            --count;
          }
        }
        catch(...) {
          this->startIndex += (itemCount - count);
          throw;
        }

        // Was all data in the segment consumed?
        // (Note: we know we're wrapped, so we don't need to check if we're empty)
        if(itemCount == availableSegmentItemCount) {
          this->startIndex = 0;
        } else {
          this->startIndex += count;
        }
      } else { // The older segment alone does not have enough data, check younger segment
        std::size_t availableItemCount = availableSegmentItemCount + this->endIndex;
        if(likely(availableItemCount >= itemCount)) { // Is there enough data with both segments together?

          // Move the items from the older segment into the caller-provided buffer
          {
            TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
            sourceItems += this->startIndex;

            std::size_t count = availableSegmentItemCount;
            try {
              while(count > 0) {
                *targetItems = std::move(*sourceItems);
                sourceItems->~TItem();
                ++sourceItems;
                ++targetItems;
                --count;
              }
            }
            catch(...) {
              this->startIndex += (availableSegmentItemCount - count);
            }

            this->startIndex += itemCount;
          }

          itemCount -= availableSegmentItemCount;

          // Move items from the younger segment into the caller-provided buffer
          {
            TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());

            std::size_t count = itemCount;
            try {
              while(count > 0) {
                *targetItems = std::move(*sourceItems);
                sourceItems->~TItem();
                ++sourceItems;
                ++targetItems;
                --count;
              }
            }
            catch(...) {
              this->startIndex = (itemCount - count);
            }
          }

          if(itemCount == this->endIndex) {
            this->startIndex = InvalidIndex;
#if !defined(NDEBUG)
            this->endIndex = InvalidIndex;
#endif
          } else {
            this->startIndex += itemCount;
          }

        } else { // There is insufficient data in the ring buffer
          assert(
            (availableItemCount >= itemCount) &&
            u8"Ring buffer must contain at least the requested number of items"
          );
        }
      }

    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_copyable<T>::value && std::is_trivially_destructible<T>::value
    >::type extractFromWrapped(TItem *targetItems, std::size_t itemCount) {
      std::size_t availableSegmentItemCount = this->capacity - this->startIndex;
      if(availableSegmentItemCount >= itemCount) { // Enough data in older segment
        TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
        sourceItems += this->startIndex;
        std::memcpy(targetItems, sourceItems, sizeof(TItem) * itemCount);

        // Was all data in the segment consumed?
        if(itemCount == availableSegmentItemCount) {
          this->startIndex = 0;
        } else {
          this->startIndex += itemCount;
        }
      } else { // The older segment alone does not have enough data, check younger segment
        std::size_t availableItemCount = availableSegmentItemCount + this->endIndex;
        if(likely(availableItemCount >= itemCount)) { // Is there enough data with both segments together?

          // Move the items from the older segment into the caller-provided buffer
          {
            TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
            sourceItems += this->startIndex;
            std::memcpy(targetItems, sourceItems, sizeof(TItem) * availableSegmentItemCount);
            targetItems += availableSegmentItemCount;
          }

          itemCount -= availableSegmentItemCount;

          // Move items from the younger segment into the caller-provided buffer
          {
            TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
            std::memcpy(targetItems, sourceItems, sizeof(TItem) * itemCount);
          }

          if(itemCount == this->endIndex) {
            this->startIndex = InvalidIndex;
#if !defined(NDEBUG)
            this->endIndex = InvalidIndex;
#endif
          } else {
            this->startIndex += itemCount;
          }

        } else { // There is insufficient data in the ring buffer
          assert(
            (availableItemCount >= itemCount) &&
            u8"Ring buffer must contain at least the requested number of items"
          );
        }
      }

    }

    /// <summary>Holds the items stored in the ring buffer</summary>
    private: std::unique_ptr<std::uint8_t[]> itemMemory;
    /// <summary>Number of items the ring buffer can currently hold</summary>
    private: std::size_t capacity;
    /// <summary>Index of the first item in the ring buffer</summary>
    private: std::size_t startIndex;
    /// <summary>Index one past the last item</summary>
    private: std::size_t endIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_RINGQUEUE_H
