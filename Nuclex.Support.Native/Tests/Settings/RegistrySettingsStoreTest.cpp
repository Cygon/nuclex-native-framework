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

#include "Nuclex/Support/Settings/RegistrySettingsStore.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>
#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanOpenHiveInShortForm) {
    EXPECT_NO_THROW(
      RegistrySettingsStore settings(u8"hkcu");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanOpenHiveInLongForm) {
    EXPECT_NO_THROW(
      RegistrySettingsStore settings(u8"HKEY_CLASSES_ROOT");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, ThrowsExceptionWhenNoHiveSpecified) {
    EXPECT_THROW(
      RegistrySettingsStore settings(u8"SOFTWARE/Microsoft"),
      std::runtime_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanAccessDeepRegistryKey) {
    RegistrySettingsStore settings(u8"HKEY_CURRENT_USER/SOFTWARE/Microsoft");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanAccessPrivilegedKeyReadOnly) {
    const RegistrySettingsStore settings(u8"HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft", false);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, NonExistentKeyCanBeAccessedInReadOnlyMode) {
    // If a non-existent key is specified in read-only mode, the settings store acts
    // as if it was completely empty. This makes the behavior consistent with the Retrieve()
    // method. If an error was thrown instead, it would make applications un-runnable unless
    // some useless, empty registry key was present rather than gracefully using defaults.
    EXPECT_NO_THROW(
      const RegistrySettingsStore settings(
        u8"HKEY_LOCAL_MACHINE/Lalala123ThisDoesntExist", false
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, AttemptsCreationOfNonExistentKey) {
    // The behavior is different in writable mode. The user expects to be able to store
    // settings in the registry, so if the key doesn't exist, it's immediately created
    // and when that isn't possible (bad path or privilege issue), an error gets thrown.
    //
    // I hope you're not running your unit test with administrative privileges...
    EXPECT_THROW(
      const RegistrySettingsStore settings(u8"HKEY_LOCAL_MACHINE/Lalala123ThisDoesntExist"),
      std::system_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanEnumerateCategories) {
    const RegistrySettingsStore settings(u8"hklm/SOFTWARE/Microsoft", false);

    std::vector<std::string> categories = settings.GetAllCategories();
    EXPECT_GE(categories.size(), 10U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanEnumerateProperties) {
    const RegistrySettingsStore settings(u8"HKLM/SYSTEM/CurrentControlSet/Control", false);

    std::vector<std::string> properties = settings.GetAllProperties();
    EXPECT_GE(properties.size(), 5U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanDeleteRegistryKey) {

    // Try to delete a key that doesn't exist.
    // Should return false because it doesn't exist.
    bool wasDeleted = RegistrySettingsStore::DeleteKey(
      u8"HKCU/UnitTesting/this/does/not/exist/123mooh"
    );
    EXPECT_FALSE(wasDeleted);

    // Open a key in writable mode so that it is created
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/TestForDelete");
    }

    // Now attempt to delete the key.
    // Should return true now to indicate it did delete something.
    wasDeleted = RegistrySettingsStore::DeleteKey(
      u8"HKCU/UnitTesting/Nuclex.Support.Native/TestForDelete"
    );
    EXPECT_TRUE(wasDeleted);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, BooleansRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/BooleanTest");

      std::optional<bool> missingValue = settings.Retrieve<bool>(std::string(), u8"TestValue");
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<bool>(std::string(), u8"A", true);
      settings.Store<bool>(u8"TestCategory", u8"B", true);

      std::optional<bool> rootValue = settings.Retrieve<bool>(std::string(), u8"A");
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(false), true);

      std::optional<bool> nestedValue = settings.Retrieve<bool>(u8"TestCategory", u8"B");
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(false), true);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, UInt32sRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/UInt32Test");

      std::optional<std::uint32_t> missingValue = settings.Retrieve<std::uint32_t>(
        std::string(), u8"TestValue"
      );
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<std::uint32_t>(std::string(), u8"A", 2345678901U);
      settings.Store<std::uint32_t>(u8"TestCategory", u8"B", 3456789012U);

      std::optional<std::uint32_t> rootValue = (
        settings.Retrieve<std::uint32_t>(std::string(), u8"A")
      );
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(0), 2345678901U);

      std::optional<std::uint32_t> nestedValue = (
        settings.Retrieve<std::uint32_t>(u8"TestCategory", u8"B")
      );
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(0), 3456789012U);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, Int32sRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/Int32Test");

      std::optional<std::int32_t> missingValue = settings.Retrieve<std::int32_t>(
        std::string(), u8"TestValue"
      );
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<std::int32_t>(std::string(), u8"A", -1234567890);
      settings.Store<std::int32_t>(u8"TestCategory", u8"B", -1234567891);

      std::optional<std::int32_t> rootValue = (
        settings.Retrieve<std::int32_t>(std::string(), u8"A")
      );
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(0), -1234567890);

      std::optional<std::int32_t> nestedValue = (
        settings.Retrieve<std::int32_t>(u8"TestCategory", u8"B")
      );
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(0), -1234567891);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, UInt64sRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/UInt64Test");

      std::optional<std::uint64_t> missingValue = settings.Retrieve<std::uint64_t>(
        std::string(), u8"TestValue"
      );
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<std::uint64_t>(std::string(), u8"A", 12345678901234567890ULL);
      settings.Store<std::uint64_t>(u8"TestCategory", u8"B", 12345678901234567891ULL);

      std::optional<std::uint64_t> rootValue = (
        settings.Retrieve<std::uint64_t>(std::string(), u8"A")
      );
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(0), 12345678901234567890ULL);

      std::optional<std::uint64_t> nestedValue = (
        settings.Retrieve<std::uint64_t>(u8"TestCategory", u8"B")
      );
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(0), 12345678901234567891ULL);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, Int64sRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/Int64Test");

      std::optional<std::int64_t> missingValue = settings.Retrieve<std::int64_t>(
        std::string(), u8"TestValue"
      );
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<std::int64_t>(std::string(), u8"A", -8901234567890123456LL);
      settings.Store<std::int64_t>(u8"TestCategory", u8"B", -9012345678901234567LL);

      std::optional<std::int64_t> rootValue = (
        settings.Retrieve<std::int64_t>(std::string(), u8"A")
      );
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(0), -8901234567890123456LL);

      std::optional<std::int64_t> nestedValue = (
        settings.Retrieve<std::int64_t>(u8"TestCategory", u8"B")
      );
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(0), -9012345678901234567LL);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, StringsRoundTripCorrectly) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/StringTest");

      std::optional<std::string> missingValue = settings.Retrieve<std::string>(
        std::string(), u8"TestValue"
      );
      EXPECT_FALSE(missingValue.has_value());

      settings.Store<std::string>(std::string(), u8"A", u8"Hello World!");
      settings.Store<std::string>(u8"TestCategory", u8"B", u8"Hello Subkey!");

      std::optional<std::string> rootValue = (
        settings.Retrieve<std::string>(std::string(), u8"A")
      );
      EXPECT_TRUE(rootValue.has_value());
      EXPECT_EQ(rootValue.value_or(std::string()), u8"Hello World!");

      std::optional<std::string> nestedValue = (
        settings.Retrieve<std::string>(u8"TestCategory", u8"B")
      );
      EXPECT_TRUE(nestedValue.has_value());
      EXPECT_EQ(nestedValue.value_or(std::string()), u8"Hello Subkey!");
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CreatedCategoriesAreEnumerated) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/SubkeyTest");

      std::vector<std::string> emptyCategories = settings.GetAllCategories();
      EXPECT_TRUE(emptyCategories.empty());

      settings.Store<bool>(u8"Category1", u8"A", true);
      settings.Store<bool>(u8"Category1", u8"B", false);
      settings.Store<bool>(u8"Category2", u8"C", true);
      settings.Store<bool>(u8"Category3", u8"D", false);

      std::vector<std::string> threeCategories = settings.GetAllCategories();
      EXPECT_EQ(threeCategories.size(), 3U);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CreatedPropertiesAreEnumerated) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/ValueTest");

      std::vector<std::string> emptyValues = settings.GetAllProperties();
      EXPECT_TRUE(emptyValues.empty());

      settings.Store<bool>(std::string(), u8"A", true);
      settings.Store<bool>(std::string(), u8"B", false);
      settings.Store<bool>(u8"SomeCategory", u8"C", true);

      std::vector<std::string> twoValues = settings.GetAllProperties();
      EXPECT_EQ(twoValues.size(), 2U);

      std::vector<std::string> oneValue = settings.GetAllProperties(u8"SomeCategory");
      EXPECT_EQ(oneValue.size(), 1U);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, DeletingDefaultCategoryKeepsOtherCatories) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/DeleteTest");

      bool wasEmptyDeleted = settings.DeleteCategory(std::string());
      EXPECT_FALSE(wasEmptyDeleted);

      settings.Store<bool>(std::string(), u8"A", true);
      settings.Store<bool>(std::string(), u8"B", false);
      settings.Store<bool>(u8"SomeCategory", u8"C", true);

      bool wasDeleted = settings.DeleteCategory(std::string());
      EXPECT_TRUE(wasDeleted);

      std::vector<std::string> noValues = settings.GetAllProperties();
      EXPECT_TRUE(noValues.empty());

      std::vector<std::string> oneValue = settings.GetAllProperties(u8"SomeCategory");
      EXPECT_EQ(oneValue.size(), 1U);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, DeletingSubCategoryKeepsRootCatory) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/SubDeleteTest");

      bool wasEmptyDeleted = settings.DeleteCategory(u8"SomeCategory");
      EXPECT_FALSE(wasEmptyDeleted);

      settings.Store<bool>(std::string(), u8"A", true);
      settings.Store<bool>(u8"SomeCategory", u8"B", false);
      settings.Store<bool>(u8"SomeCategory", u8"C", true);

      bool wasDeleted = settings.DeleteCategory(u8"SomeCategory");
      EXPECT_TRUE(wasDeleted);

      std::vector<std::string> oneValue = settings.GetAllProperties();
      EXPECT_EQ(oneValue.size(), 1U);

      std::vector<std::string> noValues = settings.GetAllProperties(u8"SomeCategory");
      EXPECT_TRUE(noValues.empty());
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, PropertiesCanBeDeleted) {
    {
      RegistrySettingsStore settings(u8"HKCU/UnitTesting/Nuclex.Support.Native/DeleteValueTest");

      bool wasDeleted = settings.DeleteProperty(std::string(), u8"DoesntExist");
      EXPECT_FALSE(wasDeleted);
      wasDeleted = settings.DeleteProperty(u8"SomeCategory", u8"DoesntExist");
      EXPECT_FALSE(wasDeleted);

      settings.Store<bool>(std::string(), u8"A", true);
      settings.Store<bool>(std::string(), u8"B", false);
      settings.Store<bool>(u8"SomeCategory", u8"C", true);
      settings.Store<bool>(u8"SomeCategory", u8"D", false);

      std::vector<std::string> twoValues = settings.GetAllProperties();
      EXPECT_EQ(twoValues.size(), 2U);
      wasDeleted = settings.DeleteProperty(std::string(), u8"A");
      EXPECT_TRUE(wasDeleted);
      std::vector<std::string> oneValue = settings.GetAllProperties();
      EXPECT_EQ(oneValue.size(), 1U);

      twoValues = settings.GetAllProperties(u8"SomeCategory");
      EXPECT_EQ(twoValues.size(), 2U);
      wasDeleted = settings.DeleteProperty(u8"SomeCategory", u8"D");
      EXPECT_TRUE(wasDeleted);
      oneValue = settings.GetAllProperties(u8"SomeCategory");
      EXPECT_EQ(oneValue.size(), 1U);
    }
    RegistrySettingsStore::DeleteKey(u8"HKCU/UnitTesting/Nuclex.Support.Native");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
