#include <gtest/gtest.h>
#include "toolkit/IntegerToolkit.hpp"
#include <string>
#include <optional>
#include <stdexcept>

using namespace common::toolkit;

// Test intToRoman functionality
TEST(IntegerToolkitTest, IntToRoman_BasicNumbers) {
    EXPECT_EQ(IntegerToolkit::intToRoman(1), "I");
    EXPECT_EQ(IntegerToolkit::intToRoman(4), "IV");
    EXPECT_EQ(IntegerToolkit::intToRoman(9), "IX");
    EXPECT_EQ(IntegerToolkit::intToRoman(58), "LVIII");
    EXPECT_EQ(IntegerToolkit::intToRoman(1994), "MCMXCIV");
}

TEST(IntegerToolkitTest, IntToRoman_EdgeCases) {
    EXPECT_EQ(IntegerToolkit::intToRoman(1), "I");
    EXPECT_EQ(IntegerToolkit::intToRoman(3999), "MMMCMXCIX");
}

TEST(IntegerToolkitTest, IntToRoman_InvalidRange_ThrowsException) {
    EXPECT_THROW(IntegerToolkit::intToRoman(0), std::out_of_range);
    EXPECT_THROW(IntegerToolkit::intToRoman(4000), std::out_of_range);
    EXPECT_THROW(IntegerToolkit::intToRoman(-1), std::out_of_range);
}

// Test tryIntToRoman functionality
TEST(IntegerToolkitTest, TryIntToRoman_ValidNumbers) {
    const auto result1 = IntegerToolkit::tryIntToRoman(1);
    const auto result2 = IntegerToolkit::tryIntToRoman(58);
    const auto result3 = IntegerToolkit::tryIntToRoman(1994);

    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "I");

    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "LVIII");

    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "MCMXCIV");
}

TEST(IntegerToolkitTest, TryIntToRoman_InvalidRange) {
    const auto result1 = IntegerToolkit::tryIntToRoman(0);
    const auto result2 = IntegerToolkit::tryIntToRoman(4000);
    const auto result3 = IntegerToolkit::tryIntToRoman(-1);

    EXPECT_FALSE(result1.has_value());
    EXPECT_FALSE(result2.has_value());
    EXPECT_FALSE(result3.has_value());
}

// Test romanToInt functionality
TEST(IntegerToolkitTest, RomanToInt_BasicNumerals) {
    EXPECT_EQ(IntegerToolkit::romanToInt("I"), 1);
    EXPECT_EQ(IntegerToolkit::romanToInt("IV"), 4);
    EXPECT_EQ(IntegerToolkit::romanToInt("IX"), 9);
    EXPECT_EQ(IntegerToolkit::romanToInt("LVIII"), 58);
    EXPECT_EQ(IntegerToolkit::romanToInt("MCMXCIV"), 1994);
}

TEST(IntegerToolkitTest, RomanToInt_CaseInsensitive) {
    EXPECT_EQ(IntegerToolkit::romanToInt("i"), 1);
    EXPECT_EQ(IntegerToolkit::romanToInt("iv"), 4);
    EXPECT_EQ(IntegerToolkit::romanToInt("mcmxciv"), 1994);
    EXPECT_EQ(IntegerToolkit::romanToInt("McmXcIv"), 1994);
}

TEST(IntegerToolkitTest, RomanToInt_EmptyString_ThrowsException) {
    EXPECT_THROW(IntegerToolkit::romanToInt(""), std::invalid_argument);
}

TEST(IntegerToolkitTest, RomanToInt_InvalidNumeral_ThrowsException) {
    EXPECT_THROW(IntegerToolkit::romanToInt("IIII"), std::invalid_argument);
    EXPECT_THROW(IntegerToolkit::romanToInt("ABC"), std::invalid_argument);
    EXPECT_THROW(IntegerToolkit::romanToInt("VV"), std::invalid_argument);
}

// Test round-trip conversion
TEST(IntegerToolkitTest, RoundTrip_IntToRomanAndBack) {
    const int test_values[] = {1, 4, 9, 58, 100, 400, 900, 1994, 3999};

    for (const auto value : test_values) {
        const auto roman = IntegerToolkit::intToRoman(value);
        const auto converted_back = IntegerToolkit::romanToInt(roman);
        EXPECT_EQ(converted_back, value);
    }
}

TEST(IntegerToolkitTest, RoundTrip_RomanToIntAndBack) {
    const auto test_romans = {"I", "IV", "IX", "LVIII", "C", "CD", "CM", "MCMXCIV", "MMMCMXCIX"};

    for (const auto &roman : test_romans) {
        const auto value = IntegerToolkit::romanToInt(roman);
        const auto converted_back = IntegerToolkit::intToRoman(value);
        EXPECT_EQ(converted_back, roman);
    }
}

// Test isPowerOfTwo functionality
TEST(IntegerToolkitTest, IsPowerOfTwo_PowersOfTwo) {
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(1));   // 2^0
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(2));   // 2^1
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(4));   // 2^2
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(8));   // 2^3
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(16));  // 2^4
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(1024)); // 2^10
}

TEST(IntegerToolkitTest, IsPowerOfTwo_NotPowersOfTwo) {
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(0));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(3));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(5));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(6));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(7));
}

TEST(IntegerToolkitTest, IsPowerOfTwo_NegativeNumbers) {
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-1));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-2));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-4));
}

// Test countSetBits functionality
TEST(IntegerToolkitTest, CountSetBits_BasicNumbers) {
    EXPECT_EQ(IntegerToolkit::countSetBits(0), 0);
    EXPECT_EQ(IntegerToolkit::countSetBits(1), 1);  // binary: 1
    EXPECT_EQ(IntegerToolkit::countSetBits(2), 1);  // binary: 10
    EXPECT_EQ(IntegerToolkit::countSetBits(3), 2);  // binary: 11
    EXPECT_EQ(IntegerToolkit::countSetBits(7), 3);  // binary: 111
    EXPECT_EQ(IntegerToolkit::countSetBits(8), 1);  // binary: 1000
}

TEST(IntegerToolkitTest, CountSetBits_PowersOfTwo) {
    // Powers of two should always have exactly 1 set bit
    EXPECT_EQ(IntegerToolkit::countSetBits(1), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(2), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(4), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(8), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(16), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(1024), 1);
}

TEST(IntegerToolkitTest, CountSetBits_AllBitsSet) {
    EXPECT_EQ(IntegerToolkit::countSetBits(0xFF), 8);      // 8 bits set
    EXPECT_EQ(IntegerToolkit::countSetBits(0xFFFF), 16);   // 16 bits set
}

// Test that constructor is deleted (compile-time check)
TEST(IntegerToolkitTest, ConstructorDeleted) {
    static_assert(std::is_constructible_v<IntegerToolkit> == false,
                  "IntegerToolkit should not be constructible");
}
