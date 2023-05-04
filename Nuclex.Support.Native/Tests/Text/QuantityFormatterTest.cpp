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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/QuantityFormatter.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintMetricByteCount) {
    std::string smallBytes = QuantityFormatter::stringFromByteCount(234, false);
    EXPECT_NE(smallBytes.find(u8"234"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::string::npos);

    std::string bigBytes = QuantityFormatter::stringFromByteCount(789, false);
    EXPECT_NE(bigBytes.find(u8"789"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"bytes"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(324'123, false);
    EXPECT_NE(smallBytes.find(u8"324.1"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"KB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(876'456, false);
    EXPECT_NE(bigBytes.find(u8"876.5"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"KB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(139'432'174, false);
    EXPECT_NE(smallBytes.find(u8"139.4"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"MB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(977'341'931, false);
    EXPECT_NE(bigBytes.find(u8"977.3"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"MB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(412'523'934'812, false);
    EXPECT_NE(smallBytes.find(u8"412.5"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"GB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(634'839'012'517, false);
    EXPECT_NE(bigBytes.find(u8"634.8"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"GB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(347'104'194'387'594, false);
    EXPECT_NE(smallBytes.find(u8"347.1"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"TB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(893'270'909'743'209, false);
    EXPECT_NE(bigBytes.find(u8"893.3"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"TB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(36'093'248'903'249'082, false);
    EXPECT_NE(smallBytes.find(u8"36.1"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"PB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(936'582'932'385'623'894, false);
    EXPECT_NE(bigBytes.find(u8"936.6"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PB"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintBinaryByteCount) {
    std::string smallBytes = QuantityFormatter::stringFromByteCount(234);
    EXPECT_NE(smallBytes.find(u8"234"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::string::npos);

    std::string bigBytes = QuantityFormatter::stringFromByteCount(789);
    EXPECT_NE(bigBytes.find(u8"789"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"bytes"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(324'123);
    EXPECT_NE(smallBytes.find(u8"316.5"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"KiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(876'456);
    EXPECT_NE(bigBytes.find(u8"855.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"KiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(139'432'174);
    EXPECT_NE(smallBytes.find(u8"133.0"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"MiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(977'341'931);
    EXPECT_NE(bigBytes.find(u8"932.1"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"MiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(412'523'934'812);
    EXPECT_NE(smallBytes.find(u8"384.2"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"GiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(634'839'012'517);
    EXPECT_NE(bigBytes.find(u8"591.3"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"GiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(347'104'194'387'594);
    EXPECT_NE(smallBytes.find(u8"315.7"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"TiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(893'270'909'743'209);
    EXPECT_NE(bigBytes.find(u8"812.4"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"TiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(936'582'932'385'623'894);
    EXPECT_NE(bigBytes.find(u8"831.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PiB"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintSimpleDuration) {
    std::string smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(18));
    EXPECT_NE(smallDuration.find(u8"18"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"seconds"), std::string::npos);

    std::string bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(46));
    EXPECT_NE(bigDuration.find(u8"46"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"seconds"), std::string::npos);

    smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(1'423));
    EXPECT_NE(smallDuration.find(u8"23.7"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"minutes"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(3'390));
    EXPECT_NE(bigDuration.find(u8"56.5"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"minutes"), std::string::npos);

    smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(28'123));
    EXPECT_NE(smallDuration.find(u8"7.8"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"hours"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(67'803));
    EXPECT_NE(bigDuration.find(u8"18.8"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"hours"), std::string::npos);

    smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(248'824));
    EXPECT_NE(smallDuration.find(u8"2.9"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"days"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(432'430));
    EXPECT_NE(bigDuration.find(u8"5.0"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"days"), std::string::npos);

    smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(2'113'029));
    EXPECT_NE(smallDuration.find(u8"3.5"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"weeks"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(4'431'029));
    EXPECT_NE(bigDuration.find(u8"7.3"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"weeks"), std::string::npos);

    smallDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(13'329'083));
    EXPECT_NE(smallDuration.find(u8"5.1"), std::string::npos);
    EXPECT_NE(smallDuration.find(u8"months"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(30'382'901));
    EXPECT_NE(bigDuration.find(u8"11.5"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"months"), std::string::npos);

    bigDuration = QuantityFormatter::stringFromDuration(std::chrono::seconds(130'382'901));
    EXPECT_NE(bigDuration.find(u8"4.1"), std::string::npos);
    EXPECT_NE(bigDuration.find(u8"years"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintFullDuration) {
    std::string smallDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(18), false
    );
    EXPECT_EQ(smallDuration, std::string(u8"18s"));

    std::string bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(46), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"46s"));

    smallDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(1'423), false
    );
    EXPECT_EQ(smallDuration, std::string(u8"23m43s"));

    bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(3'390), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"56m30s"));

    smallDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(28'123), false
    );
    EXPECT_EQ(smallDuration, std::string(u8"7h48m43s"));

    bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(67'803), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"18h50m03s"));

    smallDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(248'824), false
    );
    EXPECT_EQ(smallDuration, std::string(u8"2d 21h07m04s"));

    bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(432'430), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"5d 0h07m10s"));

    smallDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(13'329'083), false
    );
    EXPECT_EQ(smallDuration, std::string(u8"5m1d 18h31m23s"));

    bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(30'382'901), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"11m16d 3h41m41s"));

    bigDuration = QuantityFormatter::stringFromDuration(
      std::chrono::seconds(130'382'901), false
    );
    EXPECT_EQ(bigDuration, std::string(u8"4y1m20d 15h52m21s"));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
