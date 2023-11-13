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

#include "Nuclex/Support/Text/RollingLogger.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, RollingLoggerCanBeDefaultConstructed) {
    EXPECT_NO_THROW(
      RollingLogger logger;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, IsLoggingReturnsTrue) {
    Logger logger;
    EXPECT_TRUE(logger.IsLogging());

    // Negative test
    EXPECT_FALSE(Logger::Null.IsLogging());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerAcceptsPlainLogEntries) {
    RollingLogger logger;
    EXPECT_NO_THROW(
      logger.Inform(u8"This is a harmless message providing information");
      logger.Warn(u8"This is a warning indicating something is not optimal");
      logger.Complain(u8"This is an error and some action has failed completely");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryCanBeExtracted) {
    RollingLogger logger;

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 0U);

    logger.Inform(u8"This is a harmless message providing information");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_TRUE(history[0].find(u8"This is a harmless message") != std::string::npos);

    logger.Warn(u8"This is a warning indicating something is not optimal");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 2U);
    EXPECT_TRUE(history[1].find(u8"This is a warning") != std::string::npos);

    logger.Complain(u8"This is an error and some action has failed completely");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 3U);
    EXPECT_TRUE(history[2].find(u8"This is an error") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryKeepsMostRecentLines) {
    RollingLogger logger(2); // 2 lines history length

    logger.Inform(u8"First line");
    logger.Inform(u8"Second line");
    logger.Inform(u8"Third line");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 2U);
    EXPECT_TRUE(history[0].find(u8"Second line") != std::string::npos);
    EXPECT_TRUE(history[1].find(u8"Third line") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryCanBeCleared) {
    RollingLogger logger;

    logger.Inform(u8"Test");
    logger.Inform(u8"Test");
    logger.Clear();
    logger.Inform(u8"First line");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_TRUE(history[0].find(u8"First line") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendIntegers) {
    RollingLogger logger;

    logger.Append(12345);
    logger.Append(u8"Hello");
    logger.Append(54321);
    logger.Inform(u8"World");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_TRUE(history[0].find(u8"12345Hello54321World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendFloatingPointValues) {
    RollingLogger logger;

    logger.Append(1.25f);
    logger.Append(u8"Hello");
    logger.Append(0.875);
    logger.Inform(u8"World");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_TRUE(history[0].find(u8"1.25Hello0.875World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogLinesCanBeIndented) {
    RollingLogger logger;

    logger.Inform(u8"Saving configuration {");
    {
      Logger::IndentationScope configurationLogScope(logger);

      logger.Append(u8"ResolutionX = ");
      logger.Append(1920);
      logger.Inform(std::string());

      logger.Append(u8"ResolutionY = ");
      logger.Append(1080);
      logger.Inform(std::string());
    }
    logger.Inform(u8"}");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 4U);

    // Figure out which column log messages begin in
    std::string::size_type logTextStartColumn = history[0].find(u8"Saving configuration {");
    ASSERT_NE(logTextStartColumn, std::string::npos);

    // Check the indentation by looked at the first few characters of each line's message
    {
      EXPECT_EQ(history[0].at(logTextStartColumn + 0), 'S'); // ...aving Configuration

      EXPECT_EQ(history[1].at(logTextStartColumn + 0), ' ');
      EXPECT_EQ(history[1].at(logTextStartColumn + 1), ' ');
      EXPECT_EQ(history[1].at(logTextStartColumn + 2), 'R'); // ...esolutionX

      EXPECT_EQ(history[2].at(logTextStartColumn + 0), ' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 1), ' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 2), 'R'); // ...resolutionY

      EXPECT_EQ(history[3].at(logTextStartColumn + 0), '}');
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, IndendationAffectsLineBeingFormed) {
    RollingLogger logger;

    logger.Inform("Not indented");

    // Start the line appended, but then stop indentation before finalizing it
    // The logger has to remove indentation before the text without destroying it
    {
      Logger::IndentationScope configurationLogScope(logger);
      logger.Append(12345);
    }
    logger.Warn("Warning");

    // Start the line unindented, but then begin indentation before finalizing it
    // The logger has to insert indentation before the text without destroying it
    logger.Append(54321);
    {
      Logger::IndentationScope configurationLogScope(logger);
      logger.Complain("Error");
    }

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 3U);

    // Figure out which column log messages begin in
    std::string::size_type logTextStartColumn = history[0].find(u8"Not indented");
    ASSERT_NE(logTextStartColumn, std::string::npos);

    // Check the indentation by looked at the first few characters of each line's message
    {
      EXPECT_EQ(history[0].at(logTextStartColumn + 0), 'N'); // ...aving Configuration

      EXPECT_EQ(history[1].at(logTextStartColumn + 0), '1');
      EXPECT_EQ(history[1].at(logTextStartColumn + 1), '2');
      EXPECT_EQ(history[1].at(logTextStartColumn + 2), '3');
      EXPECT_EQ(history[1].at(logTextStartColumn + 3), '4');
      EXPECT_EQ(history[1].at(logTextStartColumn + 4), '5');
      EXPECT_EQ(history[1].at(logTextStartColumn + 5), 'W');

      EXPECT_EQ(history[2].at(logTextStartColumn + 0), ' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 1), ' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 2), '5');
      EXPECT_EQ(history[2].at(logTextStartColumn + 3), '4');
      EXPECT_EQ(history[2].at(logTextStartColumn + 4), '3');
      EXPECT_EQ(history[2].at(logTextStartColumn + 5), '2');
      EXPECT_EQ(history[2].at(logTextStartColumn + 6), '1');
      EXPECT_EQ(history[2].at(logTextStartColumn + 7), 'E');
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
