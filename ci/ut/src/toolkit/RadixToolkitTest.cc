/**
 * @file RadixToolkitTest.cc
 * @brief Unit tests for the RadixToolkit class
 * @details Tests cover base conversion operations including instance-based and static methods,
 *          various numeric bases (binary, octal, hexadecimal), error handling, and edge cases.
 */

#include <gtest/gtest.h>
#include "toolkit/RadixToolkit.hpp"
#include <limits>

using namespace common::toolkit;

/**
 * @brief Test default constructor with standard configuration
 * @details Verifies toolkit initializes with base-10 and standard charset
 */
TEST(RadixToolkitTest, Constructor_DefaultConfiguration) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.get_default_base(), 10);
    EXPECT_EQ(toolkit.get_charset(), "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

/**
 * @brief Test constructor with custom base and charset
 * @details Verifies custom configuration is properly stored
 */
TEST(RadixToolkitTest, Constructor_CustomConfiguration) {
    RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.get_default_base(), 16);
    EXPECT_EQ(toolkit.get_charset(), "0123456789ABCDEF");
}

/**
 * @brief Test constructor with invalid base throws exception
 * @details Validates error handling when base is less than 2
 */
TEST(RadixToolkitTest, Constructor_InvalidBaseLow_ThrowsException) {
    EXPECT_THROW(RadixToolkit toolkit(1, "01"), std::invalid_argument);
}

/**
 * @brief Test constructor with base exceeding charset size throws exception
 * @details Validates error handling when base is larger than available characters
 */
TEST(RadixToolkitTest, Constructor_BaseExceedsCharset_ThrowsException) {
    EXPECT_THROW(RadixToolkit toolkit(5, "0123"), std::invalid_argument);
}

/**
 * @brief Test to_string with decimal conversion using default base
 * @details Verifies integer to decimal string conversion
 */
TEST(RadixToolkitTest, ToString_DecimalDefaultBase) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(42), "42");
    EXPECT_EQ(toolkit.to_string(0), "0");
    EXPECT_EQ(toolkit.to_string(-123), "-123");
}

/**
 * @brief Test to_string with binary conversion using override base
 * @details Verifies override_base parameter works correctly
 */
TEST(RadixToolkitTest, ToString_BinaryOverrideBase) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(5, 2), "101");
    EXPECT_EQ(toolkit.to_string(0, 2), "0");
}

/**
 * @brief Test to_string with hexadecimal conversion
 * @details Verifies uppercase hexadecimal representation
 */
TEST(RadixToolkitTest, ToString_Hexadecimal) {
    RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.to_string(255), "FF");
    EXPECT_EQ(toolkit.to_string(10), "A");
    EXPECT_EQ(toolkit.to_string(0), "0");
}

/**
 * @brief Test to_string with unsigned types
 * @details Verifies correct handling of unsigned integers
 */
TEST(RadixToolkitTest, ToString_UnsignedTypes) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(static_cast<unsigned int>(42)), "42");
    EXPECT_EQ(toolkit.to_string(static_cast<unsigned long>(100)), "100");
}

/**
 * @brief Test from_string with decimal parsing
 * @details Verifies string to integer conversion in base-10
 */
TEST(RadixToolkitTest, FromString_DecimalDefaultBase) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("42"), 42);
    EXPECT_EQ(toolkit.from_string<int>("0"), 0);
    EXPECT_EQ(toolkit.from_string<int>("-123"), -123);
}

/**
 * @brief Test from_string with positive sign prefix
 * @details Verifies handling of explicit positive sign
 */
TEST(RadixToolkitTest, FromString_PositiveSignPrefix) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("+42"), 42);
}

/**
 * @brief Test from_string with binary override base
 * @details Verifies parsing binary strings correctly
 */
TEST(RadixToolkitTest, FromString_BinaryOverrideBase) {
    RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("101", 2), 5);
    EXPECT_EQ(toolkit.from_string<int>("0", 2), 0);
}

/**
 * @brief Test from_string with hexadecimal parsing
 * @details Verifies parsing uppercase hexadecimal strings
 */
TEST(RadixToolkitTest, FromString_Hexadecimal) {
    RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.from_string<int>("FF"), 255);
    EXPECT_EQ(toolkit.from_string<int>("A"), 10);
}

/**
 * @brief Test from_string with lowercase letters
 * @details Verifies case-insensitive parsing of alphabetic digits
 */
TEST(RadixToolkitTest, FromString_LowercaseLetters) {
    RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.from_string<int>("ff"), 255);
    EXPECT_EQ(toolkit.from_string<int>("a"), 10);
}

/**
 * @brief Test from_string with empty string throws exception
 * @details Validates error handling for empty input
 */
TEST(RadixToolkitTest, FromString_EmptyString_ThrowsException) {
    RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>(""), std::invalid_argument);
}

/**
 * @brief Test from_string with invalid character throws exception
 * @details Validates error handling for characters outside valid range
 */
TEST(RadixToolkitTest, FromString_InvalidCharacter_ThrowsException) {
    RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("12X", 10), std::invalid_argument);
}

/**
 * @brief Test from_string with overflow throws exception
 * @details Validates error handling when value exceeds type limits
 */
TEST(RadixToolkitTest, FromString_Overflow_ThrowsException) {
    RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("99999999999999999999999"), std::out_of_range);
}

/**
 * @brief Test convert_to_string static method with binary
 * @details Verifies stateless binary conversion
 */
TEST(RadixToolkitTest, ConvertToString_StaticBinary) {
    auto result = RadixToolkit::convert_to_string<int>(10, 2, std::string_view("01"));
    EXPECT_EQ(result, "1010");
}

/**
 * @brief Test convert_to_string static method with octal
 * @details Verifies stateless octal conversion
 */
TEST(RadixToolkitTest, ConvertToString_StaticOctal) {
    auto result = RadixToolkit::convert_to_string<int>(64, 8, std::string_view("01234567"));
    EXPECT_EQ(result, "100");
}

/**
 * @brief Test convert_to_string static method with hexadecimal
 * @details Verifies stateless hexadecimal conversion
 */
TEST(RadixToolkitTest, ConvertToString_StaticHexadecimal) {
    auto result = RadixToolkit::convert_to_string<int>(255, 16, std::string_view("0123456789ABCDEF"));
    EXPECT_EQ(result, "FF");
}

/**
 * @brief Test convert_to_string with negative values
 * @details Verifies proper handling of signed integers
 */
TEST(RadixToolkitTest, ConvertToString_NegativeValues) {
    auto result = RadixToolkit::convert_to_string<int>(-42, 10, std::string_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    EXPECT_EQ(result, "-42");
}

/**
 * @brief Test convert_to_string with zero value
 * @details Verifies zero is correctly converted
 */
TEST(RadixToolkitTest, ConvertToString_ZeroValue) {
    auto result = RadixToolkit::convert_to_string<int>(0, 10, std::string_view("0123456789"));
    EXPECT_EQ(result, "0");
}

/**
 * @brief Test convert_to_string with invalid base throws exception
 * @details Validates error handling for base less than 2
 */
TEST(RadixToolkitTest, ConvertToString_InvalidBaseLow_ThrowsException) {
    EXPECT_THROW((void)RadixToolkit::convert_to_string<int>(42, 1, std::string_view("01")), std::invalid_argument);
}

/**
 * @brief Test convert_to_string with base exceeding charset throws exception
 * @details Validates error handling for base larger than charset
 */
TEST(RadixToolkitTest, ConvertToString_BaseExceedsCharset_ThrowsException) {
    EXPECT_THROW((void)RadixToolkit::convert_to_string<int>(42, 5, std::string_view("0123")), std::invalid_argument);
}

/**
 * @brief Test convert_from_string static method with decimal
 * @details Verifies stateless decimal parsing
 */
TEST(RadixToolkitTest, ConvertFromString_StaticDecimal) {
    auto result = RadixToolkit::convert_from_string<int>("42", 10);
    EXPECT_EQ(result, 42);
}

/**
 * @brief Test convert_from_string static method with binary
 * @details Verifies stateless binary parsing
 */
TEST(RadixToolkitTest, ConvertFromString_StaticBinary) {
    auto result = RadixToolkit::convert_from_string<int>("1010", 2);
    EXPECT_EQ(result, 10);
}

/**
 * @brief Test convert_from_string static method with hexadecimal
 * @details Verifies stateless hexadecimal parsing
 */
TEST(RadixToolkitTest, ConvertFromString_StaticHexadecimal) {
    auto result = RadixToolkit::convert_from_string<int>("FF", 16);
    EXPECT_EQ(result, 255);
}

/**
 * @brief Test convert_from_string with negative values
 * @details Verifies parsing of negative numbers
 */
TEST(RadixToolkitTest, ConvertFromString_NegativeValues) {
    auto result = RadixToolkit::convert_from_string<int>("-42", 10);
    EXPECT_EQ(result, -42);
}

/**
 * @brief Test convert_from_string with invalid base throws exception
 * @details Validates error handling for unsupported base values
 */
TEST(RadixToolkitTest, ConvertFromString_InvalidBase_ThrowsException) {
    EXPECT_THROW((void)RadixToolkit::convert_from_string<int>("42", 1), std::invalid_argument);
    EXPECT_THROW((void)RadixToolkit::convert_from_string<int>("42", 37), std::invalid_argument);
}

/**
 * @brief Test to_binary shorthand method
 * @details Verifies convenient binary conversion
 */
TEST(RadixToolkitTest, ToBinary_ShorthandMethod) {
    EXPECT_EQ(RadixToolkit::to_binary(0), "0");
    EXPECT_EQ(RadixToolkit::to_binary(1), "1");
    EXPECT_EQ(RadixToolkit::to_binary(10), "1010");
    EXPECT_EQ(RadixToolkit::to_binary(255), "11111111");
}

/**
 * @brief Test to_binary with negative values
 * @details Verifies binary conversion handles signed integers
 */
TEST(RadixToolkitTest, ToBinary_NegativeValues) {
    EXPECT_EQ(RadixToolkit::to_binary(-5), "-101");
}

/**
 * @brief Test to_octal shorthand method
 * @details Verifies convenient octal conversion
 */
TEST(RadixToolkitTest, ToOctal_ShorthandMethod) {
    EXPECT_EQ(RadixToolkit::to_octal(0), "0");
    EXPECT_EQ(RadixToolkit::to_octal(8), "10");
    EXPECT_EQ(RadixToolkit::to_octal(64), "100");
    EXPECT_EQ(RadixToolkit::to_octal(511), "777");
}

/**
 * @brief Test to_hex uppercase shorthand method
 * @details Verifies uppercase hexadecimal conversion
 */
TEST(RadixToolkitTest, ToHex_UppercaseShorthand) {
    EXPECT_EQ(RadixToolkit::to_hex(0), "0");
    EXPECT_EQ(RadixToolkit::to_hex(10), "A");
    EXPECT_EQ(RadixToolkit::to_hex(255), "FF");
    EXPECT_EQ(RadixToolkit::to_hex(4096), "1000");
}

/**
 * @brief Test to_hex_lower lowercase shorthand method
 * @details Verifies lowercase hexadecimal conversion
 */
TEST(RadixToolkitTest, ToHexLower_LowercaseShorthand) {
    EXPECT_EQ(RadixToolkit::to_hex_lower(0), "0");
    EXPECT_EQ(RadixToolkit::to_hex_lower(10), "a");
    EXPECT_EQ(RadixToolkit::to_hex_lower(255), "ff");
    EXPECT_EQ(RadixToolkit::to_hex_lower(4096), "1000");
}

/**
 * @brief Test from_string_nothrow with valid input
 * @details Verifies non-throwing conversion returns correct value
 */
TEST(RadixToolkitTest, FromStringNothrow_ValidInput) {
    std::errc ec;
    auto result = RadixToolkit::from_string_nothrow<int>("42", 10, ec);
    EXPECT_EQ(result, 42);
    EXPECT_EQ(ec, std::errc{});
}

/**
 * @brief Test from_string_nothrow with invalid input
 * @details Verifies error code is set on invalid input
 */
TEST(RadixToolkitTest, FromStringNothrow_InvalidInput) {
    std::errc ec;
    auto result = RadixToolkit::from_string_nothrow<int>("XYZ", 10, ec);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ec, std::errc::invalid_argument);
}

/**
 * @brief Test from_string_nothrow with partial parse
 * @details Verifies error when not all characters are consumed
 */
TEST(RadixToolkitTest, FromStringNothrow_PartialParse) {
    std::errc ec;
    auto result = RadixToolkit::from_string_nothrow<int>("42abc", 10, ec);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ec, std::errc::invalid_argument);
}

/**
 * @brief Test boundary values with INT_MAX
 * @details Verifies correct handling of maximum integer value
 */
TEST(RadixToolkitTest, Boundary_INT_MAX) {
    RadixToolkit toolkit;
    constexpr auto max_val = std::numeric_limits<int>::max();
    auto str = toolkit.to_string(max_val);
    auto parsed = toolkit.from_string<int>(str);
    EXPECT_EQ(parsed, max_val);
}

/**
 * @brief Test boundary values with INT_MIN
 * @details Verifies correct handling of minimum integer value
 */
TEST(RadixToolkitTest, Boundary_INT_MIN) {
    RadixToolkit toolkit;
    constexpr auto min_val = std::numeric_limits<int>::min();
    auto str = toolkit.to_string(min_val);
    auto parsed = toolkit.from_string<int>(str);
    EXPECT_EQ(parsed, min_val);
}

/**
 * @brief Test boundary overflow detection for positive values
 * @details Verifies overflow is detected when parsing exceeds INT_MAX
 */
TEST(RadixToolkitTest, Boundary_OverflowPositive_ThrowsException) {
    RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("2147483648"), std::out_of_range);
}

/**
 * @brief Test boundary overflow detection for negative values
 * @details Verifies underflow is detected when parsing below INT_MIN
 */
TEST(RadixToolkitTest, Boundary_UnderflowNegative_ThrowsException) {
    RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("-2147483649"), std::out_of_range);
}

/**
 * @brief Test with different integral types (short, long, long long)
 * @details Verifies template works with various integral types
 */
TEST(RadixToolkitTest, TypeVariants_DifferentIntegralTypes) {
    RadixToolkit toolkit;
    
    // short
    EXPECT_EQ(toolkit.to_string(static_cast<short>(100)), "100");
    EXPECT_EQ(toolkit.from_string<short>("100"), 100);
    
    // long
    EXPECT_EQ(toolkit.to_string(static_cast<long>(1000)), "1000");
    EXPECT_EQ(toolkit.from_string<long>("1000"), 1000);
    
    // long long
    EXPECT_EQ(toolkit.to_string(static_cast<long long>(10000)), "10000");
    EXPECT_EQ(toolkit.from_string<long long>("10000"), 10000);
}

/**
 * @brief Test with unsigned types boundary values
 * @details Verifies correct handling of unsigned integer limits
 */
TEST(RadixToolkitTest, TypeVariants_UnsignedBoundary) {
    RadixToolkit toolkit;
    constexpr auto max_uint = std::numeric_limits<unsigned int>::max();
    auto str = toolkit.to_string(max_uint);
    auto parsed = toolkit.from_string<unsigned int>(str);
    EXPECT_EQ(parsed, max_uint);
}

/**
 * @brief Test round-trip conversion consistency
 * @details Verifies to_string and from_string are inverse operations
 */
TEST(RadixToolkitTest, RoundTrip_ConversionConsistency) {
    RadixToolkit toolkit;
    std::vector<int> test_values = {0, 1, -1, 42, -42, 100, -100, 255, 1024, -1024};
    
    for (int val : test_values) {
        auto str = toolkit.to_string(val);
        auto parsed = toolkit.from_string<int>(str);
        EXPECT_EQ(parsed, val) << "Failed for value: " << val;
    }
}

/**
 * @brief Test round-trip with different bases
 * @details Verifies consistency across multiple numeric bases
 */
TEST(RadixToolkitTest, RoundTrip_DifferentBases) {
    std::vector<int> bases = {2, 8, 10, 16, 32};
    std::vector<int> values = {0, 1, 42, 255, 1024};
    
    for (int base : bases) {
        for (int val : values) {
            auto str = RadixToolkit::convert_to_string<int>(val, base, std::string_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
            auto parsed = RadixToolkit::convert_from_string<int>(str, base);
            EXPECT_EQ(parsed, val) << "Failed for base " << base << ", value " << val;
        }
    }
}
