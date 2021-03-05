#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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
#define NUCLEX_STORAGE_SOURCE 1

#include "XmlBlobReader.Impl.h"
#include "Nuclex/Storage/Blob.h"

#include <algorithm>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified character is a whitespace</summary>
  /// <param elementName="character">Character that will be checked for being a whitespace</param>
  /// <returns>True if the character is a whitespace character, false otherwise</returns>
  bool IsWhitespace(char character) {
    return
      (character == ' ') ||
      (character == '\t') ||
      (character == '\r') ||
      (character == '\n');
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  const std::size_t XmlBlobReader::Impl::BufferSize = 4096;

  // ------------------------------------------------------------------------------------------- //

  XmlBlobReader::Impl::Impl(const Blob &blob) :
    blob(blob),
    blobLength(blob.GetSize()),
    position(0),
    isSuspended(false),
    elementEndOutstanding(false) {

    this->parser.SetUserData(static_cast<void *>(this));
    this->parser.SetElementHandler(
      &XmlBlobReader::Impl::elementStartEncountered,
      &XmlBlobReader::Impl::elementEndEncountered
    );
    this->parser.SetCharacterDataHandler(&XmlBlobReader::Impl::textEncountered);
  }

  // ------------------------------------------------------------------------------------------- //

  XmlBlobReader::Impl::~Impl() {}

  // ------------------------------------------------------------------------------------------- //

  XmlReadEvent XmlBlobReader::Impl::Read() {
    this->parser.ThrowIfErrorRecorded();

    // If we received a call to elementEndEncountered() after requesting the parser
    // to pause in elementStartEncountered(), forward the element end notification first
    if(this->elementEndOutstanding) {
      this->elementEndOutstanding = false;
      this->attributes.clear();
      return XmlReadEvent::ElementEnd;
    }

    XML_Status status = parseNextChunk();
    switch(status) {
      case XML_STATUS_SUSPENDED: {
        return this->lastReadEvent;
      }
      case XML_STATUS_OK: {
        return XmlReadEvent::End;
      }
      case XML_STATUS_ERROR: {
        this->parser.ThrowIfErrorRecorded();
      }
      default: {
        throw std::runtime_error("eXpat parser reported an unknown status");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  XML_Status XmlBlobReader::Impl::parseNextChunk() {
    XML_Status status;

    do {
      if(this->isSuspended) {
        this->isSuspended = false;
        status = this->parser.ResumeParser();
      } else {
        std::size_t length = static_cast<std::size_t>(
          std::min<std::uint64_t>(BufferSize, this->blobLength - this->position)
        );

        // Ask eXpat for a buffer and fill it. According to the documentation, this
        // avoids an additional copy of the data (probably to ensure the pointer stays
        // valid after XML_StopParser() is called)
        {
          void *buffer = this->parser.GetBuffer(length);
          if(buffer == nullptr) {
            throw std::runtime_error("eXpat failed to allocate a buffer for XML parsing");
          }

          this->blob.ReadAt(this->position, buffer, length);
          this->position += length;
        }

        status = this->parser.ParseBuffer(length, (this->position >= this->blobLength));
      }
    } while((status == XML_STATUS_OK) && (this->position < this->blobLength));

    return status;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::elementStartEncountered(
    const char *elementName,
    const char **firstAttribute, std::size_t attributeCount
  ) {
    this->name.assign(elementName);

    this->attributes.resize(attributeCount);
    for(std::size_t index = 0; index < attributeCount; ++index) {
      this->attributes[index].first.assign(*firstAttribute);
      ++firstAttribute;
      this->attributes[index].second.assign(*firstAttribute);
      ++firstAttribute;
    }

    bool resumable = true;
    this->parser.StopParser(resumable);

    this->isSuspended = true;
    this->lastReadEvent = XmlReadEvent::ElementStart;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::elementEndEncountered(const char *elementName) {
    if(this->isSuspended) {
      this->elementEndOutstanding = true;
      return;
    }

    this->attributes.clear();
    this->name.assign(elementName);

    bool resumable = true;
    this->parser.StopParser(resumable);

    this->isSuspended = true;
    this->lastReadEvent = XmlReadEvent::ElementEnd;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::textEncountered(const char *encounteredText, int length) {
    if(length <= 0) {
      return;
    }

    int firstCharacterIndex = 0;
    while(IsWhitespace(encounteredText[firstCharacterIndex])) {
      ++firstCharacterIndex;
    }

    if(firstCharacterIndex < length) {
      this->attributes.clear();
      this->text.assign(encounteredText, static_cast<std::string::size_type>(length));

      bool resumable = true;
      this->parser.StopParser(resumable);

      this->isSuspended = true;
      this->lastReadEvent = XmlReadEvent::Content;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::elementStartEncountered(
    void *impl, const char *name, const char **attributes
  ) {
    std::size_t stringCount = 0;
    if(attributes != nullptr) {
      while(*(attributes + stringCount) != nullptr) {
        stringCount += 2;
      }
    }

    Impl *self = static_cast<Impl *>(impl);
    self->elementStartEncountered(name, attributes, stringCount / 2);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::elementEndEncountered(void *impl, const char *name) {
    Impl *self = static_cast<Impl *>(impl);
    self->elementEndEncountered(name);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Impl::textEncountered(void *impl, const char *text, int length) {
    Impl *self = static_cast<Impl *>(impl);
    self->textEncountered(text, length);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml
