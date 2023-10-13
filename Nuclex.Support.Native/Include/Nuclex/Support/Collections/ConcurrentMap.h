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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTMAP_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTMAP_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Associative key-value map that can safely be used from multiple threads</summary>
  /// <typeparam name="TKey">Type of the key the map uses</typeparam>
  /// <typeparam name="TValue">Type of values that are stored in the map</typeparam>
  template<typename TKey, typename TValue>
  class ConcurrentMap {

    /// <summary>Destroys the concurrent map</summary>
    public: virtual ~ConcurrentMap() = default;

    /// <summary>Tries to insert an element into the map in a thread-safe manner</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key in the map</param>
    /// <returns>
    ///   True if the element was inserted,
    ///   false if the key already existed or there was no space left
    /// </returns>
    public: virtual bool TryInsert(const TKey &key, const TValue &value) = 0;

    /// <summary>Tries to move-insert an element into the map in a thread-safe manner</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be move-inserted under its key in the map</param>
    /// <returns>
    ///   True if the element was inserted,
    ///   false if the key already existed or there was no space left
    /// </returns>
    public: virtual bool TryInsert(const TKey &key, TValue &&value) = 0;

    /// <summary>Tries to take an element from the map (removing it)</summary>
    /// <param name="key">Key of the element that will be taken from the map</param>
    /// <param name="value">Will receive the value taken from the map</param>
    /// <returns>
    ///   True if an element was taken from the map, false if the key didn't exist (anymore?)
    /// </returns>
    public: virtual bool TryTake(const TKey &key, TValue &value) = 0;

    // CHECK: Could provide a TryTake() with output method to allow for move semantics.
    //public: virtual bool TryTake(const TKey &key, const std::function<TValue &&> &valueCallback) = 0;

    /// <summary>Counts the numebr of elements currently in the map</summary>
    /// <returns>
    ///   The approximate number of elements that had been in the map during the call
    /// </returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the map is empty</summary>
    /// <returns>True if the map had been empty during the call</returns>
    public: virtual bool IsEmpty() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTMAP_H
