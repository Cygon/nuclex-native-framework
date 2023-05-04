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
#define NUCLEX_PIXELS_SOURCE 1

#include "LibTiffHelpers.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBTIFF)

#include "Nuclex/Pixels/Errors/FileFormatError.h"

#include <cassert> // for assert()
#include <algorithm> // for std::min()
#include <type_traits> // for std::make_signed()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Stores all information required to adapt a virtual file to the LibTIFF io methods
  /// </summary>
  struct TiffVirtualFile {

    /// <summary>Whether the const version of the virtual file must be used</summary>
    public: bool IsReadOnly;
    /// <summary>Current position of the file pointer</summary>
    public: std::uint64_t Position;
    /// <summary>Total length of the virtual file</summary>
    public: std::uint64_t Length;

#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
    /// <summary>Buffer holding the whole file *if* mapVirtualFiletoMemory() is called</summary>
    /// <remarks>
    ///   Obviously, we don't want that.
    /// </remarks>
    public: std::uint8_t *Buffer;
#endif // NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING

    public: union {
      /// <summary>File from which the read method is reading data</summary>
      public: const Nuclex::Pixels::Storage::VirtualFile *ReadableFile;
      /// <summary>File to which the write method is writing data</summary>
      public: Nuclex::Pixels::Storage::VirtualFile *WritableFile;
    };

    /// <summary>Exception that happened in one of the LibTIFF file IO callbacks</summary>
    public: std::exception_ptr Exception;
    /// <summary>Error source location reported via the LibTIFF error handler</summary>
    public: std::string ErrorSource;
    /// <summary>Error message reported via the LibTIFF error handler</summary>
    public: std::string ErrorMessage;

  };

  // ------------------------------------------------------------------------------------------- //
#if 0
  void discardExtendedTiffWarning(
    ::thandle_t fileHandle, const char *sourceModule, const char *format, ::va_list arguments
  ) {}
#endif
  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores the error message reported from LibTIFF on internal errors</summary>
  /// </param name="fileHandle">
  ///   File handle on which LibTIFF was working when the error happened
  /// </param>
  /// <param name="sourceModule">Internal LibTIFF method reporting the error</param>
  /// <param name="format">printf()-style format string for the error message</param>
  /// <param name="arguments">
  ///   Variable-length argument list referenced in the format string
  /// </param>
  void handleExtendedTiffError(
    ::thandle_t fileHandle, const char *sourceModule, const char *format, ::va_list arguments
  ) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);

    // Only go through the trouble if the origin of the error was not an exception
    // of our own virtual file (that got recorded while LibTIFF got a normal error return).
    if(!virtualFile.Exception) {
      virtualFile.ErrorSource.assign(sourceModule);

      virtualFile.ErrorMessage.resize(1024);
      int characterCount = ::vsnprintf(
        virtualFile.ErrorMessage.data(), virtualFile.ErrorMessage.size(), format, arguments
      );
      if(characterCount < 0) {
        const static std::string vsnprintfFailedMessage(
          u8"<vsnprintf() failed>, original (unexpanded) error message: ", 59
        );
        // Could do ::explain_vsnprintf() here, but we're way more interested in
        // why LibTIFF failed to do its work.
        virtualFile.ErrorMessage.assign(vsnprintfFailedMessage);
        virtualFile.ErrorMessage.append(format);
      } else {
        virtualFile.ErrorMessage.resize(characterCount);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data from a virtual file</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <param name="buffer">Buffer into which the data will be placed</param>
  /// <param name="byteCount">Number of bytes to attempt to read</param>
  /// <returns>The number of bytes actually read from the file</returns>
  ::tmsize_t readFromVirtualFile(
    ::thandle_t fileHandle, void *buffer, ::tmsize_t byteCount
  ) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    try {
      assert(virtualFile.IsReadOnly && u8"Read is performed on file opened for reading");

      std::size_t maximumBytesReadable;
      {
        std::uint64_t bytesRemaining = virtualFile.Length - virtualFile.Position;
        if(bytesRemaining < static_cast<std::uint64_t>(byteCount)) {
          maximumBytesReadable = static_cast<std::size_t>(bytesRemaining);
        } else {
          maximumBytesReadable = static_cast<std::size_t>(byteCount);
        }
      }
      virtualFile.ReadableFile->ReadAt(
        virtualFile.Position, maximumBytesReadable, reinterpret_cast<std::uint8_t *>(buffer)
      );

      virtualFile.Position += maximumBytesReadable;

      return maximumBytesReadable;
    }
    catch(const std::exception &) {
      virtualFile.Exception = std::current_exception();
      return static_cast<::tmsize_t>(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes data to a virtual file</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <param name="buffer">Buffer holding the data that will be written</param>
  /// <param name="byteCount">Number of bytes that will be written into the file</param>
  /// <returns>The number of bytes actually written into the file</returns>
  ::tmsize_t writeToVirtualFile(
    ::thandle_t fileHandle, void *buffer, ::tmsize_t byteCount
  ) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    try {
      assert(!virtualFile.IsReadOnly && u8"Write is performed on file opened for writing");
      virtualFile.WritableFile->WriteAt(
        virtualFile.Position,
        static_cast<std::size_t>(byteCount),
        reinterpret_cast<const std::uint8_t *>(buffer)
      );

      virtualFile.Position += static_cast<std::size_t>(byteCount);
      virtualFile.Length = std::max(virtualFile.Position, virtualFile.Length);

      return byteCount;
    }
    catch(const std::exception &) {
      virtualFile.Exception = std::current_exception();
      return static_cast<::tmsize_t>(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Changes the position of the virtual file cursor</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <param name="location">
  ///   Location (relative to the anchor) where the file cursor will be placed
  /// </param>
  /// <param name="referencePoint">Anchor relative to which the location will be set</param>
  /// <returns>The new absolute location of the file cursor</returns>
  ::toff_t seekInVirtualFile(::thandle_t fileHandle, ::toff_t location, int referencePoint) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    try {
      switch(referencePoint) {
        case SEEK_SET: {
          virtualFile.Position = std::min(
            static_cast<std::uint64_t>(location), virtualFile.Length
          );
          break;
        }
        case SEEK_CUR: {
          virtualFile.Position = std::min(
            static_cast<std::uint64_t>(virtualFile.Position + location), virtualFile.Length
          );
          break;
        }
        case SEEK_END: {
          // Clamp the location to the size of the virtual file. On some platforms (ARM),
          // LibTIFF's ::toff_t appears to be an unsigned type, so we're doing some gymnastics
          // to ensure correct behavior.
          typedef typename std::make_signed<::toff_t>::type SignedOffsetType;
          SignedOffsetType signedLocation = *reinterpret_cast<SignedOffsetType *>(location);
          if(signedLocation < 0) {
            std::uint64_t reverseLocation = static_cast<std::uint64_t>(-signedLocation);
            if(reverseLocation > virtualFile.Length) {
              virtualFile.Position = 0;
            } else {
              virtualFile.Position = virtualFile.Length - reverseLocation;
            }
          } else {
            virtualFile.Position = virtualFile.Length;
          }
          break;
        }
        default: {
          throw std::logic_error(u8"Invalid reference location passed to seekInVirtualFile()");
        }
      }
 
      return static_cast<::toff_t>(virtualFile.Position);
    }
    catch(const std::exception &) {
      virtualFile.Exception = std::current_exception();
      return static_cast<::toff_t>(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Closes a virtual file</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <returns>0 if the virtual file was closed successfully</returns>
  int closeVirtualFile(::thandle_t fileHandle) {
    (void)fileHandle;
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the size of a virtual file</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <returns>The size of the virtual file</returns>
  ::toff_t getVirtualFileSize(::thandle_t fileHandle) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    return static_cast<::toff_t>(virtualFile.Length);
  }

  // ------------------------------------------------------------------------------------------- //
#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
  /// <summary>Maps the contents of a virtual file into memory</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <param name="contentAddress">
  ///   Address of a pointer that will be set to the location of the virtual file's
  ///   contents in memory
  /// </param>
  /// <param name="size">Address at which the size of the virtual file will be stored</param>
  /// <returns>0 if the virtual file was successfully mapped or loaded into memory</returns>
  int mapVirtualFileToMemory(::thandle_t fileHandle, void **contentAddress, ::toff_t *size) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    try {
      assert((virtualFile.Buffer == nullptr) && u8"File is not already mapped to memory");
      {
        std::unique_ptr<std::uint8_t[]> content(new std::uint8_t[virtualFile.Length]);
        if(virtualFile.IsReadOnly) {
          virtualFile.ReadableFile->ReadAt(0, virtualFile.Length, content.get());
        }
        virtualFile.Buffer = content.release();
      }

      *contentAddress = reinterpret_cast<void *>(virtualFile.Buffer);
      *size = static_cast<::toff_t>(virtualFile.Length);

      return 0;
    }
    catch(const std::exception &error) {
      virtualFile.Exception = std::current_exception();
      return -1;
    }
  }
#endif // NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
  // ------------------------------------------------------------------------------------------- //
#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
  /// <summary>Unmaps a virtual fiel that was mapped into memory</summary>
  /// <param name="fileHandle">User-provided pointer that was passed to LibTIFF</param>
  /// <param name="contentAddress">Memory location the virtual file was mapped to</param>
  /// <param name="size">Reported size of the mapped memory buffer</param>
  void unmapVirtualFileFromMemory(::thandle_t fileHandle, void *contentAddress, ::toff_t size) {
    TiffVirtualFile &virtualFile = *reinterpret_cast<TiffVirtualFile *>(fileHandle);
    try {
      std::uint8_t *content = reinterpret_cast<std::uint8_t *>(contentAddress);
      if(!virtualFile.IsReadOnly) {
        virtualFile.WritableFile->WriteAt(0, static_cast<std::size_t>(size), content);
      }

      virtualFile.Buffer = nullptr;
      delete[] content;
    }
    catch(const std::exception &error) {
      ::TIFFErrorExt(fileHandle, u8"unmapVirtualFileFromMemory()", error.what());
      virtualFile.Exception = std::current_exception();
    }
  }
#endif // NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Tiff {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::IsValidTiffHeader(const std::uint8_t *fileHeader) {
    return (
      (
        (
          (fileHeader[0] == 0x49) && //  1  Byte order mark (little endian)
          (fileHeader[1] == 0x49)    //  1
        ) || (
          (fileHeader[0] == 0x4D) && //  1  Byte order mark (big endian)
          (fileHeader[1] == 0x4D)    //  1
        )
      ) &&
      (
        (
          (fileHeader[0] == 0x49) && //
          (fileHeader[2] == 0x2a) && //  2  Magic number (little endian)
          (fileHeader[3] == 0x00)    //  2
        ) || (
          (fileHeader[0] == 0x4D) &&
          (fileHeader[2] == 0x00) && //  2  Magic number (big endian)
          (fileHeader[3] == 0x2a)    //  2
        )
      ) &&
      (
        (
          (fileHeader[0] == 0x49) && //
          ((fileHeader[7] & 1) == 0) //  3  Offset to first IFD (little endian)
        ) || (                       //     Must be aligned to word
          (fileHeader[0] == 0x4D) && //
          ((fileHeader[4] & 1) == 0) //  3  Offset to first IFD (big endian)
        )                            //     Must be aligned to word
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  ::TIFF *Helpers::OpenForReading(
    const Nuclex::Pixels::Storage::VirtualFile &file, bool headerOnly /* = false */
  ) {
    ::TIFFSetErrorHandler(nullptr);
    ::TIFFSetErrorHandlerExt(&handleExtendedTiffError);
    //::TIFFSetWarningHandlerExt(&discardExtendedTiffWarning);

    // Set up a new TiffVirtualFile wrapper that will work as an adapter from
    // LibTIFF's virtual IO functions to this library's VirtualFile interface
    std::unique_ptr<TiffVirtualFile> wrapper = std::make_unique<TiffVirtualFile>();
    {
      wrapper->IsReadOnly = true;
      wrapper->ReadableFile = &file;
      wrapper->Position = 0;
      wrapper->Length = file.GetSize();
#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
      wrapper->Buffer = nullptr;
#endif
      wrapper->ErrorSource.reserve(64);
      wrapper->ErrorMessage.reserve(1024);
    }

    TIFF *tiffFile = ::TIFFClientOpen(
      u8"<virtual file>",
      headerOnly ? "rmh" : "rm",
      reinterpret_cast<::thandle_t>(wrapper.get()),
      &readFromVirtualFile,
      &writeToVirtualFile,
      &seekInVirtualFile,
      &closeVirtualFile,
      &getVirtualFileSize,
      nullptr, //&mapVirtualFileToMemory,
      nullptr //&unmapVirtualFileFromMemory
    );
    if(tiffFile == nullptr) {
      if(wrapper->Exception) {
        std::rethrow_exception(wrapper->Exception);
      } else if(!wrapper->ErrorMessage.empty()) {
        const static std::string messageStart(
          u8"Error reported by LibTIFF opening .tif file for reading: ", 57
        );
        std::string message;
        message.reserve(57 + wrapper->ErrorMessage.length());
        message.assign(messageStart);
        message.append(wrapper->ErrorMessage);
        throw Errors::FileFormatError(message);
      } else {
        throw Errors::FileFormatError(u8"Could not open .tif file for reading via LibTIFF");
      }
    } else {
      wrapper.release();
    }

    return tiffFile;
  }

  // ------------------------------------------------------------------------------------------- //

  // Set up a new TiffVirtualFile wrapper that will work as an adapter from
  // LibTIFF's virtual IO functions to this library's VirtualFile interface
  ::TIFF *Helpers::OpenForWriting(Nuclex::Pixels::Storage::VirtualFile &file) {
    ::TIFFSetErrorHandler(nullptr);
    ::TIFFSetErrorHandlerExt(&handleExtendedTiffError);
    //::TIFFSetWarningHandlerExt(&discardExtendedTiffWarning);

    std::unique_ptr<TiffVirtualFile> wrapper(new TiffVirtualFile());
    {
      wrapper->IsReadOnly = false;
      wrapper->WritableFile = &file;
      wrapper->Position = 0;
      wrapper->Length = 0; //file.GetSize();
#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
      wrapper->Buffer = nullptr;
#endif
      wrapper->ErrorSource.reserve(64);
      wrapper->ErrorMessage.reserve(1024);
    }
    
    TIFF *tiffFile = ::TIFFClientOpen(
      u8"<virtual file>",
      "wm",
      reinterpret_cast<::thandle_t>(wrapper.get()),
      &writeToVirtualFile,
      &readFromVirtualFile,
      &seekInVirtualFile,
      &closeVirtualFile,
      &getVirtualFileSize,
      nullptr, // &mapVirtualFileToMemory,
      nullptr // &unmapVirtualFileFromMemory
    );
    if(tiffFile == nullptr) {
      if(wrapper->Exception) {
        std::rethrow_exception(wrapper->Exception);
      } else if(!wrapper->ErrorMessage.empty()) {
        const static std::string messageStart(
          u8"Error reported by LibTIFF opening .tif file for writing: ", 57
        );
        std::string message;
        message.reserve(57 + wrapper->ErrorMessage.length());
        message.assign(messageStart);
        message.append(wrapper->ErrorMessage);
        throw Errors::FileFormatError(message);
      } else {
        throw Errors::FileFormatError(u8"Could not open .tif file for writing via LibTIFF");
      }
    } else {
      wrapper.release();
    }

    return tiffFile;
  }

  // ------------------------------------------------------------------------------------------- //

  void Helpers::Close(::TIFF *tiffFile) {
    ::thandle_t clientData = ::TIFFClientdata(tiffFile);
    TiffVirtualFile *virtualFile = reinterpret_cast<TiffVirtualFile *>(clientData);

#if NUCLEX_PIXELS_ENABLE_LIBTIFF_MEMORY_MAPPING
    if((!virtualFile->IsReadOnly) && (virtualFile->Buffer != nullptr)) {
      virtualFile->WritableFile->WriteAt(0, virtualFile->Length, virtualFile->Buffer);
      delete[] virtualFile->Buffer;
    }
#endif

    delete virtualFile;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Tiff

#endif // defined(NUCLEX_PIXELS_HAVE_LIBTIFF)
