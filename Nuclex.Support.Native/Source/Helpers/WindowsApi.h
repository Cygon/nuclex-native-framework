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

#ifndef NUCLEX_SUPPORT_HELPERS_WINDOWSAPI_H
#define NUCLEX_SUPPORT_HELPERS_WINDOWSAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WIN32)

// The Windows headers tend to include a ton of crap and pollute the global namespace
// like nothing else. These macros cut down on that a bit.
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

// These symbols are redefined globally when you include the Windows header. Needless
// to say that we don't want that, because it would export our Container::CreateFile()
// method as Container::CreateFileW() internally and cause linker errors for the caller
// who doesn't include Windows.h and calls the method as Container::CreateFile().
#undef CreateFile
#undef DeleteFile
#undef MoveFile
#undef CreateDirectory
#undef RemoveDirectory
#undef GetFileAttributes
#undef GetFileAttributesEx
#undef FindFirstFile
#undef FindNextFile
#undef GetTempPath
#undef GetModuleFileName
#undef GetFullPathName

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Offers generic methods for dealing with the Windows API</summary>
  class WindowsApi {

    /// <summary>Returns the error message for the specified error number</summary>
    /// <param name="errorNumber">
    ///   Error number for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error number</param>
    /// <remarks>
    ///   Some posix methods can also be found in the Windows API, usually with
    ///   non-standard underscore prefixes. For these methods, Microsoft's reimplementation
    ///   of strerror(), named _wcserror_s(), needs to be used with the error number
    ///   found in the 'errno' variable (like on Posix systems). This method handles
    ///   calling _wcserror_s() to obtain a meaningful error message for 'errno'.
    /// </remarks>
    public: static std::string GetErrorMessage(int errorNumber);

    /// <summary>Returns the error message for the specified error code</summary>
    /// <param name="errorCode">
    ///   Error code for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error code</param>
    /// <remarks>
    ///   Standard Windows API methods that only exist on Microsoft systems usually
    ///   signal error/success with their return code. The actual error type can be
    ///   looked up by calling GetLastError(). This method fetches a meaningful error
    ///   message for the error code returned by GetLastError().
    /// </remarks>
    public: static std::string GetErrorMessage(DWORD errorCode);

    /// <summary>Returns the error message for the specified HRESULT</summary>
    /// <param name="resultHandle">
    ///   HRESULT for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified HRESULT</param>
    /// <remarks>
    ///   COM (a cross-language ABI that defines vtable layout, calling convention,
    ///   error handling etc.) uses HRESULTs for all method returns. A HRESULT is
    ///   a combination of flags, the most significant bit indicates error/success
    ///   (so all negative HRESULTS are error codes). This method fetches a meaningful
    ///   error message for the HRESULT returned by a COM method.
    /// </remarks>
    public: static std::string GetErrorMessage(HRESULT resultHandle);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorCode">
    ///   Value that GetLastError() returned at the time of failure
    /// </param>
    public: static void ThrowExceptionForSystemError(
      const std::string &errorMessage, DWORD errorCode
    );

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="resultHandle">HRESULT that was returned by the failed function</param>
    public: static void ThrowExceptionForHResult(
      const std::string &errorMessage, HRESULT resultHandle
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Helpers

#endif // defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_HELPERS_WINDOWSAPI_H
