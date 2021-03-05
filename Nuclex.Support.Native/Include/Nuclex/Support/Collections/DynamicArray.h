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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_DYNAMICARRAY_H
#define NUCLEX_SUPPORT_COLLECTIONS_DYNAMICARRAY_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Collections/IndexedCollection.h"

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Simple dynamic array exposed under the <see cref="IndexedCollection" /> interface
  /// </summary>
  /// <remarks>
  ///   Use std::vector&lt;&gt; directly for library-internal data. This wrapper is intended
  ///   for when you want to expose a collection of items across DLL boundaries or if you
  ///   need to hide the actual container used from a public interface in order to stay
  ///   flexible in its implementation.
  /// </remarks>
  template<typename TValue>
  class DynamicArray : public IndexedCollection<TValue> {

    public: using IndexedCollection<TValue>::InvalidIndex;

    /// <summary>Initializes a new dynamic array</summary>
    public: DynamicArray() = default;

    /// <summary>Frees all memory used by the collection</summary>
    public: virtual ~DynamicArray() = default;

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
      this->items.at(index) = value;
    }

    /// <summary>Inserts the specified item at a specified index</summary>
    /// <param name="index">Index at which the item will be inserted</param>
    /// <param name="value">Item that will be inserted into the collection</param>
    public: void InsertAt(std::size_t index, const TValue &value) override {
      typename std::vector<TValue>::iterator where = this->items.begin() + index;
      this->items.insert(where, value);
    }

    /// <summary>Removes the item at the specified index from the collection</summary>
    /// <param name="index">Index at which the item will be removed</param>
    public: void RemoveAt(std::size_t index) override {
      typename std::vector<TValue>::iterator where = this->items.begin() + index;
      this->items.erase(where);
    }

    /// <summary>Adds the specified item to the collection</summary>
    /// <param name="item">Item that will be added to the collection</param>
    public: void Add(const TValue &item) override {
      this->items.push_back(item);
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
          return true;
        }
      }

      return false;
    }

    /// <summary>Removes all items from the collection</summary>
    public: void Clear() override {
      this->items.clear();
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

#endif // NUCLEX_SUPPORT_COLLECTIONS_DYNAMICARRAY_H
