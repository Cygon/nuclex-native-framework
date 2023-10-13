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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Set that can safely be used from multiple threads</summary>
  /// <typeparam name="TKey">Type of the key the set will keep track of</typeparam>
  template<typename TKey>
  class ConcurrentSet {

    /// <summary>Destroys the concurrent set</summary>
    public: virtual ~ConcurrentSet() = default;

    /// <summary>Tries to insert a key into the set in a thread-safe manner</summary>
    /// <param name="key">Key that will be inserted into the set</param>
    /// <returns>True if the key was inserted, false if the key already existed</returns>
    public: virtual bool TryInsert(const TKey &key) = 0;

    /// <summary>Tries to remove a key from the set</summary>
    /// <param name="key">Key that will be removed from the set</param>
    /// <returns>
    ///   True if the key was removed from the set, false if the key didn't exist (anymore?)
    /// </returns>
    public: virtual bool TryRemove(const TKey &key) = 0;

    /// <summary>Counts the numebr of keys currently in the set</summary>
    /// <returns>
    ///   The approximate number of keys that had been in the set during the call
    /// </returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the set is empty</summary>
    /// <returns>True if the set had been empty during the call</returns>
    public: virtual bool IsEmpty() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
