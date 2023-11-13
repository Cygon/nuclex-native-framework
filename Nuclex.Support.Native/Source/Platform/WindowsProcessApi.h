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

#ifndef NUCLEX_SUPPORT_PLATFORM_WINDOWSPROCESSAPI_H
#define NUCLEX_SUPPORT_PLATFORM_WINDOWSPROCESSAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Directional pipe that can be used for inter-process communication</summary>
  class Pipe {

    /// <summary>Opens a new directional pipe</summary>
    /// <param name="securityAttributes">
    ///   Security attributes controlling whether the pipe is inherited to child processes
    /// </param>
    public: Pipe(const SECURITY_ATTRIBUTES &securityAttributes);

    /// <summary>Closes whatever end(s) of the pipe have not been used yet</summary>
    public: ~Pipe();

    /// <summary>Sets one end of the pipe to be a non-inheritable handle</summary>
    /// <param name="whichEnd">Which end of the pipe will become non-inheritable</param>
    public: void SetEndNonInheritable(std::size_t whichEnd);

    /// <summary>Configures one end of the pipe to not block on read/write calls</summary>
    /// <param name="whichEnd">Which end of the pipe will become non-blocking</param>
    public: void SetEndNonBlocking(std::size_t whichEnd);

    /// <summary>Closes one end of the pipe</summary>
    /// <param name="whichEnd">Which end of the pipe to close</param>
    public: void CloseOneEnd(std::size_t whichEnd);

    /// <summary>Relinquishes ownership of the handle for one end of the pipe</summary>
    /// <param name="whichEnd">For which end of the pipe ownership will be released</param>
    /// <returns>The handle of the relinquished end of the pipe</returns>
    public: HANDLE ReleaseOneEnd(std::size_t whichEnd);

    /// <summary>Fetches the handle of one end of the pipe</summary>
    /// <param name="whichEnd">
    ///   Index of the pipe end (0 or 1) whose handle will be returned
    /// </param>
    /// <returns>The handle of requested end of the pipe</returns>
    public: HANDLE GetOneEnd(std::size_t whichEnd) const {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      return this->ends[whichEnd];
    }

    /// <summary>Handle for the readable and the writable end of the pipe</summary>
    /// <remarks>
    ///   Index 0 is the readable end of the pipe, 1 is the writable end
    /// </remarks>
    private: HANDLE ends[2];

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Windows process and inter-process communication API</summary>
  class WindowsProcessApi {

    /// <summary>Asks the process to gracefully exit</summary>
    /// <param name="processHandle">
    ///   Handle of the process that will be asked to terminate
    /// </param>
    /// <remarks>
    ///   <para>
    ///     The concept of gracefully terminating processes seems to be a weakness of
    ///     Windows. There's no SIGTERM or truly standardized way to do it.
    ///   </para>
    ///   <para>
    ///     You can post a WM_QUIT message to the message pumping thread, but there's
    ///     no indication which thread is pumping messages. And you can only enumerate
    ///     all threads in the system rather than those of a single process.
    ///   </para>
    ///   <para>
    ///     This method tries to post a WM_QUIT message to all threads of the target
    ///     process in the hope that there's a message pump that listens.
    ///   </para>
    ///   <para>
    ///     You can also post a WM_CLOSE message to the application's main window to
    ///     nicely ask it to close (just like clicking on the X in the corner).
    ///     However, windows belong to threads and there a invisible IPC windows, too,
    ///     so the only good way of finding the window is to... enumerate all existing
    ///     top-level windows in the system.
    ///   </para>
    ///   <para>
    ///     This method tries to post a WM_CLOSE message to all top-level windows
    ///     belonging to the target process. This may just lead to a &quot;do you want
    ///     to save your work before quitting&quot; message, though.
    ///   </para>
    ///   <para>
    ///     A command-line application (like ffmpeg, for example) may have neither
    ///     message pump nor a window, so this method would have no effect on it.
    ///     Nor does Microsoft seem to know a way to deal with it, shutting down
    ///     Windows while running ffmpeg simply kills it with no questions asked...
    ///   </para>
    /// </remarks>
    public: static void RequestProcessToTerminate(HANDLE processHandle);

    /// <summary>Forcefully terminated the specified process</summary>
    /// <param name="processHandle">Handle of the process that will be terminated</param>
    public: static void KillProcess(HANDLE processHandle);

    /// <summary>Retrieves the exit code a process has exited with</summary>
    /// <param name="processHandle">Handle of the process whose exit code will be checked</param>
    /// <returns>
    ///   The exit code of the process or STILL_ACTIVE if the process has not exited yet
    /// </returns>
    public: static DWORD GetProcessExitCode(HANDLE processHandle);

    /// <summary>Determines the path of the running executable</summary>
    /// <param name="target">Target string to store the executable path in</param>
    public: static void GetOwnExecutablePath(std::wstring &target);

    /// <summary>Locates an executable by emulating the search of ::LoadLibrary()</summary>
    /// <param name="target">Target string to store the executable path in</param>
    /// <param name="executable">Executable, with or without path</param>
    /// <remarks>
    ///   <para>
    ///     This looks in the &quot;executable image path&quot; first, just like
    ///     ::LoadLibrary() would do and how ::CreateProcess() would, if we weren't forced
    ///     to use its &quot;module name&quot; parameter.
    ///   </para>
    ///   <para>
    ///     We're not looking to perfectly emulate ::LoadLibrary(), just guarantee a behavior
    ///     that allows executables from the application's own directory to be reliably
    ///     called first.
    ///   </para>
    ///   <para>
    ///     If this method can't find the executable in any of the locations it checks,
    ///     or if the executable is an absolute path, the executable will be returned as-is.
    ///   </para>
    /// </remarks>
    public: static void GetAbsoluteExecutablePath(
      std::wstring &target, const std::wstring &executable
    );

    /// <summary>Determines the absolute path of the working directory</summary>
    /// <param name="target">String into which the working directory will be written</param>
    /// <param name="workingDirectory">Working directory as specified by the user</param>
    /// <remarks>
    ///   This either keeps the working directory as-is (if it's an absolute path) or
    ///   interprets it relative to the executable's path for consistent behavior.
    /// </remarks>
    public: static void GetAbsoluteWorkingDirectory(
      std::wstring &target, const std::wstring &workingDirectory
    );

    /// <summary>Obtains the full path of the specified module</summary>
    /// <param name="moduleHandle">
    ///   Handle of the module whose path will be determined, nullptr for executable
    /// </param>
    /// <returns>The full path to the specified module</returns>
    private: static void getModuleFileName(
      std::wstring &target, HMODULE moduleHandle = nullptr
    );

    /// <summary>
    ///   Determines the absolute path of an executable by checking the system's search paths
    /// </summary>
    /// <param name="target">Target string to store the executable path in</param>
    /// <param name="executable">Executable, with or without path</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the executable could not be found
    /// </param>
    /// <remarks>
    ///   <para>
    ///     This simply wraps the SearchPath() method. A warning on MSDN states that this
    ///     method works differently from how LoadLibrary() searches paths, one of the differences
    ///     is that it doesn't look in the executable's own directory first.
    ///   </para>
    ///   <para>
    ///     However, if we want passing the executable as the zeroeth parameter in
    ///     CreateProcess() optional, we need to use the ModuleName argument which only
    ///     accepts the absolute, full path of an executable file.
    ///   </para>
    /// </remarks>
    private: static void searchExecutablePath(
      std::wstring &target, const std::wstring &executable, bool throwOnError = true
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_WINDOWSPROCESSAPI_H
