#include <gtest/gtest.h>
#include "time/Clock.hpp"
#include <chrono>
#include <regex>
#include <thread>

using namespace common::time;

/**
 * @brief Test getCurrentDateTimeString returns valid format
 */
TEST(ClockTest, GetCurrentDateTimeString_ValidFormat) {
    const auto dateTimeStr = Clock::getCurrentDateTimeString();
    
    // Expected format: YYYY-MM-DD HH:MM:SS
    const std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(dateTimeStr, pattern));
}

/**
 * @brief Test getCurrentDateTimeString returns non-empty string
 */
TEST(ClockTest, GetCurrentDateTimeString_NonEmpty) {
    const auto dateTimeStr = Clock::getCurrentDateTimeString();
    EXPECT_FALSE(dateTimeStr.empty());
}

/**
 * @brief Test getCompressedCurrentDateTimeString returns valid format
 */
TEST(ClockTest, GetCompressedCurrentDateTimeString_ValidFormat) {
    const auto dateTimeStr = Clock::getCompressedCurrentDateTimeString();
    
    // Expected format: YYYYMMDD_HHMMSS
    const std::regex pattern(R"(\d{8}_\d{6})");
    EXPECT_TRUE(std::regex_match(dateTimeStr, pattern));
}

/**
 * @brief Test getCompressedCurrentDateTimeString returns non-empty string
 */
TEST(ClockTest, GetCompressedCurrentDateTimeString_NonEmpty) {
    const auto dateTimeStr = Clock::getCompressedCurrentDateTimeString();
    EXPECT_FALSE(dateTimeStr.empty());
}

/**
 * @brief Test getCurrentTimestampMs returns positive value
 */
TEST(ClockTest, GetCurrentTimestampMs_PositiveValue) {
    const auto timestamp = Clock::getCurrentTimestampMs();
    EXPECT_GT(timestamp, 0);
}

/**
 * @brief Test getCurrentTimestampMs returns reasonable value (after year 2020)
 */
TEST(ClockTest, GetCurrentTimestampMs_ReasonableValue) {
    const auto timestamp = Clock::getCurrentTimestampMs();
    // Timestamp for 2020-01-01 00:00:00 UTC in milliseconds
    const auto minTimestamp = 1577836800000LL;
    EXPECT_GT(timestamp, minTimestamp);
}

/**
 * @brief Test getCurrentTimestamp returns positive value
 */
TEST(ClockTest, GetCurrentTimestamp_PositiveValue) {
    const auto timestamp = Clock::getCurrentTimestamp();
    EXPECT_GT(timestamp, 0);
}

/**
 * @brief Test getCurrentTimestamp returns reasonable value (after year 2020)
 */
TEST(ClockTest, GetCurrentTimestamp_ReasonableValue) {
    const auto timestamp = Clock::getCurrentTimestamp();
    // Timestamp for 2020-01-01 00:00:00 UTC in seconds
    const auto minTimestamp = 1577836800LL;
    EXPECT_GT(timestamp, minTimestamp);
}

/**
 * @brief Test relationship between millisecond and second timestamps
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
 */
TEST(ClockTest, FormatCurrentTime_StandardFormat) {
    const auto formatted = Clock::formatCurrentTime("%Y-%m-%d %H:%M:%S");
    
    const std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime with date only format
 */
TEST(ClockTest, FormatCurrentTime_DateOnly) {
    const auto formatted = Clock::formatCurrentTime("%Y/%m/%d");
    
    const std::regex pattern(R"(\d{4}/\d{2}/\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime with time only format
 */
TEST(ClockTest, FormatCurrentTime_TimeOnly) {
    const auto formatted = Clock::formatCurrentTime("%H:%M:%S");
    
    const std::regex pattern(R"(\d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(formatted, pattern));
}

/**
 * @brief Test formatCurrentTime returns non-empty string
 */
TEST(ClockTest, FormatCurrentTime_NonEmpty) {
    const auto formatted = Clock::formatCurrentTime("%Y-%m-%d");
    EXPECT_FALSE(formatted.empty());
}

/**
 * @brief Test that consecutive calls return different or equal timestamps (monotonic)
 */
TEST(ClockTest, TimestampMonotonicity_Milliseconds) {
    const auto timestamp1 = Clock::getCurrentTimestampMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const auto timestamp2 = Clock::getCurrentTimestampMs();
    
    EXPECT_GE(timestamp2, timestamp1);
}

/**
 * @brief Test consistency between different time retrieval methods
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
