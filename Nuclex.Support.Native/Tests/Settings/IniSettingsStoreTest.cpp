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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Settings/IniSettingsStore.h"
#include "Nuclex/Support/TemporaryFileScope.h"
#include "Nuclex/Support/TemporaryDirectoryScope.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An average .ini file without any special or ambiguous contents</summary>
  const char ExampleIniFile[] =
    u8"NumericBoolean = 1\n"
    u8"TrueFalseBoolean = TRUE\n"
    u8"YesNoBoolean = YES\n"
    u8"OnOffBoolean = ON\n"
    u8"\n"
    u8"[Integers]\n"
    u8"Tiny = 42\n"
    u8"Negative = -42\n"
    u8"Big = 1152921504606846976\n"
    u8"BigNegative = -1152921504606846976\n"
    u8"\n"
    u8"[Strings]\n"
    u8"Simple = Hello\n"
    u8"Quoted = \"World\"\n"
    u8"\n";

  // ------------------------------------------------------------------------------------------- //

  //const char *writeTemporaryIniFile() {
  //}

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      IniSettingsStore settings;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, FileCanBeLoadedFromMemory) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, FileCanBeLoadedFromHardDrive) {
    IniSettingsStore settings;
    {
      TemporaryFileScope testIniFile(u8"ini");
      testIniFile.SetFileContents(ExampleIniFile);

      settings.Load(testIniFile.GetPath());
    }

    std::optional<std::uint32_t> testValue = settings.Retrieve<std::uint32_t>(
      u8"Integers", u8"Tiny"
    );
    EXPECT_TRUE(testValue.has_value());
    EXPECT_EQ(testValue.value_or(0), 42U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, FileCanBeWrittenToHardDrive) {
    IniSettingsStore settings;
    settings.Store<bool>(std::string(), u8"FirstValue", true);
    settings.Store<std::uint32_t>(u8"MyCategory", u8"SecondValue", 12345);

    std::string savedFileContents;
    {
      TemporaryDirectoryScope testDirectory(u8"ini");
      settings.Save(testDirectory.GetPath(u8"test.ini"));

      testDirectory.ReadFile(u8"test.ini", savedFileContents);
    }

    EXPECT_NE(savedFileContents.find(u8"FirstValue"), std::string::npos);
    EXPECT_NE(savedFileContents.find(u8"SecondValue"), std::string::npos);
    EXPECT_NE(savedFileContents.find(u8"[MyCategory]"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadBooleanTypes) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::string none;
    std::optional<bool> numericBoolean = settings.Retrieve<bool>(none, u8"NumericBoolean");
    ASSERT_TRUE(numericBoolean.has_value());
    EXPECT_TRUE(numericBoolean.value());

    std::optional<bool> trueFalseBoolean = settings.Retrieve<bool>(none, u8"TrueFalseBoolean");
    ASSERT_TRUE(trueFalseBoolean.has_value());
    EXPECT_TRUE(trueFalseBoolean.value());

    std::optional<bool> yesNoBoolean = settings.Retrieve<bool>(none, u8"YesNoBoolean");
    ASSERT_TRUE(yesNoBoolean.has_value());
    EXPECT_TRUE(yesNoBoolean.value());

    std::optional<bool> onOffBoolean = settings.Retrieve<bool>(none, u8"OnOffBoolean");
    ASSERT_TRUE(onOffBoolean.has_value());
    EXPECT_TRUE(onOffBoolean.value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadUnsigned32BitIntegers) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::uint32_t> integer = settings.Retrieve<std::uint32_t>(
      u8"Integers", u8"Tiny"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 42U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadSigned32BitIntegers) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::int32_t> integer = settings.Retrieve<std::int32_t>(
      u8"Integers", u8"Tiny"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 42);

    std::optional<std::int32_t> negativeInteger = settings.Retrieve<std::int32_t>(
      u8"Integers", u8"Negative"
    );
    ASSERT_TRUE(negativeInteger.has_value());
    EXPECT_EQ(negativeInteger.value(), -42);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadUnsigned64BitIntegers) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::uint64_t> integer = settings.Retrieve<std::uint64_t>(
      u8"Integers", u8"Big"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 1152921504606846976ULL);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadSigned64BitIntegers) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::int64_t> integer = settings.Retrieve<std::int64_t>(
      u8"Integers", u8"Big"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 1152921504606846976LL);

    std::optional<std::int64_t> negativeInteger = settings.Retrieve<std::int64_t>(
      u8"Integers", u8"BigNegative"
    );
    ASSERT_TRUE(negativeInteger.has_value());
    EXPECT_EQ(negativeInteger.value(), -1152921504606846976LL);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadStrings) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::string> simpleString = settings.Retrieve<std::string>(
      u8"Strings", u8"Simple"
    );
    ASSERT_TRUE(simpleString.has_value());
    EXPECT_EQ(simpleString.value(), u8"Hello");

    std::optional<std::string> quotedString = settings.Retrieve<std::string>(
      u8"Strings", u8"Quoted"
    );
    ASSERT_TRUE(quotedString.has_value());
    EXPECT_EQ(quotedString.value(), u8"World");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CategoriesCanBeEnumerated) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::vector<std::string> categories = settings.GetAllCategories();
    EXPECT_EQ(categories.size(), 3U); // default, integers, strings
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, PropertiesCanBeEnumerated) {
    IniSettingsStore settings;
    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::vector<std::string> rootProperties = settings.GetAllProperties();
    EXPECT_EQ(rootProperties.size(), 4U);

    std::vector<std::string> stringProperties = settings.GetAllProperties(u8"Strings");
    EXPECT_EQ(stringProperties.size(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, PropertiesInDefaultCategoryCanBeDeleted) {
    IniSettingsStore settings;

    bool wasDeleted = settings.DeleteProperty(std::string(), u8"DoesNotExist");
    EXPECT_FALSE(wasDeleted);

    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    wasDeleted = settings.DeleteProperty(std::string(), u8"NumericBoolean");
    EXPECT_TRUE(wasDeleted);

    std::vector<std::string> rootProperties = settings.GetAllProperties();
    EXPECT_EQ(rootProperties.size(), 3U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, DefaultCategoryCanBeDeleted) {
    IniSettingsStore settings;

    bool wasDeleted = settings.DeleteCategory(std::string());
    EXPECT_FALSE(wasDeleted);

    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    wasDeleted = settings.DeleteCategory(std::string());
    EXPECT_TRUE(wasDeleted);

    std::vector<std::string> rootProperties = settings.GetAllProperties();
    EXPECT_EQ(rootProperties.size(), 0U);

    std::vector<std::string> integerProperties = settings.GetAllProperties(u8"Integers");
    EXPECT_EQ(integerProperties.size(), 4U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, NamedCategoryCanBeDeleted) {
    IniSettingsStore settings;

    bool wasDeleted = settings.DeleteCategory(u8"Integers");
    EXPECT_FALSE(wasDeleted);

    settings.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    wasDeleted = settings.DeleteCategory(u8"Integers");
    EXPECT_TRUE(wasDeleted);

    std::vector<std::string> rootProperties = settings.GetAllProperties();
    EXPECT_EQ(rootProperties.size(), 4U);

    std::vector<std::string> integerProperties = settings.GetAllProperties(u8"Integers");
    EXPECT_EQ(integerProperties.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
