#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/QuantityFormatter.h"
#include "Nuclex/Support/Text/LexicalAppend.h"

#include <cstdlib> // for std::strtoul(), std::strtoull(), std::strtol(), std::strtoll()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a number treating the last digit as behind a decimal point</summary>
  /// <param name="target">String to which the number will be appended</param>
  /// <param name="numberTimesTen">Number that will be appended</param>
  void appendWithOneDecimalDigit(std::string &target, std::size_t numberTimesTen) {
    Nuclex::Support::Text::lexical_append(target, numberTimesTen / 10);
    target.push_back(u8'.');
    Nuclex::Support::Text::lexical_append(target, numberTimesTen % 10);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a number with a leading '0' if it is less than two digits long</summary>
  /// <param name="target">String to which the number will be appended</param>
  /// <param name="number">Number that will be appended to the string</param>
  void appendAsTwoDigits(std::string &target, std::size_t number) {
    if(number < 10) {
      target.push_back(u8'0');
    }

    Nuclex::Support::Text::lexical_append(target, number);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::string QuantityFormatter::stringFromByteCount(
    std::uint64_t byteCount, bool useBinaryMagnitudes /* = true */
  ) {
    std::string result;

    if(useBinaryMagnitudes) {
      if(byteCount < 1'023'936) { // up to 1000 KiB
        if(byteCount < 1'000) { // less than 1000 bytes
          result.reserve(9);
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 1000 bytes to 1000 KiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 64) * 10 / 1'024);
          const char *kiloBinaryBytesText = u8" KiB";
          result.append(kiloBinaryBytesText, kiloBinaryBytesText + 4);
        }
      } else if(byteCount < 1'073'674'715'136) { // up to 1000 GiB
        if(byteCount < 1'048'510'464) { // less than 1000 MiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 65'536) * 10 / 1'048'576);
          const char *megaBinaryBytesText = u8" MiB";
          result.append(megaBinaryBytesText, megaBinaryBytesText + 4);
        } else { // 1000 MiB to 1000 GiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 67'108'864) * 10 / 1'073'741'824);
          const char *gigaBinaryBytesText = u8" GiB";
          result.append(gigaBinaryBytesText, gigaBinaryBytesText + 4);
        }
      } else {
        if(byteCount < 1'099'442'908'299'264) { // less than 1000 TiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 68'719'476'736) * 10 / 1'099'511'627'776);
          const char *teraBinaryBytesText = u8" TiB";
          result.append(teraBinaryBytesText, teraBinaryBytesText + 4);
        } else { // more than 1000 TiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 70'368'744'177'664) * 10 / 1'125'899'906'842'624);
          const char *petaBinaryBytesText = u8" PiB";
          result.append(petaBinaryBytesText, petaBinaryBytesText + 4);
        }
      }
    } else { // metric magnitudes rather than binary
      result.reserve(9);

      if(byteCount < 999'950) { // up to 1000 KB
        if(byteCount < 1000) { // less than 1000 bytes
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 512 bytes to 500 KB
          appendWithOneDecimalDigit(
            result, (byteCount + 50) * 10 / 1'000
          );
          const char *kilobytesText = u8" KB";
          result.append(kilobytesText, kilobytesText + 3);
        }
      } else if(byteCount < 999'950'000'000) { // up to 1000 GB
        if(byteCount < 999'950'000) { // less than 1000 MB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000) * 10 / 1'000'000
          );
          const char *megabytesText = u8" MB";
          result.append(megabytesText, megabytesText + 3);
        } else { // 1000 MB to 1000 GB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000) * 10 / 1'000'000'000
          );
          const char *gigabytesText = u8" GB";
          result.append(gigabytesText, gigabytesText + 3);
        }
      } else {
        if(byteCount < 999'950'000'000'000) { // less than 1000 TB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000) * 10 / 1'000'000'000'000
          );
          const char *terabytesText = u8" TB";
          result.append(terabytesText, terabytesText + 3);
        } else { // more than 1000 TB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000'000) * 10 / 1'000'000'000'000'000
          );
          const char *petabytesText = u8" PB";
          result.append(petabytesText, petabytesText + 3);
        }
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string QuantityFormatter::stringFromDuration(
    std::chrono::seconds duration, bool useSimpleFormat /* = true */
  ) {
    std::string result;

    if(useSimpleFormat) {
      if(duration < std::chrono::seconds(3'597)) { // less than an hour?
        if(duration < std::chrono::seconds(60)) { // less than a minute?
          result.reserve(10);
          lexical_append(result, duration.count());
          const char *secondsText = u8" seconds";
          result.append(secondsText, secondsText + 8);
        } else { // a minute to an hour
          result.reserve(12);
          appendWithOneDecimalDigit(result, (duration.count() + 3) * 10 / 60);
          const char *minutesText = u8" minutes";
          result.append(minutesText, minutesText + 8);
        }
      } else if(duration < std::chrono::seconds(1'205'280)) { // less than two weeks?
        if(duration < std::chrono::seconds(86'220)) { // less than a day?
          result.reserve(10);
          appendWithOneDecimalDigit(result, (duration.count() + 180) * 10 / 3'600);
          const char *hoursText = u8" hours";
          result.append(hoursText, hoursText + 6);
        } else { // a day to two weeks
          result.reserve(9);
          appendWithOneDecimalDigit(result, (duration.count() + 4'320) * 10 / 86'400);
          const char *daysText = u8" days";
          result.append(daysText, daysText + 5);
        }
      } else if(duration < std::chrono::seconds(31'490'640)) { // less than a year?
        if(duration < std::chrono::seconds(4'808'160)) { // less than eight weeks?
          result.reserve(10);
          appendWithOneDecimalDigit(result, (duration.count() + 30'240) * 10 / 604'800);
          const char *weeksText = u8" weeks";
          result.append(weeksText, weeksText + 6);
        } else { // eight weeks to a year
          result.reserve(11);
          appendWithOneDecimalDigit(result, (duration.count() + 131'760) * 10 / 2'635'200);
          const char *monthsText = u8" months";
          result.append(monthsText, monthsText + 7);
        }
      } else { // more than a year
        result.reserve(12);
        appendWithOneDecimalDigit(result, (duration.count() + 1'576'800) * 10 / 31'536'000);
        const char *yearsText = u8" years";
        result.append(yearsText, yearsText + 6);
      }
    } else { // exact duration format
      if(duration >= std::chrono::seconds(2'635'200)) { // more than a month?
        if(duration >= std::chrono::seconds(31'490'640)) {  // more than a year?
          result.reserve(19);
          lexical_append(result, duration.count() / 31'490'640);
          result.push_back(u8'y');
        } else {
          result.reserve(16);
        }

        lexical_append(result, (duration.count() % 31'490'640) / 2'635'200);
        result.push_back(u8'm');
        lexical_append(result, (duration.count() % 31'490'640) % 2'635'200 / 86'400);
        const char *daysText = u8"d ";
        result.append(daysText, daysText + 2);

        lexical_append(
          result, (duration.count() % 31'490'640) % 2'635'200 % 86'400 / 3'600
        );
        result.push_back(u8'h');
        appendAsTwoDigits(
          result, (duration.count() % 31'490'640) % 2'635'200 % 86'400 % 3'600 / 60
        );
        result.push_back(u8'm');
        appendAsTwoDigits(
          result, (duration.count() % 31'490'640) % 2'635'200 % 86'400 % 3'600 % 60
        );
        result.push_back(u8's');
      } else if(duration >= std::chrono::seconds(3'600)) { // more than an hour?
        if(duration >= std::chrono::seconds(86'400)) { // more than a day?
          result.reserve(13);
          lexical_append(result, duration.count() / 86'400);
          const char *daysText = u8"d ";
          result.append(daysText, daysText + 2);
        } else {
          result.reserve(9);
        }

        lexical_append(result, (duration.count() % 86'400) / 3'600);
        result.push_back(u8'h');
        appendAsTwoDigits(result, (duration.count() % 86'400) % 3'600 / 60);
        result.push_back(u8'm');
        appendAsTwoDigits(result, (duration.count() % 86'400) % 3'600 % 60);
        result.push_back(u8's');
      } else { // less than an hour
        if(duration >= std::chrono::seconds(60)) { // more than a minute?
          result.reserve(6);
          appendAsTwoDigits(result, duration.count() / 60);
          result.push_back(u8'm');
        } else {
          result.reserve(3);
        }

        appendAsTwoDigits(result, duration.count() % 60);
        result.push_back(u8's');
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
