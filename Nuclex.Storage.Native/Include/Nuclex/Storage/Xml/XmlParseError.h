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

#ifndef NUCLEX_STORAGE_XML_XMLPARSEERROR_H
#define NUCLEX_STORAGE_XML_XMLPARSEERROR_H

#include "Nuclex/Storage/Config.h"

#include <stdexcept>
#include <cstddef>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates an error when parsing XML plaintext</summary>
  class XmlParseError : public std::runtime_error {

    /// <summary>Used when the line number or column is unknown</summary>    
    private: static const std::size_t Unknown = 0;

    /// <summary>Initializes a new XML parsing error</summary>
    /// <param name="message">Message that describes the parsing error</param>
    /// <param name="line">Line number at which the error was encountered</param>
    /// <param name="column">Column index at which the error was encountered</param>
	  public: explicit XmlParseError(
      const std::string &message,
      std::size_t line = Unknown, std::size_t column = Unknown
    ) :
      std::runtime_error(message),
      line(line),
      column(column) {}

    /// <summary>Initializes a new XML parsing error</summary>
    /// <param name="message">Message that describes the parsing error</param>
    /// <param name="line">Line number at which the error was encountered</param>
    /// <param name="column">Column index at which the error was encountered</param>
	  public: explicit XmlParseError(
      const char *message,
      std::size_t line = Unknown, std::size_t column = Unknown
    ) :
		  std::runtime_error(message),
      line(line),
      column(column) {}

    /// <summary>Retrieves the line number at which the parsing error occurred</summary>
    /// <returns>The line number of the parsing error, if known</returns>
    public: std::size_t GetLine() const { return this->line; }

    /// <summary>Retrieves the column index at which the parsing error occurred</summary>
    /// <returns>The column index of the parsing error, if known</returns>
    public: std::size_t GetColumn() const { return this->column; }

    /// <summary>Line number of the parsing error</summary>
    private: std::size_t line;
    /// <summary>Column index of the parsing error</summary>
    private: std::size_t column;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLPARSEERROR_H
