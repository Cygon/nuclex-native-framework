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

#include "PrintFloat.h"
#include "Dragon4.h"
#include "Math.h"

#include <string.h>

//******************************************************************************
// Helper union to decompose a 32-bit IEEE float.
// sign:      1 bit
// exponent:  8 bits
// mantissa: 23 bits
//******************************************************************************
union tFloatUnion32 {
  tB   IsNegative() const { return (m_integer >> 31) != 0; }
  tU32 GetExponent() const { return (m_integer >> 23) & 0xFF; }
  tU32 GetMantissa() const { return m_integer & 0x7FFFFF; }

  tF32 m_floatingPoint;
  tU32 m_integer;
};

//******************************************************************************
// Helper union to decompose a 64-bit IEEE float.
// sign:      1 bit
// exponent: 11 bits
// mantissa: 52 bits
//******************************************************************************
union tFloatUnion64 {
  tB   IsNegative() const { return (m_integer >> 63) != 0; }
  tU32 GetExponent() const { return (m_integer >> 52) & 0x7FF; }
  tU64 GetMantissa() const { return m_integer & 0xFFFFFFFFFFFFFull; }

  tF64 m_floatingPoint;
  tU64 m_integer;
};

//******************************************************************************
// Outputs the positive number with positional notation: ddddd.dddd
// The output is always NUL terminated and the output length (not including the
// NUL) is returned.
//******************************************************************************
tU32 FormatPositional(
  tC8 *   pOutBuffer,         // buffer to output into
  tU32    bufferSize,         // maximum characters that can be printed to pOutBuffer
  tU64    mantissa,           // value significand
  tS32    exponent,           // value exponent in base 2
  tU32    mantissaHighBitIdx, // index of the highest set mantissa bit
  tB      hasUnequalMargins,  // is the high margin twice as large as the low margin
  tS32    precision           // Negative prints as many digits as are needed for a unique
                              //  number. Positive specifies the maximum number of
                              //  significant digits to print past the decimal point.
) {
  RJ_ASSERT(bufferSize > 0);

  tS32 printExponent;
  tU32 numPrintDigits;

  tU32 maxPrintLen = bufferSize - 1;

  if(precision < 0) {
    numPrintDigits = Dragon4(
      mantissa,
      exponent,
      mantissaHighBitIdx,
      hasUnequalMargins,
      CutoffMode_Unique,
      0,
      pOutBuffer,
      maxPrintLen,
      &printExponent
    );
  } else {
    numPrintDigits = Dragon4(
      mantissa,
      exponent,
      mantissaHighBitIdx,
      hasUnequalMargins,
      CutoffMode_FractionLength,
      precision,
      pOutBuffer,
      maxPrintLen,
      &printExponent
    );
  }

  RJ_ASSERT(numPrintDigits > 0);
  RJ_ASSERT(numPrintDigits <= bufferSize);

  // track the number of digits past the decimal point that have been printed
  tU32 numFractionDigits = 0;

  // if output has a whole number
  if(printExponent >= 0) {
    // leave the whole number at the start of the buffer
    tU32 numWholeDigits = printExponent + 1;
    if(numPrintDigits < numWholeDigits) {
      // don't overflow the buffer
      if(numWholeDigits > maxPrintLen) {
        numWholeDigits = maxPrintLen;
      }

      // add trailing zeros up to the decimal point
      for(; numPrintDigits < numWholeDigits; ++numPrintDigits) {
        pOutBuffer[numPrintDigits] = '0';
      }
    } else if(numPrintDigits > (tU32)numWholeDigits) {
      // insert the decimal point prior to the fraction
      numFractionDigits = numPrintDigits - numWholeDigits;
      tU32 maxFractionDigits = maxPrintLen - numWholeDigits - 1;
      if(numFractionDigits > maxFractionDigits)
        numFractionDigits = maxFractionDigits;

      memmove(pOutBuffer + numWholeDigits + 1, pOutBuffer + numWholeDigits, numFractionDigits);
      pOutBuffer[numWholeDigits] = '.';
      numPrintDigits = numWholeDigits + 1 + numFractionDigits;
    }
  } else {
    // shift out the fraction to make room for the leading zeros
    if(maxPrintLen > 2) {
      tU32 numFractionZeros = (tU32)-printExponent - 1;
      tU32 maxFractionZeros = maxPrintLen - 2;
      if(numFractionZeros > maxFractionZeros) {
        numFractionZeros = maxFractionZeros;
      }

      tU32 digitsStartIdx = 2 + numFractionZeros;

      // shift the significant digits right such that there is room for leading zeros
      numFractionDigits = numPrintDigits;
      tU32 maxFractionDigits = maxPrintLen - digitsStartIdx;
      if(numFractionDigits > maxFractionDigits) {
        numFractionDigits = maxFractionDigits;
      }

      memmove(pOutBuffer + digitsStartIdx, pOutBuffer, numFractionDigits);

      // insert the leading zeros
      for(tU32 i = 2; i < digitsStartIdx; ++i) {
        pOutBuffer[i] = '0';
      }

      // update the counts
      numFractionDigits += numFractionZeros;
      numPrintDigits = numFractionDigits;
    }

    // add the decimal point
    if(maxPrintLen > 1) {
      pOutBuffer[1] = '.';
      numPrintDigits += 1;
    }

    // add the initial zero
    if(maxPrintLen > 0) {
      pOutBuffer[0] = '0';
      numPrintDigits += 1;
    }
  }

  // add trailing zeros up to precision length
  if(precision > (tS32)numFractionDigits && numPrintDigits < maxPrintLen) {
    // add a decimal point if this is the first fractional digit we are printing
    if(numFractionDigits == 0) {
      pOutBuffer[numPrintDigits++] = '.';
    }

    // compute the number of trailing zeros needed
    tU32 totalDigits = numPrintDigits + (precision - numFractionDigits);
    if(totalDigits > maxPrintLen) {
      totalDigits = maxPrintLen;
    }

    for(; numPrintDigits < totalDigits; ++numPrintDigits) {
      pOutBuffer[numPrintDigits] = '0';
    }
  }

  // terminate the buffer
  RJ_ASSERT(numPrintDigits <= maxPrintLen);
  pOutBuffer[numPrintDigits] = '\0';

  return numPrintDigits;
}

//******************************************************************************
// Outputs the positive number with scientific notation: d.dddde[sign]ddd
// The output is always NUL terminated and the output length (not including the
// NUL) is returned.
//******************************************************************************
tU32 FormatScientific(
  tC8 *   pOutBuffer,         // buffer to output into
  tU32    bufferSize,         // maximum characters that can be printed to pOutBuffer
  tU64    mantissa,           // value significand
  tS32    exponent,           // value exponent in base 2
  tU32    mantissaHighBitIdx, // index of the highest set mantissa bit
  tB      hasUnequalMargins,  // is the high margin twice as large as the low margin
  tS32    precision           // Negative prints as many digits as are needed for a unique
                              //  number. Positive specifies the maximum number of
                              //  significant digits to print past the decimal point.
) {
  RJ_ASSERT(bufferSize > 0);

  tS32 printExponent;
  tU32 numPrintDigits;

  if(precision < 0) {
    numPrintDigits = Dragon4(
      mantissa,
      exponent,
      mantissaHighBitIdx,
      hasUnequalMargins,
      CutoffMode_Unique,
      0,
      pOutBuffer,
      bufferSize,
      &printExponent
    );
  } else {
    numPrintDigits = Dragon4(
      mantissa,
      exponent,
      mantissaHighBitIdx,
      hasUnequalMargins,
      CutoffMode_TotalLength,
      precision + 1,
      pOutBuffer,
      bufferSize,
      &printExponent
    );
  }

  RJ_ASSERT(numPrintDigits > 0);
  RJ_ASSERT(numPrintDigits <= bufferSize);

  tC8 * pCurOut = pOutBuffer;

  // keep the whole number as the first digit
  if(bufferSize > 1) {
    pCurOut += 1;
    bufferSize -= 1;
  }

  // insert the decimal point prior to the fractional number
  tU32 numFractionDigits = numPrintDigits - 1;
  if(numFractionDigits > 0 && bufferSize > 1) {
    tU32 maxFractionDigits = bufferSize - 2;
    if(numFractionDigits > maxFractionDigits) {
      numFractionDigits = maxFractionDigits;
    }

    memmove(pCurOut + 1, pCurOut, numFractionDigits);
    pCurOut[0] = '.';
    pCurOut += (1 + numFractionDigits);
    bufferSize -= (1 + numFractionDigits);
  }

  // add trailing zeros up to precision length
  if(precision > (tS32)numFractionDigits && bufferSize > 1) {
    // add a decimal point if this is the first fractional digit we are printing
    if(numFractionDigits == 0) {
      *pCurOut = '.';
      ++pCurOut;
      --bufferSize;
    }

    // compute the number of trailing zeros needed
    tU32 numZeros = (precision - numFractionDigits);
    if(numZeros > bufferSize - 1) {
      numZeros = bufferSize - 1;
    }

    for(tC8 * pEnd = pCurOut + numZeros; pCurOut < pEnd; ++pCurOut) {
      *pCurOut = '0';
    }
  }

  // print the exponent into a local buffer and copy into output buffer
  if(bufferSize > 1) {
    tC8 exponentBuffer[5];
    exponentBuffer[0] = 'e';
    if(printExponent >= 0) {
      exponentBuffer[1] = '+';
    } else {
      exponentBuffer[1] = '-';
      printExponent = -printExponent;
    }

    RJ_ASSERT(printExponent < 1000);
    tU32 hundredsPlace = printExponent / 100;
    tU32 tensPlace = (printExponent - hundredsPlace * 100) / 10;
    tU32 onesPlace = (printExponent - hundredsPlace * 100 - tensPlace * 10);

    exponentBuffer[2] = (tC8)('0' + hundredsPlace);
    exponentBuffer[3] = (tC8)('0' + tensPlace);
    exponentBuffer[4] = (tC8)('0' + onesPlace);

    // copy the exponent buffer into the output
    tU32 maxExponentSize = bufferSize - 1;
    tU32 exponentSize = (5 < maxExponentSize) ? 5 : maxExponentSize;
    memcpy(pCurOut, exponentBuffer, exponentSize);

    pCurOut += exponentSize;
    bufferSize -= exponentSize;
  }

  RJ_ASSERT(bufferSize > 0);
  pCurOut[0] = '\0';

  return static_cast<tU32>(pCurOut - pOutBuffer);
}

//******************************************************************************
// Print a hexadecimal value with a given width.
// The output string is always NUL terminated and the string length (not
// including the NUL) is returned.
//******************************************************************************
static tU32 PrintHex(tC8 * pOutBuffer, tU32 bufferSize, tU64 value, tU32 width) {
  const tC8 digits[] = "0123456789abcdef";

  RJ_ASSERT(bufferSize > 0);

  tU32 maxPrintLen = bufferSize - 1;
  if(width > maxPrintLen) {
    width = maxPrintLen;
  }

  tC8 * pCurOut = pOutBuffer;
  while(width > 0) {
    --width;

    tU8 digit = (tU8)((value >> 4ull * (tU64)width) & 0xF);
    *pCurOut = digits[digit];

    ++pCurOut;
  }

  *pCurOut = '\0';
  return static_cast<tU32>(pCurOut - pOutBuffer);
}

//******************************************************************************
// Print special case values for infinities and NaNs.
// The output string is always NUL terminated and the string length (not
// including the NUL) is returned.
//******************************************************************************
static tU32 PrintInfNan(tC8 * pOutBuffer, tU32 bufferSize, tU64 mantissa, tU32 mantissaHexWidth) {
  RJ_ASSERT(bufferSize > 0);

  tU32 maxPrintLen = bufferSize - 1;

  // Check for infinity
  if(mantissa == 0) {
    // copy and make sure the buffer is terminated
    tU32 printLen = (3 < maxPrintLen) ? 3 : maxPrintLen;
    ::memcpy(pOutBuffer, "Inf", printLen);
    pOutBuffer[printLen] = '\0';
    return printLen;
  } else {
    // copy and make sure the buffer is terminated
    tU32 printLen = (3 < maxPrintLen) ? 3 : maxPrintLen;
    ::memcpy(pOutBuffer, "NaN", printLen);
    pOutBuffer[printLen] = '\0';

    // append HEX value
    if(maxPrintLen > 3)
      printLen += PrintHex(pOutBuffer + 3, bufferSize - 3, mantissa, mantissaHexWidth);

    return printLen;
  }
}

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
) {
  if(bufferSize == 0) {
    return 0;
  }

  if(bufferSize == 1) {
    pOutBuffer[0] = '\0';
    return 0;
  }

  // deconstruct the floating point value
  tFloatUnion32 floatUnion;
  floatUnion.m_floatingPoint = value;
  tU32 floatExponent = floatUnion.GetExponent();
  tU32 floatMantissa = floatUnion.GetMantissa();
  tU32 prefixLength = 0;

  // output the sign
  if(floatUnion.IsNegative()) {
    pOutBuffer[0] = '-';
    ++pOutBuffer;
    --bufferSize;
    ++prefixLength;
    RJ_ASSERT(bufferSize > 0);
  }

  // if this is a special value
  if(floatExponent == 0xFF) {
    return PrintInfNan(pOutBuffer, bufferSize, floatMantissa, 6) + prefixLength;
  } else { // else this is a number
    // factor the value into its parts
    tU32 mantissa;
    tS32 exponent;
    tU32 mantissaHighBitIdx;
    tB hasUnequalMargins;
    if(floatExponent != 0) {
      // normalized
      // The floating point equation is:
      //  value = (1 + mantissa/2^23) * 2 ^ (exponent-127)
      // We convert the integer equation by factoring a 2^23 out of the exponent
      //  value = (1 + mantissa/2^23) * 2^23 * 2 ^ (exponent-127-23)
      //  value = (2^23 + mantissa) * 2 ^ (exponent-127-23)
      // Because of the implied 1 in front of the mantissa we have 24 bits of precision.
      //   m = (2^23 + mantissa)
      //   e = (exponent-127-23)
      mantissa = (1UL << 23) | floatMantissa;
      exponent = floatExponent - 127 - 23;
      mantissaHighBitIdx = 23;
      hasUnequalMargins = (floatExponent != 1) && (floatMantissa == 0);
    } else {
      // denormalized
      // The floating point equation is:
      //  value = (mantissa/2^23) * 2 ^ (1-127)
      // We convert the integer equation by factoring a 2^23 out of the exponent
      //  value = (mantissa/2^23) * 2^23 * 2 ^ (1-127-23)
      //  value = mantissa * 2 ^ (1-127-23)
      // We have up to 23 bits of precision.
      //   m = (mantissa)
      //   e = (1-127-23)
      mantissa = floatMantissa;
      exponent = 1 - 127 - 23;
      mantissaHighBitIdx = LogBase2(mantissa);
      hasUnequalMargins = false;
    }

    // format the value
    switch(format) {
      case PrintFloatFormat_Positional: {
        return FormatPositional(pOutBuffer,
          bufferSize,
          mantissa,
          exponent,
          mantissaHighBitIdx,
          hasUnequalMargins,
          precision
        ) + prefixLength;
      }
      case PrintFloatFormat_Scientific: {
        return FormatScientific(pOutBuffer,
          bufferSize,
          mantissa,
          exponent,
          mantissaHighBitIdx,
          hasUnequalMargins,
          precision
        ) + prefixLength;
      }
      default: {
        pOutBuffer[0] = '\0';
        return 0;
      }
    }
  }
}

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
) {
  if(bufferSize == 0) {
    return 0;
  }

  if(bufferSize == 1) {
    pOutBuffer[0] = '\0';
    return 0;
  }

  // deconstruct the floating point value
  tFloatUnion64 floatUnion;
  floatUnion.m_floatingPoint = value;
  tU32 floatExponent = floatUnion.GetExponent();
  tU64 floatMantissa = floatUnion.GetMantissa();
  tU32 prefixLength = 0;

  // output the sign
  if(floatUnion.IsNegative()) {
    pOutBuffer[0] = '-';
    ++pOutBuffer;
    --bufferSize;
    ++prefixLength;
    RJ_ASSERT(bufferSize > 0);
  }

  // if this is a special value
  if(floatExponent == 0x7FF) {
    return PrintInfNan(pOutBuffer, bufferSize, floatMantissa, 13) + prefixLength;
  } else { // else this is a number
    // factor the value into its parts
    tU64 mantissa;
    tS32 exponent;
    tU32 mantissaHighBitIdx;
    tB hasUnequalMargins;

    if(floatExponent != 0) {
      // normal
      // The floating point equation is:
      //  value = (1 + mantissa/2^52) * 2 ^ (exponent-1023)
      // We convert the integer equation by factoring a 2^52 out of the exponent
      //  value = (1 + mantissa/2^52) * 2^52 * 2 ^ (exponent-1023-52)
      //  value = (2^52 + mantissa) * 2 ^ (exponent-1023-52)
      // Because of the implied 1 in front of the mantissa we have 53 bits of precision.
      //   m = (2^52 + mantissa)
      //   e = (exponent-1023+1-53)
      mantissa = (1ull << 52) | floatMantissa;
      exponent = floatExponent - 1023 - 52;
      mantissaHighBitIdx = 52;
      hasUnequalMargins = (floatExponent != 1) && (floatMantissa == 0);
    } else {
      // subnormal
      // The floating point equation is:
      //  value = (mantissa/2^52) * 2 ^ (1-1023)
      // We convert the integer equation by factoring a 2^52 out of the exponent
      //  value = (mantissa/2^52) * 2^52 * 2 ^ (1-1023-52)
      //  value = mantissa * 2 ^ (1-1023-52)
      // We have up to 52 bits of precision.
      //   m = (mantissa)
      //   e = (1-1023-52)
      mantissa = floatMantissa;
      exponent = 1 - 1023 - 52;
      mantissaHighBitIdx = LogBase2(mantissa);
      hasUnequalMargins = false;
    }

    // format the value
    switch(format) {
      case PrintFloatFormat_Positional: {
        return FormatPositional(pOutBuffer,
          bufferSize,
          mantissa,
          exponent,
          mantissaHighBitIdx,
          hasUnequalMargins,
          precision
        ) + prefixLength;
      }
      case PrintFloatFormat_Scientific: {
        return FormatScientific(pOutBuffer,
          bufferSize,
          mantissa,
          exponent,
          mantissaHighBitIdx,
          hasUnequalMargins,
          precision
        ) + prefixLength;
      }

      default: {
        pOutBuffer[0] = '\0';
        return 0;
      }
    }
  }
}
