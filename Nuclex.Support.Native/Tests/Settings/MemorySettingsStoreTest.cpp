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

#include "Nuclex/Support/Settings/MemorySettingsStore.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      MemorySettingsStore settings;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, StartsOutWithNoCategories) {
    MemorySettingsStore settings;

    std::vector<std::string> categories = settings.GetAllCategories();
    EXPECT_EQ(categories.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanQueryNonExistentCategory) {
    MemorySettingsStore settings;

    std::vector<std::string> properties = settings.GetAllProperties(u8"Does not exist");
    EXPECT_EQ(properties.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanStorePropertiesInDefaultCategory) {
    MemorySettingsStore settings;

    std::optional<bool> beforeStore = settings.Retrieve<bool>(std::string(), u8"Hello");
    settings.Store<bool>(std::string(), u8"Hello", true);
    std::optional<bool> afterStore = settings.Retrieve<bool>(std::string(), u8"Hello");

    EXPECT_FALSE(beforeStore.has_value());
    EXPECT_TRUE(afterStore.has_value());
    EXPECT_EQ(afterStore.value(), true);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanRetrievePropertyUnderDifferentType) {
    MemorySettingsStore settings;

    settings.Store<bool>(std::string(), u8"Bool", true);
    std::optional<std::string> myBool = settings.Retrieve<std::string>(std::string(), u8"Bool");

    EXPECT_TRUE(myBool.has_value());
    EXPECT_EQ(myBool.value(), u8"1");

    settings.Store<std::int32_t>(std::string(), u8"Int", -123);
    std::optional<std::string> myInt = settings.Retrieve<std::string>(std::string(), u8"Int");

    EXPECT_TRUE(myInt.has_value());
    EXPECT_EQ(myInt.value(), u8"-123");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanDeleteProperty) {
    MemorySettingsStore settings;

    settings.Store<bool>(std::string(), u8"Test", true);

    std::optional<bool> beforeDelete = settings.Retrieve<bool>(std::string(), u8"Test");
    EXPECT_TRUE(beforeDelete.has_value());

    settings.DeleteProperty(std::string(), u8"Test");

    std::optional<bool> afterDelete = settings.Retrieve<bool>(std::string(), u8"Test");
    EXPECT_FALSE(afterDelete.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanCreateNewCategory) {
    MemorySettingsStore settings;

    std::vector<std::string> categoriesBefore = settings.GetAllCategories();
    EXPECT_EQ(categoriesBefore.size(), 0U);

    settings.Store<bool>(u8"MyCategory", u8"Test", true);

    std::vector<std::string> categoriesAfter = settings.GetAllCategories();
    ASSERT_EQ(categoriesAfter.size(), 1U);
    EXPECT_EQ(categoriesAfter[0], u8"MyCategory");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanDeleteCategory) {
    MemorySettingsStore settings;

    settings.Store<bool>(u8"MyCategory", u8"Test", true);

    std::vector<std::string> beforeDelete = settings.GetAllCategories();
    EXPECT_EQ(beforeDelete.size(), 1U);
    std::optional<bool> valueBeforeDelete = settings.Retrieve<bool>(u8"MyCategory", u8"Test");
    EXPECT_TRUE(valueBeforeDelete.has_value());

    settings.DeleteCategory(u8"MyCategory");

    std::vector<std::string> afterDelete = settings.GetAllCategories();
    EXPECT_EQ(afterDelete.size(), 0U);
    std::optional<bool> valueAfterDelete = settings.Retrieve<bool>(u8"MyCategory", u8"Test");
    EXPECT_FALSE(valueAfterDelete.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanDeleteNonExistentCategory) {
    MemorySettingsStore settings;

    EXPECT_FALSE(settings.DeleteCategory(u8"MyCategory"));
    settings.Store<bool>(u8"MyCategory", u8"Test", true);
    EXPECT_TRUE(settings.DeleteCategory(u8"MyCategory"));
    EXPECT_FALSE(settings.DeleteCategory(u8"MyCategory"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, CanDeleteNonExistentProperty) {
    MemorySettingsStore settings;

    EXPECT_FALSE(settings.DeleteProperty(u8"MyCategory", u8"Test"));
    settings.Store<bool>(u8"MyCategory", u8"Test", true);
    EXPECT_TRUE(settings.DeleteProperty(u8"MyCategory", u8"Test"));
    EXPECT_FALSE(settings.DeleteProperty(u8"MyCategory", u8"Test"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, EmptyCategoryIsKeptWhenDeletingProperty) {
    MemorySettingsStore settings;

    settings.Store<bool>(u8"MyCategory", u8"Test", true);

    std::vector<std::string> categoriesBefore = settings.GetAllCategories();
    ASSERT_EQ(categoriesBefore.size(), 1U);
    std::vector<std::string> propertiesBefore = settings.GetAllProperties(u8"MyCategory");
    ASSERT_EQ(propertiesBefore.size(), 1U);

    settings.DeleteProperty(u8"MyCategory", u8"Test");

    std::vector<std::string> categoriesAfter = settings.GetAllCategories();
    ASSERT_EQ(categoriesAfter.size(), 1U);
    std::vector<std::string> propertiesAfter = settings.GetAllProperties(u8"MyCategory");
    ASSERT_EQ(propertiesAfter.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, PropertyValueCanChange) {
    MemorySettingsStore settings;

    settings.Store<std::string>(std::string(), u8"Test", u8"Hello");

    std::optional<std::string> valueBeforeChange = (
      settings.Retrieve<std::string>(std::string(), u8"Test")
    );
    ASSERT_TRUE(valueBeforeChange.has_value());
    EXPECT_EQ(valueBeforeChange.value(), u8"Hello");

    settings.Store<std::string>(std::string(), u8"Test", u8"World");

    std::optional<std::string> valueAfterChange = (
      settings.Retrieve<std::string>(std::string(), u8"Test")
    );
    ASSERT_TRUE(valueAfterChange.has_value());
    EXPECT_EQ(valueAfterChange.value(), u8"World");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemorySettingsStoreTest, PropertiesAreSeparatedInCategories) {
    MemorySettingsStore settings;

    settings.Store<std::int64_t>(u8"FirstCategory", u8"Value", 123456789);
    settings.Store<std::int64_t>(u8"SecondCategory", u8"Value", 987654321);

    std::optional<std::int64_t> firstValue = (
      settings.Retrieve<std::int64_t>(u8"FirstCategory", u8"Value")
    );
    ASSERT_TRUE(firstValue.has_value());
    EXPECT_EQ(firstValue.value(), 123456789);

    std::optional<std::int64_t> secondValue = (
      settings.Retrieve<std::int64_t>(u8"SecondCategory", u8"Value")
    );
    ASSERT_TRUE(secondValue.has_value());
    EXPECT_EQ(secondValue.value(), 987654321);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
