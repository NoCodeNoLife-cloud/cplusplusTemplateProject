/**
 * @file ClockTest.cc
 * @brief Unit tests for the Clock class
 * @details Tests cover core time-related functionality including date/time formatting,
 *          timestamp retrieval, and custom format support.
 */

#include <gtest/gtest.h>
#include "time/Clock.hpp"
#include <chrono>
#include <regex>
#include <thread>

using namespace common::time;

/**
 * @brief Test getCurrentDateTimeString returns valid format
 * @details Verifies that the returned string matches the expected "YYYY-MM-DD HH:MM:SS" pattern
 */
TEST(ClockTest, GetCurrentDateTimeString_ValidFormat) {
    const auto dateTimeStr = Clock::getCurrentDateTimeString();
    
    // Expected format: YYYY-MM-DD HH:MM:SS
    const std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(dateTimeStr, pattern));
}

/**
 * @brief Test getCurrentDateTimeString returns non-empty string
 * @details Ensures the method always returns a valid, non-empty result
 */
TEST(ClockTest, GetCurrentDateTimeString_NonEmpty) {
    const auto dateTimeStr = Clock::getCurrentDateTimeString();
    EXPECT_FALSE(dateTimeStr.empty());
}

/**
 * @brief Test getCompressedCurrentDateTimeString returns valid format
 * @details Verifies that the returned string matches the expected "YYYYMMDD_HHMMSS" pattern
 */
TEST(ClockTest, GetCompressedCurrentDateTimeString_ValidFormat) {
    const auto dateTimeStr = Clock::getCompressedCurrentDateTimeString();
    
    // Expected format: YYYYMMDD_HHMMSS
    const std::regex pattern(R"(\d{8}_\d{6})");
    EXPECT_TRUE(std::regex_match(dateTimeStr, pattern));
}

/**
 * @brief Test getCompressedCurrentDateTimeString returns non-empty string
 * @details Ensures the method always returns a valid, non-empty result
 */
TEST(ClockTest, GetCompressedCurrentDateTimeString_NonEmpty) {
    const auto dateTimeStr = Clock::getCompressedCurrentDateTimeString();
    EXPECT_FALSE(dateTimeStr.empty());
}

/**
 * @brief Test getCurrentTimestampMs returns positive value
 * @details Verifies that millisecond timestamp is greater than zero
 */
TEST(ClockTest, GetCurrentTimestampMs_PositiveValue) {
    const auto timestamp = Clock::getCurrentTimestampMs();
    EXPECT_GT(timestamp, 0);
}

/**
 * @brief Test getCurrentTimestampMs returns reasonable value (after year 2020)
 * @details Validates that the timestamp represents a realistic modern time value
 */
TEST(ClockTest, GetCurrentTimestampMs_ReasonableValue) {
    const auto timestamp = Clock::getCurrentTimestampMs();
    // Timestamp for 2020-01-01 00:00:00 UTC in milliseconds
    const auto minTimestamp = 1577836800000LL;
    EXPECT_GT(timestamp, minTimestamp);
}

/**
 * @brief Test getCurrentTimestamp returns positive value
 * @details Verifies that second timestamp is greater than zero
 */
TEST(ClockTest, GetCurrentTimestamp_PositiveValue) {
    const auto timestamp = Clock::getCurrentTimestamp();
    EXPECT_GT(timestamp, 0);
}

/**
 * @brief Test getCurrentTimestamp returns reasonable value (after year 2020)
 * @details Validates that the timestamp represents a realistic modern time value
 */
TEST(ClockTest, GetCurrentTimestamp_ReasonableValue) {
    const auto timestamp = Clock::getCurrentTimestamp();
    // Timestamp for 2020-01-01 00:00:00 UTC in seconds
    const auto minTimestamp = 1577836800LL;
    EXPECT_GT(timestamp, minTimestamp);
}

/**
 * @brief Test relationship between millisecond and second timestamps
 * @details Verifies consistency between ms and s timestamp values
 */
TEST(ClockTest, TimestampRelationship_MsAndSeconds) {
    const auto timestampMs = Clock::getCurrentTimestampMs();
    const auto timestampSec = Clock::getCurrentTimestamp();
    
    // Millisecond timestamp divided by 1000 should be close to second timestamp
    const auto msToSec = timestampMs / 1000;
    EXPECT_NEAR(msToSec, timestampSec, 1);
}

/**
 * @brief Test formatCurrentTime with standard format
 * @details Tests custom formatting with "%Y-%m-%d %H:%M:%S" pattern
 */
TEST(ClockTest, FormatCurrentTime_StandardFormat) {
    const auto formatted = Clock::formatCurrentTime("%Y-%m-%d %H:%M:%S");
    
    const std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime with date only format
 * @details Tests custom formatting with "%Y/%m/%d" pattern
 */
TEST(ClockTest, FormatCurrentTime_DateOnly) {
    const auto formatted = Clock::formatCurrentTime("%Y/%m/%d");
    
    const std::regex pattern(R"(\d{4}/\d{2}/\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime with time only format
 * @details Tests custom formatting with "%H:%M:%S" pattern
 */
TEST(ClockTest, FormatCurrentTime_TimeOnly) {
    const auto formatted = Clock::formatCurrentTime("%H:%M:%S");
    
    const std::regex pattern(R"(\d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime returns non-empty string
 * @details Ensures the method always returns a valid, non-empty result
 */
TEST(ClockTest, FormatCurrentTime_NonEmpty) {
    const auto formatted = Clock::formatCurrentTime("%Y-%m-%d");
    EXPECT_FALSE(formatted.empty());
}

/**
 * @brief Test that consecutive calls return different or equal timestamps (monotonic)
 * @details Verifies temporal monotonicity by ensuring later calls don't return earlier times
 */
TEST(ClockTest, TimestampMonotonicity_Milliseconds) {
    const auto timestamp1 = Clock::getCurrentTimestampMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const auto timestamp2 = Clock::getCurrentTimestampMs();
    
    EXPECT_GE(timestamp2, timestamp1);
}

/**
 * @brief Test consistency between different time retrieval methods
 * @details Validates that all time methods return coherent, valid results simultaneously
 */
TEST(ClockTest, Consistency_BetweenMethods) {
    const auto dateTimeStr = Clock::getCurrentDateTimeString();
    const auto compressedStr = Clock::getCompressedCurrentDateTimeString();
    const auto timestamp = Clock::getCurrentTimestamp();
    
    // All should return valid values
    EXPECT_FALSE(dateTimeStr.empty());
    EXPECT_FALSE(compressedStr.empty());
    EXPECT_GT(timestamp, 0);
}
