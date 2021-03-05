/******************************************************************************
  Copyright (c) 2014 Ryan Juckett
  http://www.ryanjuckett.com/
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.
 
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
 
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
 
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
 
  3. This notice may not be removed or altered from any source
     distribution.
******************************************************************************/

#ifndef RJ__Standard_h
#define RJ__Standard_h

#include <cstddef>
#include <cstdint>
#include <cassert>

// Assertion macro
#define RJ_ASSERT(condition) assert(condition)
 
// Boolean types
typedef bool           tB;
 
// Character types
typedef char           tC8;
 
// Unsigned integer types
typedef std::uint8_t   tU8;
typedef std::uint16_t  tU16;
typedef std::uint32_t  tU32;
typedef std::uint64_t  tU64;
 
// Signed integer types
typedef std::int8_t    tS8;
typedef std::int16_t   tS16;
typedef std::int32_t   tS32;
typedef std::int64_t   tS64;
 
// Floating point types
typedef float          tF32;
typedef double         tF64;

// Size types
typedef std::size_t    tSize;
typedef std::ptrdiff_t tPtrDiff;

#endif
