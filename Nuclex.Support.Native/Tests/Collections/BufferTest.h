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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_BUFFERTEST_H
#define NUCLEX_SUPPORT_COLLECTIONS_BUFFERTEST_H

#include <gtest/gtest.h>

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tracks what happens with its associated test item</summary>
  struct TestItemStats {

    /// <summary>Number of times the item was the source of a copy</summary>
    public: int CopyCount = 0;
    /// <summary>Number of times the item was the source of a move</summary>
    public: int MoveCount = 0;
    /// <summary>Number of times an associated item was destroyed</summary>
    public: int DestroyCount = 0;
    /// <summary>Number of times an associated item was assigned to</summary>
    public: int OverwriteCount = 0;
    /// <summary>Whether the associated item's copy constructor should fail</summary>
    public: bool ThrowOnCopy = false;
    /// <summary>Whether the associated item's move constructor should fail</summary>
    public: bool ThrowOnMove = false;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy item to test correct copy and move semantics of the shift buffer</summary>
  class TestItem {

    /// <summary>Initializes a new item</summary>
    public: TestItem(const std::shared_ptr<TestItemStats> &stats) :
      stats(stats) {}

    /// <summary>Initializes an item as copy of another item</summary>
    /// <param name="other">Other item that will be copied</param>
    public: TestItem(const TestItem &other) :
      stats(other.stats) {
      ++this->stats->CopyCount;
      if(this->stats->ThrowOnCopy) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
    }

    /// <summary>Initializes an item by taking over an existing item</summary>
    /// <param name="other">Other item that will be taken over</param>
    public: TestItem(TestItem &&other) :
      stats(other.stats) { // No move, we want to still track destruction
      ++this->stats->MoveCount;
      if(this->stats->ThrowOnMove) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
    }

    /// <summary>Destroys the item</summary>
    public: ~TestItem() {
      ++this->stats->DestroyCount;
    }

    /// <summary>Initializes an item as copy of another item</summary>
    /// <param name="other">Other item that will be copied</param>
    /// <returns>This instance</returns>
    public: TestItem &operator =(const TestItem &other) {
      ++this->stats->OverwriteCount;
      this->stats = other.stats;
      ++this->stats->CopyCount;
      if(this->stats->ThrowOnCopy) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
      return *this;
    }

    /// <summary>Initializes an item by taking over an existing item</summary>
    /// <param name="other">Other item that will be taken over</param>
    /// <returns>This instance</returns>
    public: TestItem &operator =(TestItem &&other) {
      ++this->stats->OverwriteCount;
      this->stats = other.stats; // No move, we want to still track destruction
      ++this->stats->MoveCount;
      if(this->stats->ThrowOnMove) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
      return *this;
    }

    /// <summary>
    ///   Status tracker for the instance, used by tests to verify expected actions took place
    /// </summary>
    private: std::shared_ptr<TestItemStats> stats;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a vector of test item status structures</summary>
  /// <param name="count">Number of test item states that will be created</param>
  /// <returns>All test item states in a vector</returns>
  std::vector<std::shared_ptr<TestItemStats>> makeStats(std::size_t count) {
    std::vector<std::shared_ptr<TestItemStats>> stats;
    stats.reserve(count);

    for(std::size_t index = 0; index < count; ++index) {
      stats.emplace_back(std::make_shared<TestItemStats>());
    }

    return stats;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fills a vector with test items associated with test item states</summary>
  /// <param name="target">Vector that will receive the test items</param>
  /// <param name="stats">Test item states the created items will be associated with</param>
  void makeItems(
    std::vector<TestItem> &target, const std::vector<std::shared_ptr<TestItemStats>> &stats
  ) {
    std::size_t statCount = stats.size();

    target.reserve(statCount);
    for(std::size_t index = 0; index < statCount; ++index) {
      target.emplace_back(stats[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkWritingInvokesCopyConstructor() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    TBuffer<TestItem> test(16);
    test.Write(&items[0], 16);

    // Writing the items should have caused them to be copied
    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkShovingInvokesMoveConstructor() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    TBuffer<TestItem> test(16);
    test.Shove(&items[0], 16);

    // Shoving the items should have caused them to be moved
    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 1);
      EXPECT_EQ(stats[index]->DestroyCount, 0); // destructor is still caller's responsibility!
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkMoveSemanticsAreUsedWhenCapacityChanges() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(17);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    TBuffer<TestItem> test(16);
    test.Write(&items[0], 16);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    // Now write one more item, forcing the shift buffer to extend its capacity
    test.Write(&items[16], 1);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 1);
      EXPECT_EQ(stats[index]->DestroyCount, 1);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
    EXPECT_EQ(stats[16]->CopyCount, 1);
    EXPECT_EQ(stats[16]->MoveCount, 0);
    EXPECT_EQ(stats[16]->DestroyCount, 0);
    EXPECT_EQ(stats[16]->OverwriteCount, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkReadUsesMoveSemanticsAndCallsDestructor() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    TBuffer<TestItem> test(16);
    test.Write(&items[0], 16);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    // Read must overwrite existing data, so check correct behavior!
    std::vector<std::shared_ptr<TestItemStats>> stats2 = makeStats(16);
    std::vector<TestItem> items2;
    makeItems(items2, stats2);

    test.Read(&items2[0], 16);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 1);
      EXPECT_EQ(stats[index]->DestroyCount, 1);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats2[index]->CopyCount, 0);
      EXPECT_EQ(stats2[index]->MoveCount, 0);
      EXPECT_EQ(stats2[index]->DestroyCount, 0);
      EXPECT_EQ(stats2[index]->OverwriteCount, 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkBufferDestroysLeftOverItemsWhenDestroyed() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }

    {
      TBuffer<TestItem> test(16);
      test.Write(&items[0], 16);

      for(std::size_t index = 0; index < 16; ++index) {
        EXPECT_EQ(stats[index]->CopyCount, 1);
        EXPECT_EQ(stats[index]->MoveCount, 0);
        EXPECT_EQ(stats[index]->DestroyCount, 0);
        EXPECT_EQ(stats[index]->OverwriteCount, 0);
      }
    }

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 1);
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkExceptionDuringCapacityChangeCausesNoLeaks() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(17);
    std::vector<TestItem> items;
    makeItems(items, stats);

    {
      TBuffer<TestItem> test(16);
      test.Write(&items[0], 16);

      for(std::size_t index = 0; index < 16; ++index) {
        EXPECT_EQ(stats[index]->CopyCount, 1);
        EXPECT_EQ(stats[index]->MoveCount, 0);
        EXPECT_EQ(stats[index]->DestroyCount, 0);
        EXPECT_EQ(stats[index]->OverwriteCount, 0);
      }

      stats[10]->ThrowOnMove = true;

      // Now write one more item, forcing the shift buffer to extend its capacity
      EXPECT_THROW(test.Write(&items[16], 1), std::runtime_error);

      // There is no guarantee whether the newest or oldest or all items are lost,
      // only the /basic/ exception guarantee: no memory will be leaked and the container
      // remains in a usable state. We use special implementation knowledge here.
      for(std::size_t index = 0; index < 16; ++index) {
        if(index <= 10) {
          EXPECT_EQ(stats[index]->MoveCount, 1);
        } else {
          EXPECT_EQ(stats[index]->MoveCount, 0);
        }
        EXPECT_EQ(stats[index]->DestroyCount, 1);
      }
    }

    // The container should have deleted all remaining items it held
    for(std::size_t index = 0; index < 16; ++index) {
      if(index < 10) {
        EXPECT_EQ(stats[index]->MoveCount, 1);
        EXPECT_EQ(stats[index]->DestroyCount, 2); // once in move source, once in undo
      } else if(index == 10) {
        EXPECT_EQ(stats[index]->MoveCount, 1);
        EXPECT_EQ(stats[index]->DestroyCount, 1); // attempted to move, killed in undo
      } else {
        EXPECT_EQ(stats[index]->MoveCount, 0);
        EXPECT_EQ(stats[index]->DestroyCount, 1); // only killed in container destructor
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkExceptionDuringWriteCausesNoLeaks() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    stats[10]->ThrowOnCopy = true;

    {
      TBuffer<TestItem> test(16);
      EXPECT_THROW(test.Write(&items[0], 16), std::runtime_error);

      // Shoving the items should have caused them to be moved
      for(std::size_t index = 0; index < 16; ++index) {
        if(index <= 10) {
          EXPECT_EQ(stats[index]->CopyCount, 1);
        } else {
          EXPECT_EQ(stats[index]->CopyCount, 0);
        }
        EXPECT_EQ(stats[index]->MoveCount, 0);
        EXPECT_EQ(stats[index]->DestroyCount, 0);
        EXPECT_EQ(stats[index]->OverwriteCount, 0);
      }
    }

    for(std::size_t index = 0; index < 16; ++index) {
      if(index <= 10) {
        EXPECT_EQ(stats[index]->CopyCount, 1);
      } else {
        EXPECT_EQ(stats[index]->CopyCount, 0);
      }
      EXPECT_EQ(stats[index]->MoveCount, 0);
      if(index < 10) {
        EXPECT_EQ(stats[index]->DestroyCount, 1);
      } else {
        EXPECT_EQ(stats[index]->DestroyCount, 0);
      }
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkExceptionDuringShoveCausesNoLeaks() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    stats[10]->ThrowOnMove = true;

    {
      TBuffer<TestItem> test(16);
      EXPECT_THROW(test.Shove(&items[0], 16), std::runtime_error);

      // Shoving the items should have caused them to be moved
      for(std::size_t index = 0; index < 16; ++index) {
        EXPECT_EQ(stats[index]->CopyCount, 0);
        if(index <= 10) {
          EXPECT_EQ(stats[index]->MoveCount, 1);
        } else {
          EXPECT_EQ(stats[index]->MoveCount, 0);
        }
        EXPECT_EQ(stats[index]->DestroyCount, 0);
        EXPECT_EQ(stats[index]->OverwriteCount, 0);
      }
    }

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      if(index <= 10) {
        EXPECT_EQ(stats[index]->MoveCount, 1);
      } else {
        EXPECT_EQ(stats[index]->MoveCount, 0);
      }
      if(index < 10) {
        EXPECT_EQ(stats[index]->DestroyCount, 1);
      } else {
        EXPECT_EQ(stats[index]->DestroyCount, 0);
      }
      EXPECT_EQ(stats[index]->OverwriteCount, 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TBuffer>
  void checkExceptionDuringReadCausesNoLeaks() {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(16);
    std::vector<TestItem> items;
    makeItems(items, stats);

    std::vector<std::shared_ptr<TestItemStats>> stats2 = makeStats(16);
    std::vector<TestItem> items2;
    makeItems(items2, stats2);

    {
      TBuffer<TestItem> test(16);
      test.Write(&items[0], 16);

      for(std::size_t index = 0; index < 16; ++index) {
        EXPECT_EQ(stats[index]->CopyCount, 1);
        EXPECT_EQ(stats[index]->MoveCount, 0);
        EXPECT_EQ(stats[index]->DestroyCount, 0);
        EXPECT_EQ(stats[index]->OverwriteCount, 0);
      }

      stats[5]->ThrowOnMove = true;

      EXPECT_THROW(test.Read(&items2[0], 8), std::runtime_error);

      for(std::size_t index = 0; index < 16; ++index) {
        EXPECT_EQ(stats[index]->CopyCount, 1);
        if(index <= 5) {
          EXPECT_EQ(stats[index]->MoveCount, 1);
          EXPECT_EQ(stats2[index]->OverwriteCount, 1);
        } else {
          EXPECT_EQ(stats[index]->MoveCount, 0);
          EXPECT_EQ(stats2[index]->OverwriteCount, 0);
        }
        if(index < 5) {
          EXPECT_EQ(stats[index]->DestroyCount, 1); // in shift buffer during read
        } else {
          EXPECT_EQ(stats[index]->DestroyCount, 0);
        }
      }
    }

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      if(index <= 5) {
        EXPECT_EQ(stats[index]->MoveCount, 1);
      } else {
        EXPECT_EQ(stats[index]->MoveCount, 0);
      }
      EXPECT_EQ(stats[index]->DestroyCount, 1); // either during read or shift buffer d'tor

    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_BUFFERTEST_H
