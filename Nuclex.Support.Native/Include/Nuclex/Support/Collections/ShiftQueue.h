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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_SHIFTQUEUE_H
#define NUCLEX_SUPPORT_COLLECTIONS_SHIFTQUEUE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/ScopeGuard.h"
#include "Nuclex/Support/BitTricks.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <memory> // for std::unique_ptr
#include <cassert> // for assert()
#include <cstring> // for std::memcpy()
#include <type_traits> // for std::enable_if<>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A buffer that acts like a ring buffer but guarantees linear memory</summary>
  /// <remarks>
  ///   <para>
  ///     <strong>Thread safety:</strong> each instance should be accessed by a single thread
  ///   </para>
  ///   <para>
  ///     <strong>Container type:</strong> unbounded linear buffer with batch operations
  ///   </para>
  ///   <para>
  ///     This is a buffer for FIFO batch operations like the ring buffer, but instead of
  ///     wrapping data around, it will keep all data linear. This can be less efficient than
  ///     a ring buffer if there are lots of partial updates, but can also be more efficient
  ///     in cases where the buffer is mostly or completely emptied regularly.
  ///   </para>
  ///   <para>
  ///     It works by naively accumulating data in a buffer. Reads advance a read pointer,
  ///     leaving unused memory behind. When writing to the buffer, if more space is wasted
  ///     than there is data in the buffer, all data is shifted back to the front of the
  ///     buffer. This is a fairly good heuristic so long as your reads typically consume
  ///     most of the buffer.
  ///   </para>
  ///   <para>
  ///     In contrast to a ring buffer, this buffer also allows you to obtain a pointer to
  ///     the data it holds, allowing for extra efficiency if the data can be processed
  ///     directly from a buffer. You can also obtain a pointer to write into the buffer.
  ///   </para>
  ///   <para>
  ///     This class offers the <em>basic</em> exception guarantee: if your items throw
  ///     in their copy or move constructors throw, the ring buffer will remain in a usable
  ///     state and not leak memory, but operations may end up applied partially,
  ///     i.e. a read may fail and return nothing, yet kill half your buffer contents.
  ///   </para>
  /// </remarks>
  template<typename TItem>
  class ShiftQueue {

    /// <summary>Initializes a new shift buffer</summary>
    /// <param name="capacity">Storage space in the shift  buffer at the beginning</param>
    public: ShiftQueue(std::size_t capacity = 256) :
      itemMemory(
        new std::uint8_t[sizeof(TItem[2]) * BitTricks::GetUpperPowerOfTwo(capacity) / 2]
      ),
      capacity(BitTricks::GetUpperPowerOfTwo(capacity)),
      startIndex(0),
      endIndex(0) {}

    /// <summary>Initializes a shift buffer as a copy of another shift buffer</summary>
    /// <param name="other">Other shift buffer that will be copied</param>
    public: ShiftQueue(const ShiftQueue &other) :
      itemMemory(new std::uint8_t[sizeof(TItem[2]) * other.capacity / 2]),
      capacity(other.capacity),
      startIndex(0),
      endIndex(0) {
      const TItem *sourceItems = (
        reinterpret_cast<const TItem *>(other.itemMemory.get()) + other.startIndex
      );
      emplaceItems(sourceItems, other.endIndex - other.startIndex);
    }

    /// <summary>Initializes a shift buffer taking over another shift buffer</summary>
    /// <param name="other">Other shift buffer that will be taken over</param>
    public: ShiftQueue(ShiftQueue &&other) :
      itemMemory(std::move(other.itemMemory)),
      capacity(other.capacity),
      startIndex(other.startIndex),
      endIndex(other.endIndex) {
      other.startIndex = other.endIndex = 0; // Ensure other doesn't try to destroy items
    }

    /// <summary>Destroys the shift buffer and all items in it</summary>
    public: ~ShiftQueue() {
      if(this->startIndex != this->endIndex) {
        TItem *items = reinterpret_cast<TItem *>(this->itemMemory.get()) + this->startIndex;
        for(std::size_t index = this->startIndex; index < this->endIndex; ++index) {
          items->~TItem();
          ++items;
        }
      }
    }

    /// <summary>Returns the number of items the shift buffer has allocated memory for</summary>
    /// <returns>The number of items the shift buffer has reserved space for</returns>
    /// <remarks>
    ///   Just like std::vector::capacity(), this is not a limit. If the capacity is
    ///   exceeded, the shift buffer will allocate a larger memory block and use that one.
    /// </remarks>
    public: std::size_t GetCapacity() const {
      return this->capacity;
    }

    /// <summary>Counts the number of items currently stored in the shift buffer</summary>
    public: std::size_t Count() const {
      return this->endIndex - this->startIndex;
    }

    /// <summary>Provides direct read access to the items stored in the buffer</summary>
    /// <returns>
    ///   A pointer to the oldest item in the buffer, following sequentially by
    ///   all newer items in the order they were written
    /// </returns>
    public: const TItem *Access() const {
      return reinterpret_cast<const TItem *>(this->itemMemory.get()) + this->startIndex;
    }

    /// <summary>Skips the specified number of items</summary>
    /// <param name="skipItemCount">Number of items that will be skipped</param>
    public: void Skip(std::size_t skipItemCount) {
      assert(
        ((this->startIndex + skipItemCount) <= this->endIndex) &&
        u8"Amount of data skipped must be less or equal to the amount of data in the buffer"
      );
      skipItems(skipItemCount);
    }

    /// <summary>Reads items out of the buffer, starting with the oldest item</summary>
    /// <param name="items">Memory to which the items will be moved</param>
    /// <param name="count">Number of items that will be read from the buffer</param>
    public: void Read(TItem *items, std::size_t count) {
      assert(
        ((this->startIndex + count) <= this->endIndex) &&
        u8"Amount of data read must be less or equal to the amount of data in the buffer"
      );
      extractItems(items, count);
    }

    /// <summary>Copies the specified number of items into the shift buffer</summary>
    /// <param name="items">Items that will be copied into the shift buffer</param>
    /// <param name="count">Number of items that will be copied</param>
    public: void Write(const TItem *items, std::size_t count) {
      makeSpace(count);
      emplaceItems(items, count);
    }

    /// <summary>Moves the specified number of items into the shift buffer</summary>
    /// <param name="items">Items that will be moves into the shift buffer</param>
    /// <param name="count">Number of items that will be moves</param>
    public: void Shove(TItem *items, std::size_t count) {
      makeSpace(count);
      moveEmplaceItems(items, count);
    }

#if 1 // Cool, efficient and an invitation to shoot yourself in the foot

    /// <summary>
    ///   Promises the shift buffer to write the specified number of items before
    ///   the next call to any non-const method
    /// </summary>
    /// <param name="itemCount">Number of items to promise the shift buffer</param>
    /// <returns>The address at which the items must be written</returns>
    /// <remarks>
    ///   <para>
    ///     Warning! The returned pointer is to uninitialized memory. That means assigning
    ///     the items is an error, they must be constructed into their addresses via
    ///     placement new, not assignment! (unless they're std::is_trivially_copyable,
    ///     in which case, std::memcpy() away)
    ///   </para>
    ///   <para>
    ///     Additional Warning! After calling this method, the shift buffer will attempt
    ///     to destroy the promised items if it is itself destroyed or needs to shuffle
    ///     items around. If you do not fill the promised items (or are interrupted by
    ///     an item constructor throwing an exception), you have to take care to call
    ///     <see cref="Unpromise" /> to revert your promise in all cases!
    ///   </para>
    /// </remarks>
    protected: TItem *Promise(std::size_t itemCount) {
      makeSpace(itemCount);

      TItem *items = reinterpret_cast<TItem *>(this->itemMemory.get()) + this->endIndex;
      this->endIndex += itemCount;

      return items;
    }

    /// <summary>Reverses a promise of data given via <see cref="Promise" /></summary>
    /// <param name="itemCount">Number of items for which to reverse the promise</param>
    /// <remarks>
    ///   Warning! You must not reverse a promise for more data than you promised with
    ///   your last call to <see cref="Promise" />. The items for which you reverse your
    ///   promise will be considered uninitialized memory again and will not have their
    ///   destructors called.
    /// </remarks>
    protected: void Unpromise(std::size_t itemCount) {
      this->endIndex -= itemCount;
      assert(
        (this->endIndex >= startIndex) &&
        u8"Promise reversal can not deny more data than was promised earlier"
      );
    }

#endif

    /// <summary>Ensures that space is available for the specified number of items</summary>
    /// <param name="itemCount">Number of items for which space will be made available</param>
    /// <remarks>
    ///   When this method finishes, there will be enough space between
    ///   <see cref="endIndex" /> and <see cref="capacity" /> to fit the requested number
    ///   of items. If there was enough space in the first place, this method does nothing.
    /// </remarks>
    private: void makeSpace(std::size_t itemCount) {
      std::size_t usedItemCount = this->endIndex - this->startIndex;

      // Is more space in the buffer inaccessible than is occupied by items?
      if(this->startIndex > usedItemCount) {

        // If the buffer needs to be resized anyway, we don't need to shift back
        // and can do the resize + shift in one operation
        std::size_t totalItemCount = usedItemCount + itemCount;
        if(likely(this->capacity >= totalItemCount)) {
          TItem *items = reinterpret_cast<TItem *>(this->itemMemory.get()) + this->startIndex;
          shiftItems(items, usedItemCount);
        } else { // No buffer resize needed, just shift the items back
          this->capacity = BitTricks::GetUpperPowerOfTwo(this->startIndex + totalItemCount);
          {
            std::unique_ptr<std::uint8_t[]> newItemMemory(
              new std::uint8_t[sizeof(TItem[2]) * this->capacity / 2]
            );
            newItemMemory.swap(this->itemMemory);

            TItem *items = reinterpret_cast<TItem *>(newItemMemory.get()) + this->startIndex;
            shiftItems(items, usedItemCount);
          }
        }

      } else { // The inaccessible space in the buffer is less than the used space

        // If the space at the end of the buffer is too small, allocate a new buffer
        // two times the required size. This ensures that the buffer will settle into
        // a read-shift-fill cycle without resizes if the current usage pattern repeats.
        std::size_t freeItemCount = this->capacity - this->endIndex;
        if(likely(freeItemCount >= itemCount)) {
          // Enough space available, no action needed
        } else {
          this->capacity = BitTricks::GetUpperPowerOfTwo((usedItemCount + itemCount) * 2);
          {
            std::unique_ptr<std::uint8_t[]> newItemMemory(
              new std::uint8_t[sizeof(TItem[2]) * this->capacity / 2]
            );
            newItemMemory.swap(this->itemMemory);

            TItem *items = reinterpret_cast<TItem *>(newItemMemory.get()) + this->startIndex;
            shiftItems(items, usedItemCount);
          }
        }

      }
    }

    /// <summary>Copies the specified items into the already available buffer</summary>
    /// <param name="sourceItems">Items that will be copied into the buffer</param>
    /// <param name="itemCount">Number of items that will be copied</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_copyable<T>::value>::type emplaceItems(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      targetItems += this->endIndex;

      std::size_t count = itemCount;
      {
        ON_SCOPE_EXIT {
          this->endIndex += itemCount - count;
        };

        while(count > 0) {
          new(targetItems) TItem(*sourceItems);
          ++sourceItems;
          ++targetItems;
          --count;
        }
      }
    }

    /// <summary>Copies the specified items into the already available buffer</summary>
    /// <param name="sourceItems">Items that will be copied into the buffer</param>
    /// <param name="itemCount">Number of items that will be copied</param>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type emplaceItems(
      const TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      targetItems += this->endIndex;
      std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));
      this->endIndex += itemCount;
    }

    /// <summary>Moves the specified items into the already available buffer</summary>
    /// <param name="sourceItems">Items that will be copied into the buffer</param>
    /// <param name="itemCount">Number of items that will be copied</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_copyable<T>::value>::type moveEmplaceItems(
      TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      targetItems += this->endIndex;

      std::size_t count = itemCount;
      {
        ON_SCOPE_EXIT {
          this->endIndex += itemCount - count;
        };

        while(count > 0) {
          new(targetItems) TItem(std::move(*sourceItems));
          // no d'tor call here, source isn't ours and will be destroyed externally
          ++sourceItems;
          ++targetItems;
          --count;
        }
      }
    }

    /// <summary>Moves the specified items into the already available buffer</summary>
    /// <param name="sourceItems">Items that will be copied into the buffer</param>
    /// <param name="itemCount">Number of items that will be copied</param>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type moveEmplaceItems(
      TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      targetItems += this->endIndex;
      std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));
      this->endIndex += itemCount;
    }

    /// <summary>Takes the specified number of items out of the buffer</summary>
    /// <param name="targetItems">Address at which the items will be placed</param>
    /// <param name="itemCount">Number of items that will be extracted</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_copyable<T>::value || !std::is_trivially_destructible<T>::value
    >::type extractItems(
      TItem *targetItems, std::size_t itemCount
    ) {
      TItem *sourceItems = (
        reinterpret_cast<TItem *>(this->itemMemory.get()) + this->startIndex
      );
      std::size_t count = itemCount;
      try {
        while(count > 0) {
          *targetItems = std::move(*sourceItems);
          sourceItems->~TItem();
          ++targetItems;
          ++sourceItems;
          --count;
        }

        this->startIndex += itemCount;
      }
      catch(...) {
        this->startIndex += (itemCount - count);
        throw;
      }
    }

    /// <summary>Takes the specified number of items out of the buffer</summary>
    /// <param name="targetItems">Address at which the items will be placed</param>
    /// <param name="itemCount">Number of items that will be extracted</param>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_copyable<T>::value && std::is_trivially_destructible<T>::value
    >::type extractItems(
      TItem *targetItems, std::size_t itemCount
    ) {
      TItem *sourceItems = reinterpret_cast<TItem *>(this->itemMemory.get());
      std::memcpy(targetItems, sourceItems, itemCount * sizeof(TItem));
      this->startIndex += itemCount;
    }

    /// <summary>Skips the specified number of items in the buffer</summary>
    /// <param name="itemCount">Number of items that will be skipped</param>
    private: template<typename T = TItem>
    typename std::enable_if<!std::is_trivially_destructible<T>::value>::type skipItems(
      std::size_t itemCount
    ) {
      TItem *sourceItems = (
        reinterpret_cast<TItem *>(this->itemMemory.get()) + this->startIndex
      );

      std::size_t count = itemCount;
      while(count > 0) {
        sourceItems->~TItem();
        ++sourceItems;
        --count;
      }

      this->startIndex += itemCount;
    }

    /// <summary>Skips the specified number of items in the buffer</summary>
    /// <param name="itemCount">Number of items that will be skipped</param>
    /// <remarks>
    ///   Storage occupied by trivially destructible objects may be reused without
    ///   calling the destructor - so we don't.
    /// </remarka>
    private: template<typename T = TItem>
    typename std::enable_if<std::is_trivially_destructible<T>::value>::type skipItems(
      std::size_t itemCount
    ) {
      this->startIndex += itemCount;
    }

    /// <summary>Moves the items from another location into the buffer</summary>
    /// <param name="sourceItems">Items that will be moved into the buffer</param>
    /// <param name="itemCount">Number of items that will be moved</param>
    /// <remarks>
    ///   <para>
    ///     The source buffer can be the ShiftQueue's own memory so long as there is
    ///     no overlap between the items to be moved and the target memory range.
    ///   </para>
    ///   <para>
    ///     In all cases, the target is this buffer's own memory, starting at index 0.
    ///     The start and end index of the instance will be updated accordingly.
    ///   </para>
    ///   <para>
    ///     Guarantees that all items in the source buffer up to the specified item count
    ///     are either moved or destroyed after the call.
    ///   </para>
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      !std::is_trivially_copyable<T>::value || !std::is_trivially_destructible<T>::value
    >::type shiftItems(
      TItem *sourceItems, std::size_t itemCount
    ) {
      TItem *targetItems = reinterpret_cast<TItem *>(this->itemMemory.get());

      this->startIndex = 0;

      std::size_t count = itemCount;
      {
        auto cleanUp = ON_SCOPE_EXIT_TRANSACTION {
          this->endIndex = itemCount - count;

          // Move failed, kill all items that remain in the source buffer. Moving
          // the rest would result in skipping an item in the buffer and risking
          // another exception. We can't deal with segmented buffers either.
          while(count > 0) {
            sourceItems->~TItem();
            ++sourceItems;
            --count;
          }
        };

        // Move all items from their old location to their new location
        while(count > 0) {
          new(targetItems) TItem(std::move(*sourceItems));
          sourceItems->~TItem();
          ++sourceItems;
          ++targetItems;
          --count;
        }

        cleanUp.Commit();
      }

      // Move succeeded, the new end index is the number of items we have moved
      this->endIndex = itemCount;
    }

    /// <summary>Moves the items from another location into the buffer</summary>
    /// <param name="sourceItems">Items that will be moved into the buffer</param>
    /// <param name="itemCount">Number of items that will be moved</param>
    /// <remarks>
    ///   Variant for trivially copyable items where we don't have to worry about
    ///   individual items throwing inside the move constructor.
    /// </remarks>
    private: template<typename T = TItem>
    typename std::enable_if<
      std::is_trivially_copyable<T>::value && std::is_trivially_destructible<T>::value
    >::type shiftItems(
      TItem *sourceItems, std::size_t itemCount
    ) {
      // std::copy_n() would use std::memmove(), but we know there is no overlap, so:
      std::memcpy(this->itemMemory.get(), sourceItems, itemCount * sizeof(TItem));
      this->startIndex = 0;
      this->endIndex = itemCount;
    }

    /// <summary>Holds the items stored in the shift buffer</summary>
    private: std::unique_ptr<std::uint8_t[]> itemMemory;
    /// <summary>Number of items the shift buffer can currently hold</summary>
    private: std::size_t capacity;
    /// <summary>Index of the first item in the shift buffer</summary>
    private: std::size_t startIndex;
    /// <summary>Index one past the last item</summary>
    private: std::size_t endIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_SHIFTQUEUE_H
