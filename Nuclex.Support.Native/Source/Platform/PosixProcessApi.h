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

#ifndef NUCLEX_SUPPORT_PLATFORM_POSIXPROCESSAPI_H
#define NUCLEX_SUPPORT_PLATFORM_POSIXPROCESSAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "PosixApi.h"

#include <cassert> // for assert()
#include <chrono> // for std::chrono::milliseconds

#include <sys/types.h> // for ::pid_t

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets up a pipe that can be used for inter-process communication</summary>
  class Pipe {

    /// <summary>Opens a new pipe</summary>
    public: Pipe();

    /// <summary>Closes whatever end(s) of the pipe have not been used yet</summary>
    public: ~Pipe();

    /// <summary>Closes one end of the pipe</summary>
    /// <param name="whichEnd">Which end of the pipe to close</param>
    public: void CloseOneEnd(int whichEnd);

    /// <summary>Relinquishes ownership of the file number for one end of the pipe</summary>
    /// <param name="whichEnd">For which end of the pipe ownership will be released</param>
    /// <returns>The file number of the relinquished end of the pipe</returns>
    public: int ReleaseOneEnd(int whichEnd);

    /// <summary>Enabled non-blocking IO for one end of the pipe</summary>
    /// <param name="whichEnd">For which end non-blocking IO will be enabled</param>
    public: void SetEndNonBlocking(int whichEnd);

    /// <summary>Fetches the file number of one end of the pipe</summary>
    /// <param name="whichEnd">
    ///   Index of the pipe end (0 or 1) whose file number will be returned
    /// </param>
    /// <returns>The file number for the requested end of the pipe</returns>
    public: int GetOneEnd(int whichEnd) const {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      return this->ends[whichEnd];
    }

    /// <summary>File numbers for each end of the pipe</summary>
    private: int ends[2];

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix process and inter-process communication API</summary>
  class PosixProcessApi {

    /// <summary>Sends the SIGTERM signal to the process, requesting it to exit</summary>
    /// <param name="processId">Id of the process that will be requested to quit</param>
    /// <remarks>
    ///   This is the nice way of asking a process to exit. If the process does not
    ///   explicitly handle SIGTERM, it will be caught by its standard library and usually
    ///   do the equivalent of an ::exit(1).
    /// </remarks>
    public: static void RequestProcessTermination(::pid_t processId);

    /// <summary>Sends the SIGKILL signal to the process to end it forcefully</summary>
    /// <param name="processId">Id of the process that will be killed</param>
    /// <remarks>
    ///   SIGKILL cannot be ignored by the process and will kill it (if the caller has
    ///   sufficient rights). Only use this as a last resort.
    /// </remarks>
    public: static void KillProcess(::pid_t processId);

    /// <summary>Locates an executable by emulating the search of ::LoadLibrary()</summary>
    /// <param name="target">Target string to store the executable path in</param>
    /// <param name="executable">Executable, with or without path</param>
    /// <remarks>
    ///   <para>
    ///     Posix' exec*() methods already have a well-defined search order (use the PATH
    ///     environment variable unless the string contains a slash, in which case it's
    ///     relative to the current working directory), but we want to alter it slightly
    ///     to offer consistent behavior on both Linux and Windows
    ///   </para>
    ///   <para>
    ///     Namely, the running application's own install directory should be search first
    ///     for any executables that do not contain a path (or a relative path).
    ///     This method guarantees that behavior by looking in the directory holding
    ///     the running application's executable and only then fall back to Posix behavior.
    ///   </para>
    /// </remarks>
    public: static void GetAbsoluteExecutablePath(
      std::string &target, const std::string &executable
    );

    /// <summary>Determines the absolute path of the working directory</summary>
    /// <param name="target">String into which the working directory will be written</param>
    /// <param name="workingDirectory">Working directory as specified by the user</param>
    /// <remarks>
    ///   This either keeps the working directory as-is (if it's an absolute path) or
    ///   interprets it relative to the executable's path for consistent behavior.
    /// </remarks>
    public: static void GetAbsoluteWorkingDirectory(
      std::string &target, const std::string &workingDirectory
    );

    /// <summary>Searches for an executable using the PATH environment variable</summary>
    /// <param name="target">String into which the absolute path will be written</param>
    /// <param name="executable">Relative path to the executable that will be searched</param>
    private: static void searchExecutableInPath(
      std::string &target, const std::string &executable
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXPROCESSAPI_H
