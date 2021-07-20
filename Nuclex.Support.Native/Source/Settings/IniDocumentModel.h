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

#ifndef NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_H
#define NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/StringMatcher.h"

#include <vector> // for std::vector
#include <string> // for std::string
#include <cstdint> // for std::uint8_t
#include <optional> // for std::optional

#include <unordered_map> // for std::unordered_map
#include <unordered_set> // for std::unordered_set

// IDEA: Provide second constructor with unique_ptr that transfers memory ownership
//   This could, perhaps, save on a few allocations
//   Downside is that ther parse would have to support two different allocation models

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Document model storing the contents of an .ini file in an easily traversable format
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     This is the same concept as you might find in a DOM (document object model) style
  ///     XML parser, a representation of the .ini file's contents as a set of objects
  ///     allowing easy manipulation and search through all nodes/elements.
  ///   </para>
  ///   <para>
  ///     This document model takes great care to preserve the original lines and merely
  ///     memorize where each lines' important characters are. Meaningless lines (comments
  ///     and un-parseable ones) are preserved as well, allowing the reconstruction of
  ///     the whole .ini file in its original format, even if values are modified.
  ///   </para>
  ///   <para>
  ///     Allocation is done in chunks, reducing memory fragmentation and improving
  ///     cache locality.
  ///   </para>
  /// </remarks>
  class IniDocumentModel {

    /// <summary>Initializes a new empty .ini file document model</summary>
    public: IniDocumentModel();

    /// <summary>
    ///   Initializes a new .ini file document model parsing the specified file contents
    /// </summary>
    /// <param name="fileContents">The whole contents of an .ini file</param>
    /// <param name="byteCount">Lenght of the .ini file in bytes</param>
    public: IniDocumentModel(const std::uint8_t *fileContents, std::size_t byteCount);

    /// <summary>Frees all memory owned by the instance</summary>
    public: ~IniDocumentModel();

    /// <summary>Serializes the entire document model into a memory block</summary>
    /// <returns>Vector holding the entire .ini file contents</returns>
    public: std::vector<std::uint8_t> Serialize() const;

    /// <summary>Serializes the entire document model back into an .ini file</summary>
    /// <param name="path">Path of the .ini file in to save the document model</param>
    public: void Serialize(
      void *context, void write(void *context, const std::uint8_t *, std::size_t)
    ) const;

    /// <summary>Retrieves a list of all sections that exist in the .ini file</summary>
    /// <returns>A list of all sections contained in the .ini file</returns>
    public: std::vector<std::string> GetAllSections() const;

    /// <summary>Retrieves a list of all properties defined within a section</summary>
    /// <param name="sectionName">Name of the section whose properties will be liste</param>
    /// <returns>A list of all properties defined in the specified section</returns>
    public: std::vector<std::string> GetAllProperties(const std::string &sectionName) const;

    /// <summary>Looks up the value of a property</summary>
    /// <param name="sectionName">Name of the section in which the property exists</param>
    /// <param name="propertyName">Name of the property that will be looked up</param>
    /// <returns>The value of the property if the property exists</returns>
    public: std::optional<std::string> GetPropertyValue(
      const std::string &sectionName, const std::string &propertyName
    ) const;

    /// <summary>Creates a property or updates an existing property's value</summary>
    /// <param name="sectionName">Name of the section in which the property will be set</param>
    /// <param name="propertyName">Name of the property that will be set</param>
    /// <param name="propertyValue">Value that will be assigned to the property</param>
    public: void SetPropertyValue(
      const std::string &sectionName,
      const std::string &propertyName,
      const std::string &propertyValue
    );

    /// <summary>Deletes a property if it exists</summary>
    /// <param name="sectionName">Name of the section in which the property exists</param>
    /// <param name="propertyName">Name of the property that will be deleted</param>
    /// <returns>True if the property existed and was deleted, false otherwise</returns>
    public: bool DeleteProperty(
      const std::string &sectionName,
      const std::string &propertyName
    );

    /// <summary>Deletes an entire section from the document if it exists</summary>
    /// <param name="sectionName">Name of the section that will be deleted</param>
    /// <returns>True if the section existed and was deleted, false otherwise</returns>
    public: bool DeleteSection(const std::string &sectionName);

    #pragma region struct Line

    /// <summary>An arbitrary line from an .ini file</summary>
    protected: struct Line {

      /// </summary>Pointer to the previous line</summary>
      public: Line *Previous;
      /// </summary>Pointer to the next line</summary>
      public: Line *Next;

      /// <summary>The text contained in this line, including CR or CR-LF</summary>
      public: std::uint8_t *Contents;
      /// <summary>Length of the line in bytes</summary>
      public: std::size_t Length;

    };

    #pragma endregion // struct Line

    #pragma region struct SectionLine

    /// <summary>A line in an .ini file declaring a section</summary>
    protected: struct SectionLine : public Line {

      /// <summary>Byte index at which the section name begins</summary>
      public: std::size_t NameStartIndex;
      /// <summary>Length of the section name in bytes</summary>
      public: std::size_t NameLength;

    };

    #pragma endregion // struct SectionLine

    #pragma region struct PropertyLine

    /// <summary>A line in an .ini file containing a property assignment</summary>
    protected: struct PropertyLine : public Line {

      /// <summary>Byte index at which the property name begins</summary>
      public: std::size_t NameStartIndex;
      /// <summary>Length of the property name in bytes</summary>
      public: std::size_t NameLength;
      /// <summary>Byte index at which the property's value begins</summary>
      public: std::size_t ValueStartIndex;
      /// <summary>Length of the property's value in bytes</summary>
      public: std::size_t ValueLength;

    };

    #pragma endregion // struct PropertyLine

    #pragma region class IndexedSection

    /// <summary>A line in an .ini file containing a property assignment</summary>
    protected: class IndexedSection {

      /// <summary>Map from (case-insensitive) property name to property line</summary>
      public: typedef std::unordered_map<
        std::string, PropertyLine *,
        Text::CaseInsensitiveUtf8Hash, Text::CaseInsensitiveUtf8EqualTo
      > PropertyMap;

      /// <summary>Line in which this section is declared. Can be a nullptr.</summary>
      public: SectionLine *DeclarationLine;
      /// <summary>Index of property lines in this section by their property name</summary>
      public: PropertyMap Properties;
      /// <summary>Last line in this section</summary>
      public: Line *LastLine;

    };

    #pragma endregion // class IndexedSection

    // Internal helper that parses an existing .ini file into the document model
    private: class FileParser;
    //private: class LineBuilder;

    /// <summary>Retrieves or creates the section with the specified name</summary>
    /// <param name="sectionName">Name of the section that will be retrieved or created</param>
    /// <returns>The new or existing section of the specified name</returns>
    private: IndexedSection *getOrCreateSection(const std::string &sectionName);

    /// <summary>Creates a new line to declare the specified property</summary>
    /// <param name="propertyName">Name of the property the line will declare</param>
    /// <param name="propertyValue">Value that will be assigned to the property</param>
    /// <returns>The new property declaration line</returns>
    private: PropertyLine *createPropertyLine(
      const std::string &propertyName, const std::string &propertyValue
    );

    /// <summary>Integrates a line into the linked list of lines</summary>
    /// <param name="previous">Line after which the new line will appear</param>
    /// <param name="newLine">New line that will be integrated</param>
    /// <param name="blankLineBefore">Whether generate an extra blank line first</param>
    private: void integrateLine(Line *previous, Line *newLine, bool blankLineBefore = false);

    /// <summary>Parses the contents of an existing .ini file</summary>
    /// <param name="fileContents">Buffer holding the entire .ini file in memory</param>
    /// <param name="byteCount">Size of the .ini file in bytes</param>
    /// <param name="allocatedBytCount">
    ///   Amount of memory allocated in <see cref="createdLinesMemory" />
    /// </param>
    private: void parseFileContents(
      const std::uint8_t *fileContents, std::size_t byteCount
    );

    /// <summary>Changes the value stored in an existing line</summary>
    /// <param name="existingPropertyLine">Existing line containing the old value</param>
    /// <param name="newValue">New property value that will be stored in the line</param>
    /// <param name="addQuotes">Whether quotes will be added around the property line</param>
    private: static void updateExistingPropertyLine(
      PropertyLine *existingPropertyLine, const std::string &newValue, bool addQuotes
    );

    /// <summary>Checks whether the specified property's value has quotes around it</summary>
    /// <param name="propertyLine">Property line that will be checked</param>
    /// <returns>True if the property line uses quotes, false if not</returns>
    private: static bool hasQuotes(PropertyLine *propertyLine);

    /// <summary>Checks whether the specified property value requires quotes</summary>
    /// <param name="propertyValue">Value that will be checked for requiring quotes</param>
    /// <returns>True if the property value has to be surrounded with quotes</returns>
    private: static bool requiresQuotes(const std::string &propertyValue);

    /// <summary>Allocates memory for a single line</summary>
    /// <typeparam name="TLine">Type of line that will be allocated</typeparam>
    /// <param name="contents">The bytes this line consists of, including CR / CR-LF</param>
    /// <param name="byteCount">Length of the line in bytes</param>
    /// <returns>The new line</returns>
    private: template<typename TLine>
    TLine *allocateLine(const std::uint8_t *contents, std::size_t byteCount);

    /// <summary>Allocates memory for the specified type</summary>
    /// <typeparam name="T">Type for which memory will be allocated</typeparam>
    /// <param name="extraByteCount">Extra bytes to make available after the type</param>
    /// <returns>The memory address of the newly allocated type</returns>
    /// <remarks>
    ///   An uninitialized instance is returned. No constructors will be called.
    ///   This is for internal use and should only ever be used with POD types.
    /// </remarks>
    private: template<typename T>
    T *allocate(std::size_t extraByteCount = 0);

    /// <summary>Frees the memory allocated for a line type</summary>
    /// <typeparam name="TLine">Type of line whose memory will be freed</typeparam>
    /// <param name="line">The line instance that will be freed</param>
    private: template<typename TLine>
    void freeLine(TLine *line);

    /// <summary>Map from property name to the lines containing a property</summary>
    private: typedef IndexedSection::PropertyMap PropertyMap;
    /// <summary>Map from section name to a type holding the properties in the section</summary>
    private: typedef std::unordered_map<
      std::string, IndexedSection *,
      Text::CaseInsensitiveUtf8Hash, Text::CaseInsensitiveUtf8EqualTo
    > SectionMap;

    /// <summary>Memory holding all Line instances from when the .ini file was loaded</summary>
    /// <remarks>
    ///   Instead of allocating lines individually, this document model allocates a big memory
    ///   chunk that holds all line instances and their respective text, too. This avoids
    ///   memory fragmentation and is fairly efficient as usually, .ini files aren't completely
    ///   restructured during an application run.
    /// </remarks>
    private: std::vector<std::uint8_t *> loadedLinesMemory;
    /// <summary>Memory for all Line instances that were created after loading</summary>
    private: std::unordered_set<std::uint8_t *> createdLinesMemory;

    /// <summary>Pointer to the first line, useful to reconstruct the file</summary>
    private: Line *firstLine;
    /// <summary>Map allowing quick access to all the sections in the .ini file</summary>
    /// <remarks>
    ///   The global section (containing all properties before the first section declaration)
    ///   is nameless and always present.
    /// </remarks>
    private: SectionMap sections;

    /// <summary>Should there be spaces before and after the equals sign?</summary>
    private: bool hasSpacesAroundAssignment;
    /// <summary>Should property assignments be padded with empty lines between them?</summary>
    private: bool usesPaddingLines;
    /// <summary>Whether the configuration file uses weird Windows line breaks</summary>
    private: bool usesCrLf;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_H
