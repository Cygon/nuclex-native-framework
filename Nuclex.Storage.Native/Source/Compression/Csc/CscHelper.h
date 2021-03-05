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

#ifndef NUCLEX_STORAGE_COMPRESSION_CSC_CSCHELPER_H
#define NUCLEX_STORAGE_COMPRESSION_CSC_CSCHELPER_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

// CSC includes, through 7Zip's types.h, the terrible Windows header,
// so at least try to prevent it from messing up our whole environment...
#include "../../Helpers/WindowsApi.h"

#include <csc_memio.h>
#include <csc_encoder_main.h>
#include <csc_dec.h>

#include "../../Helpers/WriteBuffer.h"

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom allocator for libcsc to allow some memory reuse</summary>
  /// <remarks>
  ///   The CSC encoder allocates memory every time it is called to obtain a buffer
  ///   into which the input data is read. This class tries to reuse at least the memory
  ///   for that one buffer and passes all other allocations on to C++ new/delete.
  /// </remarks>
  class ReusingAllocator : public ::ISzAlloc {

    /// <summary>Initializes a new memory reusing allocator</summary>
    public: ReusingAllocator() {
      this->Alloc = &alloc;
      this->Free = &free;
    }

    /// <summary>Enables or disables zero-initialization of memory</summary>
    /// <param name="enable">Whether zero-initialization will be enabled</param>
    /// <remarks>
    ///   Zero-initialization in non-standard for allocation functions and should not be
    ///   necessary, but CSC somehow *relies* on malloc() doing that... and it does.
    ///   Apparently, new[] doesn't, so unless we want a pointless performance hit,
    ///   we have to get sketchy and only enable it for the CSC initialization phase O_o
    /// </remarks>
    public: void EnableZeroInitialization(bool enable = true) {
      if(enable) {
        this->Alloc = &allocAndZero;
      } else {
        this->Alloc = &alloc;
      }
    }
    
    /// <summary>Allocates memory for libcsc</summary>
    /// <param name="self">Equivalent to the this pointer (ISzAlloc is a C-style struct)</param>
    /// <param name="size">Number of bytes that need to be allocated</param>
    /// <returns>The starting address of the allocated memory block</returns>
    private: static void *alloc(void *self, size_t size) {
      (void)self; // Until the /reusing/ part of ReusingAllocator is actually implemented
      return new std::uint8_t[size];
    }

    /// <summary>Allocates memory for libcsc</summary>
    /// <param name="self">Equivalent to the this pointer (ISzAlloc is a C-style struct)</param>
    /// <param name="size">Number of bytes that need to be allocated</param>
    /// <returns>The starting address of the allocated memory block</returns>
    private: static void *allocAndZero(void *self, size_t size) {
      (void)self; // Until the /reusing/ part of ReusingAllocator is actually implemented
      std::uint8_t *memory = new std::uint8_t[size];
      std::fill_n(memory, size, std::uint8_t(0));
      return memory;
    }

    /// <summary>Frees a memory block allocated with alloc()</summary>
    /// <param name="self">Equivalent to the this pointer (ISzAlloc is a C-style struct)</param>
    /// <param name="address">Starting address of the memory block that will be freed</param>
    private: static void free(void *self, void *address) {
      (void)self; // Until the /reusing/ part of ReusingAllocator is actually implemented
      delete[] reinterpret_cast<std::uint8_t *>(address);
    }

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a thrown integer into the appropriate exception class</summary>
  /// <param name="thrownInteger">Integer error code that will be converted</param>
  void throwErrorForThrownInteger(int thrownInteger);

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)

#endif // NUCLEX_STORAGE_COMPRESSION_CSC_CSCHELPER_H
