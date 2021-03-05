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

#ifndef NUCLEX_PIXELS_CONFIG_H
#define NUCLEX_PIXELS_CONFIG_H

// --------------------------------------------------------------------------------------------- //

// Whether pixel format conversions should ensure they're float-equivalent
// Otherwise an error in the least significant bit is accepted (greatly increasing performance)
#define NUCLEX_PIXELS_CONVERT_EXACT 1

// --------------------------------------------------------------------------------------------- //

// Platform recognition
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  #error The Nuclex.Pixels.Native library does not support WinRT
#elif defined(WIN32) || defined(_WIN32)
  #define NUCLEX_PIXELS_WIN32 1
#else
  #define NUCLEX_PIXELS_LINUX 1
#endif

// --------------------------------------------------------------------------------------------- //

// C++ language features
#if defined(_MSC_VER) && (_MSC_VER >= 1900) // Visual Studio 2015 has the C++14 features we use
  #define NUCLEX_PIXELS_CXX14 1
#elif defined(__clang__) && defined(__cplusplus) && (__cplusplus >= 201402)
  #define NUCLEX_PIXELS_CXX14 1
#elif (defined(__GNUC__) || defined(__GNUG__)) && defined(__cplusplus) && (__cplusplus >= 201402)
  #define NUCLEX_PIXELS_CXX14 1
#else
  #error The Nuclex.Pixels.Native library requires a C++14 compiler
#endif

// --------------------------------------------------------------------------------------------- //

// Endianness detection
#if defined(_MSC_VER) // MSVC is always little endian, including Windows on ARM
  #define NUCLEX_PIXELS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) // GCC
  #define NUCLEX_PIXELS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) // GCC
  #define NUCLEX_PIXELS_BIG_ENDIAN 1
#else
  #error Could not determine whether platform is big or little endian
#endif

// --------------------------------------------------------------------------------------------- //

//#if defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
//  #define NUCLEX_PIXELS_HAVE_BUILTIN_INT128
//#endif

// --------------------------------------------------------------------------------------------- //

// Strong suggestion to the compiler to inline something
#if defined(_MSC_VER)
  #define NUCLEX_PIXELS_ALWAYS_INLINE __forceinline
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  #define NUCLEX_PIXELS_ALWAYS_INLINE __attribute__((always_inline))
#else
  #define NUCLEX_PIXELS_ALWAYS_INLINE
#endif

// --------------------------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (Nuclex.Storage.Native library). The NUCLEX_PIXELS_SOURCE symbol is defined by
// all source files of the library, so you don't have to worry about a thing.
#if defined(_MSC_VER)

  #if defined(NUCLEX_PIXELS_STATICLIB) || defined(NUCLEX_PIXELS_EXECUTABLE)
    #define NUCLEX_PIXELS_API
  #else
    #if defined(NUCLEX_PIXELS_SOURCE)
      // If we are building the DLL, export the symbols tagged like this
      #define NUCLEX_PIXELS_API __declspec(dllexport)
    #else
      // If we are consuming the DLL, import the symbols tagged like this
      #define NUCLEX_PIXELS_API __declspec(dllimport)
    #endif
  #endif

#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)

  #if defined(NUCLEX_PIXELS_STATICLIB) || defined(NUCLEX_PIXELS_EXECUTABLE)
    #define NUCLEX_PIXELS_API
  #else
    #if defined(NUCLEX_PIXELS_SOURCE)
      #define NUCLEX_PIXELS_API __attribute__ ((visibility ("default")))
    #else
      // If you use -fvisibility=hidden in GCC, exception handling and RTTI would break 
      // if visibility wasn't set during export _and_ import because GCC would immediately
      // forget all type infos encountered. See http://gcc.gnu.org/wiki/Visibility
      #define NUCLEX_PIXELS_API __attribute__ ((visibility ("default")))
    #endif
  #endif

#else

  #error Unknown compiler, please implement shared library macros for your system

#endif

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_PIXELS_CONFIG_H
