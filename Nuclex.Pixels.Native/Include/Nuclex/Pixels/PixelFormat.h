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

#ifndef NUCLEX_PIXELS_PIXELFORMAT_H
#define NUCLEX_PIXELS_PIXELFORMAT_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/Size.h"

#include <cstddef>

// Further information on pixel formats offered by popular APIs
//
// A. Frantzis Pixel Format Guide:
// https://afrantzis.com/pixel-format-guide/
//
// SDL pixel format enumeration:
// https://wiki.libsdl.org/SDL_PixelFormatEnum
//
// OpenGL mandatory accelerated pixel formats, ARB pixel format defines:
// https://www.khronos.org/opengl/wiki/Image_Format
// https://www.opengl.org/registry/api/GL/glcorearb.h
//
// DirectX pixel format enumeration:
// https://docs.microsoft.com/en-us/uwp/api/Windows.Graphics.DirectX.DirectXPixelFormat
// https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
//
// Vulkan pixel formats:
// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkFormat
// https://github.com/KhronosGroup/Vulkan-Headers/blob/master/include/vulkan/vulkan_core.h#L635

// Supported pixel formats in GPUs of 2012 (safe baseline)
//
// GeForce GTX 760 Vulkan Formats
// https://vulkan.gpuinfo.org/displayreport.php?id=5165#formats
// https://vulkan.gpuinfo.org/displayreport.php?id=6604#formats
//
// Radeon HD 7900 Vulkan Formats:
// https://vulkan.gpuinfo.org/displayreport.php?id=5126#formats
// https://vulkan.gpuinfo.org/displayreport.php?id=5133#formats

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  //#define SMALLEST_UNIT(size) (size << 24)
  //#define BITS_PER_PIXEL(count) (count << 16)
  //#define ID(number) (number)

  /// <summary>Data formats that can be used to describe a pixel</summary>
  /// <remarks>
  ///   <para>
  ///     All pixel formats specify the in-memory ordering of the color channels (anything
  ///     else would be insane, considering non-byte-sized color channels in R5-G6-B5 and
  ///     A2-R10-G10-B10). Thus, R8-G8-B8-A8 is the same in memory, no matter if big endian
  ///     or little endian.
  ///   </para>
  ///   <para>
  ///     The exception to this are pixel formats ending in _NativeNN and _FlippedNN.
  ///     These consist of native types sized 'NN' bytes, so R16_Native16 would be
  ///     a little endian integer on Intel platforms, for example. Some popular GPU
  ///     pixel formats are in fact little endian. Native indicates the current platform's
  ///     byte order, flipped the opposite one.
  ///   </para>
  ///   <para>
  ///     The enum values are defined as follows:
  ///   </para>
  ///   <code>
  ///     0sssssss pppppppp nnnnnnnn nnnnnnnn
  ///   </code>
  ///   <para>
  ///     Where 's' indicates the size of the smallest unit addressable in the pixel format
  ///     in bytes. For a 32 bit RGBA format, this would be 4 (if a write to the texture
  ///     was off by two bytes, R would become B, G would become A and so on). Compressed
  ///     pixel formats may have larger chunks - DXT5 for example would only be addressable
  ///     in units of 128 bits / 16 bytes.
  ///   </para>
  ///   <para>
  ///     The next byte, 'p' contains the number of bits per pixel. It is useful for
  ///     calculating the amount of memory required to hold an image of size x by y.
  ///   </para>
  ///   <para>
  ///     In the remaining two bytes, tagged as 'n', a unique id of each pixel format
  ///     will be stored.
  ///   </para>
  /// </remarks>
  enum class PixelFormat {

    // Last bits, not strictly enforced:
    //  +0 = unsigned
    //  +1 = signed
    //  +2 = (unsigned float -- not used)
    //  +3 = float
    //  +4 = unsigned, little endian
    //  +5 = signed, little endian
    //  +6 = (unsigned float, little endian -- not used)
    //  +7 = float, little endian
    // Note to self: yes, endianness for floats exists.

    /// <summary>8 bit unsigned single color stored in the red channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is a good format for look-up-tables for gradients, ramps and such.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible to VK_FORMAT_R8_UNORM, GL_RED+GL_UNSIGNED_BYTE, GL_R8 and
    ///     DXGI_FORMAT_R8_UNORM. 
    ///   </para>
    /// </remarks>
    R8_Unsigned = (1 << 24) | (8 << 16) | 0 | 0,

    /// <summary>16 bit unsigned single color stored in the red channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R16_UNORM, GL_RED+GL_UNSIGNED_SHORT and
    ///     GL_R16. Probably compatible with DXGI_FORMAT_R16_UNORM.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_Unsigned_Native16 = (2 << 24) | (16 << 16) | 8 | 4,
#else
    R16_Unsigned_Native16 = (2 << 24) | (16 << 16) | 8 | 0,
#endif

    /// <summary>16 bit floating point single color stored in the red channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R16_SFLOAT, GL_RED+GL_HALF_FLOAT and
    ///     GL_R16F. Probably compatible with DXGI_FORMAT_R16_FLOAT.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_Float_Native16 = (2 << 24) | (16 << 16) | 8 | 7,
#else
    R16_Float_Native16 = (2 << 24) | (16 << 16) | 8 | 3,
#endif

    /// <summary>32 bit floating point single color stored in the red channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀  | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈
    ///                       R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆ | R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄
    ///     Memory layout BE: R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R32_SFLOAT, GL_RED+GL_FLOAT and
    ///     GL_R32F. Probably compatible with DXGI_FORMAT_R32_FLOAT.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R32_Float_Native32 = (4 << 24) | (32 << 16) | 16 | 7,
#else
    R32_Float_Native32 = (4 << 24) | (23 << 16) | 16 | 3,
#endif

    /// <summary>16 bits total with unsigned red and green channels</summary>
    /// <remarks>
    ///   <para>
    ///     Popular with normal maps where the Z component is derived from
    ///     X and Y. If you read these pixels as 16 bit integers, they need
    ///     to be flipped on little-endian systems.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///   </para>
    ///   <para>
    ///     Compatible to VK_FORMAT_R8G8_UNORM, GL_RG+GL_UNSIGNED_BYTE,
    ///     GL_RG8 and DXGI_FORMAT_R8G8_UNORM.
    ///   </para>
    /// </remarks>
    R8_G8_Unsigned = (2 << 24) | (16 << 16) | 1024 | 0,

    /// <summary>32 bits total with unsigned red and green channels</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///   </para>
    ///   <para>
    ///     Compatible to VK_FORMAT_R16G16_UNORM, GL_RG+GL_UNSIGNED_SHORT and
    ///     GL_RG16. Probably compatible with DXGI_FORMAT_R16G16_UNORM.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_G16_Unsigned_Native16 = (4 << 24) | (32 << 16) | 1032 | 4,
#else
    R16_G16_Unsigned_Native16 = (4 << 24) | (32 << 16) | 1032 | 0,
#endif

    /// <summary>32 bits total with floating point red and green channels</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///   </para>
    ///   <para>
    ///     Compatible to VK_FORMAT_R16G16_SFLOAT, GL_RG+GL_HALF_FLOAT and
    ///     GL_RG16F. Probably compatible with DXGI_FORMAT_R16G16_FLOAT.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_G16_Float_Native16 = (4 << 24) | (32 << 16) | 1032 | 7,
#else
    R16_G16_Float_Native16 = (4 << 24) | (32 << 16) | 1032 | 3,
#endif

    #pragma region R5_G6_B5 and B5_G6_R5 formats

    /// <summary>16 bit in memory order with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving RGB format.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₄R₃R₂R₁R₀G₅G₄G₃ | G₂G₁G₀B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
    R5_G6_B5_Unsigned = (2 << 24) | (16 << 16) | 2048 | 0,

    /// <summary>16 bit in native endianness with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving RGB format. This uses the native format, so what ends
    ///     up in memory depends on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: G₂G₁G₀B₄B₃B₂B₁B₀ | R₄R₃R₂R₁R₀G₅G₄G₃
    ///     Memory layout BE: R₄R₃R₂R₁R₀G₅G₄G₃ | G₂G₁G₀B₄B₃B₂B₁B₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R5G6B5_UNORM_PACK16, GL_RGB565,
    ///     GL_RGB+GL_UNSIGNED_SHORT_5_6_5 and SDL_PIXELFORMAT_RGB565.
    ///     Probably compatible with DXGI_FORMAT_B5G6R5_UNORM.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R5_G6_B5_Unsigned_Native16 = (2 << 24) | (16 << 16) | 2048 | 4,
#else
    R5_G6_B5_Unsigned_Native16 = R5_G6_B5_Unsigned,
#endif

    /// <summary>16 bit in reversed native endianness with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving RGB format. This uses the native format, so what ends
    ///     up in memory depends on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₄R₃R₂R₁R₀G₅G₄G₃ | G₂G₁G₀B₄B₃B₂B₁B₀
    ///     Memory layout BE: G₂G₁G₀B₄B₃B₂B₁B₀ | R₄R₃R₂R₁R₀G₅G₄G₃
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R5_G6_B5_Unsigned_Flipped16 = R5_G6_B5_Unsigned,
#else
    R5_G6_B5_Unsigned_Flipped16 = (2 << 24) | (16 << 16) | 2048 | 4,
#endif

    /// <summary>16 bit in memory order with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving BGR format.
    ///   </para>
    ///   <para>
    ///     Memory layout: B₄B₃B₂B₁B₀G₅G₄G₃ | G₂G₁G₀R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
    B5_G6_R5_Unsigned = (2 << 24) | (16 << 16) | 2056 | 0,

    /// <summary>16 bit in native endianness with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving BGR format. This uses the native format, so what ends
    ///     up in memory depends on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: G₂G₁G₀R₄R₃R₂R₁R₀ | B₄B₃B₂B₁B₀G₅G₄G₃
    ///     Memory layout BE: B₄B₃B₂B₁B₀G₅G₄G₃ | G₂G₁G₀R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_B5G6R5_UNORM_PACK16, SDL_PIXELFORMAT_BGR565
    ///     and GL_RGB+GL_UNSIGNED_SHORT_5_6_5_REV.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B5_G6_R5_Unsigned_Native16 = (2 << 24) | (16 << 16) | 2056 | 4,
#else
    B5_G6_R5_Unsigned_Native16 = B5_G6_R5_Unsigned,
#endif

    /// <summary>16 bit in reversed native endianness with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving BGR format. This uses the native format, so what ends
    ///     up in memory depends on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₄B₃B₂B₁B₀G₅G₄G₃ | G₂G₁G₀R₄R₃R₂R₁R₀
    ///     Memory layout BE: G₂G₁G₀R₄R₃R₂R₁R₀ | B₄B₃B₂B₁B₀G₅G₄G₃
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B5_G6_R5_Unsigned_Flipped16 = B5_G6_R5_Unsigned,
#else
    B5_G6_R5_Unsigned_Flipped16 = (2 << 24) | (16 << 16) | 2056 | 4,
#endif

    #pragma endregion // R5_G6_B5 and B5_G6_R5 formats

    #pragma region R8_G8_B8 and B8_G8_R8 formats

    /// <summary>24 bits total with unsigned red, green and blue channels</summary>
    /// <remarks>
    ///   <para>
    ///     Common for image storage due to minimal space but not popular with
    ///     GPUs due to poor memory alignment multiplier.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R8G8B8_UNORM, SDL_PIXELFORMAT_RGB24,
    ///     GL_RGB+GL_UNSIGNED_BYTE and GL_RGB8.
    ///   </para>
    /// </remarks>
    R8_G8_B8_Unsigned = (3 << 24) | (24 << 16) | 3072 | 0,

    /// <summary>16 bits total with signed red, green and blue channels</summary>
    /// <summary>24 bits total with unsigned red, green and blue channels</summary>
    /// <remarks>
    ///   <para>
    ///     Common for object-space normal map storage due to minimal space but
    ///     not popular with GPUs due to poor memory alignment multiplier.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R8G8B8_SNORM and GL_RGB+GL_BYTE
    ///   </para>
    /// </remarks>
    R8_G8_B8_Signed = (3 << 24) | (24 << 16) | 3072 | 1,

    /// <summary>24 bits total with unsigned blue, green and red channels</summary>
    /// <remarks>
    ///   <para>
    ///     Not popular with GPUs due to poor memory alignment multiplier.
    ///   </para>
    ///   <para>
    ///     Memory layout: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_B8G8R8_UNORM, SDL_PIXELFORMAT_BGR24 and
    ///     GL_BGR+GL_UNSIGNED_BYTE
    ///   </para>
    /// </remarks>
    B8_G8_R8_Unsigned = (3 << 24) | (24 << 16) | 3080 | 0,

    /// <summary>24 bits total with signed blue, green and red channels</summary>
    /// <remarks>
    ///   <para>
    ///     Not popular with GPUs due to poor memory alignment multiplier.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_B8G8R8_SNORM and GL_RGB+GL_BYTE
    ///   </para>
    /// </remarks>
    B8_G8_R8_Signed = (3 << 24) | (24 << 16) | 3080 | 1,

    #pragma endregion // R8_G8_B8 and B8_G8_R8 formats

    #pragma region A8_B8_G8_R8 + R8_G8_B8_A8 formats

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with SDL_PIXELFORMAT_ABGR32
    ///   </para>
    /// </remarks>
    A8_B8_G8_R8_Unsigned = (4 << 24) | (32 << 16) | 4096 | 0,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage, but also
    ///     the most popular general-purpose format supported by virtually any
    ///     GPU you can get.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                    B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_R8G8B8A8_UNORM, SDL_PIXELFORMAT_RGBA32,
    ///     GL_RGBA+GL_UNSIGNED_BYTE, GL_RGBA8 and DXGI_FORMAT_R8G8B8A8_UNORM.
    ///   </para>
    /// </remarks>
    R8_G8_B8_A8_Unsigned = (4 << 24) | (32 << 16) | 4096 | 4,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory
    ///     is different depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_A8B8G8R8_UNORM_PACK32, SDL_PIXELFORMAT_ABGR8888
    ///     and GL_RGBA+GL_UNSIGNED_INT_8_8_8_8
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_B8_G8_R8_Unsigned_Native32 = R8_G8_B8_A8_Unsigned,
#else
    A8_B8_G8_R8_Unsigned_Native32 = A8_B8_G8_R8_Unsigned,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    ///   <para>
    ///     Compatible with SDL_PIXELFORMAT_ABGR8888 and
    ///     GL_RGBA+GL_UNSIGNED_INT_8_8_8_8_REV.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_B8_G8_R8_Unsigned_Flipped32 = A8_B8_G8_R8_Unsigned,
#else
    A8_B8_G8_R8_Unsigned_Flipped32 = R8_G8_B8_A8_Unsigned,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R8_G8_B8_A8_Unsigned_Native32 = A8_B8_G8_R8_Unsigned,
#else
    R8_G8_B8_A8_Unsigned_Native32 = R8_G8_B8_A8_Unsigned,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R8_G8_B8_A8_Unsigned_Flipped32 = R8_G8_B8_A8_Unsigned,
#else
    R8_G8_B8_A8_Unsigned_Flipped32 = A8_B8_G8_R8_Unsigned,
#endif

    #pragma endregion // A8_B8_G8_R8 + R8_G8_B8_A8 formats

    // ----- Everything below here needs to be checked for API compatibility still -----

    #pragma region A8_B8_G8_R8 + R8_G8_B8_A8 signed formats

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
    A8_B8_G8_R8_Signed = (4 << 24) | (32 << 16) | 4096 | 1,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                    B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    ///   <para>
    ///     Compatible to DXGI_FORMAT_R8G8B8A8_SNORM
    ///   </para>
    /// </remarks>
    R8_G8_B8_A8_Signed = (4 << 24) | (32 << 16) | 4096 | 5,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_A8B8G8R8_SNORM_PACK32 and GL_RGBA+GL_BYTE.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_B8_G8_R8_Signed_Native32 = R8_G8_B8_A8_Signed,
#else
    A8_B8_G8_R8_Signed_Native32 = A8_B8_G8_R8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_B8_G8_R8_Signed_Flipped32 = A8_B8_G8_R8_Signed,
#else
    A8_B8_G8_R8_Signed_Flipped32 = R8_G8_B8_A8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R8_G8_B8_A8_Signed_Native32 = A8_B8_G8_R8_Signed,
#else
    R8_G8_B8_A8_Signed_Native32 = R8_G8_B8_A8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R8_G8_B8_A8_Signed_Flipped32 = R8_G8_B8_A8_Signed,
#else
    R8_G8_B8_A8_Signed_Flipped32 = A8_B8_G8_R8_Signed,
#endif

    #pragma endregion // A8_B8_G8_R8 and R8_G8_B8_A8 signed formats

    #pragma region A16_B16_G16_R16 + R16_G16_B16_A16 float formats

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
    A16_B16_G16_R16_Float = (8 << 24) | (64 << 16) | 4104 | 3,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///     Memory layout BE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_B16_G16_R16_Float_Native16 = (8 << 24) | (64 << 16) | 4104 | 7,
#else
    A16_B16_G16_R16_Float_Native16 = A16_B16_G16_R16_Float,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_B16_G16_R16_Float_Flipped16 = A16_B16_G16_R16_Float,
#else
    A16_B16_G16_R16_Float_Flipped16 = (8 << 24) | (64 << 16) | 4104 | 7,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
    R16_G16_B16_A16_Float = (8 << 24) | (64 << 16) | 4112 | 3,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ 
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ 
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ 
    ///                       A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ 
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_G16_B16_A16_Float_Native16 = (8 << 24) | (64 << 16) | 4112 | 7,
#else
    R16_G16_B16_A16_Float_Native16 = R16_G16_B16_A16_Float,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ 
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ 
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ 
    ///                       A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ 
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_G16_B16_A16_Float_Flipped16 = R16_G16_B16_A16_Float,
#else
    R16_G16_B16_A16_Float_Flipped16 = (8 << 24) | (64 << 16) | 4112 | 7,
#endif

    #pragma endregion // A16_B16_G16_R16 and R16_G16_B16_A16 float formats

    #pragma region A32_B32_G32_R32 + R32_G32_B32_A32 float formats

    /// <summary>128 bit color with alpha using 32 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 128 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                   A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///                   B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                   G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                   R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///   </para>
    /// </remarks>
    A32_B32_G32_R32_Float = (16 << 24) | (128 << 16) | 4120 | 3,

    /// <summary>128 bit color with alpha using 32 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 128 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀  | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈
    ///                       A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆ | A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄
    ///                       B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀  | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈
    ///                       B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆ | B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄
    ///                       G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀  | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈
    ///                       G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆ | G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄
    ///                       R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀  | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈
    ///                       R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆ | R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄
    ///     Memory layout BE: A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///                       B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                       G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                       R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A32_B32_G32_R32_Float_Native32 = (16 << 24) | (64 << 16) | 4120 | 7,
#else
    A32_B32_G32_R32_Float_Native32 = A32_B32_G32_R32_Float,
#endif

    /// <summary>128 bit color with alpha using 32 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///                       B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                       G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                       R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///     Memory layout BE: A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀  | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈
    ///                       A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆ | A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄
    ///                       B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀  | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈
    ///                       B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆ | B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄
    ///                       G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀  | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈
    ///                       G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆ | G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄
    ///                       R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀  | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈
    ///                       R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆ | R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A32_B32_G32_R32_Float_Flipped32 = A32_B32_G32_R32_Float,
#else
    A32_B32_G32_R32_Float_Flipped32 = (16 << 24) | (64 << 16) | 4120 | 7,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///                   G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                   B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                   A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                   A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///   </para>
    /// </remarks>
    R32_G32_B32_A32_Float = (16 << 24) | (128 << 16) | 4128 | 3,

    /// <summary>128 bit color with alpha using 32 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 128 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀  | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈
    ///                       R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆ | R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄
    ///                       G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀  | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈
    ///                       G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆ | G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄
    ///                       B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀  | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈
    ///                       B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆ | B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄
    ///                       A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀  | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈
    ///                       A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆ | A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄
    ///     Memory layout BE: R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///                       G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                       B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                       A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R32_G32_B32_A32_Float_Native32 = (16 << 24) | (64 << 16) | 4128 | 7,
#else
    R32_G32_B32_A32_Float_Native32 = R32_G32_B32_A32_Float,
#endif

    /// <summary>128 bit color with alpha using 32 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄ | R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈  | R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀
    ///                       G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄ | G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈  | G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀
    ///                       B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄ | B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈  | B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀
    ///                       A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///     Memory layout BE: R₇ R₆ R₅ R₄ R₃ R₂ R₁ R₀  | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉ R₈
    ///                       R₂₃R₂₂R₂₁R₂₀R₁₉R₁₈R₁₇R₁₆ | R₃₁R₃₀R₂₉R₂₈R₂₇R₂₆R₂₅R₂₄
    ///                       G₇ G₆ G₅ G₄ G₃ G₂ G₁ G₀  | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉ G₈
    ///                       G₂₃G₂₂G₂₁G₂₀G₁₉G₁₈G₁₇G₁₆ | G₃₁G₃₀G₂₉G₂₈G₂₇G₂₆G₂₅G₂₄
    ///                       B₇ B₆ B₅ B₄ B₃ B₂ B₁ B₀  | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉ B₈
    ///                       B₂₃B₂₂B₂₁B₂₀B₁₉B₁₈B₁₇B₁₆ | B₃₁B₃₀B₂₉B₂₈B₂₇B₂₆B₂₅B₂₄
    ///                       A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀  | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈
    ///                       A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆ | A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R32_G32_B32_A32_Float_Flipped32 = R32_G32_B32_A32_Float,
#else
    R32_G32_B32_A32_Float_Flipped32 = (16 << 24) | (64 << 16) | 4128 | 7,
#endif

    #pragma endregion // A32_B32_G32_R32 and R32_G32_B32_A32 float formats

    #pragma region B8_G8_R8_A8 + A8_R8_G8_B8 formats

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
    B8_G8_R8_A8_Unsigned = (4 << 24) | (32 << 16) | 5120 | 0,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                    G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀ 
    ///   </para>
    /// </remarks>
    A8_R8_G8_B8_Unsigned = (4 << 24) | (32 << 16) | 5120 | 4,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory
    ///     is different depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B8_G8_R8_A8_Unsigned_Native32 = A8_R8_G8_B8_Unsigned,
#else
    B8_G8_R8_A8_Unsigned_Native32 = B8_G8_R8_A8_Unsigned,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B8_G8_R8_A8_Unsigned_Flipped32 = B8_G8_R8_A8_Unsigned,
#else
    B8_G8_R8_A8_Unsigned_Flipped32 = A8_R8_G8_B8_Unsigned,
#endif

    #pragma endregion // B8_G8_R8_A8 + A8_R8_G8_B8 formats

    #pragma region B8_G8_R8_A8 + A8_R8_G8_B8 signed formats

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
    B8_G8_R8_A8_Signed = (4 << 24) | (32 << 16) | 5128 | 1,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 32 bit format with static byte order, useful for storage.
    ///     Signed pixel formats are symmetric, so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                    G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
    A8_R8_G8_B8_Signed = (4 << 24) | (32 << 16) | 5128 | 5,

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B8_G8_R8_A8_Signed_Native32 = A8_R8_G8_B8_Signed,
#else
    B8_G8_R8_A8_Signed_Native32 = B8_G8_R8_A8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B8_G8_R8_A8_Signed_Flipped32 = B8_G8_R8_A8_Signed,
#else
    B8_G8_R8_A8_Signed_Flipped32 = A8_R8_G8_B8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_R8_G8_B8_Signed_Native32 = B8_G8_R8_A8_Signed,
#else
    A8_R8_G8_B8_Signed_Native32 = A8_R8_G8_B8_Signed,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness. Signed pixel formats are symmetric,
    ///     so -127 and -128 are both -1.0.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_R8_G8_B8_Signed_Flipped32 = A8_R8_G8_B8_Signed,
#else
    A8_R8_G8_B8_Signed_Flipped32 = B8_G8_R8_A8_Signed,
#endif

    #pragma endregion // B8_G8_R8_A8 and A8_R8_G8_B8 signed formats

    #pragma region B16_G16_R16_A16 + A16_R16_G16_B16 float formats

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                   A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
    B16_G16_R16_A16_Float = (8 << 24) | (64 << 16) | 8192 | 3,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///     Memory layout BE: B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B16_G16_R16_A16_Float_Native16 = (8 << 24) | (64 << 16) | 8192 | 7,
#else
    B16_G16_R16_A16_Float_Native16 = B16_G16_R16_A16_Float,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    B16_G16_R16_A16_Float_Flipped16 = B16_G16_R16_A16_Float,
#else
    B16_G16_R16_A16_Float_Flipped16 = (8 << 24) | (64 << 16) | 8192 | 7,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with static byte order, useful for storage.
    ///   </para>
    ///   <para>
    ///     Memory layout A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
    A16_R16_G16_B16_Float = (8 << 24) | (64 << 16) | 8200 | 3,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with floating point values in native byte order,
    ///     widely what GPUs and 3D APIs expect.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///     Memory layout BE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_R16_G16_B16_Float_Native16 = (8 << 24) | (64 << 16) | 8200 | 7,
#else
    A16_R16_G16_B16_Float_Native16 = A16_R16_G16_B16_Float,
#endif

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_R16_G16_B16_Float_Flipped16 = A16_R16_G16_B16_Float,
#else
    A16_R16_G16_B16_Float_Flipped16 = (8 << 24) | (64 << 16) | 8200 | 7,
#endif

    #pragma endregion // B16_G16_R16_A16 and A16_R16_G16_B16 float formats

    #pragma region A2_R10_G10_B10 and A2_B10_G10_R10 formats

    /// <summary>32 bit in memory order with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving RGB format.
    ///   </para>
    ///   <para>
    ///     Memory layout: A₁A₀R₉R₈R₇R₆R₅R₄ | R₃R₂R₁R₀G₉G₈G₇G₆
    ///                    G₅G₄G₃G₂G₁G₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
    A2_R10_G10_B10_Unsigned = (4 << 24) | (32 << 16) | 6144 | 0,

    /// <summary>32 bit in native endianness with three colors as 10 bit integers<summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ | G₅G₄G₃G₂G₁G₀B₉B₈
    ///                       R₃R₂R₁R₀G₉G₈G₇G₆ | A₁A₀R₉R₈R₇R₆R₅R₄
    ///     Memory layout BE: A₁A₀R₉R₈R₇R₆R₅R₄ | R₃R₂R₁R₀G₉G₈G₇G₆
    ///                       G₅G₄G₃G₂G₁G₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    ///     GL_BGRA+GL_UNSIGNED_INT_2_10_10_10_REV and SDL_PIXELFORMAT_ARGB2101010.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A2_R10_G10_B10_Unsigned_Native16 = (4 << 24) | (32 << 16) | 6144 | 4,
#else
    A2_R10_G10_B10_Unsigned_Native16 = A2_R10_G10_B10_Unsigned,
#endif

    /// <summary>32 bit in native endianness with three colors as 10 bit integers<summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₁A₀R₉R₈R₇R₆R₅R₄ | R₃R₂R₁R₀G₉G₈G₇G₆
    ///                       G₅G₄G₃G₂G₁G₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ | G₅G₄G₃G₂G₁G₀B₉B₈
    ///                       R₃R₂R₁R₀G₉G₈G₇G₆ | A₁A₀R₉R₈R₇R₆R₅R₄
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A2_R10_G10_B10_Unsigned_Flipped16 = A2_R10_G10_B10_Unsigned,
#else
    A2_R10_G10_B10_Unsigned_Flipped16 = (4 << 24) | (32 << 16) | 6144 | 4,
#endif

    /// <summary>32 bit in memory order with three colors<summary>
    /// <remarks>
    ///   <para>
    ///     Space-saving RGB format.
    ///   </para>
    ///   <para>
    ///     Memory layout: A₁A₀B₉B₈B₇B₆B₅B₄ | B₃B₂B₁B₀G₉G₈G₇G₆
    ///                    G₅G₄G₃G₂G₁G₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
    A2_B10_G10_R10_Unsigned = (4 << 24) | (32 << 16) | 6152 | 0,

    /// <summary>32 bit in native endianness with three colors as 10 bit integers<summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | G₅G₄G₃G₂G₁G₀R₉R₈
    ///                       B₃B₂B₁B₀G₉G₈G₇G₆ | A₁A₀B₉B₈B₇B₆B₅B₄
    ///     Memory layout BE: A₁A₀B₉B₈B₇B₆B₅B₄ | B₃B₂B₁B₀G₉G₈G₇G₆
    ///                       G₅G₄G₃G₂G₁G₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    ///   <para>
    ///     Compatible with VK_FORMAT_A2B10G10R10_UNORM_PACK32.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A2_B10_G10_R10_Unsigned_Native16 = (4 << 24) | (32 << 16) | 6152 | 4,
#else
    A2_B10_G10_R10_Unsigned_Native16 = A2_B10_G10_R10_Unsigned,
#endif

    /// <summary>32 bit in native endianness with three colors as 10 bit integers<summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends
    ///     on the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₁A₀B₉B₈B₇B₆B₅B₄ | B₃B₂B₁B₀G₉G₈G₇G₆
    ///                       G₅G₄G₃G₂G₁G₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///     Memory layout BE: R₇R₆R₅R₄R₃R₂R₁R₀ | G₅G₄G₃G₂G₁G₀R₉R₈
    ///                       B₃B₂B₁B₀G₉G₈G₇G₆ | A₁A₀B₉B₈B₇B₆B₅B₄
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A2_B10_G10_R10_Unsigned_Flipped16 = A2_B10_G10_R10_Unsigned,
#else
    A2_B10_G10_R10_Unsigned_Flipped16 = (4 << 24) | (32 << 16) | 6152 | 4,
#endif

    #pragma endregion // A2_R10_G10_B10 and A2_B10_G10_R10 formats

  };

  //#define SMALLEST_UNIT(size) (size << 24)
  //#define BITS_PER_PIXEL(count) (count << 16)
  //#define ID(number) (number)

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest bits used per pixel in the specified pixel format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose bits per pixel will be determined</param>
  /// <returns>The bits per pixel in the specified pixel format</returns>
  constexpr inline std::size_t CountBitsPerPixel(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) >> 16) & 0xFF;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest number of bits that can be modified in the given format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose unit size will be determined</param>
  /// <returns>The smallest changeable number of bytes in the specified pixel format</returns>
  constexpr inline std::size_t CountBytesPerBlock(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) >> 24);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the number of bytes required to store the specified number of pixels
  /// </summary>
  /// <param name="pixelFormat">Pixel format for which the size will be determined</param>
  /// <param name="pixelCount">
  ///   Number of pixels for which the memory required will be calculated
  /// </param>
  /// <returns>The size of a single pixel in the specified pixel format</returns>
  constexpr inline std::size_t CountRequiredBytes(
    PixelFormat pixelFormat, std::size_t pixelCount
  ) {
    return ((CountBitsPerPixel(pixelFormat) * pixelCount) + 7) / 8; // Always round up
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the size of the smallest interdepenent pixel block</summary>
  /// <param name="pixelFormat">
  ///   Pixel format whose smallest interdependent block size will be returned
  /// </param>
  /// <returns>The size fo the smallest interdependent block in the pixel format</returns>
  inline NUCLEX_PIXELS_API Size GetBlockSize(PixelFormat pixelFormat) {
    switch(pixelFormat) {
#if 0
      case OldPixelFormat::BC1_Compressed:
      case OldPixelFormat::BC2_Compressed:
      case OldPixelFormat::BC3_Compressed:
      case OldPixelFormat::BC4_Compressed:
      case OldPixelFormat::BC5_Compressed:
      case OldPixelFormat::BC6_Compressed:
      case OldPixelFormat::BC7_Compressed: {
        return Size(4, 4);
      }
#endif
      case PixelFormat::R8_Unsigned: { // Shut up compiler warnings until BCx reintroduced
        return Size(1, 1);
      }
      default: {
        return Size(1, 1);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_PIXELFORMAT_H
