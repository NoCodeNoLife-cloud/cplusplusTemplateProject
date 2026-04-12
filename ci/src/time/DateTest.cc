/**
 * @file DateTest.cc
 * @brief Unit tests for the Date class
 * @details Tests cover date construction, manipulation, comparison, and conversion functionality.
 */

#include <gtest/gtest.h>
#include "time/Date.hpp"
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace common::time;

/**
 * @brief Test default constructor initializes to current time
 * @details Verifies default constructor creates a valid Date with current timestamp
 */
TEST(DateTest, DefaultConstructor_CurrentTime) {
    const Date now;
    const auto timestamp = now.getTime();
    
    // Should be a reasonable modern timestamp (after year 2020)
    const auto minTimestamp = 1577836800000LL; // 2020-01-01 in milliseconds
    EXPECT_GT(timestamp, minTimestamp);
}

/**
 * @brief Test constructor with year, month, day
 * @details Verifies date-only construction works correctly
 */
TEST(DateTest, Constructor_YearMonthDay) {
    const Date date(2024, 1, 15);
    
    EXPECT_EQ(date.getYear(), 2024);
    EXPECT_EQ(date.getMonth(), 1);
    EXPECT_EQ(date.getDay(), 15);
    EXPECT_EQ(date.getHours(), 0);
    EXPECT_EQ(date.getMinutes(), 0);
    EXPECT_EQ(date.getSeconds(), 0);
}

/**
 * @brief Test constructor with full date-time components
 * @details Verifies complete date-time construction works correctly
 */
TEST(DateTest, Constructor_FullDateTime) {
    const Date date(2024, 6, 15, 14, 30, 45);
    
    EXPECT_EQ(date.getYear(), 2024);
    EXPECT_EQ(date.getMonth(), 6);
    EXPECT_EQ(date.getDay(), 15);
    EXPECT_EQ(date.getHours(), 14);
    EXPECT_EQ(date.getMinutes(), 30);
    EXPECT_EQ(date.getSeconds(), 45);
}

/**
 * @brief Test constructor from timestamp
 * @details Verifies construction from millisecond timestamp
 */
TEST(DateTest, Constructor_FromTimestamp) {
    const int64_t timestamp = 1705276800000LL; // 2024-01-15 00:00:00 UTC (approximate)
    const Date date(timestamp);
    
    EXPECT_EQ(date.getTime(), timestamp);
}

/**
 * @brief Test constructor with invalid month throws exception
 * @details Verifies proper error handling for invalid month values
 */
TEST(DateTest, Constructor_InvalidMonth_ThrowsException) {
    EXPECT_THROW(Date(2024, 0, 1), std::invalid_argument);
    EXPECT_THROW(Date(2024, 13, 1), std::invalid_argument);
}

/**
 * @brief Test constructor with invalid day throws exception
 * @details Verifies proper error handling for invalid day values
 */
TEST(DateTest, Constructor_InvalidDay_ThrowsException) {
    EXPECT_THROW(Date(2024, 1, 0), std::invalid_argument);
    EXPECT_THROW(Date(2024, 1, 32), std::invalid_argument);
}

/**
 * @brief Test constructor with invalid hour throws exception
 * @details Verifies proper error handling for invalid hour values
 */
TEST(DateTest, Constructor_InvalidHour_ThrowsException) {
    EXPECT_THROW(Date(2024, 1, 1, 24, 0, 0), std::invalid_argument);
    EXPECT_THROW(Date(2024, 1, 1, -1, 0, 0), std::invalid_argument);
}

/**
 * @brief Test constructor handles leap year correctly
 * @details Verifies February 29 is valid in leap years
 */
TEST(DateTest, Constructor_LeapYear_ValidFeb29) {
    EXPECT_NO_THROW(Date(2024, 2, 29)); // 2024 is a leap year
    
    const Date date(2024, 2, 29);
    EXPECT_EQ(date.getYear(), 2024);
    EXPECT_EQ(date.getMonth(), 2);
    EXPECT_EQ(date.getDay(), 29);
}

/**
 * @brief Test constructor rejects Feb 29 in non-leap year
 * @details Verifies February 29 is invalid in non-leap years
 */
TEST(DateTest, Constructor_NonLeapYear_InvalidFeb29) {
    EXPECT_THROW(Date(2023, 2, 29), std::invalid_argument); // 2023 is not a leap year
}

/**
 * @brief Test clone creates independent copy
 * @details Verifies clone produces an equal but independent Date object
 */
TEST(DateTest, Clone_CreatesIndependentCopy) {
    const Date original(2024, 6, 15, 10, 30, 0);
    const Date cloned = original.clone();
    
    EXPECT_TRUE(original.equals(cloned));
    EXPECT_EQ(original.getTime(), cloned.getTime());
}

/**
 * @brief Test equals method for identical dates
 * @details Verifies equality comparison works correctly
 */
TEST(DateTest, Equals_IdenticalDates) {
    const Date date1(2024, 6, 15, 10, 30, 0);
    const Date date2(2024, 6, 15, 10, 30, 0);
    
    EXPECT_TRUE(date1.equals(date2));
}

/**
 * @brief Test equals method for different dates
 * @details Verifies inequality is detected correctly
 */
TEST(DateTest, Equals_DifferentDates) {
    const Date date1(2024, 6, 15, 10, 30, 0);
    const Date date2(2024, 6, 15, 10, 30, 1);
    
    EXPECT_FALSE(date1.equals(date2));
}

/**
 * @brief Test after method
 * @details Verifies temporal ordering detection (later than)
 */
TEST(DateTest, After_LaterDate) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    
    EXPECT_TRUE(later.after(earlier));
    EXPECT_FALSE(earlier.after(later));
}

/**
 * @brief Test before method
 * @details Verifies temporal ordering detection (earlier than)
 */
TEST(DateTest, Before_EarlierDate) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    
    EXPECT_TRUE(earlier.before(later));
    EXPECT_FALSE(later.before(earlier));
}

/**
 * @brief Test getTime returns correct timestamp
 * @details Verifies millisecond timestamp retrieval
 */
TEST(DateTest, GetTime_ReturnsTimestamp) {
    const Date date(2024, 1, 15, 12, 0, 0);
    const auto timestamp = date.getTime();
    
    // Should be a positive value representing milliseconds since epoch
    EXPECT_GT(timestamp, 0);
}

/**
 * @brief Test getter methods return correct components
 * @details Verifies all component getters work correctly
 */
TEST(DateTest, Getters_AllComponents) {
    const Date date(2024, 6, 15, 14, 30, 45);
    
    EXPECT_EQ(date.getYear(), 2024);
    EXPECT_EQ(date.getMonth(), 6);
    EXPECT_EQ(date.getDay(), 15);
    EXPECT_EQ(date.getHours(), 14);
    EXPECT_EQ(date.getMinutes(), 30);
    EXPECT_EQ(date.getSeconds(), 45);
}

/**
 * @brief Test toString returns formatted string
 * @details Verifies string representation format "YYYY-MM-DD HH:MM:SS"
 */
TEST(DateTest, ToString_FormattedString) {
    const Date date(2024, 6, 15, 14, 30, 45);
    const auto str = date.toString();
    
    EXPECT_EQ(str, "2024-06-15 14:30:45");
}

/**
 * @brief Test hashCode consistency
 * @details Verifies equal dates produce same hash code
 */
TEST(DateTest, HashCode_ConsistentForEqualDates) {
    const Date date1(2024, 6, 15, 10, 30, 0);
    const Date date2(2024, 6, 15, 10, 30, 0);
    
    EXPECT_EQ(date1.hashCode(), date2.hashCode());
}

/**
 * @brief Test hashCode difference for different dates
 * @details Verifies different dates typically produce different hash codes
 */
TEST(DateTest, HashCode_DifferentForDifferentDates) {
    const Date date1(2024, 1, 1);
    const Date date2(2024, 12, 31);
    
    EXPECT_NE(date1.hashCode(), date2.hashCode());
}

/**
 * @brief Test equality operator
 * @details Verifies operator== works correctly
 */
TEST(DateTest, OperatorEquals) {
    const Date date1(2024, 6, 15, 10, 30, 0);
    const Date date2(2024, 6, 15, 10, 30, 0);
    const Date date3(2024, 6, 15, 10, 30, 1);
    
    EXPECT_TRUE(date1 == date2);
    EXPECT_FALSE(date1 == date3);
}

/**
 * @brief Test inequality operator
 * @details Verifies operator!= works correctly
 */
TEST(DateTest, OperatorNotEquals) {
    const Date date1(2024, 6, 15, 10, 30, 0);
    const Date date2(2024, 6, 15, 10, 30, 1);
    
    EXPECT_TRUE(date1 != date2);
    EXPECT_FALSE(date1 != date1);
}

/**
 * @brief Test less-than operator
 * @details Verifies operator< works correctly
 */
TEST(DateTest, OperatorLessThan) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    
    EXPECT_TRUE(earlier < later);
    EXPECT_FALSE(later < earlier);
    EXPECT_FALSE(earlier < earlier);
}

/**
 * @brief Test less-than-or-equal operator
 * @details Verifies operator<= works correctly
 */
TEST(DateTest, OperatorLessThanOrEqual) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    const Date same(2024, 1, 1);
    
    EXPECT_TRUE(earlier <= later);
    EXPECT_TRUE(earlier <= same);
    EXPECT_FALSE(later <= earlier);
}

/**
 * @brief Test greater-than operator
 * @details Verifies operator> works correctly
 */
TEST(DateTest, OperatorGreaterThan) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    
    EXPECT_TRUE(later > earlier);
    EXPECT_FALSE(earlier > later);
    EXPECT_FALSE(earlier > earlier);
}

/**
 * @brief Test greater-than-or-equal operator
 * @details Verifies operator>= works correctly
 */
TEST(DateTest, OperatorGreaterThanOrEqual) {
    const Date earlier(2024, 1, 1);
    const Date later(2024, 12, 31);
    const Date same(2024, 12, 31);
    
    EXPECT_TRUE(later >= earlier);
    EXPECT_TRUE(later >= same);
    EXPECT_FALSE(earlier >= later);
}

/**
 * @brief Test comparison across different months
 * @details Verifies correct ordering when months differ
 */
TEST(DateTest, Comparison_DifferentMonths) {
    const Date jan(2024, 1, 15);
    const Date dec(2024, 12, 15);
    
    EXPECT_TRUE(jan < dec);
    EXPECT_TRUE(dec > jan);
}

/**
 * @brief Test comparison across different years
 * @details Verifies correct ordering when years differ
 */
TEST(DateTest, Comparison_DifferentYears) {
    const Date year2023(2023, 12, 31);
    const Date year2024(2024, 1, 1);
    
    EXPECT_TRUE(year2023 < year2024);
    EXPECT_TRUE(year2024 > year2023);
}

/**
 * @brief Test constructor with edge case: last day of month
 * @details Verifies correct handling of month boundaries
 */
TEST(DateTest, Constructor_LastDayOfMonth) {
    EXPECT_NO_THROW(Date(2024, 1, 31));
    EXPECT_NO_THROW(Date(2024, 4, 30));
    
    const Date date(2024, 1, 31);
    EXPECT_EQ(date.getDay(), 31);
}

/**
 * @brief Test constructor with edge case: first day of month
 * @details Verifies correct handling of month start
 */
TEST(DateTest, Constructor_FirstDayOfMonth) {
    EXPECT_NO_THROW(Date(2024, 1, 1));
    
    const Date date(2024, 1, 1);
    EXPECT_EQ(date.getDay(), 1);
    EXPECT_EQ(date.getMonth(), 1);
}
