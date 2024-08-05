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

// Short recap on how I pick the pixel formats that go in here:
//
// - Contains only formats that are useful
//   (i.e. popular with graphics APIs or needed to represent contents of image file)
// - Enum jumps by 1024 each time a new channel set or layout begins
//   (i.e. next pixel format has one more channel or is differently ordered)
// - Native16/Native32 formats are present when it's convenient to treat them that way
//   (i.e. if color channels are 16/32 bits or if whole pixel fits in 16/32 bits)
//   Also taking into consideration when a known API exposes such a format.
// - If Native16/Native32 exist, a Flipped16/Flipped32 may exist, too.
//   Either because the Flipped32 variant matches another HW format or for cross-endianness.
// - Big endian support is not a goal currently, but design is careful to not prevent it.
//   (i.e. no Flipped16/Flipped32 for floats that would be needed for big endian,
//   but these can be added; enum is endian-specific, so if an image is loaded on
//   a different endian platform, the opposite pixel format will be seen)
//
// The pixel formats are a combination of flags. I selected these based on the following:
//
// - Extra informations can easily be made available at compile-time by simply specializing
//   the PixelFormatDescription<> template for each pixel format.
// - So the only useful informations to encode in the pixel format enum directly are
//

// More design notes:
//
// Oww, my brain! The name of all formats gives the in-memory order,
// but when the whole 16 bit pixel format is _Native16...
//
// If we'd go with "format name is memory order" and then this won't
// be true for B5_G6_R5.
//
// Maybe we can achieve consistency if the format name is defined to
// state the /observed/ layout in the PixelType?
// And the _Flipped16 variant would require endian flipping anyway...
//
// But then the non-_Native16/_Flipped16 formats (which actually
// specify memory order) would lie on little endian (i.e. all) systems.
//
// Solution
// - non-_Native16/_Flipped16 formats are always byte-aligned
// - _NativeXX (where XX = pixel size) formats state observed order
// - _FlippedXX (where XX = pixel size) formats same, but after endian flip
// - Some formats only have _NativeXX without _FlippedXX, these are not
//   cross-endian-serialization safe, obviously. Perhaps this could be
//   another method for PixelFormatQuery (+ GetClosestEndianSafeFormat())?

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  // Considered macros instead of writing out the shifts, but lines get too long...
  //#define SMALLEST_UNIT(size) (size << 24)
  //#define BITS_PER_PIXEL(count) (count << 16)
  //#define ID(number) (number)

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Color channel sets and their bit layouts used to describe a pixel</summary>
  /// <remarks>
  ///   <para>
  ///     Pixel formats are named after the in-memory ordering of the color channels,
  ///     with _NativeXX / _FlippedXX postfixes to indicate if part of all of the bytes are
  ///     dependent on the native byte order (endianness).
  ///   </para>
  ///   <para>
  ///     For example, R8_G8_B8_A8_Unsigned would always have the bytes in that exact
  ///     order in memory. If you read it into an std::uint32_t on a little endian system,
  ///     the red channel's mask would be 0x000000ff while on a big endian system, it would
  ///     be 0xff000000.
  ///   </para>
  ///   <para>
  ///     If the format was called R8_G8_B8_A8_Unsigned_Native32, the channel order is
  ///     the <em>observed</em> order when stored in the platform's native std::uint32_t,
  ///     thus, the red channel's mask would be 0xff000000 on both endians, with memory
  ///     order being platform dependent. Formats with non-byte-aligned channels only provide
  ///     native byte order constants, i.e. R5_G6_B5_Unsigned_Native16 or
  ///     A2_R10_G10_B10_Unsigned_Native32 because (afaik) there's not crazy hardware that
  ///     demands endian-flipped channels (like G3_B5_R5_G3) as input.
  ///   </para>
  ///   <para>
  ///     All formats with _NativeXX / _FlippedXX postfixes encode their endianness:
  ///   </para>
  ///   <para>
  ///     <code>
  ///       // Library compiled for + running on little endian system
  ///       A8_R8_G8_B8_Unsigned_Native32 == 69261352
  ///       A8_R8_G8_B8_Unsigned_Flipped32 == 69261356
  ///
  ///       // Library compiles for + running on big endian system
  ///       A8_R8_G8_B8_Unsigned_Native32 == 69261356
  ///       A8_R8_G8_B8_Unsigned_Flipped32 == 69261352
  ///     </code>
  ///   </para>
  ///   <para>
  ///     For pure runtime usage, this behavior is of no consequence. But if you numerically
  ///     save the pixel format constant to a file and open that file on an opposite-endian
  ///     system, the opposite-endian system will automatically see it as endian-flipped and
  ///     load it correctly. Some format have only _NativeXX variants and no _FlippedXX
  ///     variant - these formats are not safe for cross-endian serialization and would map
  ///     to an unassigned pixel format when you load them.
  ///   </para>
  ///   <para>
  ///     Here's a complete list of the information the enum values encode in their bits:
  ///   </para>
  ///   <code>
  ///     0sssssss pppppppp ccnnnnnn nnnnnfff
  ///   </code>
  ///   <para>
  ///     The 's' indicates the size of the smallest unit addressable in the pixel format
  ///     in bytes. For a 32 bit RGBA format, this would be 4 (if a write to the texture
  ///     was off by two bytes, R would become B, G would become A and so on). Compressed
  ///     pixel formats may have larger chunks - DXT5 for example would only be addressable
  ///     in units of 128 bits / 16 bytes.
  ///   </para>
  ///   <para>
  ///     The next byte contains the number of bits per pixel, 'p'. It is useful for
  ///     calculating the amount of memory required to hold an image of size x by y.
  ///   </para>
  ///   <para>
  ///     In the final two bytes, 'c' is the number of channels stored for each pixel minus
  ///     one (00 = 1 channel, 01 = 2 channels, 10 = 3 channels, 11 = 4 channels), while
  ///     'n' is a unique id of each pixel format that generally counts up sequentially or
  ///     jumps when a new unique channel layout begins. The last three bits, 'f', try to
  ///     conform to the following meaning where possible: endianness (+4), floatness (+2),
  ///     signedness (+1).
  ///   </para>
  /// </remarks>
  enum class PixelFormat {

    // The list below contains all the supported pixel formats.
    //
    // If your code editor supports folding, just collapse all regions to get
    // a decent list of the supported pixel formats.
    //
    // Visual Studio Code/Codium users can press Ctrl + (K, 0) to collapse all
    // regions and Ctrl + (K, J) to expand all regions.
    //

    // Last bits:
    //  +0 = unsigned, big endian            rare
    //  +1 = signed, big endian              rare
    //  +2 = unsigned float, big endian      <unused>
    //  +3 = float, big endian               <unused>
    //  +4 = unsigned, little endian         common
    //  +5 = signed, little endian           rare
    //  +6 = unsigned float, little endian   <unused>
    //  +7 = float, little endian            common
    //
    // Note to self: yes, endianness for floats exists.
    // But I'm not sure it is used anywhere, even on big endian machines.
    // If it exists, the separation allows us to avoid loading serialized data wrongly.
    //

    //#pragma region Format 1024-1031 | R8 (unsigned)

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
    R8_Unsigned = (1 << 24) | (8 << 16) | (0 << 14) | 1024 | 0,

    //#pragma endregion // Format 1024-1031 | R8 (unsigned)

    //#pragma region Format 1032-1039 | R16 (unsigned, float)

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
    R16_Unsigned_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 1032 | 4,
#else
    R16_Unsigned_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 1032 | 0,
#endif

    // CHECK: Add unsigned Flipped16 formats to exchange above format with BE systems?

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
    R16_Float_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 1032 | 7,
#else
    R16_Float_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 1032 | 3,
#endif

    // CHECK: Add float flipped16 formats to exchange above format with BE systems?

    //#pragma endregion // Format 1032-1039 | R16 (unsigned, float)

    //#pragma region Format 1040-1047 | R32 (float)

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
    R32_Float_Native32 = (4 << 24) | (32 << 16) | (0 << 14) | 1040 | 7,
#else
    R32_Float_Native32 = (4 << 24) | (23 << 16) | (0 << 14) | 1040 | 3,
#endif

    // CHECK: Add float flipped32 formats to exchange above format with BE systems?

    //#pragma endregion // Format 1040-1047 | R32 (float)

    //#pragma region Format 2048-2055 | A8 (unsigned)

    /// <summary>8 bit unsigned opacity stored in the alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is a good format for masks and characters in a font.
    ///   </para>
    ///   <para>
    ///     Memory layout: A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
    A8_Unsigned = (1 << 24) | (8 << 16) | (0 << 14) | 2048 | 0,

    //#pragma endregion // Format 2048-2055 | A8 (unsigned)

    //#pragma region Format 2056-2063 | A16 (unsigned, float)

    /// <summary>16 bit unsigned opacity stored in the alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///     Memory layout BE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_Unsigned_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 2056 | 4,
#else
    A16_Unsigned_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 2056 | 0,
#endif

    // CHECK: Add unsigned Flipped16 formats to exchange above format with BE systems?

    /// <summary>16 bit floating point opacity stored in the alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///     Memory layout BE: A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A16_Float_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 2056 | 7,
#else
    A16_Float_Native16 = (2 << 24) | (16 << 16) | (0 << 14) | 2056 | 3,
#endif

    // CHECK: Add float flipped16 formats to exchange above format with BE systems?

    //#pragma endregion // Format 1032-1039 | R16 (unsigned, float)

    //#pragma region Format 2064-2071 | A32 (float)

    /// <summary>32 bit floating point opacity stored in the alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     This uses the native format, so what ends up in memory depends on
    ///     the platform the library is compiled for.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀  | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈
    ///                       A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆ | A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄
    ///     Memory layout BE: A₃₁A₃₀A₂₉A₂₈A₂₇A₂₆A₂₅A₂₄ | A₂₃A₂₂A₂₁A₂₀A₁₉A₁₈A₁₇A₁₆
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉ A₈  | A₇ A₆ A₅ A₄ A₃ A₂ A₁ A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A32_Float_Native32 = (4 << 24) | (32 << 16) | (0 << 14) | 2064 | 7,
#else
    A32_Float_Native32 = (4 << 24) | (23 << 16) | (0 << 14) | 2064 | 3,
#endif

    // CHECK: Add float flipped32 formats to exchange above format with BE systems?

    //#pragma endregion // Format 2064-2071 | A32 (float)

    //#pragma region Format 3072-3079 | R8_G8 (unsigned)

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
    R8_G8_Unsigned = (2 << 24) | (16 << 16) | (1 << 14) | 3072 | 0,

    //#pragma endregion // Format 3072-3079 | R8_G8 (unsigned)

    //#pragma region Format 3080-3087 | R16_G16 (unsigned, float)

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
    R16_G16_Unsigned_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 3080 | 4,
#else
    R16_G16_Unsigned_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 3080 | 0,
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
    R16_G16_Float_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 3080 | 7,
#else
    R16_G16_Float_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 3080 | 3,
#endif

    //#pragma endregion // Format 3080-3087 | R16_G16 (unsigned, float)

    //#pragma region Format 4096-4103 | R8_A8 (unsigned)

    /// <summary>8 bit unsigned single color with an alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     Sometimes used for masks where just an intensity value isn't enough.
    ///   </para>
    ///   <para>
    ///     Memory layout: R₇R₆R₅R₄R₃R₂R₁R₀ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    ///   <para>
    ///     Graphics APIs usually use an R8_G8 pixel format to store this as the meaning
    ///     of the channels is up to the shader anyway. Here the format is used to
    ///     semantically distinguish between R8G8 as used for normals and R8A8 as found
    ///     in grayscale + alpha PNG files.
    ///   </para>
    /// </remarks>
    R8_A8_Unsigned = (2 << 24) | (16 << 16) | (1 << 14) | 4096 | 0,

    //#pragma endregion // Format 4096-4103 | R8_A8 (unsigned)

    //#pragma region Format 4104-4111 | R16_A16 (unsigned)

    /// <summary>8 bit unsigned single color with an alpha channel</summary>
    /// <remarks>
    ///   <para>
    ///     Sometimes used for masks where just an inzzzaa  tensity value isn't enough.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///                       A₇A₆A₅A₄A₃A₂A₁A₀ | A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈
    ///     Memory layout BE: R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    ///   <para>
    ///     Graphics APIs usually use an R16_G16 pixel format to store this as the meaning
    ///     of the channels is up to the shader anyway. Here the format is used to
    ///     semantically distinguish between R16G16 as used for normals and R16A16 as found
    ///     in grayscale + alpha PNG files.
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_A16_Unsigned_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 4104 | 4,
#else
    R16_A16_Unsigned_Native16 = (4 << 24) | (32 << 16) | (1 << 14) | 4104 | 0,
#endif

    // CHECK: Add unsigned flipped32 formats to exchange above format with BE systems?

    //#pragma endregion // Format 4104-4111 | R16_A16 (unsigned)

    //#pragma region Format 5120-5127 | R5_G6_B5 (unsigned)

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
    R5_G6_B5_Unsigned_Native16 = (2 << 24) | (16 << 16) | (2 << 14) | 5120 | 4,
#else
    R5_G6_B5_Unsigned_Native16 = (2 << 24) | (16 << 16) | (2 << 14) | 5120 | 0,
#endif

    //#pragma endregion // Format 5120-5127 | R5_G6_B5 (unsigned)

    //#pragma region Format 5128-5135 | B5_G6_R5 (unsigned)

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
    B5_G6_R5_Unsigned_Native16 = (2 << 24) | (16 << 16) | (2 << 14) | 5128 | 4,
#else
    B5_G6_R5_Unsigned_Native16 = (2 << 24) | (16 << 16) | (2 << 14) | 5128 | 0,
#endif

    //#pragma endregion // Format 5128-5135 | B5_G6_R5 (unsigned)

    //#pragma region Format 5136-5143 | R8_G8_B8 (unsigned, signed)

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
    R8_G8_B8_Unsigned = (3 << 24) | (24 << 16) | (2 << 14) | 5136 | 0,

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
    R8_G8_B8_Signed = (3 << 24) | (24 << 16) | (2 << 14) | 5136 | 1,

    //#pragma endregion // Format 5136-5143 | R8_G8_B8 (unsigned, signed)

    //#pragma region Format 5144-5151 | B8_G8_R8 (unsigned, signed)

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
    B8_G8_R8_Unsigned = (3 << 24) | (24 << 16) | (2 << 14) | 5144 | 0,

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
    B8_G8_R8_Signed = (3 << 24) | (24 << 16) | (2 << 14) | 5144 | 1,

    //#pragma endregion // Format 5144-5151 | B8_G8_R8 (unsigned, signed)
#if 0
    //#pragma region Format 5152-5159 | R16_G16_B16 (unsigned)

    /// <summary>48 bits total with unsigned red, green and blue channels</summary>
    /// <remarks>
    ///   <para>
    ///     Used for compact storage in some image file formats, i.e. PNG.
    ///   </para>
    ///   <para>
    ///     Memory layout: B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                    G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                    R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///   </para>
    /// </remarks>
    R16_G16_B16_Unsigned = (3 << 24) | (24 << 16) | (2 << 14) | 5152 | 0,

    /// <summary>48 bit color using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 48 bit format with unsigned integer values in native byte order.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ | B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ | G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ | R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈
    ///     Memory layout BE: B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                       G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    R16_G16_B16_Unsigned_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 5152 | 4,
#else
    R16_G16_B16_Unsigned_Native16 = A16_B16_G16_R16_Unsigned,
#endif

    //#pragma endregion // Format 5152-5159 | R16_G16_B16 (unsigned)
#endif
    //#pragma region Format 6144-6151 | A8_B8_G8_R8 / R8_G8_B8_A8 (unsigned)

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
    A8_B8_G8_R8_Unsigned = (4 << 24) | (32 << 16) | (3 << 14) | 6144 | 0,

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
    R8_G8_B8_A8_Unsigned = (4 << 24) | (32 << 16) | (3 << 14) | 6144 | 4,

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
    ///     and GL_RGBA+GL_UNSIGNED_INT_8_8_8_8.
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

    //#pragma endregion // Format 6144-6151 | A8_B8_G8_R8 / R8_G8_B8_A8 (unsigned)

    // ----- Everything below here needs to be checked for API compatibility still -----

    //#pragma region Format 6144-6151 | A8_B8_G8_R8 / R8_G8_B8_A8 (signed)

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
    A8_B8_G8_R8_Signed = (4 << 24) | (32 << 16) | (3 << 14) | 6144 | 1,

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
    R8_G8_B8_A8_Signed = (4 << 24) | (32 << 16) | (3 << 14) | 6144 | 5,

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

    //#pragma endregion // Format 6144-6151 | A8_B8_G8_R8 / R8_G8_B8_A8 (signed)

    //#pragma region Format 6152-6159 | A16_B16_G16_R16 (unsigned, float)

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with static byte order, useful for storage or offline video
    ///     processing at high fidelity to prevent banding.
    ///   </para>
    ///   <para>
    ///     Memory layout A₁₅A₁₄A₁₃A₁₂A₁₁A₁₀A₉A₈ | A₇A₆A₅A₄A₃A₂A₁A₀
    ///                   B₁₅B₁₄B₁₃B₁₂B₁₁B₁₀B₉B₈ | B₇B₆B₅B₄B₃B₂B₁B₀
    ///                   G₁₅G₁₄G₁₃G₁₂G₁₁G₁₀G₉G₈ | G₇G₆G₅G₄G₃G₂G₁G₀
    ///                   R₁₅R₁₄R₁₃R₁₂R₁₁R₁₀R₉R₈ | R₇R₆R₅R₄R₃R₂R₁R₀
    ///   </para>
    /// </remarks>
    A16_B16_G16_R16_Unsigned = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 0,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with unsigned integer values in native byte order.
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
    A16_B16_G16_R16_Unsigned_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 4,
#else
    A16_B16_G16_R16_Unsigned_Native16 = A16_B16_G16_R16_Unsigned,
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
    A16_B16_G16_R16_Unsigned_Flipped16 = A16_B16_G16_R16_Unsigned,
#else
    A16_B16_G16_R16_Unsigned_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 4,
#endif

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
    A16_B16_G16_R16_Float = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 3,

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
    A16_B16_G16_R16_Float_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 7,
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
    A16_B16_G16_R16_Float_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6152 | 7,
#endif

    //#pragma endregion // Format 6152-6159 | A16_B16_G16_R16 (unsigned, float)

    //#pragma region Format 6160-6167 | R16_G16_B16_A16 (unsigned, float)

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
    R16_G16_B16_A16_Unsigned = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 3,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with Unsigneding point values in native byte order,
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
    R16_G16_B16_A16_Unsigned_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 7,
#else
    R16_G16_B16_A16_Unsigned_Native16 = R16_G16_B16_A16_Unsigned,
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
    R16_G16_B16_A16_Unsigned_Flipped16 = R16_G16_B16_A16_Unsigned,
#else
    R16_G16_B16_A16_Unsigned_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 7,
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
    R16_G16_B16_A16_Float = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 3,

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
    R16_G16_B16_A16_Float_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 7,
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
    R16_G16_B16_A16_Float_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6160 | 7,
#endif

    //#pragma endregion // Format 6160-6167 | R16_G16_B16_A16 (unsigned, float)

    // TODO: Insert A16_B16_G16_R16_Signed and R16_G16_B16_A16_Signed

    //#pragma region Format 6168-6175 | A32_B32_G32_R32 (float)

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
    A32_B32_G32_R32_Float = (16 << 24) | (128 << 16) | (3 << 14) | 6168 | 3,

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
    A32_B32_G32_R32_Float_Native32 = (16 << 24) | (64 << 16) | (3 << 14) | 6168 | 7,
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
    A32_B32_G32_R32_Float_Flipped32 = (16 << 24) | (64 << 16) | (3 << 14) | 6168 | 7,
#endif

    //#pragma endregion // Format 6168-6175 | A32_B32_G32_R32 (float)

    //#pragma region Format 6176-6183 | R32_G32_B32_A32 (float)

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
    R32_G32_B32_A32_Float = (16 << 24) | (128 << 16) | (3 << 14) | 6176 | 3,

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
    R32_G32_B32_A32_Float_Native32 = (16 << 24) | (64 << 16) | (3 << 14) | 6176 | 7,
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
    R32_G32_B32_A32_Float_Flipped32 = (16 << 24) | (64 << 16) | (3 << 14) | 6176 | 7,
#endif

    //#pragma endregion // A32_B32_G32_R32 and R32_G32_B32_A32 float formats

    //#pragma region Format 6184-6191 | B8_G8_R8_A8 / A8_R8_G8_B8 (unsigned)

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
    B8_G8_R8_A8_Unsigned = (4 << 24) | (32 << 16) | (3 << 14) | 6184 | 0,

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
    A8_R8_G8_B8_Unsigned = (4 << 24) | (32 << 16) | (3 << 14) | 6184 | 4,

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

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory
    ///     is different depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///     Memory layout BE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_R8_G8_B8_Unsigned_Native32 = B8_G8_R8_A8_Unsigned,
#else
    A8_R8_G8_B8_Unsigned_Native32 = A8_R8_G8_B8_Unsigned,
#endif

    /// <summary>32 bit color with alpha using 8 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     This is in native layout, so what ends up in memory is different
    ///     depending on endianness.
    ///   </para>
    ///   <para>
    ///     Memory layout LE: A₇A₆A₅A₄A₃A₂A₁A₀ R₇R₆R₅R₄R₃R₂R₁R₀
    ///                       G₇G₆G₅G₄G₃G₂G₁G₀ B₇B₆B₅B₄B₃B₂B₁B₀
    ///     Memory layout BE: B₇B₆B₅B₄B₃B₂B₁B₀ G₇G₆G₅G₄G₃G₂G₁G₀
    ///                       R₇R₆R₅R₄R₃R₂R₁R₀ A₇A₆A₅A₄A₃A₂A₁A₀
    ///   </para>
    /// </remarks>
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    A8_R8_G8_B8_Unsigned_Flipped32 = A8_R8_G8_B8_Unsigned,
#else
    A8_R8_G8_B8_Unsigned_Flipped32 = B8_G8_R8_A8_Unsigned,
#endif

    //#pragma endregion // B8_G8_R8_A8 + A8_R8_G8_B8 formats

    //#pragma region Format 6192-6199 | B8_G8_R8_A8 / A8_R8_G8_B8 (signed)

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
    B8_G8_R8_A8_Signed = (4 << 24) | (32 << 16) | (3 << 14) | 6192 | 1,

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
    A8_R8_G8_B8_Signed = (4 << 24) | (32 << 16) | (3 << 14) | 6192 | 5,

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

    //#pragma endregion // Format 6192-6199 | B8_G8_R8_A8 / A8_R8_G8_B8 (signed)

    //#pragma region Format 6200-6207 | B16_G16_R16_A16 (unsigned, float)

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
    B16_G16_R16_A16_Unsigned = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 0,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with unsigned integers in native byte order.
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
    B16_G16_R16_A16_Unsigned_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 4,
#else
    B16_G16_R16_A16_Unsigned_Native16 = B16_G16_R16_A16_Unsigned,
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
    B16_G16_R16_A16_Unsigned_Flipped16 = B16_G16_R16_A16_Unsigned,
#else
    B16_G16_R16_A16_Unsigned_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 4,
#endif

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
    B16_G16_R16_A16_Float = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 3,

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
    B16_G16_R16_A16_Float_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 7,
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
    B16_G16_R16_A16_Float_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6200 | 7,
#endif

    //#pragma endregion // Format 6200-6207 | B16_G16_R16_A16 (unsigned, float)

    //#pragma region Format 6208-6215 | A16_R16_G16_B16 (unsigned, float)

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
    A16_R16_G16_B16_Unsigned = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 0,

    /// <summary>64 bit color with alpha using 16 bits for each channel</summary>
    /// <remarks>
    ///   <para>
    ///     A 64 bit format with unsigned integer values in native byte order.
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
    A16_R16_G16_B16_Unsigned_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 4,
#else
    A16_R16_G16_B16_Unsigned_Native16 = A16_R16_G16_B16_Unsigned,
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
    A16_R16_G16_B16_Unsigned_Flipped16 = A16_R16_G16_B16_Unsigned,
#else
    A16_R16_G16_B16_Unsigned_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 4,
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
    A16_R16_G16_B16_Float = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 3,

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
    A16_R16_G16_B16_Float_Native16 = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 7,
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
    A16_R16_G16_B16_Float_Flipped16 = (8 << 24) | (64 << 16) | (3 << 14) | 6208 | 7,
#endif

    //#pragma endregion // Format 6208-6215 | A16_R16_G16_B16 (unsigned, float)

    // TODO: Insert B16_G16_R16_A16_Signed and A16_R16_G16_B16_Signed

    //#pragma region Format 7168-7175 | A2_B10_G10_R10 (unsigned)

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
    A2_B10_G10_R10_Unsigned_Native32 = (4 << 24) | (32 << 16) | (3 << 14) | 7168 | 4,
#else
    A2_B10_G10_R10_Unsigned_Native32 = (4 << 24) | (32 << 16) | (3 << 14) | 7168 | 0,
#endif

    //#pragma endregion // Format 7168-7175 | A2_B10_G10_R10 (unsigned)

    //#pragma region Format 7176-7183 | A2_R10_G10_B10 (unsigned)

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
    A2_R10_G10_B10_Unsigned_Native32 = (4 << 24) | (32 << 16) | (3 << 14) | 7176 | 4,
#else
    A2_R10_G10_B10_Unsigned_Native32 = (4 << 24) | (32 << 16) | (3 << 14) | 7176 | 0,
#endif

    //#pragma endregion // Format 7176-7183 | A2_R10_G10_B10 (unsigned)

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest bits used per pixel in the specified pixel format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose bits per pixel will be determined</param>
  /// <returns>The bits per pixel in the specified pixel format</returns>
  NUCLEX_PIXELS_API constexpr inline std::size_t CountBitsPerPixel(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) >> 16) & 0xFF;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the smallest number of bits that can be modified in the given format
  /// </summary>
  /// <param name="pixelFormat">Pixel format whose unit size will be determined</param>
  /// <returns>The smallest changeable number of bytes in the specified pixel format</returns>
  NUCLEX_PIXELS_API constexpr inline std::size_t CountBytesPerBlock(PixelFormat pixelFormat) {
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
  NUCLEX_PIXELS_API constexpr inline std::size_t CountRequiredBytes(
    PixelFormat pixelFormat, std::size_t pixelCount
  ) {
    return ((CountBitsPerPixel(pixelFormat) * pixelCount) + 7) / 8; // Always round up
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reports the number of color channels in a pixel format</summary>
  /// <param name="pixelFormat">Pixel format whose color channels to count</param>
  /// <returns>The number of color channels in the pixel format</returns>
  NUCLEX_PIXELS_API constexpr inline std::size_t CountChannels(PixelFormat pixelFormat) {
    return ((static_cast<std::size_t>(pixelFormat) >> 14) & 3) + 1;
  }

  // ------------------------------------------------------------------------------------------- //
#if 0 // In considered embedding this information in enum values, currently it's not there.
  /// <summary>
  ///   Determines the number of color channels stored per pixel
  /// </summary>
  /// <param name="pixelFormat">
  ///   Pixel format for which the channel count will be determined
  /// </param>
  /// <returns>The number of color channels stored for each pixel in the format</returns>
  constexpr inline std::size_t CountChannels(PixelFormat pixelFormat) {
    return (static_cast<std::size_t>(pixelFormat) & 0x00E00000) >> 21;
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if 1 // I'm probably going to drop BCx support as a PixelFormat for the Bitmap class
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
#endif
  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_PIXELFORMAT_H
