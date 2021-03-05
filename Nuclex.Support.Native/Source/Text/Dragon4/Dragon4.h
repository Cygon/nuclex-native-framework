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

#ifndef RJ__Dragon4_h
#define RJ__Dragon4_h

#include "Standard.h"

//******************************************************************************
// Different modes for terminating digit output
//******************************************************************************
enum tCutoffMode {
  CutoffMode_Unique,          // as many digits as necessary to print a uniquely identifiable number
  CutoffMode_TotalLength,     // up to cutoffNumber significant digits
  CutoffMode_FractionLength,  // up to cutoffNumber significant digits past the decimal point
};

//******************************************************************************
// This is an implementation the Dragon4 algorithm to convert a binary number
// in floating point format to a decimal number in string format. The function
// returns the number of digits written to the output buffer and the output is
// not NUL terminated.
//
// The floating point input value is (mantissa * 2^exponent).
//
// See the following papers for more information on the algorithm:
//  "How to Print Floating-Point Numbers Accurately"
//    Steele and White
//    http://kurtstephens.com/files/p372-steele.pdf
//  "Printing Floating-Point Numbers Quickly and Accurately"
//    Burger and Dybvig
//    http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656&rep=rep1&type=pdf
//******************************************************************************
tU32 Dragon4(
  tU64        mantissa,           // value significand
  tS32        exponent,           // value exponent in base 2
  tU32        mantissaHighBitIdx, // index of the highest set mantissa bit
  tB          hasUnequalMargins,  // is the high margin twice as large as the low margin
  tCutoffMode cutoffMode,         // how to determine output length
  tU32        cutoffNumber,       // parameter to the selected cutoffMode
  tC8 *       pOutBuffer,         // buffer to output into
  tU32        bufferSize,         // maximum characters that can be printed to pOutBuffer
  tS32 *      pOutExponent        // the base 10 exponent of the first digit
);

#endif
