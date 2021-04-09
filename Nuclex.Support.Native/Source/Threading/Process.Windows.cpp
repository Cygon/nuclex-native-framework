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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/Process.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Errors/TimeoutError.h"
#include "Nuclex/Support/Text/StringConverter.h"
#include "../Helpers/WindowsApi.h"
#include "Windows/WindowsProcessApi.h"

#include <exception> // for std::terminate()
#include <cassert> // for assert()
#include <algorithm> // for std::min()

// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Adds quotes around a path if it contains spaces is unquoted</summary>
  /// <param name="path">Path that may be unquoted and/or contain spaces</param>
  /// <returns>The path, quoted if it was missing quotes but needing them</returns>
  std::wstring quoteIfNeeded(const std::wstring &path) {
    std::wstring::size_type firstNonSpaceIndex = path.find_first_not_of(L' ');
    if(firstNonSpaceIndex == std::wstring::npos) {
      return path; // Path has no spaces (also catches case when path has length zero)
    }

    if(path[firstNonSpaceIndex] == L'"') {
      std::wstring::size_type lastQuoteIndex = path.find_first_of(L'"', firstNonSpaceIndex + 1);
      if(lastQuoteIndex == std::wstring::npos) {
        return path + L'"';
      } else {
        return path;
      }
    }

    std::wstring result;
    result.reserve(path.length() + 3);
    result.push_back(L'"');
    result.append(path);
    result.push_back(L'"');

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Process::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the process</summary>
    public: PlatformDependentImplementationData() :
      ChildProcessHandle(INVALID_HANDLE_VALUE),
      StdinHandle(INVALID_HANDLE_VALUE),
      StdoutHandle(INVALID_HANDLE_VALUE),
      StderrHandle(INVALID_HANDLE_VALUE) {}

    /// <summary>Handle of the child process</summary>
    public: HANDLE ChildProcessHandle;
    /// <summary>File number of the writing end of the stdin pipe</summary>
    public: HANDLE StdinHandle;
    /// <summary>File number of the reading end of the stdout pipe</summary>
    public: HANDLE StdoutHandle;
    /// <summary>File numebr of the reading end of the stderr pipe</summary>
    public: HANDLE StderrHandle; 

  };

  // ------------------------------------------------------------------------------------------- //

  Process::Process(const std::string &executablePath) :
    executablePath(executablePath),
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // Things will still work, but we have to resort to an extra allocation.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      new(this->implementationDataBuffer) PlatformDependentImplementationData();
    } else {
      this->implementationData = new PlatformDependentImplementationData();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Process::~Process() {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle != INVALID_HANDLE_VALUE) {
      if(IsRunning()) {
        try {
          Kill();
        }
        catch(const std::exception &) {
          assert(false && u8"Child process exited before destructor runs");
        }
      }
      try {
        Join(std::chrono::milliseconds(1));
      }
      catch(const std::exception &) {
        assert(false && u8"Child process is Join()ed before destructor runs");
      }
    }

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      this->implementationData->~PlatformDependentImplementationData();
    } else {
      delete this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::Start(
    const std::vector<std::string> &arguments /* = std::vector<std::string>() */,
    bool prependExecutableName /* = true */
  ) {
    using Nuclex::Support::Threading::Windows::WindowsProcessApi;
    using Nuclex::Support::Text::StringConverter;

    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle != INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Child process is still running or not joined yet");
    }

    // Set up a security attribute structure that tells Windows that handles should
    // be inherited and use it when creating the pipes
    SECURITY_ATTRIBUTES pipeSecurityAttributes = {0};
    pipeSecurityAttributes.nLength = sizeof(pipeSecurityAttributes);
    pipeSecurityAttributes.bInheritHandle = TRUE; // non-default!
    pipeSecurityAttributes.lpSecurityDescriptor = nullptr;

    // Create 3 pipes and set the ends that belong to our side as non-inheritable
    Nuclex::Support::Threading::Windows::Pipe stdinPipe(pipeSecurityAttributes);
    stdinPipe.SetEndNonInheritable(1);
    stdinPipe.SetEndNonBlocking(1);
    Nuclex::Support::Threading::Windows::Pipe stdoutPipe(pipeSecurityAttributes);
    stdoutPipe.SetEndNonInheritable(0);
    Nuclex::Support::Threading::Windows::Pipe stderrPipe(pipeSecurityAttributes);
    stderrPipe.SetEndNonInheritable(0);

    ::PROCESS_INFORMATION childProcessInfo = {0};
    {
      STARTUPINFOW childProcessStartupSettings = {0};
      childProcessStartupSettings.cb = sizeof(childProcessStartupSettings);
      childProcessStartupSettings.dwFlags = STARTF_USESTDHANDLES;
      childProcessStartupSettings.hStdInput = stdinPipe.GetOneEnd(0);
      childProcessStartupSettings.hStdOutput = stdoutPipe.GetOneEnd(1);
      childProcessStartupSettings.hStdError = stderrPipe.GetOneEnd(1);

      // Launch the new process. We're using the UTF-16 version (and convert everything
      // from UTF-8 to UTF-16) to ensure we can deal with unicode paths and executable names.
      {
        std::wstring utf16ExecutablePath = StringConverter::WideFromUtf8(this->executablePath);
        std::wstring absoluteUtf16ExecutablePath;
        WindowsProcessApi::GetAbsoluteExecutablePath(
          absoluteUtf16ExecutablePath, utf16ExecutablePath
        );

        // Bundle all the command line arguments together. In Windows, they're just
        // one long string passed to the application.
        std::wstring commandLineArguments;
          {
          if(prependExecutableName) {
            commandLineArguments.append(quoteIfNeeded(utf16ExecutablePath));
            commandLineArguments.push_back(L' ');
          }
          for(std::size_t index = 0; index < arguments.size(); ++index) {
            if(index > 0) {
              commandLineArguments.push_back(L' ');
            }
            commandLineArguments.append(
              Nuclex::Support::Text::StringConverter::WideFromUtf8(arguments[index])
            );
          }
        }

        // Figure out the working directory. Like the executable path, if a relative
        // path is used, it will be relative to the running application executable.
        std::wstring utf16WorkingDirectory;
        if(!this->workingDirectory.empty()) {
          WindowsProcessApi::GetAbsoluteWorkingDirectory(
            utf16WorkingDirectory, StringConverter::WideFromUtf8(this->workingDirectory)
          );
        }

        BOOL result = ::CreateProcessW(
          absoluteUtf16ExecutablePath.c_str(), // module name (= exact absolute path)
          commandLineArguments.data(), // command line argument (with prepended exe path)
          nullptr, // use default security attributes
          nullptr, // use default thread security attributes
          TRUE, // yes, we want to inherit (some) handles
          0, // no extra creation flags
          nullptr, // use the current environment
          this->workingDirectory.empty() ? nullptr : utf16WorkingDirectory.c_str(),
          &childProcessStartupSettings,
          &childProcessInfo
        );
        if(result == FALSE) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not spawn new process", lastErrorCode
          );
        }
      }
    }

    // One end from each of the 3 pipes was inherited to the child process.
    // Here we close our copy of those ends as we're not going to be needing those.
    stdinPipe.CloseOneEnd(0);
    stdoutPipe.CloseOneEnd(1);
    stderrPipe.CloseOneEnd(1);

    // We don't need the handle to the main thread, but have ownership,
    // so be a good citizen and close it.
    {
      BOOL result = ::CloseHandle(childProcessInfo.hThread);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();

        // CHECK: Is it enough to close the process handle?
        // Might be sensible to kill the process since we're exiting with an exception.

        result = ::CloseHandle(childProcessInfo.hProcess);
        assert((result != FALSE) && u8"Child process handle closed successfully");

        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close handle for child process main thread", lastErrorCode
        );
      }
    }

    // If this point is reached, our setup work is done and we can take ownership
    // of the pipe ends (up until this point, the Pipe class would have destroyed them)
    impl.ChildProcessHandle = childProcessInfo.hProcess;
    impl.StdinHandle = stdinPipe.ReleaseOneEnd(1);
    impl.StdoutHandle = stdoutPipe.ReleaseOneEnd(0);
    impl.StderrHandle = stderrPipe.ReleaseOneEnd(0);
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::IsRunning() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      return false; // Not launched yet or joined already
    }

    // Try to get the process' exit code. If the process hasn't exited yet,
    // this method will return the special exit code STILL_ACTIVE.
    DWORD exitCode = Nuclex::Support::Threading::Windows::WindowsProcessApi::GetProcessExitCode(
      impl.ChildProcessHandle
    );

    // We got STILL_ACTIVE, but the process may have exited with this as its actual
    // exit code. So make sure the process exited via WaitForSingleObject()...
    if(exitCode == STILL_ACTIVE) {
      DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, 0U);
      if(result == WAIT_OBJECT_0) {
        return false; // Process did indeed exit with STILL_ACTIVE as its exit code.
      } else if(result == WAIT_TIMEOUT) {
        return true; // Process was really still running
      }

      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Error waiting for external process to exit", lastErrorCode
      );
    } else { // Process exited with an unambiguous exit code
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::Wait(
    std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */
  ) const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    // Wait for the process to exit, but keep servicing the output streams so
    // it won't deadlock in case it's producing (lots of) output.
    DWORD timeoutMilliseconds = static_cast<DWORD>(patience.count());
    DWORD startTickCount = ::GetTickCount();
    for(;;) {

      PumpOutputStreams();

      DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, 4);
      if(result == WAIT_OBJECT_0) {
        return true;
      } else if(result != WAIT_TIMEOUT) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Error waiting for external process to exit", lastErrorCode
        );
      }

      // Check if the timeout has been exceeded. We have to do the math this way
      // to ensure it correctly handles wraparound after 49.7 days.
      DWORD waitedTilliseconds = ::GetTickCount() - startTickCount;
      if(waitedTilliseconds >= timeoutMilliseconds) {
        return false;
      }

    } // for(;;)

  }

  // ------------------------------------------------------------------------------------------- //

  int Process::Join(std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */) {
    using Nuclex::Support::Threading::Windows::WindowsProcessApi;

    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    DWORD exitCode = WindowsProcessApi::GetProcessExitCode(impl.ChildProcessHandle);

    // Well, the process may have exited with STILL_ACTIVE as its exit code :-(
    // Also check WaitForSingleObject() here...
    if(exitCode == STILL_ACTIVE) {
      DWORD timeoutMilliseconds = static_cast<DWORD>(patience.count());
      DWORD startTickCount = ::GetTickCount();
      for(;;) {

        DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, 4);
        if(result == WAIT_OBJECT_0) {
          exitCode = WindowsProcessApi::GetProcessExitCode(impl.ChildProcessHandle);
          break; // Yep, someone returned STILL_ACTIVE as the process' exit code
        } else if(result != WAIT_TIMEOUT) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Error waiting for external process to exit", lastErrorCode
          );
        }

        // Check if the timeout has been exceeded. We have to do the math this way
        // to ensure it correctly handles wraparound after 49.7 days.
        DWORD waitedTilliseconds = ::GetTickCount() - startTickCount;
        if(waitedTilliseconds >= timeoutMilliseconds) {
          throw Nuclex::Support::Errors::TimeoutError(
            u8"Timed out waiting for external process to exit"
          );
        }

      }
    }

    // Pump the stdout and stderr pipes first. If the process ended before Join() was
    // called, this may be the only chance to obtain its output. As far as I can tell,
    // it's like a Linux zombie process and the pipe buffers stay up until ::CloseHandle()
    PumpOutputStreams();

    // Process is well and truly done, close its process handle and clear our handle
    {
      BOOL result = ::CloseHandle(impl.ChildProcessHandle);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close handle of terminated child process", lastErrorCode
        );
      }

      impl.ChildProcessHandle = INVALID_HANDLE_VALUE;
    }

    // Close the parent process ends of the stdin, stdout and stderr pipes
    {
      BOOL result = ::CloseHandle(impl.StderrHandle);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close stderr pipe to child process", lastErrorCode
        );
      }
    }
    {
      BOOL result = ::CloseHandle(impl.StdoutHandle);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close stdout pipe to child process", lastErrorCode
        );
      }
    }
    {
      BOOL result = ::CloseHandle(impl.StdinHandle);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close stdin pipe to child process", lastErrorCode
        );
      }
    }

    return exitCode;
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::PumpOutputStreams() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      return; // Should we throw an exception here?
    }

    HANDLE handles[] = { impl.StdoutHandle, impl.StderrHandle };
    for(std::size_t pipeIndex = 0; pipeIndex < 2; ++pipeIndex) {

      // Check how many bytes are available from the pipe. We need to do this before calling
      // ReadFile() because ReadFile() would block if there are no bytes available.
      DWORD availableByteCount;
      {
        BOOL result = ::PeekNamedPipe(
          handles[pipeIndex], nullptr, 0, nullptr, &availableByteCount, nullptr
        );
        if(result == FALSE) {
          DWORD lastErrorCode = ::GetLastError();
          if(lastErrorCode == ERROR_BROKEN_PIPE) {
            continue; // Process has terminated its end of the pipe, this is okay.
          } else if(pipeIndex == 0) {            
            Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
              u8"Failed to check pipe buffer for stdout", lastErrorCode
            );
          } else {
            Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
              u8"Failed to check pipe buffer for stderr", lastErrorCode
            );
          }
        }
      }

      // If there are bytes available, read them into our reusable buffer and emit
      // the appropriate events to let this instance's owner process the output.
      if(availableByteCount > 0) {
        const DWORD BatchSize = 16384;

        this->buffer.resize(std::min(availableByteCount, BatchSize));
        for(;;) {

          DWORD readByteCount;
          BOOL result = ::ReadFile(
            handles[pipeIndex],
            this->buffer.data(), static_cast<DWORD>(this->buffer.size()),
            &readByteCount,
            nullptr
          );
          if(result == FALSE) {
            DWORD lastErrorCode = ::GetLastError();
            if(lastErrorCode == ERROR_BROKEN_PIPE) {
              break; // Process has terminated its end of the pipe, this is okay.
            } else if(pipeIndex == 0) {
              Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
                u8"Failed to read pipe buffer for stdout", lastErrorCode
              );
            } else {
              Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
                u8"Failed to read pipe buffer for stderr", lastErrorCode
              );
            }
          }

          if(pipeIndex == 0) {
            this->StdOut.Emit(this->buffer.data(), readByteCount);
          } else {
            this->StdErr.Emit(this->buffer.data(), readByteCount);
          }
          if(readByteCount >= availableByteCount) {
            break;
          } else {
            availableByteCount -= readByteCount;
          }

        } // for(;;)
      }

    } // for(pipeIndex)
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::Kill(std::chrono::milliseconds patience /* = std::chrono::milliseconds(5000) */) {
    using Nuclex::Support::Threading::Windows::WindowsProcessApi;

    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    WindowsProcessApi::RequestProcessToTerminate(impl.ChildProcessHandle);

    if(Wait(patience) == false) {
      WindowsProcessApi::KillProcess(impl.ChildProcessHandle);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t Process::Write(const char *characters, std::size_t characterCount) {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    DWORD writtenByteCount;
    BOOL result = ::WriteFile(
      impl.StdinHandle,
      characters, static_cast<DWORD>(characterCount),
      &writtenByteCount,
      nullptr
    );
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Error writing data to stdin pipe of child process", lastErrorCode
      );
    }

    return static_cast<std::size_t>(writtenByteCount);
  }


  // ------------------------------------------------------------------------------------------- //

  const Process::PlatformDependentImplementationData &Process::getImplementationData() const {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<const PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Process::PlatformDependentImplementationData &Process::getImplementationData() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)