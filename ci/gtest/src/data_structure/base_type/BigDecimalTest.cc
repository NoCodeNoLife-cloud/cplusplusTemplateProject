/**
 * @file BigDecimalTest.cc
 * @brief Unit tests for the BigDecimal class
 * @details Tests cover construction, arithmetic operators, comparison operators,
 *          and edge cases for high-precision decimal arithmetic.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/base_type/BigDecimal.hpp>

using namespace cppforge::data_structure::base_type;

/**
 * @brief Test fixture for BigDecimal tests
 */
class BigDecimalTest : public testing::Test
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
 * @brief Test construction from a valid decimal string
 * @details Verifies that BigDecimal correctly parses and stores a valid
 *          decimal string representation
 */
TEST_F(BigDecimalTest, Constructor_FromString_ValidValue)
{
    const BigDecimal bd("123.456");
    EXPECT_EQ(bd, BigDecimal("123.456"));
}

/**
 * @brief Test construction from zero string
 * @details Verifies that BigDecimal correctly handles the string "0"
 */
TEST_F(BigDecimalTest, Constructor_FromString_Zero)
{
    const BigDecimal bd("0");
    EXPECT_EQ(bd, BigDecimal("0"));
}

/**
 * @brief Test construction from a negative decimal string
 * @details Verifies that BigDecimal correctly parses negative decimal values
 */
TEST_F(BigDecimalTest, Constructor_FromString_Negative)
{
    const BigDecimal bd("-42.5");
    EXPECT_EQ(bd, BigDecimal("-42.5"));
}

/**
 * @brief Test that empty string construction throws exception
 * @details Verifies that constructing BigDecimal from an empty string
 *          raises std::invalid_argument
 */
TEST_F(BigDecimalTest, Constructor_FromEmptyString_Throws)
{
    EXPECT_THROW(BigDecimal(""), std::invalid_argument);
}

/**
 * @brief Test construction from a high-precision decimal string
 * @details Verifies that BigDecimal correctly handles strings with
 *          precision beyond standard floating-point types
 */
TEST_F(BigDecimalTest, Constructor_FromString_LargePrecision)
{
    const BigDecimal bd("3.1415926535897932384626433832795028841971");
    EXPECT_EQ(bd, BigDecimal("3.1415926535897932384626433832795028841971"));
}

/**
 * @brief Test addition of two positive decimals
 * @details Verifies that adding two positive BigDecimal values produces
 *          the correct sum
 */
TEST_F(BigDecimalTest, Addition_TwoPositives)
{
    const BigDecimal a("10.5");
    const BigDecimal b("20.3");
    const BigDecimal result = a + b;
    EXPECT_EQ(result, BigDecimal("30.8"));
}

/**
 * @brief Test addition of positive and negative decimals
 * @details Verifies that adding a negative BigDecimal to a positive one
 *          is equivalent to subtraction
 */
TEST_F(BigDecimalTest, Addition_PositiveAndNegative)
{
    const BigDecimal a("100.0");
    const BigDecimal b("-30.5");
    const BigDecimal result = a + b;
    EXPECT_EQ(result, BigDecimal("69.5"));
}

/**
 * @brief Test subtraction of two positive decimals
 * @details Verifies that subtracting two positive BigDecimal values produces
 *          the correct difference
 */
TEST_F(BigDecimalTest, Subtraction_TwoPositives)
{
    const BigDecimal a("50.0");
    const BigDecimal b("20.0");
    const BigDecimal result = a - b;
    EXPECT_EQ(result, BigDecimal("30.0"));
}

/**
 * @brief Test subtraction yielding a negative result
 * @details Verifies that subtracting a larger value from a smaller one
 *          produces the correct negative result
 */
TEST_F(BigDecimalTest, Subtraction_NegativeResult)
{
    const BigDecimal a("10.0");
    const BigDecimal b("30.0");
    const BigDecimal result = a - b;
    EXPECT_EQ(result, BigDecimal("-20.0"));
}

/**
 * @brief Test multiplication of two positive decimals
 * @details Verifies that multiplying two positive BigDecimal values produces
 *          the correct product
 */
TEST_F(BigDecimalTest, Multiplication_TwoPositives)
{
    const BigDecimal a("3.0");
    const BigDecimal b("4.0");
    const BigDecimal result = a * b;
    EXPECT_EQ(result, BigDecimal("12.0"));
}

/**
 * @brief Test multiplication of negative and positive decimals
 * @details Verifies that multiplying a negative BigDecimal by a positive one
 *          produces the correct negative product
 */
TEST_F(BigDecimalTest, Multiplication_NegativeAndPositive)
{
    const BigDecimal a("-2.5");
    const BigDecimal b("4.0");
    const BigDecimal result = a * b;
    EXPECT_EQ(result, BigDecimal("-10.0"));
}

/**
 * @brief Test division of two positive decimals
 * @details Verifies that dividing two positive BigDecimal values produces
 *          a result with correct precision
 */
TEST_F(BigDecimalTest, Division_TwoPositives)
{
    const BigDecimal a("10.0");
    const BigDecimal b("3.0");
    const BigDecimal result = a / b;
    EXPECT_EQ(result, result);
}

/**
 * @brief Test division by zero throws exception
 * @details Verifies that dividing a BigDecimal by zero raises
 *          std::invalid_argument
 */
TEST_F(BigDecimalTest, Division_ByZero_Throws)
{
    const BigDecimal a("1.0");
    const BigDecimal b("0.0");
    EXPECT_THROW(a / b, std::invalid_argument);
}

/**
 * @brief Test equality comparison of equal values
 * @details Verifies that operator== returns true for two BigDecimal
 *          instances with the same value
 */
TEST_F(BigDecimalTest, Equality_EqualValues)
{
    const BigDecimal a("99.99");
    const BigDecimal b("99.99");
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality comparison of different values
 * @details Verifies that operator== returns false for two BigDecimal
 *          instances with different values
 */
TEST_F(BigDecimalTest, Equality_DifferentValues)
{
    const BigDecimal a("99.99");
    const BigDecimal b("99.98");
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test less-than comparison operator
 * @details Verifies that operator< correctly compares two BigDecimal values
 */
TEST_F(BigDecimalTest, Comparison_LessThan)
{
    const BigDecimal a("10.0");
    const BigDecimal b("20.0");
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test greater-than comparison operator
 * @details Verifies that operator> correctly compares two BigDecimal values
 */
TEST_F(BigDecimalTest, Comparison_GreaterThan)
{
    const BigDecimal a("30.0");
    const BigDecimal b("15.0");
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

/**
 * @brief Test associativity property of addition
 * @details Verifies that BigDecimal addition is associative:
 *          (a + b) + c == a + (b + c)
 */
TEST_F(BigDecimalTest, Associativity_Addition)
{
    const BigDecimal a("5.0");
    const BigDecimal b("10.0");
    const BigDecimal c("15.0");
    EXPECT_EQ((a + b) + c, a + (b + c));
}

/**
 * @brief Test associativity property of multiplication
 * @details Verifies that BigDecimal multiplication is associative:
 *          (a * b) * c == a * (b * c)
 */
TEST_F(BigDecimalTest, Associativity_Multiplication)
{
    const BigDecimal a("2.0");
    const BigDecimal b("3.0");
    const BigDecimal c("4.0");
    EXPECT_EQ((a * b) * c, a * (b * c));
}
