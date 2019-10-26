#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H

#include <cstdlib>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Set of items that can be accessed sequentially</summary>
  /// <remarks>
  ///   <para>
  ///     The collection interface is not meant for local or private use within your
  ///     code since the Standard C++ Library contains a host of very well designed and
  ///     powerful container classes that are the envy of other languages far and wide.
  ///   </para>
  ///   <para>
  ///     Use this interface where you might want to expose a collection to users of
  ///     a library without binding yourself to a specific container type. This allows
  ///     you to expose a list of things in a natural and consistent way without
  ///     duplicating Add() and Remove() methods, while keeping complete freedom over
  ///     the actual data structure used to store the items.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class Collection {

    #pragma region class Enumerator

    /// <summary>Iterates over the items in the collection</summary>
    public: class Enumerator {

      /// <summary>Frees any memory used by the enumerator</summary>
      public: virtual ~Enumerator();

      /// <summary>Advances to the next item in the collection, if available</summary>
      /// <returns>True if there was a next item, false if the end was reached</returns>
      public: bool Advance();

      /// <summary>Retrieves the item at the current enumerator position</summary>
      /// <remarks>
      ///   The enumerator starts out on an empty position, so you have to call
      ///   <see cref="Advance" /> as the very first method of a new enumerator
      ///   (and if the collection is empty, that first call will return false).
      /// </remarks>
      public: const TValue &Get();

      private: Enumerator(const Enumerator &) = delete;
      private: Enumerator &operator =(const Enumerator &) = delete;

    };

    #pragma endregion // class Enumerator

    /// <summary>Initializes a new collection</summary>
    protected: Collection() = default;
    /// <summary>Frees all memory used by the collection</summary>
    public: virtual ~Collection() = default;

    /// <summary>Adds the specified item to the collection</summary>
    /// <param name="item">Item that will be added to the collection</param>
    public: virtual void Add(const TValue &item) = 0;

    /// <summary>Removes the specified item from the collection</summary>
    /// <param name="item">Item that will be removed from the collection</param>
    /// <returns>True if the item existed in the collection and was removed</returns>
    public: virtual bool Remove(const TValue &item) = 0;

    /// <summary>Removes all items from the collection</summary>
    public: virtual void Clear() = 0;

    /// <summary>Checks if the collection contains the specified item</summary>
    /// <param name="item">Item the collection will be checked for</param>
    /// <returns>True if the collection contain the specified item, false otherwise</returns>
    public: virtual bool Contains(const TValue &item) const = 0;

    /// <summary>Counts the number of items in the collection</summary>
    /// <returns>The number of items the collection contains</returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the collection is empty</summary>
    /// <returns>True if the collection is empty</returns>
    public: virtual bool IsEmpty() const = 0;

    //private: Collection(const Collection &) = delete;
    //private: Collection &operator =(const Collection &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H
