/**
 * @file RadixToolkitTest.cc
 * @brief Unit tests for the RadixToolkit class
 * @details Tests cover base conversion operations including instance-based and static methods,
 *          various numeric bases (binary, octal, hexadecimal), error handling, and edge cases.
 */

#include <limits>
#include <gtest/gtest.h>

#include "toolkit/RadixToolkit.hpp"

using namespace cppforge::toolkit;

/**
 * @brief Test fixture for RadixToolkitTest tests
 */
class RadixToolkitTest : public testing::Test
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
 * @brief Test default constructor with standard configuration
 * @details Verifies toolkit initializes with base-10 and standard charset
 */
TEST_F(RadixToolkitTest, Constructor_DefaultConfiguration)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.get_default_base(), 10);
    EXPECT_EQ(toolkit.get_charset(), "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

/**
 * @brief Test constructor with custom base and charset
 * @details Verifies custom configuration is properly stored
 */
TEST_F(RadixToolkitTest, Constructor_CustomConfiguration)
{
    constexpr RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.get_default_base(), 16);
    EXPECT_EQ(toolkit.get_charset(), "0123456789ABCDEF");
}

/**
 * @brief Test constructor with invalid base throws exception
 * @details Validates error handling when base is less than 2
 */
TEST_F(RadixToolkitTest, Constructor_InvalidBaseLow_ThrowsException)
{
    EXPECT_THROW(RadixToolkit(1, "01"), std::invalid_argument);
}

/**
 * @brief Test constructor with base exceeding charset size throws exception
 * @details Validates error handling when base is larger than available characters
 */
TEST_F(RadixToolkitTest, Constructor_BaseExceedsCharset_ThrowsException)
{
    EXPECT_THROW(RadixToolkit(5, "0123"), std::invalid_argument);
}

/**
 * @brief Test to_string with decimal conversion using default base
 * @details Verifies integer to decimal string conversion
 */
TEST_F(RadixToolkitTest, ToString_DecimalDefaultBase)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(42), "42");
    EXPECT_EQ(toolkit.to_string(0), "0");
    EXPECT_EQ(toolkit.to_string(-123), "-123");
}

/**
 * @brief Test to_string with binary conversion using override base
 * @details Verifies override_base parameter works correctly
 */
TEST_F(RadixToolkitTest, ToString_BinaryOverrideBase)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(5, 2), "101");
    EXPECT_EQ(toolkit.to_string(0, 2), "0");
}

/**
 * @brief Test to_string with hexadecimal conversion
 * @details Verifies uppercase hexadecimal representation
 */
TEST_F(RadixToolkitTest, ToString_Hexadecimal)
{
    constexpr RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.to_string(255), "FF");
    EXPECT_EQ(toolkit.to_string(10), "A");
    EXPECT_EQ(toolkit.to_string(0), "0");
}

/**
 * @brief Test to_string with unsigned types
 * @details Verifies correct handling of unsigned integers
 */
TEST_F(RadixToolkitTest, ToString_UnsignedTypes)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.to_string(static_cast<unsigned int>(42)), "42");
    EXPECT_EQ(toolkit.to_string(static_cast<unsigned long>(100)), "100");
}

/**
 * @brief Test from_string with decimal parsing
 * @details Verifies string to integer conversion in base-10
 */
TEST_F(RadixToolkitTest, FromString_DecimalDefaultBase)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("42"), 42);
    EXPECT_EQ(toolkit.from_string<int>("0"), 0);
    EXPECT_EQ(toolkit.from_string<int>("-123"), -123);
}

/**
 * @brief Test from_string with positive sign prefix
 * @details Verifies handling of explicit positive sign
 */
TEST_F(RadixToolkitTest, FromString_PositiveSignPrefix)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("+42"), 42);
}

/**
 * @brief Test from_string with binary override base
 * @details Verifies parsing binary strings correctly
 */
TEST_F(RadixToolkitTest, FromString_BinaryOverrideBase)
{
    constexpr RadixToolkit toolkit;
    EXPECT_EQ(toolkit.from_string<int>("101", 2), 5);
    EXPECT_EQ(toolkit.from_string<int>("0", 2), 0);
}

/**
 * @brief Test from_string with hexadecimal parsing
 * @details Verifies parsing uppercase hexadecimal strings
 */
TEST_F(RadixToolkitTest, FromString_Hexadecimal)
{
    constexpr RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.from_string<int>("FF"), 255);
    EXPECT_EQ(toolkit.from_string<int>("A"), 10);
}

/**
 * @brief Test from_string with lowercase letters
 * @details Verifies case-insensitive parsing of alphabetic digits
 */
TEST_F(RadixToolkitTest, FromString_LowercaseLetters)
{
    constexpr RadixToolkit toolkit(16, "0123456789ABCDEF");
    EXPECT_EQ(toolkit.from_string<int>("ff"), 255);
    EXPECT_EQ(toolkit.from_string<int>("a"), 10);
}

/**
 * @brief Test from_string with empty string throws exception
 * @details Validates error handling for empty input
 */
TEST_F(RadixToolkitTest, FromString_EmptyString_ThrowsException)
{
    constexpr RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>(""), std::invalid_argument);
}

/**
 * @brief Test from_string with invalid character throws exception
 * @details Validates error handling for characters outside valid range
 */
TEST_F(RadixToolkitTest, FromString_InvalidCharacter_ThrowsException)
{
    constexpr RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("12X", 10), std::invalid_argument);
}

/**
 * @brief Test from_string with overflow throws exception
 * @details Validates error handling when value exceeds type limits
 */
TEST_F(RadixToolkitTest, FromString_Overflow_ThrowsException)
{
    constexpr RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("99999999999999999999999"), std::out_of_range);
}

/**
 * @brief Test convert_to_string static method with binary
 * @details Verifies stateless binary conversion
 */
TEST_F(RadixToolkitTest, ConvertToString_StaticBinary)
{
    const auto result = RadixToolkit::convert_to_string<int>(10, 2, std::string_view("01"));
    EXPECT_EQ(result, "1010");
}

/**
 * @brief Test convert_to_string static method with octal
 * @details Verifies stateless octal conversion
 */
TEST_F(RadixToolkitTest, ConvertToString_StaticOctal)
{
    const auto result = RadixToolkit::convert_to_string<int>(64, 8, std::string_view("01234567"));
    EXPECT_EQ(result, "100");
}

/**
 * @brief Test convert_to_string static method with hexadecimal
 * @details Verifies stateless hexadecimal conversion
 */
TEST_F(RadixToolkitTest, ConvertToString_StaticHexadecimal)
{
    const auto result = RadixToolkit::convert_to_string<int>(255, 16, std::string_view("0123456789ABCDEF"));
    EXPECT_EQ(result, "FF");
}

/**
 * @brief Test convert_to_string with negative values
 * @details Verifies proper handling of signed integers
 */
TEST_F(RadixToolkitTest, ConvertToString_NegativeValues)
{
    const auto result = RadixToolkit::convert_to_string<int>(-42, 10, std::string_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    EXPECT_EQ(result, "-42");
}

/**
 * @brief Test convert_to_string with zero value
 * @details Verifies zero is correctly converted
 */
TEST_F(RadixToolkitTest, ConvertToString_ZeroValue)
{
    const auto result = RadixToolkit::convert_to_string<int>(0, 10, std::string_view("0123456789"));
    EXPECT_EQ(result, "0");
}

/**
 * @brief Test convert_to_string with invalid base throws exception
 * @details Validates error handling for base less than 2
 */
TEST_F(RadixToolkitTest, ConvertToString_InvalidBaseLow_ThrowsException)
{
    EXPECT_THROW((void)RadixToolkit::convert_to_string<int>(42, 1, std::string_view("01")), std::invalid_argument);
}

/**
 * @brief Test convert_to_string with base exceeding charset throws exception
 * @details Validates error handling for base larger than charset
 */
TEST_F(RadixToolkitTest, ConvertToString_BaseExceedsCharset_ThrowsException)
{
    EXPECT_THROW((void)RadixToolkit::convert_to_string<int>(42, 5, std::string_view("0123")), std::invalid_argument);
}

/**
 * @brief Test convert_from_string static method with decimal
 * @details Verifies stateless decimal parsing
 */
TEST_F(RadixToolkitTest, ConvertFromString_StaticDecimal)
{
    constexpr auto result = RadixToolkit::convert_from_string<int>("42", 10);
    EXPECT_EQ(result, 42);
}

/**
 * @brief Test convert_from_string static method with binary
 * @details Verifies stateless binary parsing
 */
TEST_F(RadixToolkitTest, ConvertFromString_StaticBinary)
{
    constexpr auto result = RadixToolkit::convert_from_string<int>("1010", 2);
    EXPECT_EQ(result, 10);
}

/**
 * @brief Test convert_from_string static method with hexadecimal
 * @details Verifies stateless hexadecimal parsing
 */
TEST_F(RadixToolkitTest, ConvertFromString_StaticHexadecimal)
{
    constexpr auto result = RadixToolkit::convert_from_string<int>("FF", 16);
    EXPECT_EQ(result, 255);
}

/**
 * @brief Test convert_from_string with negative values
 * @details Verifies parsing of negative numbers
 */
TEST_F(RadixToolkitTest, ConvertFromString_NegativeValues)
{
    constexpr auto result = RadixToolkit::convert_from_string<int>("-42", 10);
    EXPECT_EQ(result, -42);
}

/**
 * @brief Test convert_from_string with invalid base throws exception
 * @details Validates error handling for unsupported base values
 */
TEST_F(RadixToolkitTest, ConvertFromString_InvalidBase_ThrowsException)
{
    EXPECT_THROW((void)RadixToolkit::convert_from_string<int>("42", 1), std::invalid_argument);
    EXPECT_THROW((void)RadixToolkit::convert_from_string<int>("42", 37), std::invalid_argument);
}

/**
 * @brief Test to_binary shorthand method
 * @details Verifies convenient binary conversion
 */
TEST_F(RadixToolkitTest, ToBinary_ShorthandMethod)
{
    EXPECT_EQ(RadixToolkit::to_binary(0), "0");
    EXPECT_EQ(RadixToolkit::to_binary(1), "1");
    EXPECT_EQ(RadixToolkit::to_binary(10), "1010");
    EXPECT_EQ(RadixToolkit::to_binary(255), "11111111");
}

/**
 * @brief Test to_binary with negative values
 * @details Verifies binary conversion handles signed integers
 */
TEST_F(RadixToolkitTest, ToBinary_NegativeValues)
{
    EXPECT_EQ(RadixToolkit::to_binary(-5), "-101");
}

/**
 * @brief Test to_octal shorthand method
 * @details Verifies convenient octal conversion
 */
TEST_F(RadixToolkitTest, ToOctal_ShorthandMethod)
{
    EXPECT_EQ(RadixToolkit::to_octal(0), "0");
    EXPECT_EQ(RadixToolkit::to_octal(8), "10");
    EXPECT_EQ(RadixToolkit::to_octal(64), "100");
    EXPECT_EQ(RadixToolkit::to_octal(511), "777");
}

/**
 * @brief Test to_hex uppercase shorthand method
 * @details Verifies uppercase hexadecimal conversion
 */
TEST_F(RadixToolkitTest, ToHex_UppercaseShorthand)
{
    EXPECT_EQ(RadixToolkit::to_hex(0), "0");
    EXPECT_EQ(RadixToolkit::to_hex(10), "A");
    EXPECT_EQ(RadixToolkit::to_hex(255), "FF");
    EXPECT_EQ(RadixToolkit::to_hex(4096), "1000");
}

/**
 * @brief Test to_hex_lower lowercase shorthand method
 * @details Verifies lowercase hexadecimal conversion
 */
TEST_F(RadixToolkitTest, ToHexLower_LowercaseShorthand)
{
    EXPECT_EQ(RadixToolkit::to_hex_lower(0), "0");
    EXPECT_EQ(RadixToolkit::to_hex_lower(10), "a");
    EXPECT_EQ(RadixToolkit::to_hex_lower(255), "ff");
    EXPECT_EQ(RadixToolkit::to_hex_lower(4096), "1000");
}

/**
 * @brief Test from_string_nothrow with valid input
 * @details Verifies non-throwing conversion returns correct value
 */
TEST_F(RadixToolkitTest, FromStringNothrow_ValidInput)
{
    std::errc ec;
    const auto result = RadixToolkit::from_string_nothrow<int>("42", 10, ec);
    EXPECT_EQ(result, 42);
    EXPECT_EQ(ec, std::errc{});
}

/**
 * @brief Test from_string_nothrow with invalid input
 * @details Verifies error code is set on invalid input
 */
TEST_F(RadixToolkitTest, FromStringNothrow_InvalidInput)
{
    std::errc ec;
    const auto result = RadixToolkit::from_string_nothrow<int>("XYZ", 10, ec);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ec, std::errc::invalid_argument);
}

/**
 * @brief Test from_string_nothrow with partial parse
 * @details Verifies error when not all characters are consumed
 */
TEST_F(RadixToolkitTest, FromStringNothrow_PartialParse)
{
    std::errc ec;
    const auto result = RadixToolkit::from_string_nothrow<int>("42abc", 10, ec);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ec, std::errc::invalid_argument);
}

/**
 * @brief Test boundary values with INT_MAX
 * @details Verifies correct handling of maximum integer value
 */
TEST_F(RadixToolkitTest, Boundary_INT_MAX)
{
    constexpr RadixToolkit toolkit;
    constexpr auto max_val = std::numeric_limits<int>::max();
    const auto str = toolkit.to_string(max_val);
    const auto parsed = toolkit.from_string<int>(str);
    EXPECT_EQ(parsed, max_val);
}

/**
 * @brief Test boundary values with INT_MIN
 * @details Verifies correct handling of minimum integer value
 */
TEST_F(RadixToolkitTest, Boundary_INT_MIN)
{
    constexpr RadixToolkit toolkit;
    constexpr auto min_val = std::numeric_limits<int>::min();
    const auto str = toolkit.to_string(min_val);
    const auto parsed = toolkit.from_string<int>(str);
    EXPECT_EQ(parsed, min_val);
}

/**
 * @brief Test boundary overflow detection for positive values
 * @details Verifies overflow is detected when parsing exceeds INT_MAX
 */
TEST_F(RadixToolkitTest, Boundary_OverflowPositive_ThrowsException)
{
    constexpr RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("2147483648"), std::out_of_range);
}

/**
 * @brief Test boundary overflow detection for negative values
 * @details Verifies underflow is detected when parsing below INT_MIN
 */
TEST_F(RadixToolkitTest, Boundary_UnderflowNegative_ThrowsException)
{
    constexpr RadixToolkit toolkit;
    EXPECT_THROW((void)toolkit.from_string<int>("-2147483649"), std::out_of_range);
}

/**
 * @brief Test with different integral types (short, long, long long)
 * @details Verifies template works with various integral types
 */
TEST_F(RadixToolkitTest, TypeVariants_DifferentIntegralTypes)
{
    constexpr RadixToolkit toolkit;

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
TEST_F(RadixToolkitTest, TypeVariants_UnsignedBoundary)
{
    constexpr RadixToolkit toolkit;
    constexpr auto max_uint = std::numeric_limits<unsigned int>::max();
    const auto str = toolkit.to_string(max_uint);
    const auto parsed = toolkit.from_string<unsigned int>(str);
    EXPECT_EQ(parsed, max_uint);
}

/**
 * @brief Test round-trip conversion consistency
 * @details Verifies to_string and from_string are inverse operations
 */
TEST_F(RadixToolkitTest, RoundTrip_ConversionConsistency)
{
    constexpr RadixToolkit toolkit;
    const std::vector test_values = {0, 1, -1, 42, -42, 100, -100, 255, 1024, -1024};

    for (int val : test_values)
    {
        auto str = toolkit.to_string(val);
        auto parsed = toolkit.from_string<int>(str);
        EXPECT_EQ(parsed, val) << "Failed for value: " << val;
    }
}

/**
 * @brief Test round-trip with different bases
 * @details Verifies consistency across multiple numeric bases
 */
TEST_F(RadixToolkitTest, RoundTrip_DifferentBases)
{
    const std::vector bases = {2, 8, 10, 16, 32};
    const std::vector values = {0, 1, 42, 255, 1024};

    for (int base : bases)
    {
        for (int val : values)
        {
            auto str = RadixToolkit::convert_to_string<int>(val, base, std::string_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
            auto parsed = RadixToolkit::convert_from_string<int>(str, base);
            EXPECT_EQ(parsed, val) << "Failed for base " << base << ", value " << val;
        }
    }
}
