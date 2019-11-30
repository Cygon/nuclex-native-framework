#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_PIXELS_TEMPORARYDIRECTORYSCOPE_H
#define NUCLEX_PIXELS_TEMPORARYDIRECTORYSCOPE_H

#include "Nuclex/Pixels/Config.h"

#include <string>

#if defined(NUCLEX_PIXELS_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#endif

#include <cstdio>
#include <cstdlib>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a unique temporary directory for use by the unit tests</summary>
  class TemporaryDirectoryScope {

    /// <summary>Creates the unique temporary directory</summary>
    public: TemporaryDirectoryScope();

    /// <summary>Destroys the unique temporary directory with all its contents</summary>
    public: ~TemporaryDirectoryScope();

    /// <summary>Gives the path of the unique temporary directory</summary>
    /// <returns>The absolute path of the unique temporary directory</returns>
    public: const std::string &GetPath() const { return this->path; }

    /// <summary>Builds the path for a file in the unique temporary directory</summary>
    /// <param name="filename">Name of a file the temporary directory should hold</param>
    /// <returns>The absolute path for a file in the temporary directory</returns>
    public: std::string GetPath(const std::string &filename) const;

    /// <summary>Reads the full contents of the specified file into a string</summary>
    /// <param name="filename">Name of the file that will be read</param>
    /// <returns>The full contents of the file as a string</returns>
    public: std::string ReadFullFile(const std::string &filename) const;

    /// <summary>Creates a file with the specified string as its contents</summary>
    /// <param name="filename">Name of the file that will be created</param>
    /// <param name="contents">Contents that will be written to the file</param>
    public: void WriteFullFile(const std::string &filename, const std::string &contents) const;

    /// <summary>Path of the temporary directory</summary>
    private: std::string path;

#if defined(NUCLEX_PIXELS_WIN32)
    /// <summary>Path of the temporary (placeholder) file</summary>
    private: std::wstring tempFilePath;
    /// <summary>Path of the temporary directory</summary>
    private: std::wstring tempDirectoryPath;
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_TEMPORARYDIRECTORYSCOPE_H
