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

#ifndef NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
#define NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H

#include "Nuclex/Support/Config.h"

#include "IniDocumentModel.h"

// Considered allocation schemes:
//
//   By line                      -> lots of micro-allocations
//   In blocks (custom allocator) -> I have to do reference counting to free anything
//   Load pre-alloc, then by line -> Fast for typical case, no or few micro-allocations
//                                   But requires pre-scan of entire file + more code

// This could be done with tried-and-proven parser generators such as classic Flex/Yacc/Bison
// or Boost.Spirit. However, I wanted something lean, fast and without external dependencies.
//
// A middleground option would be a modern PEG parser generator like this:
//   https://github.com/TheLartians/PEGParser
//
// But the implementation below, even if tedious, gets the job done, fast and efficient.
//

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a document model by parses an existing .ini file</summary>
  class IniDocumentModel::FileParser {

    /// <summary>Initializes a new .ini file parser</summary>
    /// <param name="fileContents">Full file contents of the .ini file in memory</param>
    /// <param name="byteCount">Length of the .ini file in bytes</param>
    public: FileParser(const std::uint8_t *fileContents, std::size_t byteCount);

    /// <summary>Parses the .ini file and fills the specified document model</summary>
    /// <param name="documentModel">
    ///   Document model into which the parsed properties will be written
    /// </param>
    public: void ParseInto(IniDocumentModel *documentModel);

    /// <summary>Whether the parsed document used CR-LF line breaks (Windows type)</summary>
    /// <returns>True if the parsed document has Windows line breaks</returns>
    public: bool UsesCarriageReturns() const { return (this->windowsLineBreaks > 0); }

    /// <summary>Whether the parsed document had blank lines between properties</summary>
    /// <returns>True if the properties were padded with blank lines</returns>
    public: bool UsesBlankLines() const { return (this->blankLines > 0); }

    /// <summary>Whether the parsed document has spaces around the equals sign</summary>
    /// <returns>True if the parsed document used spaces around the equals sign</returns>
    public: bool UsesSpacesAroundAssignment() const { return (this->paddedAssignments > 0); }

    /// <summary>Parses a comment, must be called on the comment start character</summary>
    private: void parseComment();

    /// <summary>Parses a property or section name, must be called on first character</summary>
    private: void parseName();

    /// <summary>Parses a property value, must be called on first character</summary>
    private: void parseValue();

    /// <summary>Parses an invalid line until the next line break</summary>
    private: void parseMalformedLine();

    /// <summary>Submits was has been parsed so far as a line</summary>
    private: void submitLine();

    /// <summary>Generates a line in which a property is declared</summary>
    /// <returns>A property-declaring line filled from the current parser state</returns>
    private: PropertyLine *generatePropertyLine();

    /// <summary>Generates a line in which a section is declared</summary>
    /// <returns>A section-declaring line filled from the current parser state</returns>
    private: SectionLine *generateSectionLine();

    /// <summary>Retrieves the default section or create a new one if none exists</summary>
    private: IndexedSection *getOrCreateDefaultSection();

    /// <summary>Resets the parser state</summary>
    private: void resetState();

    /// <summary>Allocates memory for the specified line and fills its content buffer</summary>
    /// <typeparam name="TLine">
    ///   Type of line that will be allocated. Must inherit from the <see cref="Line" /> type
    /// <typeparam>
    /// <param name="contents">Line contents that will be stored</param>
    /// <param name="byteCount">Number of bytes the line long</param>
    /// <returns>The newly allocated line with its content buffer filled</returns>
    private: template<typename TLine> TLine *allocateLineChunked(
      const std::uint8_t *contents, std::size_t byteCount
    );

    /// <summary>Allocates memory for the specified type with extra bytes</summary>
    /// <typeparam name="T">Type for which memory will be allocated</typeparam>
    /// <param name="extraByteCount">
    ///   Number of extra bytes to make available behind the space used by the type
    /// </param>
    /// <returns>
    ///   An *uninitialized* pointer to the requested type which is followed by
    ///   the desired amount of extra bytes, aligned to the requirements of the type
    /// </returns>
    private: template<typename T> T *allocateChunked(std::size_t extraByteCount = 0);

    /// <summary>The document model into this parser will fill</summary>
    private: IniDocumentModel *target;
    /// <summary>Remaining space in the current allocation chunk</summary>
    private: std::size_t remainingChunkByteCount;
    /// <summary>Section into which parsed elements go currently</summary>
    private: IndexedSection *currentSection;

    /// <summary>Pointer to the beginning of the .ini file in memory</summary>
    private: const std::uint8_t *fileBegin;
    /// <summary>Pointer one past the end of the .ini file in memory</memory>
    private: const std::uint8_t *fileEnd;
    /// <summary>Pointer to the current parsing location</summary>
    private: const std::uint8_t *parsePosition;

    /// <summary>Position at which the current line in the .ini file begins</summary>
    private: const std::uint8_t *lineStart;
    /// <summary>Position at which the current section or property's name starts</summary>
    private: const std::uint8_t *nameStart;
    /// <summary>Position one after the end of the current section or property name</summary>
    private: const std::uint8_t *nameEnd;
    /// <summary>Position at which the current property's value starts, if any</summary>
    private: const std::uint8_t *valueStart;
    /// <summary>Position one after the end of the current property's value, if any</summary>
    private: const std::uint8_t *valueEnd;

    /// <summary>Whether a section was found in the current line</summary>
    private: bool sectionFound;
    /// <summary>Whether an equals sign was found in the current line</summary>
    private: bool equalsSignFound;
    /// <summary>Whether we encountered something that breaks the current line</summary>
    private: bool lineIsMalformed;

    /// <summary>Heuristic - if positive, document uses Windows line breaks</summary>
    private: int windowsLineBreaks;
    /// <summary>Heuristic - if positive, document has blank lines between properties</summary>
    private: int blankLines;
    /// <summary>Heuristic - if positive, document puts spaces around assignment</summary>
    private: int paddedAssignments;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
