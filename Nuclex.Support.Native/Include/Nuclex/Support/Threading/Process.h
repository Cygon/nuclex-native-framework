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

#ifndef NUCLEX_SUPPORT_THREADING_PROCESS_H
#define NUCLEX_SUPPORT_THREADING_PROCESS_H

#include "Nuclex/Support/Config.h"

// Currently, the process wrapper only has implementations for Linux and Windows
//
// The Linux version may be fully or nearly Posix-compatible, so feel free to
// remove this check and give it a try.
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Events/Event.h" // TODO: need concurrent event

#include <chrono> // for std::chrono::milliseconds
#include <vector> // for std::vector
#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps an external executable running as an independent process</summary>
  /// <remarks>
  ///   <para>
  ///     This is a convenient helper you can use to run external programs. It will
  ///     deal with the differences between platforms in finding the target executable,
  ///     creating the new process, redirecting its stdin, stdout and stderr streams
  ///     and checking in on the process' status.
  ///   </para>
  ///   <para>
  ///     When specifying an executable name without an absolute path, the directory
  ///     containing the running application will be searched first. This allows you to
  ///     easily call supporting executables that ship with your application, such as
  ///     shader compilers, updaters and launchers.
  ///   </para>
  ///   <para>
  ///     For external processes that generate output (such as a compiler), it is very
  ///     important to keep pumping the output streams by calling
  ///     <see cref="PumpOutputStreams" /> regularly. Otherwise, child process' will
  ///     eventually fill the buffers of its stdout and stderr redirection pipes and
  ///     hang on an std::cout or printf() call waiting for buffer space to free up.
  ///   </para>
  /// </remarks>
  class Process {

    /// <summary>Returns the directory in which the running executable resides</summary>
    /// <returns>The directory holding the currently running executable</returns>
    /// <remarks>
    ///   <para>
    ///     The returned path is the application's executable directory, guaranteed to
    ///     end with the platform's native directory separator character. If you directly
    ///     append a filename to the returned string, you get a valid, absolute path to
    ///     any file stored in the same directory as your application's executable.
    ///   </para>
    ///   <para>
    ///     Do note that on Unix-like platforms it is usually not appropriate to store
    ///     data and configuration files in the application directory, unless your
    ///     application is installed in the '/opt' directory (but hardcoding such
    ///     a requirement would get in the way of a package manager installing your
    ///     application in '/usr/bin' and its data files in '/usr/share').
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::string GetExecutableDirectory();

    /// <summary>Event that is fired whenever the process writes to stdout</summary>
    public: Nuclex::Support::Events::Event<
      void(const char *, std::size_t)
    > StdOut;
    /// <summary>Event that is fired whenever the process writes to stderr</summary>
    public: Nuclex::Support::Events::Event<
      void(const char *, std::size_t)
    > StdErr;

    /// <summary>Initializes a new process without starting it</summary>
    /// <param name="executablePath">
    ///   Executable that should be run, optionally including its path
    /// </param>
    /// <remarks>
    ///   <para>
    ///     If the executable name doesn't contain a path (or is specified with a relative
    ///     path), the path is interpreted as relative to the directory in which the calling
    ///     application's executable resides.
    ///   </para>
    ///   <para>
    ///     SHould the executable not be found that way, the normal search rules of
    ///     the underlying operating system apply, i.e. the PATH environment variable.
    ///   </para>
    ///   <para>
    ///     The executable search will not take place if you specify an absolute path.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API Process(const std::string &executablePath);
    /// <summary>Kills the external process and waits until it is gone</summary>
    public: NUCLEX_SUPPORT_API ~Process();

    /// <summary>Sets the working directory the child process will start in</summary>
    /// <param name="newWorkingDirectory">
    ///   Initial working directory the child process will use. Set to an empty string
    ///   to use the current working directory of the parent process
    /// </param>
    /// <remarks>
    ///   The working directory starts out as empty, meaning it will be left to
    ///   the operating system what working directory to use. Usually that means
    ///   whatever directory the parent process was in when the child process started.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void SetWorkingDirectory(
      const std::string &newWorkingDirectory
    ) {
      this->workingDirectory = newWorkingDirectory;
    }

    /// <summary>
    ///   Starts the external process, passing the specified command-line arguments along
    /// </summary>
    /// <param name="arguments">Arguments that will be passed to the external process</param>
    /// <param name="prependExecutableName">
    ///   By convention, the first argument passed is normally the name of the executable
    ///   itself. Leaving this set to 'true' will automatically prepend the executable name
    ///   to the argument list.
    /// </param>
    /// <remarks>
    ///   There's a major difference to how arguments are passed to a process between Linux
    ///   and Windows. On Linux, arguments are an array of strings, allowing spaces to be
    ///   passed along. On Windows, the arguments become a single string, meaning that there
    ///   is no way to distinguish between an argument containing a space and two arguments.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Start(
      const std::vector<std::string> &arguments = std::vector<std::string>(),
      bool prependExecutableName = true
    );

    /// <summary>Checks whether the process is still running</summary>
    /// <returns>True if the process was still running, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool IsRunning() const;

    /// <summary>Waits for the process to exit normally</summary>
    /// <param name="patience">
    ///   Time the method will wait for the process to exit.
    /// </param>
    /// <returns>
    ///   True if the process exited within the allotted time, false if it is still running.
    /// </returns>
    public: NUCLEX_SUPPORT_API bool Wait(
      std::chrono::milliseconds patience = std::chrono::milliseconds(30000)
    ) const;

    /// <summary>Waits for the process to exit normally and returns its exit code</summary>
    /// <param name="patience">
    ///   Time the method will wait for the process to exit. If the process does no exit within
    ///   this time, an exception will be thrown.
    /// </param>
    /// <returns>
    ///   The exit code returned by the process (most programs adhere to returning 0 if
    ///   everything went well and 1 if there was a problem).
    /// </returns>
    public: NUCLEX_SUPPORT_API int Join(
      std::chrono::milliseconds patience = std::chrono::milliseconds(30000)
    );

    /// <summary>Attempts to terminate the external process</summary>
    /// <param name="patience">
    ///   Time given to the process to respond to a request to gracefully terminate.
    ///   If zero, the process is forcefully killed immediately.
    /// </param>
    /// <remarks>
    ///   This will first attempt to gracefully exit the running process (by sending it
    ///   a SIGTERM signal or closing its main window). If this doesn't result in the process
    ///   terminating within the a grace period, this method will attempt to terminate
    ///   the process forcefully via either SIGKILL or by aborting its process.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Kill(
      std::chrono::milliseconds patience = std::chrono::milliseconds(5000)
    );

    /// <summary>Sends input to the running process' stdin</summary>
    /// <param name="characters">Characters that will be sent to the process' stdin</param>
    /// <param name="characterCount">Number of characters
    /// <returns>The number of bytes that have been written to the process' stdin</returns>
    /// <remarks>
    ///   If you fill the buffer of the process' stdin pipe, it may not be possible to
    ///   write more data to stdin until the process has read from stdin.
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::size_t Write(
      const char *characters, std::size_t characterCount
    );

    /// <summary>Fetches data from the stdout and stderr streams</summary>
    /// <remarks>
    ///   <para>
    ///     All console output of the external process is redirected into pipes. These pipes
    ///     have a limited buffer. Once the buffer is full, the external process will block
    ///     until the pipe's buffer has been emptied.
    ///   </para>
    ///   <para>
    ///     Because of that, it's very important to call this method regularly, especially if
    ///     the child process is generating a lot of output. Not doing so can cause the child
    ///     process to wait forever in a printf() or std::cout call.
    ///   </para>
    ///   <para>
    ///     The provided Wait() and Join() methods will automatically flush the pipe's output
    ///     buffers adequately, but if you just let the instance linger in the background,
    ///     be sure to have some mechanism that calls PumpOutputStreams() regularly.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API void PumpOutputStreams() const;

    // Useful? I'd like to keep this class tight and focused rather then turning
    // it into a general-purpose grabbag for all your child process needs.
    //public: std::any GetNativeProcessId() const;

    // Useful? This would be easy to provide, but I'd rather expose such things
    // purely through the Nuclex.Storage.Native library.
    //public: static std::string GetExecutablePath();

    /// <summary>Path to the executable this process instance is launching</summary>
    private: std::string executablePath;
    /// <summary>Working directory the child process will start in</summary>
    private: std::string workingDirectory;
    /// <summary>Pipe buffer (uses round-robin to flush stdout and stderr)</summary>
    private: mutable std::vector<char> buffer;

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: const PlatformDependentImplementationData &getImplementationData() const;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
    private: union {
      /// <summary>Platform dependent process and file handles used for the process</summary>
      PlatformDependentImplementationData *implementationData;
      /// <summary>Used to hold the platform dependent implementation data if it fits</summary>
      /// <remarks>
      ///   Small performance / memory fragmentation improvement.
      ///   This avoids a micro-allocation for the implenmentation data structure in most cases.
      /// </remarks>
#if defined(NUCLEX_SUPPORT_WINDOWS)
      unsigned char implementationDataBuffer[32];
#else // Posix and Linux
      unsigned char implementationDataBuffer[24];
#endif
    };

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
