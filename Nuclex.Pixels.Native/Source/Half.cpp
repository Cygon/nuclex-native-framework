#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#include "Nuclex/Pixels/Half.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  union Bits {
    float f;
    int32_t si;
    uint32_t ui;
  };

  // ------------------------------------------------------------------------------------------- //

  const int shift = 13;
  const int shiftSign = 16;

  const std::int32_t infN = 0x7F800000; // flt32 infinity
  const std::int32_t maxN = 0x477FE000; // max flt16 normal as a flt32
  const std::int32_t minN = 0x38800000; // min flt16 normal as a flt32
  const std::int32_t signN = 0x80000000; // flt32 sign bit

  const std::int32_t infC = infN >> shift;
  const std::int32_t nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
  const std::int32_t maxC = maxN >> shift;
  const std::int32_t minC = minN >> shift;
  const std::int32_t signC = signN >> shiftSign; // flt16 sign bit

  const std::int32_t mulN = 0x52000000; // (1 << 23) / minN
  const std::int32_t mulC = 0x33800000; // minN / (1 << (23 - shift))

  const std::int32_t subC = 0x003FF; // max flt32 subnormal down shifted
  const std::int32_t norC = 0x00400; // min flt32 normal down shifted

  const std::int32_t maxD = infC - maxC - 1;
  const std::int32_t minD = minC - subC - 1;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  const Half Half::One = Half::FromBits(15360);

  // ------------------------------------------------------------------------------------------- //

  const Half Half::Zero = Half::FromBits(0);

  // ------------------------------------------------------------------------------------------- //

  std::uint16_t Half::BitsFromFloat(float value) {
    Bits v, s;
    v.f = value;

    std::uint32_t sign = v.si & signN;
    v.si ^= sign;
    sign >>= shiftSign; // logical shift

    s.si = mulN;
    s.si = static_cast<std::int32_t>(s.f * v.f); // correct subnormals

    v.si ^= (s.si ^ v.si) & -(minN > v.si);
    v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
    v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));

    v.ui >>= shift; // logical shift
    v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
    v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);

    return static_cast<std::uint16_t>(v.ui | sign);
  }

  // ------------------------------------------------------------------------------------------- //

  float Half::FloatFromBits(std::uint16_t value) {
    Bits v;
    v.ui = value;

    int32_t sign = v.si & signC;
    v.si ^= sign;
    sign <<= shiftSign;

    v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
    v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);

    Bits s;
    s.si = mulC;
    s.f *= v.si;

    int32_t mask = -(norC > v.si);
    v.si <<= shift;
    v.si ^= (s.si ^ v.si) & mask;
    v.si |= sign;

    return v.f;
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
