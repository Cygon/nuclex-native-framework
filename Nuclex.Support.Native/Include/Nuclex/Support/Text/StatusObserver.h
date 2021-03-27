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

#ifndef NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H
#define NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interface that accepts feedback from a long-running task</summary>
  /// <remarks>
  ///   <para>
  ///     If you implement a method or entire class that performs a long-running task,
  ///     you can allow an observer under this standard interface to be assigned in order
  ///     to report your long-running task's progress.
  ///   </para>
  ///   <para>
  ///     The purpose is to entertain the user (or at least show them that something is
  ///     happening in the background) via simple feedback (a progress value going from
  ///     0.0 to 1.0 and an updateable status message). This can then be displayed in
  ///     a progress panel, dialog or written into a console window.
  ///   </para>
  ///   <para>
  ///     Having this standard interface in Nuclex::Support lets you support this kind of
  ///     feedback even in code that should have no UI dependency. Localization of these
  ///     messages is recommended (as they're not internal technical information the way
  ///     log files or exception/error messages are, but user-displayable).
  ///   </para>
  /// </remarks>
  class StatusObserver {

    /// <summary>Frees all resources owned by the feedback receiver</summary>
    public: NUCLEX_SUPPORT_API virtual ~StatusObserver() = default;

    /// <summary>Updates the current progress of the operation</summary>
    /// <param name="progress">Achieved rogress in a range of 0.0 .. 1.0</param>
    /// <remarks>
    ///   Progress should stay within the specified range. Ideally, progress should never
    ///   go backwards, but that may be better than just freezing progress if your operation
    ///   encounters a major unexpected roadblock.
    /// </remarks>
    public: virtual void SetProgress(float progress) = 0;

    /// <summary>Updates the major operation status</summary>
    /// <param name="status">Status message that will be reported to the observer</param>
    /// <remarks>
    ///   This is typically the text you'd want displayed in an applications status bar
    ///   or in a progress window. It shouldn't be too technical or change at a fast pace.
    ///   Console applications can print the string reported through this method, so also
    ///   avoid calling it repeatedly if the text hasn't changed.
    /// </remarks>
    public: virtual void SetStatus(const std::string &status) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H
