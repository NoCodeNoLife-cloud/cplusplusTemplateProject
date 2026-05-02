/**
 * @file SimpleDateFormatterTest.cc
 * @brief Unit tests for the SimpleDateFormatter class
 * @details Tests cover core date formatting and parsing functionality including
 *          pattern management, date/time formatting, string parsing, and equality operations.
 */

#include <gtest/gtest.h>
#include "time/SimpleDateFormatter.hpp"
#include <chrono>
#include <stdexcept>
#include <regex>

using namespace common::time;

/**
 * @brief Test constructor with valid pattern
 * @details Verifies that a SimpleDateFormatter can be constructed with a valid pattern
 */
TEST(SimpleDateFormatterTest, Constructor_ValidPattern) {
    EXPECT_NO_THROW(SimpleDateFormatter("%Y-%m-%d"));

    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(formatter.toPattern(), "%Y-%m-%d %H:%M:%S");
}

/**
 * @brief Test constructor with empty pattern throws exception
 * @details Verifies proper error handling for empty pattern strings
 */
TEST(SimpleDateFormatterTest, Constructor_EmptyPattern_ThrowsException) {
    EXPECT_THROW(SimpleDateFormatter(""), std::invalid_argument);
}

/**
 * @brief Test applyPattern with valid pattern
 * @details Verifies that patterns can be changed after construction
 */
TEST(SimpleDateFormatterTest, ApplyPattern_ValidPattern) {
    SimpleDateFormatter formatter("%Y-%m-%d");
    EXPECT_EQ(formatter.toPattern(), "%Y-%m-%d");

    formatter.applyPattern("%Y/%m/%d");
    EXPECT_EQ(formatter.toPattern(), "%Y/%m/%d");
}

/**
 * @brief Test applyPattern with empty pattern throws exception
 * @details Verifies proper error handling when applying empty pattern
 */
TEST(SimpleDateFormatterTest, ApplyPattern_EmptyPattern_ThrowsException) {
    SimpleDateFormatter formatter("%Y-%m-%d");
    EXPECT_THROW(formatter.applyPattern(""), std::invalid_argument);

    // Original pattern should remain unchanged
    EXPECT_EQ(formatter.toPattern(), "%Y-%m-%d");
}

/**
 * @brief Test toPattern returns current pattern
 * @details Verifies pattern retrieval works correctly
 */
TEST(SimpleDateFormatterTest, ToPattern_ReturnsCurrentPattern) {
    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(formatter.toPattern(), "%Y-%m-%d %H:%M:%S");
}

/**
 * @brief Test format with std::tm using date-only pattern
 * @details Verifies formatting of tm structure with date format
 */
TEST(SimpleDateFormatterTest, Format_Tm_DateOnly) {
    const SimpleDateFormatter formatter("%Y-%m-%d");

    std::tm date = {};
    date.tm_year = 2024 - 1900; // Years since 1900
    date.tm_mon = 6 - 1; // Months since January (0-11)
    date.tm_mday = 15;

    const auto result = formatter.format(date);
    EXPECT_EQ(result, "2024-06-15");
}

/**
 * @brief Test format with std::tm using datetime pattern
 * @details Verifies formatting of tm structure with full datetime format
 */
TEST(SimpleDateFormatterTest, Format_Tm_DateTime) {
    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");

    std::tm date = {};
    date.tm_year = 2024 - 1900;
    date.tm_mon = 12 - 1;
    date.tm_mday = 25;
    date.tm_hour = 14;
    date.tm_min = 30;
    date.tm_sec = 45;

    const auto result = formatter.format(date);
    EXPECT_EQ(result, "2024-12-25 14:30:45");
}

/**
 * @brief Test format with std::tm using custom separator
 * @details Verifies formatting with different separators works correctly
 */
TEST(SimpleDateFormatterTest, Format_Tm_CustomSeparator) {
    const SimpleDateFormatter formatter("%Y/%m/%d");

    std::tm date = {};
    date.tm_year = 2024 - 1900;
    date.tm_mon = 1 - 1;
    date.tm_mday = 1;

    const auto result = formatter.format(date);
    EXPECT_EQ(result, "2024/01/01");
}

/**
 * @brief Test format with system_clock::time_point
 * @details Verifies formatting of chrono time_point works correctly
 */
TEST(SimpleDateFormatterTest, Format_TimePoint_Success) {
    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");

    // Create a specific time point: 2024-06-15 10:30:00 UTC
    std::tm timeinfo = {};
    timeinfo.tm_year = 2024 - 1900;
    timeinfo.tm_mon = 6 - 1;
    timeinfo.tm_mday = 15;
    timeinfo.tm_hour = 10;
    timeinfo.tm_min = 30;
    timeinfo.tm_sec = 0;

    const std::time_t time = mktime(&timeinfo);
    const auto tp = std::chrono::system_clock::from_time_t(time);

    const auto result = formatter.format(tp);

    // Verify the format matches expected pattern (may vary by timezone)
    const std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(result, pattern));
}

/**
 * @brief Test parse with date-only string
 * @details Verifies parsing of date-only strings works correctly
 */
TEST(SimpleDateFormatterTest, Parse_DateOnly_Success) {
    const SimpleDateFormatter formatter("%Y-%m-%d");

    const auto result = formatter.parse("2024-06-15");

    EXPECT_EQ(result.tm_year, 2024 - 1900);
    EXPECT_EQ(result.tm_mon, 6 - 1);
    EXPECT_EQ(result.tm_mday, 15);
}

/**
 * @brief Test parse with datetime string
 * @details Verifies parsing of full datetime strings works correctly
 */
TEST(SimpleDateFormatterTest, Parse_DateTime_Success) {
    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");

    const auto result = formatter.parse("2024-12-25 14:30:45");

    EXPECT_EQ(result.tm_year, 2024 - 1900);
    EXPECT_EQ(result.tm_mon, 12 - 1);
    EXPECT_EQ(result.tm_mday, 25);
    EXPECT_EQ(result.tm_hour, 14);
    EXPECT_EQ(result.tm_min, 30);
    EXPECT_EQ(result.tm_sec, 45);
}

/**
 * @brief Test parse with invalid format throws exception
 * @details Verifies proper error handling for malformed date strings
 */
TEST(SimpleDateFormatterTest, Parse_InvalidFormat_ThrowsException) {
    const SimpleDateFormatter formatter("%Y-%m-%d");

    EXPECT_THROW(formatter.parse("2024/06/15"), std::runtime_error);
    EXPECT_THROW(formatter.parse("invalid-date"), std::runtime_error);
}

/**
 * @brief Test parse with extra characters throws exception
 * @details Verifies strict parsing rejects strings with trailing content
 */
TEST(SimpleDateFormatterTest, Parse_ExtraCharacters_ThrowsException) {
    const SimpleDateFormatter formatter("%Y-%m-%d");

    EXPECT_THROW(formatter.parse("2024-06-15 extra"), std::runtime_error);
}

/**
 * @brief Test round-trip: format then parse
 * @details Verifies that formatting and parsing are inverse operations
 */
TEST(SimpleDateFormatterTest, RoundTrip_FormatThenParse) {
    const SimpleDateFormatter formatter("%Y-%m-%d %H:%M:%S");

    std::tm original = {};
    original.tm_year = 2024 - 1900;
    original.tm_mon = 6 - 1;
    original.tm_mday = 15;
    original.tm_hour = 10;
    original.tm_min = 30;
    original.tm_sec = 45;

    const auto formatted = formatter.format(original);
    const auto parsed = formatter.parse(formatted);

    EXPECT_EQ(parsed.tm_year, original.tm_year);
    EXPECT_EQ(parsed.tm_mon, original.tm_mon);
    EXPECT_EQ(parsed.tm_mday, original.tm_mday);
    EXPECT_EQ(parsed.tm_hour, original.tm_hour);
    EXPECT_EQ(parsed.tm_min, original.tm_min);
    EXPECT_EQ(parsed.tm_sec, original.tm_sec);
}

/**
 * @brief Test equals with identical patterns
 * @details Verifies equality comparison for formatters with same pattern
 */
TEST(SimpleDateFormatterTest, Equals_IdenticalPatterns) {
    const SimpleDateFormatter formatter1("%Y-%m-%d");
    const SimpleDateFormatter formatter2("%Y-%m-%d");

    EXPECT_TRUE(formatter1.equals(formatter2));
}

/**
 * @brief Test equals with different patterns
 * @details Verifies inequality detection for formatters with different patterns
 */
TEST(SimpleDateFormatterTest, Equals_DifferentPatterns) {
    const SimpleDateFormatter formatter1("%Y-%m-%d");
    const SimpleDateFormatter formatter2("%Y/%m/%d");

    EXPECT_FALSE(formatter1.equals(formatter2));
}

/**
 * @brief Test hashCode consistency
 * @details Verifies that equal formatters produce the same hash code
 */
TEST(SimpleDateFormatterTest, HashCode_ConsistentForEqualFormatters) {
    const SimpleDateFormatter formatter1("%Y-%m-%d %H:%M:%S");
    const SimpleDateFormatter formatter2("%Y-%m-%d %H:%M:%S");

    EXPECT_EQ(formatter1.hashCode(), formatter2.hashCode());
}

/**
 * @brief Test hashCode difference for different patterns
 * @details Verifies that different patterns typically produce different hash codes
 */
TEST(SimpleDateFormatterTest, HashCode_DifferentForDifferentPatterns) {
    const SimpleDateFormatter formatter1("%Y-%m-%d");
    const SimpleDateFormatter formatter2("%Y/%m/%d");

    EXPECT_NE(formatter1.hashCode(), formatter2.hashCode());
}

/**
 * @brief Test format with various standard patterns
 * @details Verifies support for multiple strftime format specifiers
 */
TEST(SimpleDateFormatterTest, Format_VariousPatterns) {
    std::tm date = {};
    date.tm_year = 2024 - 1900;
    date.tm_mon = 6 - 1;
    date.tm_mday = 15;
    date.tm_hour = 14;
    date.tm_min = 30;
    date.tm_sec = 45;

    // Test year-month-day format
    {
        const SimpleDateFormatter formatter("%Y-%m-%d");
        const auto result = formatter.format(date);
        EXPECT_EQ(result, "2024-06-15");
    }

    // Test month/day/year format
    {
        const SimpleDateFormatter formatter("%m/%d/%Y");
        const auto result = formatter.format(date);
        EXPECT_EQ(result, "06/15/2024");
    }

    // Test time-only format
    {
        const SimpleDateFormatter formatter("%H:%M:%S");
        const auto result = formatter.format(date);
        EXPECT_EQ(result, "14:30:45");
    }
}

/**
 * @brief Test parse with various date formats
 * @details Verifies parsing works with different date string formats
 */
TEST(SimpleDateFormatterTest, Parse_VariousFormats) {
    // Test ISO format
    {
        const SimpleDateFormatter formatter("%Y-%m-%d");
        const auto result = formatter.parse("2024-06-15");
        EXPECT_EQ(result.tm_year, 2024 - 1900);
        EXPECT_EQ(result.tm_mon, 6 - 1);
        EXPECT_EQ(result.tm_mday, 15);
    }

    // Test US format
    {
        const SimpleDateFormatter formatter("%m/%d/%Y");
        const auto result = formatter.parse("06/15/2024");
        EXPECT_EQ(result.tm_year, 2024 - 1900);
        EXPECT_EQ(result.tm_mon, 6 - 1);
        EXPECT_EQ(result.tm_mday, 15);
    }
}