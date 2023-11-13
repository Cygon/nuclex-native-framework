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

#include "IniDocumentModel.FileParser.h"

#include "Nuclex/Support/Text/ParserHelper.h"

#include <memory> // for std::unique_ptr, std::align()
#include <type_traits> // for std::is_base_of
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

// Ambiguous cases and their resolution:
//
//   ["Hello]"       -> Malformed
//   [World          -> Malformed
//   [Foo] = Bar     -> Assignment, no section
//   [Woop][Woop]    -> Two sections, one w/newline one w/o
//   [Foo] Bar = Baz -> Section and assignment
//   [[Yay]          -> Malformed, section
//   Foo = Bar = Baz -> Malformed
//   [Yay = Nay]     -> Malformed
//   "Hello          -> Malformed
//   Foo = [Bar]     -> Assignment, no section
//   Foo = ]][Bar    -> Assignment
//   "Foo" Bar = Baz -> Malformed
//   Foo = "Bar" Baz -> Malformed
//

// Allocation schemes:
//
//   By line                      -> lots of micro-allocations
//   In blocks (custom allocator) -> I have to do reference counting to free anything
//   Load pre-alloc, then by line -> Fast for typical case, no or few micro-allocations
//                                   But requires pre-scan of entire file + more code

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size if the chunks in which memory is allocated</summary>
  const std::size_t AllocationChunkSize = 4096; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the size of a type plus padding for another aligned member</summary>
  /// <typeparam name="T">Type whose size plus padding will be determined</typeparam>
  /// <returns>The size of the type plus padding with another aligned member</returns>
  template<typename T>
  constexpr std::size_t getSizePlusAlignmentPadding() {
    constexpr std::size_t misalignment = (sizeof(T) % alignof(T));
    if constexpr(misalignment > 0) {
      return sizeof(T) + (alignof(T) - misalignment);
    } else {
      return sizeof(T);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::FileParser::FileParser(
    const std::uint8_t *fileContents, std::size_t byteCount
  ) :
    target(nullptr),
    remainingChunkByteCount(0),
    currentSection(nullptr),
    fileBegin(fileContents),
    fileEnd(fileContents + byteCount),
    parsePosition(nullptr),
    lineStart(nullptr),
    nameStart(nullptr),
    nameEnd(nullptr),
    valueStart(nullptr),
    valueEnd(nullptr),
    sectionFound(false),
    equalsSignFound(false),
    lineIsMalformed(false),
    windowsLineBreaks(0),
    blankLines(0),
    paddedAssignments(0) {}

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::ParseInto(IniDocumentModel *documentModel) {
    this->target = documentModel;

    // Reset the parser, just in case someone re-uses an instance
    resetState();
    this->currentSection = nullptr;

    // These are only to collect heuristics on the loaded .ini file's formatting
    // They are not used for the parser state.
    bool previousWasCR = false;
    bool previousWasSpace = false;
    bool encounteredNonBlankCharacter = false;
    bool previousLineWasEmpty = false;
    //bool previousWasEqualsSign = false;

    // Go through the entire file contents byte-by-byte and select the correct parse
    // mode for the elements we encounter. All of these characters are in the ASCII range,
    // thus there are no UTF-8 sequences that could be mistaken for them (multi-byte UTF-8
    // codepoints will have the highest bit set in all bytes)
    this->parsePosition = this->lineStart = this->fileBegin;
    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      switch(current) {

        // Comments (any section or property already found still counts)
        case '#':
        case ';': { parseComment(); break; }

        // Equals sign, line is a property assignment
        case '=': {
          if(equalsSignFound) {
            parseMalformedLine();
          } else {
            if(this->parsePosition > this->lineStart) {
              previousWasSpace = Text::ParserHelper::IsWhitespace(*(this->parsePosition - 1));
            }
            if(previousWasSpace) {
              ++this->paddedAssignments;
            } else {
              --this->paddedAssignments;
            }

            this->equalsSignFound = true;
            ++this->parsePosition;
          }
          break;
        }

        // Line break, submits the current line to the document model
        case '\n': {
          if(previousWasCR) {
            ++this->windowsLineBreaks;
          } else {
            --this->windowsLineBreaks;
          }
          submitLine();

          // Update heuristics
          if(previousLineWasEmpty) {
            ++this->blankLines;
          } else {
            --this->blankLines;
          }
          previousLineWasEmpty = !encounteredNonBlankCharacter;
          encounteredNonBlankCharacter = false;
          break;
        }

        // Other character, parse as section name, property name or property value
        default: {
          previousWasCR = (current == '\r');
          previousWasSpace = Text::ParserHelper::IsWhitespace(std::uint8_t(current));
          encounteredNonBlankCharacter |= (!previousWasSpace);

          if(previousWasSpace) {
            ++this->parsePosition; // skip over it
          } else if(equalsSignFound) {
            parseValue();
          } else {
            parseName();
          }
          break;
        }

      } // switch on current byte
    } // while parse position is before end of file

    // Even if the file's last line didn't end with a line break,
    // we still treat it as a line of its own
    if(this->parsePosition > this->lineStart) {
      submitLine();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseComment() {
    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      if(current == '\n') {
        //submitLine();
        break;
      } else { // Skip everything that isn't a newline character
        ++this->parsePosition;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseName() {
    bool isInQuote = false;
    bool quoteEncountered = false;
    bool isInSection = false;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;

      // When inside a quote, ignore everything but the closing quote
      // (or newline / end-of-file which are handled in all cases)
      if(isInQuote) {
        nameEnd = this->parsePosition; // Quotes name includes anything until closing quote
        switch(current) {
          case '"': {
            isInQuote = false;
            break;
          }
          case '\n': { // Newline without closing quote? -> Line is malformed
            this->lineIsMalformed = true;
            return;
          }
        }
        isInQuote = (current != '"');
        nameEnd = this->parsePosition;
      } else { // Outside of quote
        switch(current) {

          // Comment start found?
          case ';':
          case '#': {
            parseMalformedLine(); // Name without equals sign? -> Line is malformed
            return;
          }

          // Section start found?
          case '[': {
            if((this->nameStart != nullptr) || isInSection) { // Bracket is not first char?
              parseMalformedLine();
              return;
            } else if(this->sectionFound) { // Did we already see a section in this line?
              submitLine();
            }

            isInSection = true;
            //nameStart = this->parsePosition + 1;
            break;
          }

          // Section end found?
          case ']': {
            if((this->nameStart == nullptr) || !isInSection) { // Bracket is first char?
              parseMalformedLine();
              return;
            }

            isInSection = false;
            //this->nameEnd = this->parsePosition;
            this->sectionFound = true;
            break;
          }

          // Quoted name found?
          case '"': {
            if((this->nameStart != nullptr) || quoteEncountered) { // Quote is not first char?
              parseMalformedLine();
              return;
            } else { // Quote is first char encountered
              quoteEncountered = true;
              isInQuote = true;
              nameStart = this->parsePosition + 1;
            }
            break;
          }

          // Equals sign found? The name part is over, assignment follows
          case '=': {
            if(isInSection) { // Equals sign inside section name? -> line is malformed
              parseMalformedLine();
            }
            // Just return, the root parser will set the equalsSignFound property.
            return;
          }

          // Newline found? Either the section was closed or the line is malformed.
          case '\n': {
            this->lineIsMalformed |= isInSection;
            return;
          }

          // Other characters without special meaning
          default: {
            if(!Text::ParserHelper::IsWhitespace(std::uint8_t(current))) {
              if(quoteEncountered) { // Characters after quote? -> line is malformed
                parseMalformedLine();
                return;
              }
              if(nameStart == nullptr) {
                nameStart = this->parsePosition;
              }
              nameEnd = this->parsePosition + 1;
            }
            break;
          }

        } // switch on current byte
      } // is outside of quote

      ++this->parsePosition;
    } // while parse position is before end of file
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseValue() {
    bool isInQuote = false;
    bool quoteEncountered = false;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;

      // When inside a quote, ignore everything but the closing quote
      // (or newline / end-of-file which are handled in all cases)
      if(isInQuote) {
        valueEnd = this->parsePosition; // Quotes name includes anything until closing quote
        switch(current) {
          case '"': {
            isInQuote = false;
            break;
          }
          case '\n': { // Newline without closing quote? -> Line is malformed
            this->lineIsMalformed = true;
            return;
          }
        }
      } else { // Outside of quote
        switch(current) {

          // Comment start found?
          case ';':
          case '#': {
            parseComment();
            return;
          }

          // Quoted value found?
          case '"': {
            if((this->valueStart != nullptr) || quoteEncountered) { // Quote is not first char?
              parseMalformedLine();
              return;
            } else { // Quote is first char encountered
              quoteEncountered = true;
              isInQuote = true;
              valueStart = this->parsePosition + 1;
            }
            break;
          }

          // Another equals sign found? -> line is malformed
          case '=': {
            parseMalformedLine();
            return;
          }

          // Newline found? The value ends, we're done
          case '\n': {
            return;
          }

          // Other characters without special meaning
          default: {
            if(!Text::ParserHelper::IsWhitespace(std::uint8_t(current))) {
              if(quoteEncountered) { // Characters after quote? -> line is malformed
                parseMalformedLine();
                return;
              }
              if(valueStart == nullptr) {
                valueStart = this->parsePosition;
              }
              valueEnd = this->parsePosition + 1;
            }
            break;
          }

        } // switch on current byte
      } // is outside of quote

      ++this->parsePosition;
    } // while parse position is before end of file
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseMalformedLine() {
    this->lineIsMalformed = true;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      if(current == '\n') {
        break;
      }

      ++this->parsePosition;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::submitLine() {
    ++this->parsePosition;

    Line *newLine;
    if(this->lineIsMalformed) {
      newLine = allocateLineChunked<Line>(
        this->lineStart, this->parsePosition - this->lineStart
      );
    } else if(this->equalsSignFound) {
      newLine = generatePropertyLine();
    } else if(this->sectionFound) {
      newLine = generateSectionLine();
    } else {
      newLine = allocateLineChunked<Line>(
        this->lineStart, this->parsePosition - this->lineStart
      );
    }

    // If this is the first line we submit to the document model,
    // initialize the firstLine attribute so the file can be serialized top-to-bottom
    if(this->target->firstLine == nullptr) {
      this->target->firstLine = newLine;
      newLine->Previous = newLine;
      newLine->Next = newLine;
    } else {
      Line *lastLine = this->target->firstLine->Previous;

      newLine->Next = this->target->firstLine;
      newLine->Previous = lastLine;

      lastLine->Next = newLine;
      this->target->firstLine->Previous = newLine;
    }

    // The currentSection and index work is done by the generatePropertyLine()
    // and generateSectionLine() methods, so we're already done here!

    resetState();
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::PropertyLine *IniDocumentModel::FileParser::generatePropertyLine() {
    PropertyLine *newPropertyLine = allocateLineChunked<PropertyLine>(
      this->lineStart, this->parsePosition - this->lineStart
    );

    // Initialize the property value. This will allow the document model to look up
    // and read or write the property's value quickly when accessed by the user.
    if((this->valueStart != nullptr) && (this->valueEnd != nullptr)) {
      newPropertyLine->ValueStartIndex = this->valueStart - this->lineStart;
      newPropertyLine->ValueLength = this->valueEnd - this->valueStart;
    } else {
      newPropertyLine->ValueStartIndex = 0;
      newPropertyLine->ValueLength = 0;
    }

    // Place the property name in the declaration line and also properly initialize
    // a string we can use to look up or insert this property into the index.
    std::string propertyName;
    {
      if((this->nameStart != nullptr) && (this->nameEnd != nullptr)) {
        newPropertyLine->NameStartIndex = this->nameStart - this->lineStart;
        newPropertyLine->NameLength = this->nameEnd - this->nameStart;
        propertyName.assign(nameStart, nameEnd);
      } else {
        newPropertyLine->NameStartIndex = 0;
        newPropertyLine->NameLength = 0;
        // intentionally leaves propertyName as an empty string
      }
    }

    // Add the new property to the index so it can be looked up by name
    if(this->currentSection == nullptr) {
      this->currentSection = getOrCreateDefaultSection();
    }
    if(this->currentSection->LastLine == nullptr) {
      this->currentSection->LastLine = newPropertyLine;
    }
    this->currentSection->Properties.insert(
      PropertyMap::value_type(propertyName, newPropertyLine)
    );

    return newPropertyLine;
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::SectionLine *IniDocumentModel::FileParser::generateSectionLine() {
    SectionLine *newSectionLine = allocateLineChunked<SectionLine>(
      this->lineStart, this->parsePosition - this->lineStart
    );

    // Place the section name in the declaration line and also properly initialize
    // a string we can use to look up or insert this section into the index.
    std::string sectionName;
    {
      if((this->nameStart != nullptr) && (this->nameEnd != nullptr)) {
        newSectionLine->NameStartIndex = this->nameStart - this->lineStart;
        newSectionLine->NameLength = this->nameEnd - this->nameStart;
        sectionName.assign(nameStart, nameEnd);
      } else {
        newSectionLine->NameStartIndex = 0;
        newSectionLine->NameLength = 0;
        // intentionally leaves sectionName as an empty string
      }
    }

    // Update the currentSection attribute to
    SectionMap::iterator sectionIterator = this->target->sections.find(sectionName);
    if(sectionIterator == this->target->sections.end()) {
      IndexedSection *newSection = allocateChunked<IndexedSection>(0);
      new(newSection) IndexedSection();
      newSection->DeclarationLine = newSectionLine;
      newSection->LastLine = newSectionLine;
      this->target->sections.insert(
        SectionMap::value_type(sectionName, newSection)
      );
      this->currentSection = newSection;
    } else { // If a section appears twice or multiple .inis are loaded
      this->currentSection = sectionIterator->second;
    }

    this->currentSection->LastLine = newSectionLine;

    return newSectionLine;
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IndexedSection *IniDocumentModel::FileParser::getOrCreateDefaultSection() {
    SectionMap::iterator sectionIterator = this->target->sections.find(std::string());
    if(sectionIterator == this->target->sections.end()) {
      IndexedSection *newSection = allocateChunked<IndexedSection>(0);
      new(newSection) IndexedSection();
      this->target->sections.insert(
        SectionMap::value_type(std::string(), newSection)
      );
      return newSection;
    } else {
      return sectionIterator->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::resetState() {
    this->lineStart = this->parsePosition;

    this->nameStart = this->nameEnd = nullptr;
    this->valueStart = this->valueEnd = nullptr;

    this->sectionFound = this->equalsSignFound = this->lineIsMalformed = false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  TLine *IniDocumentModel::FileParser::allocateLineChunked(
    const std::uint8_t *contents, std::size_t byteCount
  ) {
    static_assert(std::is_base_of<Line, TLine>::value && u8"TLine inherits from Line");

    // Allocate memory for a new line, assign its content pointer to hold
    // the line loaded from the .ini file and copy the line contents into it.
    TLine *newLine = allocateChunked<TLine>(byteCount);
    {
      newLine->Contents = (
        reinterpret_cast<std::uint8_t *>(newLine) + getSizePlusAlignmentPadding<TLine>()
      );
      newLine->Length = byteCount;

      std::copy_n(contents, byteCount, newLine->Contents);
    }

    return newLine;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename T>
  T *IniDocumentModel::FileParser::allocateChunked(std::size_t extraByteCount /* = 0 */) {

    // While we're asked to allocate a specific type, making extra bytes available
    // requires us to allocate as std::uint8_t. The start address still needs to be
    // appropriately aligned for the requested type (otherwise we'd have to keep
    // separate pointers for delete[] and for the allocated type).
    #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(T));
    #endif

    // Try to obtain the requested memory. If it is larger than half the allocation chunk
    // size, it gets its own special allocation. Otherwise, it either fits in the current
    // chunk or we need to start a new one. The alignment of the extra bytes is only for
    // good manners but uses the same alignment as members of type T, relative to the start
    // address of type T, which is also aligned, so we don't need to look at the pointer.
    std::size_t totalByteCount = getSizePlusAlignmentPadding<T>() + extraByteCount;
    if(totalByteCount * 2 < AllocationChunkSize) {

      // Calculate the offset within the chunk at which the new instance would start.
      // Since the chunk itself is already aligned (__STDCPP_DEFAULT_NEW_ALIGNMENT__),
      // we don't have to even look at the memory address itself.
      std::size_t occupiedByteCount = AllocationChunkSize - this->remainingChunkByteCount;
      {
        std::size_t misalignment = occupiedByteCount % alignof(T);
        if(misalignment > 0) {
          occupiedByteCount += alignof(T) - misalignment;
        }
      }

      // If the new instance fits into the current chunk, place it there.
      if(occupiedByteCount + totalByteCount < AllocationChunkSize) {
        this->remainingChunkByteCount = AllocationChunkSize - occupiedByteCount - totalByteCount;
        std::size_t chunkCount = this->target->loadedLinesMemory.size();
        std::uint8_t *memory = this->target->loadedLinesMemory[chunkCount - 1];
        return reinterpret_cast<T *>(memory + occupiedByteCount);
      } else { // Instance didn't fit in the current chunk or no chunk allocated
        std::unique_ptr<std::uint8_t[]> newChunk(new std::uint8_t[AllocationChunkSize]);
        this->target->loadedLinesMemory.push_back(newChunk.get());
        this->remainingChunkByteCount = AllocationChunkSize - totalByteCount;
        return reinterpret_cast<T *>(newChunk.release());
      }

    } else { // Requested instance would take half the allocation chunk size or more
      std::unique_ptr<std::uint8_t[]> newChunk(new std::uint8_t[totalByteCount]);
      this->target->createdLinesMemory.insert(newChunk.get());
      return reinterpret_cast<T *>(newChunk.release());
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
