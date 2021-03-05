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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Collections/DynamicArray.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      DynamicArray<int> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, MemoryCanBeAllocatedUpFront) {
    EXPECT_NO_THROW(
      DynamicArray<int> test;

      std::size_t initialCapacity = 256;
      test.Reserve(initialCapacity);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, IndexOfItemCanBeDetermined) {
    DynamicArray<int> test;
    test.Add(10);
    test.Add(21);
    test.Add(32);
    test.Add(43);

    EXPECT_EQ(2U, test.GetIndexOf(32));
    EXPECT_EQ(3U, test.GetIndexOf(43));
    EXPECT_EQ(1U, test.GetIndexOf(21));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, IndexOfNonExistentItemIsInvalid) {
    DynamicArray<int> test;
    test.Add(10);

    EXPECT_EQ(DynamicArray<int>::InvalidIndex, test.GetIndexOf(20));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ConstItemCanBeAccessedByIndex) {
    DynamicArray<int> test;
    test.Add(123);
    test.Add(456);
    test.Add(789);

    const DynamicArray<int> &constReference = test;
    EXPECT_EQ(789, constReference.GetAt(2));
    EXPECT_EQ(123, constReference.GetAt(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeAccessedByIndex) {
    DynamicArray<int> test;
    test.Add(123);
    test.Add(456);
    test.Add(789);

    EXPECT_EQ(789, test.GetAt(2));
    EXPECT_EQ(123, test.GetAt(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeReplacedAtIndex) {
    DynamicArray<int> test;
    test.Add(12);
    test.Add(45);
    test.Add(78);

    EXPECT_EQ(12, test.GetAt(0));
    EXPECT_EQ(45, test.GetAt(1));
    EXPECT_EQ(78, test.GetAt(2));

    test.SetAt(1, 333);

    EXPECT_EQ(12, test.GetAt(0));
    EXPECT_EQ(333, test.GetAt(1));
    EXPECT_EQ(78, test.GetAt(2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeInserted) {
    DynamicArray<int> test;
    test.Add(111);
    test.Add(222);
    test.Add(333);

    EXPECT_EQ(111, test.GetAt(0));
    EXPECT_EQ(222, test.GetAt(1));
    EXPECT_EQ(333, test.GetAt(2));

    test.InsertAt(1, 999);

    EXPECT_EQ(111, test.GetAt(0));
    EXPECT_EQ(999, test.GetAt(1));
    EXPECT_EQ(222, test.GetAt(2));
    EXPECT_EQ(333, test.GetAt(3));
  }
    
  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeRemovedByIndex) {
    DynamicArray<int> test;
    test.Add(987);
    test.Add(654);
    test.Add(321);

    EXPECT_EQ(987, test.GetAt(0));
    EXPECT_EQ(654, test.GetAt(1));
    EXPECT_EQ(321, test.GetAt(2));

    test.RemoveAt(1);

    EXPECT_EQ(987, test.GetAt(0));
    EXPECT_EQ(321, test.GetAt(1));
    EXPECT_EQ(2U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeAdded) {
    DynamicArray<int> test;
    test.Add(1212);
    test.Add(2323);
    test.Add(3434);

    EXPECT_EQ(1212, test.GetAt(0));
    EXPECT_EQ(2323, test.GetAt(1));
    EXPECT_EQ(3434, test.GetAt(2));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemCanBeRemoved) {
    DynamicArray<int> test;
    test.Add(2121);
    test.Add(3232);
    test.Add(4343);

    EXPECT_EQ(3U, test.Count());
    EXPECT_TRUE(test.Remove(3232));

    EXPECT_EQ(2121, test.GetAt(0));
    EXPECT_EQ(4343, test.GetAt(1));
    EXPECT_EQ(2U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, RemovingNonExistentItemReturnsFalse) {
    DynamicArray<int> test;
    test.Add(1);
    test.Add(2);
    test.Add(3);

    EXPECT_FALSE(test.Remove(4));
    EXPECT_EQ(3U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, RemovingDuplicateItemRemovesFirstOnly) {
    DynamicArray<int> test;
    test.Add(12);
    test.Add(33);
    test.Add(45);
    test.Add(33);
    test.Add(78);

    EXPECT_EQ(5U, test.Count());
    EXPECT_TRUE(test.Remove(33));

    EXPECT_EQ(12, test.GetAt(0));
    EXPECT_EQ(45, test.GetAt(1));
    EXPECT_EQ(33, test.GetAt(2));
    EXPECT_EQ(78, test.GetAt(3));

    EXPECT_EQ(4U, test.Count());
    EXPECT_TRUE(test.Remove(33));

    EXPECT_EQ(12, test.GetAt(0));
    EXPECT_EQ(45, test.GetAt(1));
    EXPECT_EQ(78, test.GetAt(2));
    EXPECT_EQ(3U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemsCanBeCleared) {
    DynamicArray<int> test;
    test.Add(11);
    test.Add(33);
    test.Add(55);

    EXPECT_EQ(3U, test.Count());
    test.Clear();
    EXPECT_EQ(0U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, CanCheckIfItemContained) {
    DynamicArray<int> test;
    test.Add(999);
    test.Add(888);
    test.Add(777);

    EXPECT_TRUE(test.Contains(777));
    EXPECT_FALSE(test.Contains(666));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, ItemsCanBeCounted) {
    DynamicArray<int> test;

    EXPECT_EQ(0U, test.Count());
    test.Add(11);
    EXPECT_EQ(1U, test.Count());
    test.Add(33);
    EXPECT_EQ(2U, test.Count());
    test.Add(55);
    EXPECT_EQ(3U, test.Count());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(DynamicArrayTest, CanBeCheckedForEmptiness) {
    DynamicArray<int> test;

    EXPECT_TRUE(test.IsEmpty());
    test.Add(1);
    EXPECT_FALSE(test.IsEmpty());
    test.Clear();
    EXPECT_TRUE(test.IsEmpty());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
