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

#include "PosixProcessApi.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include "PosixFileApi.h"

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <cstdlib> // for ::getenv(), ::ldiv(), ::ldiv_t

#include <unistd.h> // for ::pipe(), ::readlink()
#include <fcntl.h> // for ::fcntl()
#include <signal.h> // for ::kill()
#include <limits.h> // for PATH_MAX

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the path of the process image file for the runnign application
  /// </summary>
  /// <param name="target">String that will receive the path of the executable</param>
  void getExecutablePath(std::string &target) {
    target.resize(PATH_MAX);

    std::string ownProcessLink;
    ownProcessLink.reserve(16);
    ownProcessLink.assign(u8"/proc/self/exe", 14);

    // Try to read the symlink to obtain the path to the running executable
    ::ssize_t characterCount = ::readlink(ownProcessLink.c_str(), target.data(), PATH_MAX);
    if(characterCount == -1) {
      int errorNumber = errno;
      if((errorNumber != EACCES) && (errorNumber != ENOTDIR) && (errorNumber != ENOENT)) {
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not follow '/proc/self/exe' to own path", errorNumber
        );
      }

      // Make another attempt with the PID file accessed directly (no recursive symlink).
      {
        ownProcessLink.resize(6);

        ::pid_t ownPid = ::getpid();
        Nuclex::Support::Text::lexical_append(target, ownPid);

        ownProcessLink.append(u8"/exe", 4);
      }
      characterCount = ::readlink(ownProcessLink.c_str(), target.data(), PATH_MAX);
      if(characterCount == -1) {
        errorNumber = errno;

        //std::string message;
        //message.reserve(18 + ownProcessLink.size() + 13 + 1);
        //message.append(u8"Could not follow '", 18);
        //message.append(ownProcessLink);
        //message.append(u8"' to own path", 13);
        //Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(message, errorNumber);

        // Let's stay with the original error message, /proc/self/exe gives
        // the user a much better idea at what the application wanted to do than
        // a random PID that doesn't exist anymore after the error is printed.
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not follow '/proc/self/exe' to own path", errorNumber
        );
      }
    }

    target.resize(characterCount);

    Nuclex::Support::Threading::Posix::PosixFileApi::RemoveFileFromPath(target);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  Pipe::Pipe() :
    ends {-1, -1} {

    int result = ::pipe(this->ends);
    if(unlikely(result != 0)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not set up a pipe", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Pipe::~Pipe() {
    if(this->ends[1] != -1) {
      int result = ::close(this->ends[1]);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Right end of temporary pipe closed successfully");
    }
    if(this->ends[0] != -1) {
      int result = ::close(this->ends[0]);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Left end of temporary pipe closed successfully");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::CloseOneEnd(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int result = ::close(this->ends[whichEnd]);
    if(unlikely(result != 0)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not close one end of a pipe", errorNumber
      );
    }

    this->ends[whichEnd] = -1;
  }

  // ------------------------------------------------------------------------------------------- //

  int Pipe::ReleaseOneEnd(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int end = this->ends[whichEnd];
    this->ends[whichEnd] = -1;
    return end;
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::SetEndNonBlocking(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int result = ::fcntl(this->ends[whichEnd], F_GETFD);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not query file descriptor flags of a pipe end", errorNumber
      );
    }

    int newFlags = result | O_NONBLOCK;
    result = ::fcntl(this->ends[whichEnd], F_SETFD, newFlags);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not add O_NONBLOCK to the file descriptor flags of a pipe end", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  struct ::timespec PosixProcessApi::GetTimePlusMilliseconds(
    ::clockid_t clock, std::chrono::milliseconds addedTime
  ) {
    struct ::timespec futureTime;

    // Query the specified clock's current time
    int result = ::clock_gettime(clock, &futureTime);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get time from clock", errorNumber
      );
    }

    // Calculate the future point in time by adding the requested number of milliseconds
    {
      const std::size_t NanoSecondsPerMillisecond = 1000000; // 1,000,000 ns = 1 ms
      const std::size_t NanoSecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

      // timespec has seconds and nanoseconds, so divide the milliseconds into full seconds
      // and remainder milliseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(addedTime.count(), 1000);

      // If the summed nanoseconds add up to more than one second, increment the timespec's
      // seconds, otherwise just assign the summed nanoseconds.
      std::size_t nanoseconds = (
        divisionResults.rem * NanoSecondsPerMillisecond + futureTime.tv_nsec
      );
      if(nanoseconds >= NanoSecondsPerSecond) {
        futureTime.tv_sec += divisionResults.quot + 1;
        futureTime.tv_nsec = nanoseconds - NanoSecondsPerSecond;
      } else {
        futureTime.tv_sec += divisionResults.quot;
        futureTime.tv_nsec = nanoseconds;
      }
    }

    return futureTime;
  }

  // ------------------------------------------------------------------------------------------- //

  bool PosixProcessApi::HasTimedOut(::clockid_t clock, const struct ::timespec &endTime) {
    struct ::timespec currentTime;
    {
      int result = ::clock_gettime(clock, &currentTime);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not get time from clock", errorNumber
        );
      }
    }

    return (
      (currentTime.tv_sec > endTime.tv_sec) ||
      (
        (currentTime.tv_sec == endTime.tv_sec) &&
        (currentTime.tv_nsec >= endTime.tv_nsec)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::RequestProcessTermination(::pid_t processId){
    int result = ::kill(processId, SIGTERM);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not send SIGTERM to a process", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::KillProcess(::pid_t processId) {
    int result = ::kill(processId, SIGKILL);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not send SIGTERM to a process", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::GetAbsoluteExecutablePath(
    std::string &target, const std::string &executable
  ) {
    if(PosixFileApi::IsPathRelative(executable)) {
      getExecutablePath(target);
      PosixFileApi::AppendPath(target, executable);
      if(PosixFileApi::DoesFileExist(target)) {
        return;
      }

      searchExecutableInPath(target, executable);
    } else {
      target.assign(executable);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::GetAbsoluteWorkingDirectory(
    std::string &target, const std::string &workingDirectory
  ) {
    if(PosixFileApi::IsPathRelative(workingDirectory)) {
      getExecutablePath(target);
      PosixFileApi::AppendPath(target, workingDirectory);
    } else {
      target.assign(workingDirectory);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::searchExecutableInPath(
    std::string &target, const std::string &executable
  ) {
    const char *path = ::getenv(u8"PATH");
    if(path != nullptr) {
      const char *start = path;
      while(*path != 0) {
        if(*path == ':') {
          if(path > start) {
            target.assign(start, path);
            PosixFileApi::AppendPath(target, executable);
            if(PosixFileApi::DoesFileExist(target)) {
              return;
            }
          }
          start = path + 1;
        }

        ++path;
      }

      // Final path in list.
      if(path > start) {
        target.assign(start, path);
        PosixFileApi::AppendPath(target, executable);
        if(PosixFileApi::DoesFileExist(target)) {
          return;
        }
      }
    }

    target.assign(executable);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Posix

#endif // !defined(NUCLEX_SUPPORT_WIN32)
