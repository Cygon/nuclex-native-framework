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
  #define NUCLEX_PIXELS_WINDOWS 1
#else
  #define NUCLEX_PIXELS_LINUX 1
#endif

// --------------------------------------------------------------------------------------------- //

// Compiler support checking
#if defined(_MSC_VER)
  #if (_MSC_VER < 1910) // Visual Studio 2017 has the C++17 features we use
    #error At least Visual Studio 2017 is required to compile Nuclex.Pixels.Native
  #elif defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
    #define NUCLEX_PIXELS_CXX17 1
  #endif
#elif defined(__clang__) && defined(__clang_major__)
  #if (__clang_major__ < 5) // clang 5.0 has the C++17 features we use
    #error At least clang 5.0 is required to compile Nuclex.Pixels.Native
  #elif defined(__cplusplus) && (__cplusplus >= 201703L)
    #define NUCLEX_PIXELS_CXX17 1
  #endif
#elif defined(__GNUC__)
  #if (__GNUC__ < 8) // GCC 8.0 has the C++17 features we use
    #error At least GCC 8.0 is required to compile Nuclex.Pixels.Native
  #elif defined(__cplusplus) && (__cplusplus >= 201703L)
    #define NUCLEX_PIXELS_CXX17 1
  #endif
#else
  #error Unknown compiler. Nuclex.Pixels.Native is tested with GCC, clang and MSVC only
#endif

// This library uses std::optional, 'if constexpr' and new C++17 containers,
// so anything earlier than C++ 17 will only result in compilation errors.
#if !defined(NUCLEX_PIXELS_CXX17)
  #error The Nuclex.Pixels.Native library must be compiled in at least C++17 mode
#endif

// We've got tons of u8"hello" strings that will become char8_t in C++20 and fail to build!
// Bail out instead of letting the user scratch their head over weird compiler errors.
#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202002)
  #error The Nuclex.Pixels.Native library does not work in C++20 mode yet
#elif defined(__cplusplus) && (__cplusplus >= 202002)
  #error The Nuclex.Pixels.Native library does not work in C++20 mode yet
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

// Enable built-in 128 bit integer type if present
#if defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  #if defined(__SIZEOF_INT128__) // Extra check, since GCC __int128 exists only on amd64
    #define NUCLEX_PIXELS_HAVE_BUILTIN_INT128 1
  #endif
#endif

// --------------------------------------------------------------------------------------------- //

// Strong suggestion to the compiler to inline something
#if defined(_MSC_VER)
  #define NUCLEX_PIXELS_ALWAYS_INLINE __forceinline
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  #define NUCLEX_PIXELS_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
  #define NUCLEX_PIXELS_ALWAYS_INLINE inline
#endif

// --------------------------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (Nuclex.Pixels.Native library). The NUCLEX_PIXELS_SOURCE symbol is defined by
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
  // MSVC doesn't have to import/export the whole type, it includes the vtable
  // and RTTI stuff automatically when at least one member is exported. Exporting
  // the whole type would only cause useless things to be exported as well.
  #define NUCLEX_PIXELS_TYPE

#elif defined(__GNUC__) || defined(__clang__)

  #if defined(NUCLEX_PIXELS_STATICLIB) || defined(NUCLEX_PIXELS_EXECUTABLE)
    #define NUCLEX_PIXELS_API
    #define NUCLEX_PIXELS_TYPE
  #else
    // If you use -fvisibility=hidden in GCC, exception handling and RTTI would break
    // if visibility wasn't set during export _and_ import because GCC would immediately
    // forget all type infos encountered. See http://gcc.gnu.org/wiki/Visibility
    #define NUCLEX_PIXELS_API __attribute__ ((visibility ("default")))
    #define NUCLEX_PIXELS_TYPE __attribute__ ((visibility ("default")))
  #endif

#else

  #error Unknown compiler, please implement shared library macros for your system

#endif

// --------------------------------------------------------------------------------------------- //

// Optimization macros
#if defined(__GNUC__) || defined(__clang__)

  #if !defined(likely)
    #define likely(x) __builtin_expect((x), 1)
  #endif
  #if !defined(unlikely)
    #define unlikely(x) __builtin_expect((x), 0)
  #endif

#else

  #if !defined(likely)
    #define likely(x) (x)
  #endif
  #if !defined(unlikely)
    #define unlikely(x) (x)
  #endif

#endif

// --------------------------------------------------------------------------------------------- //

// Silences unused variable warning, but only in release builds
// (NDEBUG is also what decides whether the assert() macro does anything)
#if defined(NDEBUG)
  #define NUCLEX_PIXELS_NDEBUG_UNUSED(x) (void)x
#else
  #define NUCLEX_PIXELS_NDEBUG_UNUSED(x) {}
#endif

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_PIXELS_CONFIG_H
