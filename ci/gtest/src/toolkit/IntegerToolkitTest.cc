/**
 * @file IntegerToolkitTest.cc
 * @brief Unit tests for the IntegerToolkit class
 * @details The tests cover Roman numeral conversion, power-of-two checking, and bit counting.
 */

#include <optional>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

#include <cppforge/toolkit/IntegerToolkit.hpp>

using namespace cppforge::toolkit;

/**
 * @brief Test fixture for IntegerToolkitTest tests
 */
class IntegerToolkitTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test intToRoman with basic numbers
 * @details Verifies correct Roman numeral representation for standard values
 */
TEST_F(IntegerToolkitTest, IntToRoman_BasicNumbers)
{
    EXPECT_EQ(IntegerToolkit::intToRoman(1), "I");
    EXPECT_EQ(IntegerToolkit::intToRoman(4), "IV");
    EXPECT_EQ(IntegerToolkit::intToRoman(9), "IX");
    EXPECT_EQ(IntegerToolkit::intToRoman(58), "LVIII");
    EXPECT_EQ(IntegerToolkit::intToRoman(1994), "MCMXCIV");
}

/**
 * @brief Test intToRoman with edge cases
 * @details Verifies boundary values (1 and 3999)
 */
TEST_F(IntegerToolkitTest, IntToRoman_EdgeCases)
{
    EXPECT_EQ(IntegerToolkit::intToRoman(1), "I");
    EXPECT_EQ(IntegerToolkit::intToRoman(3999), "MMMCMXCIX");
}

/**
 * @brief Test intToRoman with invalid range throws exception
 * @details Verifies proper error handling for out-of-range values
 */
TEST_F(IntegerToolkitTest, IntToRoman_InvalidRange_ThrowsException)
{
    EXPECT_THROW(static_cast<void>(IntegerToolkit::intToRoman(0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(IntegerToolkit::intToRoman(4000)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(IntegerToolkit::intToRoman(-1)), std::out_of_range);
}

/**
 * @brief Test tryIntToRoman with valid numbers
 * @details Verifies safe conversion returns std::optional with value
 */
TEST_F(IntegerToolkitTest, TryIntToRoman_ValidNumbers)
{
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

/**
 * @brief Test tryIntToRoman with invalid range
 * @details Verifies safe conversion returns std::nullopt for invalid input
 */
TEST_F(IntegerToolkitTest, TryIntToRoman_InvalidRange)
{
    const auto result1 = IntegerToolkit::tryIntToRoman(0);
    const auto result2 = IntegerToolkit::tryIntToRoman(4000);
    const auto result3 = IntegerToolkit::tryIntToRoman(-1);

    EXPECT_FALSE(result1.has_value());
    EXPECT_FALSE(result2.has_value());
    EXPECT_FALSE(result3.has_value());
}

/**
 * @brief Test romanToInt with basic numerals
 * @details Verifies correct integer conversion from Roman numeral strings
 */
TEST_F(IntegerToolkitTest, RomanToInt_BasicNumerals)
{
    EXPECT_EQ(IntegerToolkit::romanToInt("I"), 1);
    EXPECT_EQ(IntegerToolkit::romanToInt("IV"), 4);
    EXPECT_EQ(IntegerToolkit::romanToInt("IX"), 9);
    EXPECT_EQ(IntegerToolkit::romanToInt("LVIII"), 58);
    EXPECT_EQ(IntegerToolkit::romanToInt("MCMXCIV"), 1994);
}

/**
 * @brief Test romanToInt case insensitivity
 * @details Verifies parsing works regardless of letter case
 */
TEST_F(IntegerToolkitTest, RomanToInt_CaseInsensitive)
{
    EXPECT_EQ(IntegerToolkit::romanToInt("i"), 1);
    EXPECT_EQ(IntegerToolkit::romanToInt("iv"), 4);
    EXPECT_EQ(IntegerToolkit::romanToInt("mcmxciv"), 1994);
    EXPECT_EQ(IntegerToolkit::romanToInt("McmXcIv"), 1994);
}

/**
 * @brief Test romanToInt with empty string throws exception
 * @details Verifies proper error handling for empty input
 */
TEST_F(IntegerToolkitTest, RomanToInt_EmptyString_ThrowsException)
{
    EXPECT_THROW(static_cast<void>(IntegerToolkit::romanToInt("")), std::invalid_argument);
}

/**
 * @brief Test romanToInt with invalid numerals throws exception
 * @details Verifies proper error handling for malformed Roman numerals
 */
TEST_F(IntegerToolkitTest, RomanToInt_InvalidNumeral_ThrowsException)
{
    EXPECT_THROW(static_cast<void>(IntegerToolkit::romanToInt("IIII")), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(IntegerToolkit::romanToInt("ABC")), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(IntegerToolkit::romanToInt("VV")), std::invalid_argument);
}

/**
 * @brief Test round-trip conversion: int to Roman and back
 * @details Verifies bidirectional conversion preserves original integer value
 */
TEST_F(IntegerToolkitTest, RoundTrip_IntToRomanAndBack)
{
    const int test_values[] = {1, 4, 9, 58, 100, 400, 900, 1994, 3999};

    for (const auto value : test_values)
    {
        const auto roman = IntegerToolkit::intToRoman(value);
        const auto converted_back = IntegerToolkit::romanToInt(roman);
        EXPECT_EQ(converted_back, value);
    }
}

/**
 * @brief Test round-trip conversion: Roman to int and back
 * @details Verifies bidirectional conversion preserves original Roman numeral string
 */
TEST_F(IntegerToolkitTest, RoundTrip_RomanToIntAndBack)
{
    const auto test_romans = {"I", "IV", "IX", "LVIII", "C", "CD", "CM", "MCMXCIV", "MMMCMXCIX"};

    for (const auto& roman : test_romans)
    {
        const auto value = IntegerToolkit::romanToInt(roman);
        const auto converted_back = IntegerToolkit::intToRoman(value);
        EXPECT_EQ(converted_back, roman);
    }
}

/**
 * @brief Test isPowerOfTwo with actual powers of two
 * @details Verifies correct identification of 2^n values
 */
TEST_F(IntegerToolkitTest, IsPowerOfTwo_PowersOfTwo)
{
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(1)); // 2^0
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(2)); // 2^1
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(4)); // 2^2
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(8)); // 2^3
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(16)); // 2^4
    EXPECT_TRUE(IntegerToolkit::isPowerOfTwo(1024)); // 2^10
}

/**
 * @brief Test isPowerOfTwo with non-powers of two
 * @details Verifies false is returned for non-power-of-two values
 */
TEST_F(IntegerToolkitTest, IsPowerOfTwo_NotPowersOfTwo)
{
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(0));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(3));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(5));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(6));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(7));
}

/**
 * @brief Test isPowerOfTwo with negative numbers
 * @details Verifies negative values are correctly identified as non-powers-of-two
 */
TEST_F(IntegerToolkitTest, IsPowerOfTwo_NegativeNumbers)
{
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-1));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-2));
    EXPECT_FALSE(IntegerToolkit::isPowerOfTwo(-4));
}

/**
 * @brief Test countSetBits with basic numbers
 * @details Verifies correct counting of 1-bits in binary representation
 */
TEST_F(IntegerToolkitTest, CountSetBits_BasicNumbers)
{
    EXPECT_EQ(IntegerToolkit::countSetBits(0), 0);
    EXPECT_EQ(IntegerToolkit::countSetBits(1), 1); // binary: 1
    EXPECT_EQ(IntegerToolkit::countSetBits(2), 1); // binary: 10
    EXPECT_EQ(IntegerToolkit::countSetBits(3), 2); // binary: 11
    EXPECT_EQ(IntegerToolkit::countSetBits(7), 3); // binary: 111
    EXPECT_EQ(IntegerToolkit::countSetBits(8), 1); // binary: 1000
}

/**
 * @brief Test countSetBits with powers of two
 * @details Verifies powers of two always have exactly one set bit
 */
TEST_F(IntegerToolkitTest, CountSetBits_PowersOfTwo)
{
    // Powers of two should always have exactly 1 set bit
    EXPECT_EQ(IntegerToolkit::countSetBits(1), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(2), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(4), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(8), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(16), 1);
    EXPECT_EQ(IntegerToolkit::countSetBits(1024), 1);
}

/**
 * @brief Test countSetBits with all bits set
 * @details Verifies correct counting for maximum bit patterns
 */
TEST_F(IntegerToolkitTest, CountSetBits_AllBitsSet)
{
    EXPECT_EQ(IntegerToolkit::countSetBits(0xFF), 8); // 8 bits set
    EXPECT_EQ(IntegerToolkit::countSetBits(0xFFFF), 16); // 16 bits set
}

/**
 * @brief Test countSetBits with negative numbers (bug fix regression)
 * @details Verifies negative numbers don't cause infinite loop
 */
TEST_F(IntegerToolkitTest, CountSetBits_NegativeNumbers)
{
    EXPECT_EQ(IntegerToolkit::countSetBits(-1), 32); // All 32 bits set in two's complement
    EXPECT_EQ(IntegerToolkit::countSetBits(-2), 31);
    EXPECT_EQ(IntegerToolkit::countSetBits(-3), 31);
    EXPECT_EQ(IntegerToolkit::countSetBits(static_cast<int32_t>(0x80000000)), 1); // INT_MIN
}

/**
 * @brief Test countSetBits with other common values
 * @details Verifies correctness for various inputs
 */
TEST_F(IntegerToolkitTest, CountSetBits_AdditionalValues)
{
    EXPECT_EQ(IntegerToolkit::countSetBits(0xFFFFFFFF), 32);
    EXPECT_EQ(IntegerToolkit::countSetBits(0xF0F0F0F0), 16);
    EXPECT_EQ(IntegerToolkit::countSetBits(0x11111111), 8);
}

/**
 * @brief Test that constructor is deleted (compile-time check)
 * @details Verifies IntegerToolkit cannot be instantiated as it's a utility class
 */
TEST_F(IntegerToolkitTest, ConstructorDeleted)
{
    static_assert(std::is_constructible_v<IntegerToolkit> == false,
                  "IntegerToolkit should not be constructible");
}
