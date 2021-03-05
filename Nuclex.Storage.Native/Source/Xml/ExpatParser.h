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

#ifndef NUCLEX_STORAGE_XML_EXPATPARSER_H
#define NUCLEX_STORAGE_XML_EXPATPARSER_H

#include "Nuclex/Storage/Config.h"

#include "ExpatApi.h"

#include <string>
#include <memory>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the eXpat XML parser API</summary>
  class ExpatParser {

    /// <summary>Initializes a new eXpat parser</summary>
    public: ExpatParser(const std::string &charset = "UTF-8");

    /// <summary>Retrieves the wrapped eXpat parser instance</summary>
    /// <returns>The wrapped eXpat parser instance</returns>
    public: XML_Parser Get() const { return this->parser.get(); }

    /// <summary>Builds a string describing the first encountered error</summary>
    /// <returns>A string descrbing the first encountered error</returns>
    public: std::string BuildErrorDescription() const;

    /// <summary>Checks whether the parser has encountered an error</summary>
    /// <returns>True if the parser has encountered an error</returns>
    public: bool ErrorEncountered() const {
      return (this->errorCode != XML_ERROR_NONE);
    }

    /// <summary>Throws an exception if the parser has encountered an error</summary>
    public: void ThrowIfErrorRecorded() const;

    /// <summary>Sets the user data that will be passed to all callbacks</summary>
    /// <param name="userData">User data that will be passed to all callbacks</param>
    public: void SetUserData(void *userData) {
      ::XML_SetUserData(this->parser.get(), userData);
    }

    /// <summary>
    ///   Sets the callbacks that will be invoked when XML elements are encountered
    /// </summary>
    /// <param name="start">
    ///   Callback that will be invoked when an opening XML element is encountered
    /// </param>
    /// <param name="end">
    ///   Callback that will be invoked when a closing XML element is encountered
    /// </param>
    public: void SetElementHandler(XML_StartElementHandler start, XML_EndElementHandler end) {
      ::XML_SetElementHandler(this->parser.get(), start, end);
    }

    /// <summary>
    ///   Sets the callback that will be invoked when text is encountered in the XML input
    /// </summary>
    /// <param name="handler">Callback that will be invoked when encountering text</param>
    public: void SetCharacterDataHandler(XML_CharacterDataHandler handler) {
      ::XML_SetCharacterDataHandler(this->parser.get(), handler);
    }

    /// <summary>Pauses or aborts the parser</summary>
    /// <param name="resumable">Whether the parser can be resumed later</param>
    /// <returns>The status reported by the eXpat parser</returns>
    public: XML_Status StopParser(bool resumable = true) {
      XML_Status status = ::XML_StopParser(
        this->parser.get(), resumable ? XML_TRUE : XML_FALSE
      );
      if(status == XML_STATUS_ERROR) {
        recordError();
      }

      return status;
    }

    /// <summary>Resumes parsing after the parser was paused</summary>
    /// <returns>The status reported by the eXpat parser</returns>
    public: XML_Status ResumeParser() {
      XML_Status status = ::XML_ResumeParser(this->parser.get());
      if(status == XML_STATUS_ERROR) {
        recordError();
      }

      return status;
    }

    /// <summary>Allocates an internal buffer the parser will use</summary>
    /// <param name="length">Length of the buffer in bytes</param>
    public: void *GetBuffer(std::size_t length);

    /// <summary>
    ///   Parses the contents of eXpat's current buffer (<see cref="GetBuffer" />)
    /// </summary>
    /// <param name="length">Amount of data the buffer has been filled with</param>
    /// <param name="isFinal">Whether this call provides the final chunk of data</param>
    /// <returns>The status reported by the eXpat parser</returns>
    public: XML_Status ParseBuffer(std::size_t length, bool isFinal);

    /// <summary>Records the first error the parser has encountered</summary>
    private: void recordError() {
      if(this->errorCode == XML_ERROR_NONE) { // Only record the first error
        this->errorCode = ::XML_GetErrorCode(this->parser.get());
        this->errorLineNumber = ::XML_GetErrorLineNumber(this->parser.get());
        this->errorColumnIndex = ::XML_GetErrorColumnNumber(this->parser.get());
      }
    }

    /// <summary>The expat parser used to walk through the XML elements</summary>
    private: std::unique_ptr<XML_ParserStruct, decltype(&::XML_ParserFree)> parser;

    /// <summary>Code of the last error that has occurred</summary>
    private: XML_Error errorCode;
    /// <summary>Line number at which the error has occurred, if any</summary>
    private: XML_Size errorLineNumber;
    /// <summary>Column index at which the error has occurred, if any</summary>
    private: XML_Size errorColumnIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_EXPATPARSER_H
