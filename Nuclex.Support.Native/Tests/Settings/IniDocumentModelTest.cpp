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

#include "../../Source/Settings/IniDocumentModel.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An average .ini file without any special or ambiguous contents</summary>
  const char VanillaIniFile[] =
    u8"GlobalProperty=1\n"
    u8"\n"
    u8"[ImportantStuff]\n"
    u8";CommentedOut=5000\n"
    u8"Normal=42\n"
    u8"\n";

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An .ini file with empty assignments and a padded section</summary>
  const char EmptyAssignments[] =
    u8"WithoutValue=\n"
    u8"\n"
    u8"[ MoreStuff ]\n"
    u8"AlsoNoValue = ;\n"
    u8"TrailingSpaces = Hello  \n"
    u8"Quoted = \"Hello \" \n"
    u8"WeirdOne = \"\n"
    u8"YetAgain = #";

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An .ini file with lots of corner cases and malformed statements</summary>
  const char MalformedLines[] =
    u8"ThisLineIsMeaningless\n"
    u8"\n"
    u8"]BadLine1=123\n"
    u8"\"BadLine2=234\"\n"
    u8"[NotASection]=345\n"
    u8"[AlsoNoSection]=[Value]\n"
    u8"Funny = [Hello] [World]\n"
    u8"\n"
    u8"[BadLine3 = 456]\n"
    u8"BadLine4 = 567 = 789\n"
    u8"\"Bad\" Line5=890\n"
    u8"Bad \"Line6\"=1\n"
    u8"\n"
    u8"[\"Quoted Section\"]\n"
    u8"[\"BadSection]\"\n"
    u8"GoodLine=2 3\n"
    u8"BadLine7=\"4\" 5\n"
    u8"BadLine7=6 \"7\"";

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, DefaultConstructedModelHasNoSections) {
    IniDocumentModel dom;
    std::vector<std::string> sections = dom.GetAllSections();
    EXPECT_EQ(sections.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasFileContentsConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
        sizeof(VanillaIniFile) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, EmptyDocumentCanBeSerialized) {
    IniDocumentModel dom;

    std::vector<std::uint8_t> contents = dom.Serialize();
    EXPECT_EQ(contents.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanParseVanillaProperty) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"GlobalProperty");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanParseVanillaSection) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"ImportantStuff", u8"Normal");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"42");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, NamesAreCaseInsensitive) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"impOrtantstUff", u8"nOrmAl");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"42");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, IgnoresComments) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"ImportantStuff", u8"CommentedOut");
    EXPECT_FALSE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanHandleEmptyAssignments) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
        sizeof(EmptyAssignments) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, AssignmentWithoutValueIsValid) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"WithoutValue");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), std::string());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SectionCanBePaddedWithSpaces) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"AlsoNoValue");
    EXPECT_TRUE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CommentAfterPropertyValueIsOmitted) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );

    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"AlsoNoValue");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), std::string());

    value = dom.GetPropertyValue(u8"MoreStuff", u8"YetAgain");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), std::string());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SpacesAfterPropertyValueAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"TrailingSpaces");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"Hello");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SpacesInsideQuotesAreKept) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"Quoted");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"Hello ");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, UnclosedQuoteInvalidatesLine) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"WeirdOne");
    EXPECT_FALSE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanHandleMalformedLines) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::uint8_t *>(MalformedLines),
        sizeof(MalformedLines) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, MalformedLinesAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );

    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"ThisLineIsMeaningless").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"]BadLine1").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"BadLine1").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"BadLine2").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"\"BadLine2").has_value()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SectionNameCanHaveQuotes) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::vector<std::string> sections = dom.GetAllSections();
    bool found = false;
    for(std::size_t index = 0; index < sections.size(); ++index) {
      if(sections[index] == u8"Quoted Section") {
        found = true;
      }
    }
    EXPECT_TRUE(found);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyNameCanHaveBrackets) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"NotASection");
    EXPECT_TRUE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyNameAndValueCanHaveBrackets) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"AlsoNoSection");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"[Value]");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, AllMalformedElementsAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );

    // All "bad" (malformed) sections and lines conveniently have a name
    // that includes the word "Bad" :-)
    bool badSectionOrPropertyFound = false;

    std::vector<std::string> sections = dom.GetAllSections();
    for(std::size_t index = 0; index < sections.size(); ++index) {
      if(sections[index].find("Bad") != std::string::npos) {
        badSectionOrPropertyFound = true;
      }
    }
    for(std::size_t sectionIndex = 0; sectionIndex < sections.size(); ++sectionIndex) {
      std::vector<std::string> properties = dom.GetAllProperties(sections[sectionIndex]);
      for(std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex) {
        if(properties[propertyIndex].find("Bad") != std::string::npos) {
          badSectionOrPropertyFound = true;
        }
      }
    }

    EXPECT_FALSE(badSectionOrPropertyFound);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, NewPropertiesCanBeCreated) {
    IniDocumentModel dom;
    dom.SetPropertyValue(u8"MySection", u8"World", u8"Hello");
    dom.SetPropertyValue(std::string(), u8"Hello", u8"World");

    std::vector<std::uint8_t> fileContents = dom.Serialize();

    std::string fileContentsAsString(fileContents.begin(), fileContents.end());
    EXPECT_TRUE(fileContentsAsString.find(u8"Hello = World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyValueCanBeChangedToShorter) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    dom.SetPropertyValue(u8"ImportantStuff", u8"Normal", u8"2");

    std::vector<std::uint8_t> fileContents = dom.Serialize();

    std::string fileContentsAsString(fileContents.begin(), fileContents.end());
    EXPECT_TRUE(fileContentsAsString.find(u8"Normal=2\n") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyValueCanBeChangedToLonger) {
    IniDocumentModel dom(
      reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    dom.SetPropertyValue(u8"ImportantStuff", u8"Normal", u8"Crazy");

    std::vector<std::uint8_t> fileContents = dom.Serialize();

    std::string fileContentsAsString(fileContents.begin(), fileContents.end());
    EXPECT_TRUE(fileContentsAsString.find(u8"Normal=Crazy\n") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
