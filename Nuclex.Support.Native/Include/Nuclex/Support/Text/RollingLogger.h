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

#ifndef NUCLEX_SUPPORT_TEXT_ROLLINGLOGGER_H
#define NUCLEX_SUPPORT_TEXT_ROLLINGLOGGER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/Logger.h"
#include "Nuclex/Support/Text/LexicalAppend.h"

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Logger that buffers lines cheaply in memory until they're needed</summary>
  /// <remarks>
  ///   <para>
  ///     This implementation of a logger is intended to be light on CPU time and constantly
  ///     collect log output in a circular buffer in the background. It will not cause
  ///     hard drive accesses and even avoids memory allocations if you avoid external string
  ///     formatting (i.e. you rely on Append() instead of std::format() or lexical_cast)
  ///     and keep you log lines below 100 characters.
  ///   </para>     
  ///   <para>
  ///     When an error happens, you can use the GetLines() method of the rolling logger to
  ///     obtain the last 1024 lines from the log's history and display this as technical
  ///     error information, save it to an error report file or upload it in a reporting tool.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       logger.Inform(u8"Saving configuration {");
  ///       {
  ///         Logger::IndentationScope configurationLogScope(logger);
  ///
  ///         logger.Append(u8"ResolutionX = ");
  ///         logger.Append(1920);
  ///         logger.Inform(std::string());
  ///
  ///         logger.Append(u8"ResolutionY = ");
  ///         logger.Append(1080);
  ///         logger.Inform(std::string());
  ///       }
  ///       logger.Inform(u8"}");
  ///     </code>
  ///   </example>
  /// </remarks>
  class RollingLogger : public Logger {

    /// <summary>initialized a new rolling logger</summary>
    /// <param name="historyLineCount">Number of lines the logger will keep</param>
    /// <param name="lineSizeHint">Length the logger expects most lines to stay under</param>
    public: NUCLEX_SUPPORT_API RollingLogger(
      std::size_t historyLineCount = 1024U, std::size_t lineSizeHint = 100U
    );

    /// <summary>Frees all resources owned by the logger</summary>
    public: NUCLEX_SUPPORT_API virtual ~RollingLogger() = default;

    /// <summary>Advises the logger that all successive output should be indented</summary>
    /// <remarks>
    ///   <para>
    ///     This method is provided because logging often involves printing status in
    ///     multiple lines. Having an official method to indent output in the basic logger
    ///     interface helps keep output readable.
    ///   </para>
    ///   <para>
    ///     This method can be called any number of times and will apply increasing
    ///     indentation to all log output performed. It needs to be followed by an equal
    ///     number of calls to the Unindent() method eventually.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Indent() override;

    /// <summary>Advises the logger to go back up by one level of indentation</summary>
    /// <remarks>
    ///   <para>
    ///     This is the counterpart to the <see cref="Indent" /> method. It needs to be
    ///     called exactly one time for each call to the <see cref="Indent" /> method.
    ///     In order to ensure the logger isn't accumulating indentation levels, use
    ///     the nested <see cref="IndentationScope" /> class provided by the logger interface.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Unindent() override;

    /// <summary>Whether the logger is actually doing anything with the log messages</summary>
    /// <returns>True if the log messages are processed in any way, false otherwise</returns>
    /// <remarks>
    ///   Forming the log message strings may be non-trivial and cause memory allocations, too,
    ///   so by checking this method just once, you can skip all logging if they would be
    ///   discarded anyway.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool IsLogging() const override;

    /// <summary>Logs a diagnostic message</summary>
    /// <param name="message">Message the operation wishes to log</param>
    /// <remarks>
    ///   Use this for diagnostic output that may help with debugging or verifying that
    ///   things are indeed happening the way you intended to. These messages typically
    ///   go into some log, a details window or are discarded outright.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Inform(const std::string &message) override;

    /// <summary>Logs a warning</summary>
    /// <param name="warning">Warning the operation wishes to log</param>
    /// <remarks>
    ///   <para>
    ///     Use this if your operation encounters a problem that isn't fatal but means
    ///     that the outcome will not be as intended. Also use if your operation discovers
    ///     something that isn't the way it should be (i.e. a filename doesn't follow
    ///     conventions, data uses deprecated format, etc.)
    ///   </para>
    ///   <para>
    ///     Logged warnings may be displayed to the user, for example as a summary after
    ///     the operation completed with warnings.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Warn(const std::string &warning) override;

    /// <summary>Logs an error</summary>
    /// <param name="error">Error the operation wishes to log</param>
    /// <remarks>
    ///   <para>
    ///     Only use this if the operation is bound to fail. An exception should be thrown
    ///     from the operation as a result.
    ///   </para>
    ///   <para>
    ///     The error logger may provide additional information beyond the exception
    ///     message and may be displayed to the user, for example in an error dialog after
    ///     the operation has failed.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Complain(const std::string &error) override;

    /// <summary>Appends something to the log line currently being formed</summary>
    /// <param name="value">
    ///   Value that will be appended to the line-in-progress as text.
    ///   Must be a primitive type or std::string
    /// </param>
    /// <remarks>
    ///   This method appends the specified value to the logger's internal line buffer.
    ///   When you call <see cref="Inform" />, <see cref="Warn" /> or <see cref="Complain" />,
    ///   the line will be closed and appear in the log history.
    /// </remarks>
    public: template<typename TValue> inline void Append(const TValue &value) {
      lexical_append(*this->currentLine, value);
    }

    /// <summary>Appends text from a buffer to the line currently being formed</summary>
    /// <param name="buffer">Buffer holding the characters that will be appended</param>
    /// <param name="count">Number of bytes to append from the buffer</param>
    public: NUCLEX_SUPPORT_API void Append(const char *buffer, std::size_t count);

    /// <summary>Removes all history and clears the line currently being formed</summary>
    public: NUCLEX_SUPPORT_API void Clear();

    /// <summary>Returns a vector holding all lines currently in the log history</summary>
    /// <returns>A vector of all lines in the log history</returns>
    /// <remarks>
    ///   <para>
    ///     The rolling logger is designed as a logger you can feed all the time at a low
    ///     performance price (achieved by efficient append operations and by not writing
    ///     anything to a file).
    ///   </para>
    ///   <para>
    ///     If and when an error happens, you can log it and then use this method to obtain
    ///     the recent log history. This will let you save the error details themselves as
    ///     well as the actions leading up to it when needed.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::vector<std::string> GetLines() const;

    /// <summary>Advances to the next line</summary>
    private: void advanceLine();

    /// <summary>Updates the time stamp stored in the line with the specified index</summary>
    /// <param name="lineIndex">Index of the line in which the time stamp will be stored</param>
    /// <remarks>
    ///   Assumes that the line is long enough have the time stamp written into it.
    /// </remarks>
    private: static void updateTimeInLine(std::string &line);

    /// <summary>Index of the line that is currently being formed</summary>
    private: std::size_t nextLineIndex;
    /// <summary>Index of the oldest line in the ring buffer</summary>
    private: std::size_t oldestLineIndex;
    /// <summary>Ring buffer holding the log history as strings that get reused</summary>
    private: std::vector<std::string> lines;
    /// <summary>String from the lines array with index nextLineIndex</summary>
    private: std::string *currentLine;
    /// <summary>Number of spaces the current line is indented by</summary>
    private: std::size_t indentationCount;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_ROLLINGLOGGER_H
