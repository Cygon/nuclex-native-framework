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

#include "Nuclex/Support/Text/RollingLogger.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NO_MINMAX
#include <Windows.h> // for GetSystemTime()
#else
#include <ctime> // for ::timespec, ::clock_gettime() and ::gmtime_r()
#include <cerrno> // for ::errno
#include "../Platform/PosixApi.h" // for strerror() wrapper
#endif

#include <cassert> // for assert()

// IDEA: Use Unicode symbols rather than writing the severity
//
// 🛈 for information
// ⚠ for warning (this is way too small with my font?)
// ⛔ for errors
//
// These all look inconsistent, non-proportional and hard to distinguish.
// Idea canned for now.

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Digits 0-59 for conversion of numbers in a stamestamp</summary>
  const char TimestampDigits[][2] = {
    { '0', '0' }, { '0', '1' }, { '0', '2' }, { '0', '3' }, { '0', '4' }, { '0', '5' },
    { '0', '6' }, { '0', '7' }, { '0', '8' }, { '0', '9' }, { '1', '0' }, { '1', '1' },
    { '1', '2' }, { '1', '3' }, { '1', '4' }, { '1', '5' }, { '1', '6' }, { '1', '7' },
    { '1', '8' }, { '1', '9' }, { '2', '0' }, { '2', '1' }, { '2', '2' }, { '2', '3' },
    { '2', '4' }, { '2', '5' }, { '2', '6' }, { '2', '7' }, { '2', '8' }, { '2', '9' },
    { '3', '0' }, { '3', '1' }, { '3', '2' }, { '3', '3' }, { '3', '4' }, { '3', '5' },
    { '3', '6' }, { '3', '7' }, { '3', '8' }, { '3', '9' }, { '4', '0' }, { '4', '1' },
    { '4', '2' }, { '4', '3' }, { '4', '4' }, { '4', '5' }, { '4', '6' }, { '4', '7' },
    { '4', '8' }, { '4', '9' }, { '5', '0' }, { '5', '1' }, { '5', '2' }, { '5', '3' },
    { '5', '4' }, { '5', '5' }, { '5', '6' }, { '5', '7' }, { '5', '8' }, { '5', '9' }
    //{ '6', '0' }, { '6', '1' }, { '6', '2' }, { '6', '3' }, { '6'}
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Length of the timestamp in textual form</summary>
  /// <remarks>
  ///   'hh:mm:ss.uuu ' (including the space character that is always present)
  /// </remarks>
  const std::size_t TimeStampLength = 13;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Length of the severity tag</summary>
  /// <remarks>
  ///   'INFO    ',
  ///   'WARNING ' or
  ///   'ERROR   ' (including the space)
  /// </remarks>
  const std::size_t SeverityLength = 8;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Number of space characters added for one indentation level</summary>
  const std::size_t IndentationSpaceCount = 2;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  RollingLogger::RollingLogger(
    std::size_t historyLineCount /* = 1024U */, std::size_t lineSizeHint /* = 100U */
  ) :
    nextLineIndex(0),
    oldestLineIndex(0),
    lines(historyLineCount + 1), // +1 for the line being formed (it's constructed in place)
    currentLine(nullptr),
    indentationCount(0) {
    assert((historyLineCount >= 1) && u8"History line count must be at least one line");

    // Reserve memory on all lines so we have one up-front allocation that will hopefully
    // stay the only one (assuming our user doesn't build overlong lines)
    for(std::size_t index = 0; index < historyLineCount + 1; ++index) {
      this->lines[index].reserve(lineSizeHint);
    }

    this->currentLine = &this->lines[0];
    this->currentLine->resize(TimeStampLength + SeverityLength);
  }

  // ------------------------------------------------------------------------------------------- //

  //RollingLogger::~RollingLogger() = default;

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Indent() {
    this->indentationCount += IndentationSpaceCount;
    this->currentLine->insert(TimeStampLength + SeverityLength, IndentationSpaceCount, ' ');
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Unindent() {
    assert(
      (this->indentationCount >= IndentationSpaceCount) &&
      u8"Indentation is at least one level deep"
    );
    this->indentationCount -= IndentationSpaceCount;
    this->currentLine->erase(
      TimeStampLength + SeverityLength + this->indentationCount, IndentationSpaceCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool RollingLogger::IsLogging() const {
    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Inform(const std::string &message) {
    updateTimeInLine(*this->currentLine);
    {
      std::string::value_type *currentCharacter = this->currentLine->data();
      currentCharacter[TimeStampLength + 0] = 'I';
      currentCharacter[TimeStampLength + 1] = 'N';
      currentCharacter[TimeStampLength + 2] = 'F';
      currentCharacter[TimeStampLength + 3] = 'O';
      currentCharacter[TimeStampLength + 4] = ' ';
      currentCharacter[TimeStampLength + 5] = ' ';
      currentCharacter[TimeStampLength + 6] = ' ';
      currentCharacter[TimeStampLength + 7] = ' ';
    }

    this->currentLine->append(message);

    advanceLine();
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Warn(const std::string &warning) {
    updateTimeInLine(*this->currentLine);
    {
      std::string::value_type *currentCharacter = this->currentLine->data();
      currentCharacter[TimeStampLength + 0] = 'W';
      currentCharacter[TimeStampLength + 1] = 'A';
      currentCharacter[TimeStampLength + 2] = 'R';
      currentCharacter[TimeStampLength + 3] = 'N';
      currentCharacter[TimeStampLength + 4] = 'I';
      currentCharacter[TimeStampLength + 5] = 'N';
      currentCharacter[TimeStampLength + 6] = 'G';
      currentCharacter[TimeStampLength + 7] = ' ';
    }

    this->currentLine->append(warning);

    advanceLine();
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Complain(const std::string &error) {
    updateTimeInLine(*this->currentLine);
    {
      std::string::value_type *currentCharacter = this->currentLine->data();
      currentCharacter[TimeStampLength + 0] = 'E';
      currentCharacter[TimeStampLength + 1] = 'R';
      currentCharacter[TimeStampLength + 2] = 'R';
      currentCharacter[TimeStampLength + 3] = 'O';
      currentCharacter[TimeStampLength + 4] = 'R';
      currentCharacter[TimeStampLength + 5] = ' ';
      currentCharacter[TimeStampLength + 6] = ' ';
      currentCharacter[TimeStampLength + 7] = ' ';
    }

    this->currentLine->append(error);

    advanceLine();
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Append(const char *buffer, std::size_t count) {
    this->currentLine->append(buffer, count);
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::Clear() {
    assert(
      (this->indentationCount == 0) && u8"Indentation should be zero when calling Clear()"
    );
    this->oldestLineIndex = 0;
    this->nextLineIndex = 0;
    this->currentLine = &this->lines[0];
    this->indentationCount = 0;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RollingLogger::GetLines() const {
    if(this->oldestLineIndex == this->nextLineIndex) {
      return std::vector<std::string>();
    }

    std::vector<std::string> orderedLines;
    if(this->oldestLineIndex < this->nextLineIndex) {
      orderedLines.reserve(this->nextLineIndex - this->oldestLineIndex);
      for(std::size_t index = this->oldestLineIndex; index < this->nextLineIndex; ++index) {
        orderedLines.push_back(this->lines[index]);
      }
    } else {
      {
        std::size_t historyLineCount = this->lines.size();
        orderedLines.reserve(historyLineCount - this->oldestLineIndex + this->nextLineIndex);
        for(std::size_t index = this->oldestLineIndex; index < historyLineCount; ++index) {
          orderedLines.push_back(this->lines[index]);
        }
      }
      for(std::size_t index = 0; index < this->nextLineIndex; ++index) {
        orderedLines.push_back(this->lines[index]);
      }
    }

    return orderedLines;
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::advanceLine() {
    std::size_t historyLineCount = this->lines.size();

    this->nextLineIndex = (this->nextLineIndex + 1) % historyLineCount;
    if(this->nextLineIndex == this->oldestLineIndex) {
      this->oldestLineIndex = (this->oldestLineIndex + 1) % historyLineCount;
    }

    const std::string &previousLine = *this->currentLine;

    this->currentLine = &this->lines[this->nextLineIndex];
    this->currentLine->resize(TimeStampLength + SeverityLength);
    this->currentLine->append(this->indentationCount, ' ');

    // Call this last, if the override messes up and throws,
    // at least our internal state will be intact...
    OnLineAdded(previousLine);
  }

  // ------------------------------------------------------------------------------------------- //

  void RollingLogger::updateTimeInLine(std::string &line) {
    assert((line.length() >= 12) && u8"Line is long enough to hold the current time");

#if defined(NUCLEX_SUPPORT_WINDOWS)

    // Interestingly, Microsoft's GetSystemTime() has no error return.
    ::SYSTEMTIME splitUtcTime;
    ::GetSystemTime(&splitUtcTime);
    {
      std::string::value_type *currentCharacter = line.data();
      currentCharacter[0] = TimestampDigits[splitUtcTime.wHour][0];
      currentCharacter[1] = TimestampDigits[splitUtcTime.wHour][1];
      currentCharacter[2] = ':';
      currentCharacter[3] = TimestampDigits[splitUtcTime.wMinute][0];
      currentCharacter[4] = TimestampDigits[splitUtcTime.wMinute][1];
      currentCharacter[5] = ':';
      currentCharacter[6] = TimestampDigits[splitUtcTime.wSecond][0];
      currentCharacter[7] = TimestampDigits[splitUtcTime.wSecond][1];
      currentCharacter[8] = '.';
      std::size_t count = lexical_append(currentCharacter + 9, 3, splitUtcTime.wMilliseconds);
      if(count == 1) {
        currentCharacter[11] = currentCharacter[9];
        currentCharacter[9] = '0';
        currentCharacter[10] = '0';
      } else if(count == 2) {
        currentCharacter[11] = currentCharacter[10];
        currentCharacter[10] = currentCharacter[9];
        currentCharacter[9] = '0';
      }
      currentCharacter[12] = ' ';
    }

#else // Posix and Linux through Posix

    // Obtain the current wall clock time. This clock /may/ skip or jump backwards if time
    // is synchronized by, for example, an NTP daemon. For logging, this doesn't matter much
    // as the lines are still ordered and the log isn't intended for benchmarking.
    ::timespec time;
    {
      int result = ::clock_gettime(CLOCK_REALTIME, &time);
      if(result != 0) {
        int errorNumber = errno;
        Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not obtain the current wall clock via ::clock_gettime(CLOCK_REALTIME...)",
          errorNumber
        );
      }
    }

    // Turn the 'seconds since the epoch' value into hours, minutes and seconds in UTC
    // According to docs, time.tv_sec should always be ::time_t, but some Posix implementations
    // have a lot of defines going on and 32/64 bit variants, so we're being explicit here.
    ::tm splitUtcTime;
    {
      ::time_t secondsSinceEpoch = static_cast<::time_t>(time.tv_sec);
      ::gmtime_r(&secondsSinceEpoch, &splitUtcTime);
    }

    // Finally, form the time stamp in the log line
    {
      const std::size_t nanosecondsPerMillisecond = 1000000U;

      std::string::value_type *currentCharacter = line.data();
      currentCharacter[0] = TimestampDigits[splitUtcTime.tm_hour][0];
      currentCharacter[1] = TimestampDigits[splitUtcTime.tm_hour][1];
      currentCharacter[2] = ':';
      currentCharacter[3] = TimestampDigits[splitUtcTime.tm_min][0];
      currentCharacter[4] = TimestampDigits[splitUtcTime.tm_min][1];
      currentCharacter[5] = ':';
      currentCharacter[6] = TimestampDigits[splitUtcTime.tm_sec][0];
      currentCharacter[7] = TimestampDigits[splitUtcTime.tm_sec][1];
      currentCharacter[8] = '.';

      std::size_t timeMilliseconds = time.tv_nsec / nanosecondsPerMillisecond;
      std::size_t count = lexical_append(currentCharacter + 9, 3, timeMilliseconds);
      if(count == 1) {
        currentCharacter[11] = currentCharacter[9];
        currentCharacter[9] = '0';
        currentCharacter[10] = '0';
      } else if(count == 2) {
        currentCharacter[11] = currentCharacter[10];
        currentCharacter[10] = currentCharacter[9];
        currentCharacter[9] = '0';
      }
      currentCharacter[12] = ' ';
    }

#endif

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
