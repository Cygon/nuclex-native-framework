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

#if defined(NUCLEX_SUPPORT_LINUX)

#include "Posix/PosixProcessApi.h" // for Pipe, PosixProcessApi
#include "Posix/PosixFileApi.h" // for PosixFileApi

#include "Nuclex/Support/Errors/TimeoutError.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <exception> // for std::terminate()
#include <cstring> // for ::strsignal()

#include <sys/wait.h> // for ::waitpid()
#include <unistd.h> // for ::fork()
#include <signal.h> // for ::sigemptyset(), sigaddset(), etc.

// http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A signal set that only contains the SIGCHLD signal</summary>
  class ChildSignalSet {

    /// <summary>Initializes a new signal set</summary>
    public: ChildSignalSet() :
      blocked(false) {

      int result = ::sigemptyset(&this->signalSet);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not create an empty signal set", errorNumber
        );
      }

      result = ::sigaddset(&this->signalSet, SIGCHLD);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not add a signal to a signal set", errorNumber
        );
      }
    }

    /// <summary>Restores the previous signal set if SIOGCHLD was blocked</summary>
    public: ~ChildSignalSet() {
      if(this->blocked) {
        int result = ::sigprocmask(SIG_SETMASK, &this->previousSignalSet, nullptr);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result != -1) && u8"Previous signal mask could be restored");
      }
    }

    /// <summary>Provides the signal set containing only SIGCHLD</summary>
    /// <returns>The signal set with SIGCHLD in it</returns>
    public: const ::sigset_t &GetSignalSet() const { return this->signalSet; }

    /// <summary>Begins blocking SIGCHLD in the calling thread</summary>
    /// <remarks>
    ///   The documentation is confusing, but blocking probably means we can handle
    ///   it here and the default signal handler isn't invoked.
    /// </remarks>
    public: void Block() {
      assert(!this->blocked && u8"Block is only called once");

      // Add SIGCHLD to the blocked signals and remember the previous signal set
      int result = ::sigprocmask(SIG_BLOCK, &this->signalSet, &this->previousSignalSet);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not update signal mask for thread", errorNumber
        );
      }
    }

    /// <summary>Signal set containing only SIGCHLD</summary>
    private: ::sigset_t signalSet;
    /// <summary>Set of signals that were blocked before adding SIGCHLD</summary>
    private: ::sigset_t previousSignalSet;
    /// <summary>Whether SIGCHLD has been blocked for the calling thread yet</summary>
    private: bool blocked;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Replaces a standard file (stdin, stdout or stderr)</summary>
  /// <param name="standardFileName">
  ///   File name of the standard file that will be replaced (only used for error reporting)
  /// </param>
  /// <param name="standardFileNumber">
  ///   File number of the standard file that will be replaced
  /// </param>
  /// <param name="replacementFileNumber">
  ///   File number of the file that will replace the standard file
  /// </param>
  void replaceStandardFile(
    const std::string &standardFileName, int standardFileNumber, int replacementFileNumber
  ) {
    static const std::string errorMessage(u8"Error redirecting ", 18);

    // See the documentation for dup2(), this drops the target file number's
    // current file and makes it points to the same file as our replacement file number.
    int result = ::dup2(replacementFileNumber, standardFileNumber);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      std::string message;
      message.reserve(18 + standardFileName.length() + 1);
      message.append(errorMessage);
      message.append(standardFileName);
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(message, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //


  /// <summary>Replaces the running process with the specified executable</summary>
  /// <param name="executablePath">
  ///   Path ot the executable with which the running process will continue
  /// </param>
  /// <param name="arguments">
  ///   Command line arguments that will be passed to the new executable
  /// </param>
  [[noreturn]] void executeChildProcess(
    const std::string &workingDirectory,
    const std::string &executablePath,
    const std::string &prependedExecutablePath,
    const std::vector<std::string> &arguments
  ) {
    static const std::string errorMessage(u8"Could not execute ", 18);

    // Build an array with the (non-const) values of all arguments. Using const_cast
    // here is safe so long as the OS is POSIX-compatible, which promises not to
    // modify the argument strings passed to any of the exec*() methods:
    // https://www.man7.org/linux/man-pages/man3/exec.3p.html
    std::vector<char *> argumentValues;
    {
      if(prependedExecutablePath.empty()) {
        argumentValues.reserve(arguments.size() + 1);
      } else {
        argumentValues.reserve(arguments.size() + 2);
        argumentValues.push_back(const_cast<char *>(prependedExecutablePath.c_str()));
      }
      for(std::size_t index = 0; index < arguments.size(); ++index) {
        argumentValues.push_back(const_cast<char *>(arguments[index].c_str()));
      }
      argumentValues.push_back(nullptr); // Terminator
    }

    // Change into the desired working directory before handing off to the new executable
    if(!workingDirectory.empty()) {
      int result = ::chdir(workingDirectory.c_str());
      if(result == -1) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could change working directory", errorNumber
        );
      }
    }

    // Calling any of the ::exec*() methods will replace the process with the specified
    // executable, so if it succeeds, this code is no longer running or even present.
    // Some system resources, such as stdin, stdout and stderr do propagate to the
    // switched-out process, however.
    int result = ::execvp(executablePath.c_str(), &argumentValues[0]);
    if(likely(result == -1)) {
      int errorNumber = errno;
      std::string message;
      message.reserve(18 + executablePath.length() + 1);
      message.append(errorMessage);
      message.append(executablePath);
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(message, errorNumber);
    } else { // execvp() should only return if an error occurs
      assert((result == -1) && u8"execvp() returned and reported a result that was not -1");
      std::terminate();
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Process::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the process</summary>
    public: PlatformDependentImplementationData() :
      ChildProcessId(0),
      Finished(false),
      ExitCode(-1),
      StdinFileNumber(-1),
      StdoutFileNumber(-1),
      StderrFileNumber(-1) {}

    /// <summary>Process ID of the child (forked) process</summary>
    public: ::pid_t ChildProcessId;
    /// <summary>Whether we already spotted that the process has exited</summary>
    public: mutable bool Finished;
    /// <summary>Code by which the process exited the last time around</summary>
    public: mutable int ExitCode;
    /// <summary>File number of the writing end of the stdin pipe</summary>
    public: int StdinFileNumber;
    /// <summary>File number of the reading end of the stdout pipe</summary>
    public: int StdoutFileNumber;
    /// <summary>File numebr of the reading end of the stderr pipe</summary>
    public: int StderrFileNumber; 

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
    if(impl.ChildProcessId != 0) {
      if(!impl.Finished) {
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
    if constexpr(!implementationDataFitsInBuffer) {
      delete this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::Start(
    const std::vector<std::string> &arguments /* = std::vector<std::string>() */,
    bool prependExecutableName /* = true */
  ) {
    using Nuclex::Support::Threading::Posix::Pipe;

    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId != 0) {
      throw std::logic_error(u8"Child process is still running or not joined yet");
    }

    // These directories need to be resolved in the parent process because relative
    // paths are interpreted as relative to the running process and if we do it after
    // the call to fork(), the running process for the code interested in these
    // variables happens to be the child process.
    std::string absoluteWorkingDirectory, absoluteExecutablePath;

    Nuclex::Support::Threading::Posix::PosixProcessApi::GetAbsoluteExecutablePath(
      absoluteExecutablePath, this->executablePath
    );
    if(!this->workingDirectory.empty()) {
      Nuclex::Support::Threading::Posix::PosixProcessApi::GetAbsoluteWorkingDirectory(
        absoluteWorkingDirectory, this->workingDirectory
      );
    }

    Pipe stdinPipe, stdoutPipe, stderrPipe;

    // Calling fork() will clone the current process' main thread (no other threads).
    // The original process will have the process id of the child process in the return
    // value while the child process will have 0 returned.
    ::pid_t childOrZeroPid = ::fork();
    if(unlikely(childOrZeroPid == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not fork process", errorNumber
      );
    }

    // Are we the original process?
    if(likely(childOrZeroPid != 0)) {

      // Close the unwanted ends of each pipe
      stdinPipe.CloseOneEnd(0);
      stdoutPipe.CloseOneEnd(1);
      stderrPipe.CloseOneEnd(1);

      stdinPipe.SetEndNonBlocking(1); // Don't block when writing to stdin either
      stdoutPipe.SetEndNonBlocking(0);
      stderrPipe.SetEndNonBlocking(0);

      // And take hold of the wanted ends of each pipe
      PlatformDependentImplementationData &impl = getImplementationData();
      impl.ChildProcessId = childOrZeroPid;
      impl.StdinFileNumber = stdinPipe.ReleaseOneEnd(1);
      impl.StdoutFileNumber = stdoutPipe.ReleaseOneEnd(0);
      impl.StderrFileNumber = stderrPipe.ReleaseOneEnd(0);

    } else { // No we're the child process

      // Close the unwanted ends of each pipe (these are the opposite ends from
      // the ones the parent process closes)
      stdinPipe.CloseOneEnd(1);
      stdoutPipe.CloseOneEnd(0);
      stderrPipe.CloseOneEnd(0);

      // Remap stdin, stdout and stderr to the pipes (by duplicating each file),
      // then close the original files, too, since the duplicates are enough.
      replaceStandardFile(u8"stdin", STDIN_FILENO, stdinPipe.GetOneEnd(0));
      stdinPipe.CloseOneEnd(0);
      replaceStandardFile(u8"stdout", STDOUT_FILENO, stdoutPipe.GetOneEnd(1));
      stdoutPipe.CloseOneEnd(1);
      replaceStandardFile(u8"stderr", STDERR_FILENO, stderrPipe.GetOneEnd(1));
      stderrPipe.CloseOneEnd(1);

      // Load a new executable image, completely replacing this (child) process.
      executeChildProcess(
        absoluteWorkingDirectory,
        absoluteExecutablePath,
        prependExecutableName ? executablePath : std::string(),
        arguments
      );
      std::terminate(); // Should never be reached, executeChildProcess() doesn't return

    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::IsRunning() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      return false; // Not launched yet or joined already
    }

    // If we already saw the process finish, don't wall ::waitpaid() again (it wouldn't work)
    if(impl.Finished) {
      return false;
    }

    // Check whether the process has returned an exit code yet.
    // The call may be interrupted by signals, so keep checking if it's interrupted.
    for(;;) {
      int result = ::waitpid(impl.ChildProcessId, &impl.ExitCode, WNOHANG);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(errorNumber == EINTR) {
          continue;
        } else {
          Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
            u8"Could not check if process is running", errorNumber
          );
        }
      }

      // If no status (exit code) is available, that means the process is still running
      if(likely(result == 0)) {
        return true;
      } else {
        impl.Finished = true;
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::Wait(
    std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */
  ) const {
    using Nuclex::Support::Threading::Posix::PosixProcessApi;

    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    // If we already saw the process finish, don't wall ::waitpaid() again (it wouldn't work)
    if(impl.Finished) {
      return true;
    }

    // Calculate the absolute time at which the timeout occurs (the clock is
    // monotonic, so even if the system clock is adjusted, this won't be affected)
    struct ::timespec timeoutTime = PosixProcessApi::GetTimePlusMilliseconds(
      CLOCK_MONOTONIC, patience
    );
    struct ::timespec waitTime;
    {
      waitTime.tv_sec = 0;
      waitTime.tv_nsec = 4000000; // 4 ms
    }

    // SIGCHLD is blocked for the thread during this scope
    {
      ChildSignalSet sigChild;
      sigChild.Block();
      for(;;) {

        // Pump the stdout and stderr pipes first. If the process ended before Wait() was
        // called, this may be the only chance to obtain its output before ::waitpid() reaps
        // the zombie process (sending the pipe buffers to the afterlife).
        PumpOutputStreams();

        // Check if the child process our caller is interested in has already exited
        {
          int result = ::waitpid(impl.ChildProcessId, &impl.ExitCode, WNOHANG);
          if(unlikely(result == -1)) {
            int errorNumber = errno;
            Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
              u8"Could not check status of child process", errorNumber
            );
          } else if(result != 0) {
            assert((result == impl.ChildProcessId) && u8"Correct child process has exited");
            impl.Finished = true;
            return true; // The child process terminated, our wait was successful
          }
        }

        // Check if the caller-specified patience time has been exceeded.
        // If the provided timeout was 0, this will bail out after the first ::waitpid().
        if(PosixProcessApi::HasTimedOut(CLOCK_MONOTONIC, timeoutTime)) {
          return false;
        }

        // There should not be a race condition between ::waitpid() at the top and
        // ::sigtimedwait() here. If SIGCHLD is signalled inbetween, a flag should
        // be set, ::sigtimedwait() should clear it again and return immediately.
        //
        // Whilst debugging, I once watched it wait for the full wait time despite
        // this. We need to keep pumping stdout and stderr anyway, so we'll re-check
        // once every few milliseconds, meaning that in the worst case, we report
        // process termination a few milliseconds to late.

        // Wait until SIGCHLD is signaled. This indicates that /some/ child process
        // has terminated and it may be the one we're waiting for.
        {
          int result = ::sigtimedwait(&sigChild.GetSignalSet(), nullptr, &waitTime);
          if(unlikely(result == -1)) {
            int errorNumber = errno;
            if(unlikely(errorNumber == EINTR)) {
              continue; // Another signal interrupted the wait, just keep trying...
            } else if(unlikely(errorNumber != EAGAIN)) { // EAGAIN means timeout
              Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
                u8"Could not wait for signal from child process", errorNumber
              );
            }
          }
        }

      } // for(;;)
    } // ChildSignalSet scope
  }

  // ------------------------------------------------------------------------------------------- //

  int Process::Join(std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */) {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    // If the process is potentially still running, wait for it to exit
    // (or fetch the exit code if it exited while nobody was looking)
    if(!impl.Finished) {
      bool hasFinished = Wait(patience);
      if(!hasFinished) {
        throw Nuclex::Support::Errors::TimeoutError(
          u8"Timed out waiting for external process to exit"
        );
      }
    }

    // Reset the child process id and finished flag so the process can be launched again
    impl.ChildProcessId = 0;
    impl.Finished = false;

    // Close the parent process ends of the stdin, stdout and stderr pipes
    if(impl.StderrFileNumber != -1) {
      int result = ::close(impl.StderrFileNumber);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Pipe forwarding stderr could be closed");
      impl.StderrFileNumber = -1;
    }
    if(impl.StdoutFileNumber != -1) {
      int result = ::close(impl.StdoutFileNumber);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Pipe forwarding stdout could be closed");
      impl.StdoutFileNumber = -1;
    }
    if(impl.StdinFileNumber != -1) {
      int result = ::close(impl.StdinFileNumber);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Pipe feeding stdin could be closed");
      impl.StdinFileNumber = -1;
    }

    // If the process was terminated due to a signal (i.e. crashed or killed), there is
    // no exit code. So in the case of abnormal termination, we instead throw an exception.
    if(WIFSIGNALED(impl.ExitCode)) {
      const std::string errorMessage("Child process terminated by signal ", 35);
      std::string message;
      message.reserve(35 + 7 + 1);
      message.append(errorMessage);
      message.append(::strsignal(WTERMSIG(impl.ExitCode)));
      throw std::runtime_error(message);
    }

    return WEXITSTATUS(impl.ExitCode);
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::Kill(std::chrono::milliseconds patience /* = std::chrono::milliseconds(5000) */) {
    using Nuclex::Support::Threading::Posix::PosixProcessApi;

    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    if(patience > std::chrono::milliseconds::zero()) {
      PosixProcessApi::RequestProcessTermination(impl.ChildProcessId);
    }

    if(Wait(patience) == false) {
      PosixProcessApi::KillProcess(impl.ChildProcessId);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t Process::Write(const char *characters, std::size_t characterCount) {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    ssize_t writtenByteCount = ::write(impl.StdinFileNumber, characters, characterCount);
    if(writtenByteCount == -1) {
      int errorNumber = errno;
      if(errorNumber == EAGAIN) {
        return 0; // write() would block because the stdin pipe buffer is full
      } else {
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Failed writing character to child process stdin pipe", errorNumber
        );
      }
    }

    assert((writtenByteCount >= 0) && u8"Written byte count is not negative");
    return static_cast<std::size_t>(writtenByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::PumpOutputStreams() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessId == 0) {
      return; // Should we throw an exception here?
    }

    const ::size_t BatchSize = 16384;
    this->buffer.resize(BatchSize);

    int fileNumbers[] = { impl.StdoutFileNumber, impl.StderrFileNumber };
    for(std::size_t pipeIndex = 0; pipeIndex < 2; ++pipeIndex) {

      for(;;) {

        // Try to read data up to our buffer size from the pipe
        ::ssize_t readByteCount = ::read(fileNumbers[pipeIndex], this->buffer.data(), BatchSize);
        if(readByteCount == -1) {
          int errorNumber = errno;
          if(errorNumber == EINTR) {
            continue; // A signal interrupted us, just try again
          } else if(errorNumber == EAGAIN) {
            break; // There was no data waiting in the pipe
          } else if(pipeIndex == 0) {
            Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
              u8"Failed to read pipe buffer for stdout", errorNumber
            );
          } else {
            Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
              u8"Failed to read pipe buffer for stderr", errorNumber
            );
          }
        }

        // If we got any data, deliver it to any possible subscribers.
        if(readByteCount > 0) {
          if(pipeIndex == 0) {
            this->StdOut.Emit(this->buffer.data(), readByteCount);
          } else {
            this->StdErr.Emit(this->buffer.data(), readByteCount);
          }

          // We don't know how much data is waiting in the pipe, so we simply do it
          // like this: if is filled the whole buffer, there's likely more, if it
          // wasn't enough to fill the buffer, we know the pipe has been emptied.
          if(static_cast<std::size_t>(readByteCount) < BatchSize) {
            break;
          }
        } else {
          break;
        }
      } // for(;;)

    } // for(pipeIndex)
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

#endif // defined(NUCLEX_SUPPORT_LINUX)