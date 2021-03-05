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

#ifndef RJ__Math_h
#define RJ__Math_h

#include "Standard.h"

//******************************************************************************
// Get the log base 2 of a 32-bit unsigned integer.
// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
//******************************************************************************
tU32 LogBase2(tU32 val);

//******************************************************************************
// Get the log base 2 of a 64-bit unsigned integer.
//******************************************************************************
inline tU32 LogBase2(tU64 val) {
  tU64 temp;

  temp = val >> 32;
  if(temp) {
    return 32 + LogBase2((tU32)temp);
  }

  return LogBase2((tU32)val);
}

#endif
