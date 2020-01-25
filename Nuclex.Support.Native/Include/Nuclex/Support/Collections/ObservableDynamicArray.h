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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEDYNAMICARRAY_H
#define NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEDYNAMICARRAY_H

#include "IndexedCollection.h"
#include "ObservableCollection.h"
#include "ObservableIndexedCollection.h"

#include <vector>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dynamic array that sends out change notifications</summary>
  /// <remarks>
  ///   This collection sends out notifications to any interested party when its contents
  ///   change (items being reordered, added or removed). It has no way of knowing when
  ///   internal changes to an item itself occur.
  /// </remarks>
  template<typename TValue>
  class ObservableDynamicArray :
    public IndexedCollection<TValue>,
    public virtual ObservableCollection<TValue>,
    public virtual ObservableIndexedCollection<TValue> {

    /// <summary>Invalid index used to indicate when a requested item wasn't found</summary>
    public: using IndexedCollection<TValue>::InvalidIndex;

    /// <summary>Initializes a new dynamic array</summary>
    public: ObservableDynamicArray() = default;

    /// <summary>Frees all memory used by the collection</summary>
    public: virtual ~ObservableDynamicArray() = default;

    /// <summary>Initializes a dynamic array reserving memory up-front</summary>
    /// <param name="capacity">Capacity for which memory will be reserved</param>
    public: void Reserve(std::size_t capacity) {
      this->items.reserve(capacity);
    }

    /// <summary>Determines the index of the specified item in the collection</summary>
    /// <param name="value">Item whose index will be determined</param>
    /// <returns>The index of the specified item</returns>
    public: std::size_t GetIndexOf(const TValue &value) const override {
      std::size_t count = this->items.size();
      for(std::size_t index = 0; index < count; ++index) {
        if(this->items[index] == value) {
          return index;
        }
      }

      return InvalidIndex;
    }

    /// <summary>Retrieves the item at the specified index</summary>
    /// <param name="index">Index of the item that will be retrieved</param>
    /// <returns>The item at the specified index</returns>
    public: const TValue &GetAt(std::size_t index) const override {
      return this->items.at(index);
    }

    /// <summary>Accesses the item at the specified index</summary>
    /// <param name="index">Index of the item that will be accessed</param>
    /// <returns>The item at the specified index</returns>
    public: TValue &GetAt(std::size_t index) override {
      return this->items.at(index);
    }

    /// <summary>Assigns the specified item to the specified index</summary>
    /// <param name="index">Index at which the item will be stored</param>
    /// <param name="value">Item that will be stored at the specified index</param>
    public: void SetAt(std::size_t index, const TValue &value) override {
      if(index < this->items.size()) {
        bool removedItemNeeded = (
          (ObservableIndexedCollection<TValue>::ItemReplaced.CountSubscribers() > 0) ||
          (ObservableCollection<TValue>::ItemRemoved.CountSubscribers() > 0)
        );
        if(removedItemNeeded) {
          TValue old = this->items[index];
          this->items[index] = value;
          ObservableIndexedCollection<TValue>::ItemReplaced(index, old, value);
          ObservableCollection<TValue>::ItemRemoved(old);
          ObservableCollection<TValue>::ItemAdded(value);
        } else {
          this->items[index] = value;
          ObservableCollection<TValue>::ItemAdded(value);
        }
      } else { // Let .at() throw the appropriate out-of-bounds exception
        this->items.at(index) = value;
      }
    }

    /// <summary>Inserts the specified item at a specified index</summary>
    /// <param name="index">Index at which the item will be inserted</param>
    /// <param name="value">Item that will be inserted into the collection</param>
    public: void InsertAt(std::size_t index, const TValue &value) override {
      typename std::vector<TValue>::iterator where = this->items.begin() + index;
      this->items.insert(where, value);
      ObservableIndexedCollection<TValue>::ItemAdded(index, value);
      ObservableCollection<TValue>::ItemAdded(value);
    }

    /// <summary>Removes the item at the specified index from the collection</summary>
    /// <param name="index">Index at which the item will be removed</param>
    public: void RemoveAt(std::size_t index) override {
      typename std::vector<TValue>::iterator where = this->items.begin() + index;
      bool erasedItemNeeded = (
        (ObservableIndexedCollection<TValue>::ItemRemoved.CountSubscribers() > 0) ||
        (ObservableCollection<TValue>::ItemRemoved.CountSubscribers() > 0)
      );
      if(erasedItemNeeded) {
        TValue value = *where;
        this->items.erase(where);
        ObservableIndexedCollection<TValue>::ItemRemoved(index, value);
        ObservableCollection<TValue>::ItemRemoved(value);
      } else {
        this->items.erase(where);
      }
    }

    /// <summary>Adds the specified item to the collection</summary>
    /// <param name="item">Item that will be added to the collection</param>
    public: void Add(const TValue &item) override {
      this->items.push_back(item);
      ObservableIndexedCollection<TValue>::ItemAdded(this->items.size() - 1, item);
      ObservableCollection<TValue>::ItemAdded(item);
    }

    /// <summary>Removes the specified item from the collection</summary>
    /// <param name="item">Item that will be removed from the collection</param>
    /// <returns>True if the item existed in the collection and was removed</returns>
    public: bool Remove(const TValue &item) override {
      for(
        typename std::vector<TValue>::iterator iterator = this->items.begin();
        iterator != this->items.end();
        ++iterator
      ) {
        if(*iterator == item) {
          this->items.erase(iterator);
          ObservableIndexedCollection<TValue>::ItemRemoved(iterator - this->items.begin, item);
          ObservableCollection<TValue>::ItemRemoved(item);
          return true;
        }
      }

      return false;
    }

    /// <summary>Removes all items from the collection</summary>
    public: void Clear() override {
      std::size_t count = this->items.size();
      bool erasedItemNeeded = (
        (ObservableIndexedCollection<TValue>::ItemRemoved.CountSubscribers() > 0) ||
        (ObservableCollection<TValue>::ItemRemoved.CountSubscribers() > 0)
      );
      if(erasedItemNeeded && (count > 0)) {
        std::vector<TValue> removed;
        removed.reserve(this->items.capacity());
        removed.swap(this->items);
        while(count > 0) {
          --count;
          ObservableIndexedCollection<TValue>::ItemRemoved(count, removed[count]);
          ObservableCollection<TValue>::ItemRemoved(removed[count]);
        }
      } else {
        this->items.clear();
      }
    }

    /// <summary>Checks if the collection contains the specified item</summary>
    /// <param name="item">Item the collection will be checked for</param>
    /// <returns>True if the collection contain the specified item, false otherwise</returns>
    public: bool Contains(const TValue &item) const override {
      std::size_t count = this->items.size();
      for(std::size_t index = 0; index < count; ++index) {
        if(this->items[index] == item) {
          return true;
        }
      }

      return false;
    }

    /// <summary>Counts the number of items in the collection</summary>
    /// <returns>The number of items the collection contains</returns>
    public: std::size_t Count() const override {
      return this->items.size();
    }

    /// <summary>Checks if the collection is empty</summary>
    /// <returns>True if the collection is empty</returns>
    public: bool IsEmpty() const override {
      return this->items.empty();
    }

    /// <summary>Items stored in the dynamic array</summary>
    private: std::vector<TValue> items;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEDYNAMICARRAY_H
