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

#ifndef NUCLEX_SUPPORT_TEXT_LOGGER_H
#define NUCLEX_SUPPORT_TEXT_LOGGER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Accepts diagnostic messages and information for error reporting</summary>
  /// <remarks>
  ///   <para>
  ///     This interface is provided by Nuclex::Support, i.e. at the foundational layer
  ///     of the framework in order to allow components at all levels to support logging.
  ///   </para>
  ///   <para>
  ///     Writing logs files can serve various purposes, from monitoring the actions of
  ///     a server-based application to aiding debugging after applications have been
  ///     rolled out to production and error diagnosis cannot connect a debugger anymore.
  ///   </para>
  ///   <para>
  ///     Here's an example showing basic usage of the logger interface:
  ///   </para>
  ///   <example>
  ///     <code>
  ///       void example(Logger &logger) {
  ///         logger.Inform(u8"Performing action XY");
  ///         {
  ///           Logger::IndentationScope xyLoggerScope(logger);
  ///
  ///           logger.Inform(u8"Now doing this");
  ///           logger.Inform(u8"Now doing that");
  ///         }
  ///
  ///         int i = rand();
  ///         if(i < 42) {
  ///           logger.Warn("The random number generator has a bad day");
  ///         }
  ///       }
  ///     </code>
  ///   </example>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE Logger {

    /// <summary>A logger that doesn't log anything</summary>
    public: NUCLEX_SUPPORT_API static Logger &Null;

    #pragma region class IndentationScope

    /// <summary>Simple scope that adds indentation to a logger while it exists</summary>
    public: class IndentationScope {

      /// <summary>Adds indentation to the specified logger</summary>
      /// <param name="logger">Logger to which an indentation level will be added</param>
      public: NUCLEX_SUPPORT_API IndentationScope(Logger &logger) : logger(logger) {
        this->logger.Indent();
      }

      /// <summary>Goes back up by one indentation level on the logger</summary>
      public: NUCLEX_SUPPORT_API ~IndentationScope() {
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
    public: NUCLEX_SUPPORT_API virtual bool IsLogging() const { return true; }

    /// <summary>Logs a diagnostic message</summary>
    /// <param name="message">Message the operation wishes to log</param>
    /// <remarks>
    ///   Use this for diagnostic output that may help with debugging or verifying that
    ///   things are indeed happening the way you intended to. These messages typically
    ///   go into some log, a details window or are discarded outright.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void Inform(const std::string &message) {
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
    public: NUCLEX_SUPPORT_API virtual void Warn(const std::string &warning) {
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
    public: NUCLEX_SUPPORT_API virtual void Complain(const std::string &error) {
      (void)error;
    }

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LOGGER_H
