#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_TEXT_LOGGER_H
#define NUCLEX_SUPPORT_TEXT_LOGGER_H

#include "Nuclex/Support/Config.h"

#include <string>

// DONE: Rename FeedbackReceiver to Logger?
//   This class is purely for logging and many other logging libraries, such as
//   log4j / log4net, use an ILogger interface in their design.
//
//   But: this is just an interface, it doesn't 'log' per se. And 'LogSink'
//   is shorter but nobody understands Microsoftese (nor do we want it...)

// CHECK: Use simple verbs for the logging methods?
//   LogMessage()  ->  Inform()
//   LogWarning()  ->  Warn()
//   LogError()    ->  ? Complain() ? Panic() ? Bail() ?

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interface to accept diagnostic messages and information</summary>
  class Logger {

    #pragma region class IndentationScope

    /// <summary>Simple scope that adds indentation to a logger while it exists</summary>
    public: class IndentationScope {

      /// <summary>Adds indentation to the specified logger</summary>
      /// <param name="logger">Logger to which an indentation level will be added</param>
      public: IndentationScope(Logger &logger) : logger(logger) {
        this->logger.Indent();
      }

      /// <summary>Goes back up by one indentation level on the logger</summary>
      public: ~IndentationScope() {
        this->logger.Unindent();
      }

      /// <summary>Logger this scope is dealing with</summary>
      private: Logger &logger;

    };

    #pragma endregion // class IndentationScope

    /// <summary>Frees all resources owned by the logger</summary>
    public: NUCLEX_SUPPORT_API virtual ~Logger() = default;

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
    public: NUCLEX_SUPPORT_API virtual void Indent() {}

    /// <summary>Advises the logger to go back up by one level of indentation</summary>
    /// <remarks>
    ///   <para>
    ///     This is the counterpart to the <see cref="Indent" /> method. It needs to be
    ///     called exactly one time for each call to the <see cref="Indent" /> method.
    ///     In order to ensure the logger isn't accumulating indentation levels, use
    ///     the nested <see cref="IndentationScope" /> class provided by the logger interface.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void Unindent() {}

    /// <summary>Whether the logger is actually doing anything with the log messages</summary>
    /// <returns>True if the log messages are processed in any way, false otherwise</returns>
    /// <remarks>
    ///   Forming the log message strings may be non-trivial and cause memory allocations, too,
    ///   so by checking this method just once, you can skip all logging if they would be
    ///   discarded anyway.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual bool IsLogging() const { return false; }

    /// <summary>Logs a diagnostic message</summary>
    /// <param name="message">Message the operation wishes to log</param>
    /// <remarks>
    ///   Use this for diagnostic output that may help with debugging or verifying that
    ///   things are indeed happening the way you intended to. These messages typically
    ///   go into some log, a details window or are discarded outright.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void LogMessage(const std::string &message) {
      (void)message;
    }

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
    public: NUCLEX_SUPPORT_API virtual void LogWarning(const std::string &warning) {
      (void)warning;
    }

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
    public: NUCLEX_SUPPORT_API virtual void LogError(const std::string &error) {
      (void)error;
    }

#if defined(WOULD_BE_NICE_IF_PORTABLE)

    /// <summary>Whether the feedback receiver is checking logged messages at all</summary>
    /// <returns>True if the feedback receiver is checking logged messages, false otherwise</returns>
    /// <remarks>
    ///   If your messages are costly to form, you can check this property once in your
    ///   operation to see whether issuing log messages is needed at all.
    /// </remarks>
    public: inline bool IsLogging() const {
      //FeedbackReceiver *dummy = nullptr;
      //(FeedbackReceiver::void (*baseMethod)(const std::string &)) =
      return (
        (&this->LogMessage != &FeedbackReceiver::LogMessage) ||
        (&this->LogWarning != &FeedbackReceiver::LogWarning) ||
        (&this->LogError != &FeedbackReceiver::LogError)
      );
    }

#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LOGGER_H
