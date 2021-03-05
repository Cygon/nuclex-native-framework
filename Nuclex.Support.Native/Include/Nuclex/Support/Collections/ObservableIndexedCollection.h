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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Event.h"

#include <cstdlib>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides notifications for when an indexed collection changes</summary>
  /// <remarks>
  ///   <para>
  ///     This does not monitor the state of items stored in the collection, it just reports
  ///     when the items are added, removed or replaced in the collection.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class ObservableIndexedCollection {

    /// <summary>Fired when an item has been added to the collection</summary>
    /// <param name="index">Index at which the item has been added</param>
    /// <param name="value">Item that has been added to the collection</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &value)
    > ItemAdded;

    /// <summary>Fired when an item has been removed from the collection</summary>
    /// <param name="index">Index at which the item has been removed</param>
    /// <param name="value">Item that has been removed from the collection</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &value)
    > ItemRemoved;

    /// <summary>Fired when an item in the collection has been replaced</summary>
    /// <param name="index">Index at which the item has been replaced</param>
    /// <param name="oldValue">Item that is no longer part of the collection</param>
    /// <param name="newValue">Item that has taken the place of the old item</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &oldValue, const TValue &newValue)
    > ItemReplaced;

    // public: mutable Event Clearing();
    // public: mutable Event Cleared();

    /// <summary>Initializes a new observable indexed collection</summary>
    protected: ObservableIndexedCollection() = default;
    /// <summary>Frees all memory used by the observable collection</summary>
    public: virtual ~ObservableIndexedCollection() = default;

    //private: ObservableIndexedCollection(const ObservableIndexedCollection &) = delete;
    //private: ObservableIndexedCollection &operator =(
    //  const ObservableIndexedCollection &
    //) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H
