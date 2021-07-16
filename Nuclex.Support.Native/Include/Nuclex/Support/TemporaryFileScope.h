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

#ifndef NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H
#define NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a temporary file that is deleted when the scope is destroyed</summary>
  /// <remarks>
  ///   This is very useful for unit tests or if you're dealing with a poorly designed
  ///   library that can only read resources from the file system rather than providing
  ///   an abstract IO interface.
  /// </remarks>
  class TemporaryFileScope {

    /// <summary>Reserves and creates a unique temporary file</summary>
    /// <param name="namePrefix">Prefix for the temporary filename</param>
    public: NUCLEX_SUPPORT_API TemporaryFileScope(const std::string &namePrefix = u8"tmp");

    /// <summary>Deletes the temporary file again</summary>
    public: NUCLEX_SUPPORT_API ~TemporaryFileScope();

    /// <summary>Returns the full, absolute path to the temporary file</summary>
    /// <returns>The absolute path of the temporary file as an UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API const std::string &GetPath() const { return this->path; }

    /// <summary>Replaces the file contents with the specified string</summary>
    /// <param name="text">String whose contents will be written into the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(const std::string &text) {
      SetFileContents(reinterpret_cast<const std::uint8_t *>(text.c_str()), text.length());
    }

    /// <summary>Replaces the file contents with the data in the specified vector</summary>
    /// <param name="contents">Contents that will be written into the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(const std::vector<std::uint8_t> &contents) {
      SetFileContents(contents.data(), contents.size());
    }

    /// <summary>Replaces the file contents with the specified memory block</summary>
    /// <param name="contents">Memory block containing the new file contents</param>
    /// <param name="byteCount">Number of bytes that will be written to the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(
      const std::uint8_t *contents, std::size_t byteCount
    );

    /// <summary>The full path to the temporary file</summary>
    private: std::string path;
    /// <summary>Memory used to store the open file handle</summary>
    private: std::uint8_t privateImplementationData[sizeof(std::uintptr_t)];

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H
