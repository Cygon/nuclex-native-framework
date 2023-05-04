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

#ifndef NUCLEX_SUPPORT_TEMPORARYDIRECTORYSCOPE_H
#define NUCLEX_SUPPORT_TEMPORARYDIRECTORYSCOPE_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a directory that is deleted when the scope is destroyed</summary>
  /// <remarks>
  ///   <para>
  ///     This is very useful for unit tests or if you're dealing with a poorly designed
  ///     library that can only read resources from the file system rather than providing
  ///     an abstract IO interface.
  ///   </para>
  ///   <para>
  ///     When the scope is destroyed, it deletes <strong>all</strong> files inside
  ///     the created temporary directory, include those placed in there by means other
  ///     than the <see cref="PlaceFile" /> method.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       void test() {
  ///         TemporaryDirectoryScope tempDir(u8"abc"); // custom directory name prefix
  ///
  ///         // GetPath() can provide you with the absolute path to a file inside
  ///         // the temporary directory (it does not create the requested file itself)
  ///         save_current_settings(tempDir.GetPath(u8"settings.bin"));
  ///
  ///         // Settings can be loaded into an std::string or std::vector using different
  ///         // overloads provided by the temporary directory scope.
  ///         std::vector<std::uint8_t> savedSettings = tempDir.ReadFile(u8"settings.bin");
  ///
  ///         // Similarly, you can also place your own file in the temporary directory
  ///         tempDir.PlaceFile(u8"message.txt", u8"Hello World");
  ///
  ///         // The temporary directory and all files in it are deleted here
  ///       }
  ///     </code>
  ///   </example>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE TemporaryDirectoryScope {

    /// <summary>Reserves and creates a unique temporary directory</summary>
    /// <param name="namePrefix">Prefix for the temporary directory name</param>
    public: NUCLEX_SUPPORT_API TemporaryDirectoryScope(
      const std::string &namePrefix = u8"tmp"
    );

    /// <summary>Deletes the temporary directory again</summary>
    public: NUCLEX_SUPPORT_API ~TemporaryDirectoryScope();

    /// <summary>Returns the full, absolute path to the temporary directory</summary>
    /// <returns>The absolute path of the temporary directory as an UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API const std::string &GetPath() const { return this->path; }

    /// <summary>Returns the absolute path to a file in the temporary directory</summary>
    /// <param name="filename">
    ///   Name of the file for which the absolute path will be returned
    /// </param>
    /// <returns>
    ///   The absolute path of a file with the specified name as an UTF-8 string
    /// </returns>
    /// <remarks>
    ///   This method does not create a file. It is inteded to be used when you need to
    ///   obtain an absolute path to pass to some external library that writes a file.
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::string GetPath(const std::string &filename) const;

    /// <summary>Places a file with the specified contents in the temporary directory</summary>
    /// <param name="name">Name of the file that will be created</param>
    /// <param name="text">String whose contents will be written into the file</param>
    /// <returns>The full path of the newly created file</returns>
    public: NUCLEX_SUPPORT_API std::string PlaceFile(
      const std::string &name, const std::string &text
    ) {
      return PlaceFile(
        name, reinterpret_cast<const std::uint8_t *>(text.c_str()), text.length()
      );
    }

    /// <summary>Places a file with the specified contents in the temporary directory</summary>
    /// <param name="name">Name of the file that will be created</param>
    /// <param name="contents">Contents that will be written into the file</param>
    /// <returns>The full path of the newly created file</returns>
    public: NUCLEX_SUPPORT_API std::string PlaceFile(
      const std::string &name, const std::vector<std::uint8_t> &contents
    ) {
      return PlaceFile(name, contents.data(), contents.size());
    }

    /// <summary>Places a file with the specified contents in the temporary directory</summary>
    /// <param name="name">Name of the file that will be created</param>
    /// <param name="contents">Memory block containing the new file contents</param>
    /// <param name="byteCount">Number of bytes that will be written to the file</param>
    /// <returns>The full path of the newly created file</returns>
    public: NUCLEX_SUPPORT_API std::string PlaceFile(
      const std::string &name, const std::uint8_t *contents, std::size_t byteCount
    );

    /// <summary>Reads the whole contents of a file in the temporary directory</summary>
    /// <param name="name">Name of the file that will be read</param>
    /// <returns>A vector containing all of the file's contents</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::uint8_t> ReadFile(
      const std::string &name
    ) const {
      std::vector<std::uint8_t> contents;
      ReadFile(name, contents);
      return contents;
    }

    /// <summary>Reads the whole contents of a file in the temporary directory</summary>
    /// <param name="name">Name of the file that will be read</param>
    /// <param name="contents">A vector to which the file's contents will be appended</param>
    public: NUCLEX_SUPPORT_API void ReadFile(
      const std::string &name, std::vector<std::uint8_t> &contents
    ) const;

    /// <summary>Reads the whole contents of a file in the temporary directory</summary>
    /// <param name="name">Name of the file that will be read</param>
    /// <param name="contents">A string to which the file's contents will be appended</param>
    public: NUCLEX_SUPPORT_API void ReadFile(const std::string &name, std::string &contents) const;

    /// <summary>The full path to the temporary directory</summary>
    private: std::string path;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_TEMPORARYDIRECTORYSCOPE_H
