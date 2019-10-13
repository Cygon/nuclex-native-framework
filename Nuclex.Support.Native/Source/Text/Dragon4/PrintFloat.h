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

#ifndef RJ__PrintFloat_h
#define RJ__PrintFloat_h

#include "Standard.h"

//******************************************************************************
//******************************************************************************
enum tPrintFloatFormat {
  PrintFloatFormat_Positional,    // [-]ddddd.dddd
  PrintFloatFormat_Scientific,    // [-]d.dddde[sign]ddd
};

//******************************************************************************
// Print a 32-bit floating-point number as a decimal string.
// The output string is always NUL terminated and the string length (not
// including the NUL) is returned.
//******************************************************************************
tU32 PrintFloat32(
  tC8 *               pOutBuffer,     // buffer to output into
  tU32                bufferSize,     // size of pOutBuffer
  tF32                value,          // value to print
  tPrintFloatFormat   format,         // format to print with
  tS32                precision       // If negative, the minimum number of digits to represent a
                                      // unique 32-bit floating point value is output. Otherwise,
                                      // this is the number of digits to print past the decimal point.
);

//******************************************************************************
// Print a 64-bit floating-point number as a decimal string.
// The output string is always NUL terminated and the string length (not
// including the NUL) is returned.
//******************************************************************************
tU32 PrintFloat64(
  tC8 *               pOutBuffer,     // buffer to output into
  tU32                bufferSize,     // size of pOutBuffer
  tF64                value,          // value to print
  tPrintFloatFormat   format,         // format to print with
  tS32                precision       // If negative, the minimum number of digits to represent a
                                      // unique 64-bit floating point value is output. Otherwise,
                                      // this is the number of digits to print past the decimal point.
);

#endif
