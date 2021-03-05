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

#include "Nuclex/Pixels/ColorModels/ColorModelConverter.h"

#include <cmath>
#include <limits>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps an angle into the positive 0..Tau range</summary>
  /// <param name="angle">Angle that will be wrapped</param>
  /// <returns>The equivalent angle in the positive 0..Tau range</returns>  
  float wrapAngle(float angle) {
    static constexpr float Tau = (
      6.283185307179586476925286766559005768394338798750211641949889184615632812572417997256069651f
    );
    return angle - (Tau * std::floor(angle / Tau));
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  HsvColor ColorModelConverter::HsvFromRgb(const RgbColor &color) {
    HsvColor result;

    // Hue calculation needs to know highest and lowest value, so here's
    // an entire expanded decision tree. Lots of branching, but gives
    // the compiler good opportunity to use conditional move instructions.
    if(color.Red < color.Green) {
      if(color.Green < color.Blue) {
        result.Value = color.Blue; // Blue is highest
        result.Saturation = color.Blue;
        result.Saturation -= color.Red; // Red is lowest

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 4.0f;
        } else {
          result.Hue = (color.Red - color.Green) / result.Saturation + 4.0f;
          result.Saturation /= result.Value; // Safe; value is greather or equal to saturation
        }
      } else {
        result.Value = color.Green; // Green is highest
        result.Saturation = color.Green;
        if(color.Red < color.Blue) {
          result.Saturation -= color.Red; // Red is lowest
        } else {
          result.Saturation -= color.Blue; // Blue is lowest
        }

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 2.0f;
        } else {
          result.Hue = (color.Blue - color.Red) / result.Saturation + 2.0f;
          result.Saturation /= result.Value; // Safe; value is greather or equal to saturation
        }
      }
    } else {
      if(color.Red < color.Blue) {
        result.Value = color.Blue; // Blue is highest
        result.Saturation = color.Blue;
        result.Saturation -= color.Green; // Green is lowest

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 4.0f;
        } else {
          result.Hue = (color.Red - color.Green) / result.Saturation + 4.0f;
          result.Saturation /= result.Value; // Safe; value is greather or equal to saturation
        }
      } else {
        result.Value = color.Red; // Red is highest
        result.Saturation = color.Red;
        if(color.Green < color.Blue) {
          result.Saturation -= color.Green; // Green is lowest
        } else {
          result.Saturation -= color.Blue; // Blue is lowest
        }

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 0.0f;
        } else {
          result.Hue = (color.Green - color.Blue) / result.Saturation;
          result.Saturation /= result.Value; // Safe; value is greather or equal to saturation
        }
      }
    }

    // Normalize into 0..6 range
    if(result.Hue < 0.0f) {
      result.Hue += 6.0f;
    }

    // Convert to radian
    result.Hue *= HsvColor::YellowHue; // YellowHue happens to be 1/6th turn :)

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  RgbColor ColorModelConverter::RgbFromHsv(const HsvColor &color) {
    RgbColor result;

    // Calculate the color sextant floor and fraction for the specified hue
    float floor, fraction;
    {
      float sextant = wrapAngle(color.Hue) / HsvColor::YellowHue;
      floor = std::floor(sextant);
      fraction = sextant - floor;
    }

    // Determine the red, green and blue values depending on the sextant the hue is in
    switch(static_cast<std::size_t>(floor + 0.5f)) {
      case 0: {
        result.Red = color.Value;
        result.Green = color.Value * (1.0f - (color.Saturation * (1.0f - fraction)));
        result.Blue = color.Value * (1.0f - color.Saturation);
        break;
      }
      case 1: {
        result.Red = color.Value * (1.0f - (color.Saturation * fraction));
        result.Green = color.Value;
        result.Blue = color.Value * (1.0f - color.Saturation);
        break;
      }
      case 2: {
        result.Red = color.Value * (1.0f - color.Saturation);
        result.Green = color.Value;
        result.Blue = color.Value * (1.0f - (color.Saturation * (1.0f - fraction)));
        break;
      }
      case 3: {
        result.Red = color.Value * (1.0f - color.Saturation);
        result.Green = color.Value * (1.0f - (color.Saturation * fraction));
        result.Blue = color.Value;
        break;
      }
      case 4: {
        result.Red = color.Value * (1.0f - (color.Saturation * (1.0f - fraction)));
        result.Green = color.Value * (1.0f - color.Saturation);
        result.Blue = color.Value;
        break;
      }
      case 5: {
        result.Red = color.Value;
        result.Green = color.Value * (1.0f - color.Saturation);
        result.Blue = color.Value * (1.0f - (color.Saturation * fraction));
        break;
      }
    }

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  HslColor ColorModelConverter::HslFromRgb(const RgbColor &color) {
    HslColor result;

    // Hue calculation needs to know highest and lowest value, so here's
    // an entire expanded decision tree. Lots of branching, but gives
    // the compiler good opportunity to use conditional move instructions.
    if(color.Red < color.Green) {
      if(color.Green < color.Blue) {
        result.Saturation = result.Lightness = color.Blue; // Blue is highest
        result.Saturation -= color.Red; // Red is lowest
        result.Lightness += color.Red;
        result.Lightness /= 2.0f;

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 4.0f;
        } else {
          result.Hue = (color.Red - color.Green) / result.Saturation + 4.0f;
          result.Saturation /= (1.0f - std::fabs(2.0f * result.Lightness - 1.0));
        }
      } else {
        result.Saturation = result.Lightness = color.Green; // Green is highest
        if(color.Red < color.Blue) {
          result.Saturation -= color.Red; // Red is lowest
          result.Lightness += color.Red;
        } else {
          result.Saturation -= color.Blue; // Blue is lowest
          result.Lightness += color.Blue;
        }
        result.Lightness /= 2.0f;

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 2.0f;
        } else {
          result.Hue = (color.Blue - color.Red) / result.Saturation + 2.0f;
          result.Saturation /= (1.0f - std::fabs(2.0f * result.Lightness - 1.0));
        }
      }
    } else {
      if(color.Red < color.Blue) {
        result.Saturation = result.Lightness = color.Blue; // Blue is highest
        result.Saturation -= color.Green; // Green is lowest
        result.Lightness += color.Green;
        result.Lightness /= 2.0f;

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 4.0f;
        } else {
          result.Hue = (color.Red - color.Green) / result.Saturation + 4.0f;
          result.Saturation /= (1.0f - std::fabs(2.0f * result.Lightness - 1.0));
        }
      } else {
        result.Saturation = result.Lightness = color.Red; // Red is highest
        if(color.Green < color.Blue) {
          result.Saturation -= color.Green; // Green is lowest
          result.Lightness += color.Green;
        } else {
          result.Saturation -= color.Blue; // Blue is lowest
          result.Lightness += color.Blue;
        }
        result.Lightness /= 2.0f;

        // Calculate hue and fix saturation (it's currently just delta between low..high)
        if(result.Saturation < std::numeric_limits<float>::epsilon()) {
          result.Hue = 0.0f;
        } else {
          result.Hue = (color.Green - color.Blue) / result.Saturation;
          result.Saturation /= (1.0f - std::fabs(2.0f * result.Lightness - 1.0));
        }
      }
    }

    // Normalize into 0..6 range
    if(result.Hue < 0.0f) {
      result.Hue += 6.0f;
    }

    // Convert to radian
    result.Hue *= HslColor::YellowHue; // YellowHue happens to be 1/6th turn :)

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  RgbColor ColorModelConverter::RgbFromHsl(const HslColor &color) {
    RgbColor result;

    // Calculate the color sextant floor and fraction for the specified hue
    float floor, fraction;
    {
      float sextant = wrapAngle(color.Hue) / HslColor::YellowHue;
      floor = std::floor(sextant);
      fraction = sextant - floor;
    }

    // Calculate primary color channel saturation
    //   lightness 0.0  ->  primary 0.0 (desaturated black)
    //   lightness 0.5  ->  primary 1.0 (or rather, saturation)
    //   lightness 1.0  ->  primary 0.0 (desaturated white)
    float primary = (1.0f - std::fabs(2.0f * color.Lightness - 1.0f)) * color.Saturation;

    // Determine the red, green and blue values depending on the sextant the hue is in
    switch(static_cast<std::size_t>(floor + 0.5f)) {
      case 0: {
        result.Red = primary;
        result.Green = fraction * primary;
        result.Blue = 0.0f;
        break;
      }
      case 1: {
        result.Red = (1.0f - fraction) * primary;
        result.Green = primary;
        result.Blue = 0.0f;
        break;
      }
      case 2: {
        result.Red = 0.0f;
        result.Green = primary;
        result.Blue = fraction * primary;
        break;
      }
      case 3: {
        result.Red = 0.0f;
        result.Green = (1.0f - fraction) * primary;
        result.Blue = primary;
        break;
      }
      case 4: {
        result.Red = fraction * primary;
        result.Green = 0.0f;
        result.Blue = primary;
        break;
      }
      case 5: {
        result.Red = primary;
        result.Green = 0.0f;
        result.Blue = (1.0f - fraction) * primary;
        break;
      }
    }

    // Lightness above 0.5 blends into pure white,
    // so we have to adjust all color channels once more
    float adjustment = color.Lightness - primary / 2.0f;
    result.Red += adjustment;
    result.Green += adjustment;
    result.Blue += adjustment;

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  HslColor ColorModelConverter::HslFromHsv(const HsvColor &color) {
    HslColor result;

    result.Hue = color.Hue;

    // Calculate equivalent lightness and needed adjustment for saturation
    float adjustment = (2.0f - color.Saturation) * color.Value;

    result.Lightness = adjustment / 2.0f;

    // Adjust saturation so it matches the HSL interpretation
    if(adjustment < 1.0f) {
      if(adjustment < std::numeric_limits<float>::epsilon()) {
        result.Saturation = 0.0f;
      } else {
        result.Saturation = color.Saturation * color.Value / adjustment;
      }
    } else {
      adjustment = 2.0f - adjustment;
      if(adjustment < std::numeric_limits<float>::epsilon()) {
        result.Saturation = 0.0f;
      } else {
        result.Saturation = color.Saturation * color.Value / adjustment;
      }
    }

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  HsvColor ColorModelConverter::HsvFromHsl(const HslColor &color) {
    HsvColor result;

    result.Hue = color.Hue;

    // Calculate needed adjustment from lightness to value
    float adjustment = color.Saturation;
    if(color.Lightness < 0.5f) {
      adjustment *= color.Lightness;
    } else {
      adjustment *= (1.0f - color.Lightness);
    }

    // Convert saturation to HSV
    float quotient = color.Lightness + adjustment;
    if(quotient < std::numeric_limits<float>::epsilon()) {
      result.Saturation = 0.0f;
    } else {
      result.Saturation = 2.0f * adjustment / quotient;
    }

    // Move the value up so 1.0 rather than 0.5 means full saturation
    result.Value = adjustment + color.Lightness;

    result.Alpha = color.Alpha;

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels
