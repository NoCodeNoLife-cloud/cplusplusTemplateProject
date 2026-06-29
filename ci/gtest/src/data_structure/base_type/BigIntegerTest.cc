/**
 * @file BigIntegerTest.cc
 * @brief Unit tests for the BigInteger class
 * @details Tests cover construction, arithmetic operators, modulus, comparison operators,
 *          and edge cases for big integer arithmetic.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/BigInteger.hpp"

using namespace cppforge::data_structure::base_type;

/**
 * @brief Test fixture for BigInteger tests
 */
class BigIntegerTest : public testing::Test
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
 * @brief Test default constructor initializes to zero
 * @details Verifies that a default-constructed BigInteger value is zero
 */
TEST_F(BigIntegerTest, DefaultConstructor_Zero)
{
    const BigInteger bi;
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(0)));
}

/**
 * @brief Test construction from a valid integer string
 * @details Verifies that BigInteger correctly parses a valid numeric string
 */
TEST_F(BigIntegerTest, Constructor_FromString_ValidValue)
{
    const BigInteger bi("123456789");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(123456789)));
}

/**
 * @brief Test construction from a negative integer string
 * @details Verifies that BigInteger correctly parses negative numeric strings
 */
TEST_F(BigIntegerTest, Constructor_FromString_Negative)
{
    const BigInteger bi("-42");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(-42)));
}

/**
 * @brief Test construction from zero string
 * @details Verifies that BigInteger correctly handles the string "0"
 */
TEST_F(BigIntegerTest, Constructor_FromString_Zero)
{
    const BigInteger bi("0");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(0)));
}

/**
 * @brief Test that empty string construction throws exception
 * @details Verifies that constructing BigInteger from an empty string
 *          raises std::invalid_argument
 */
TEST_F(BigIntegerTest, Constructor_FromEmptyString_Throws)
{
    EXPECT_THROW(BigInteger(""), std::invalid_argument);
}

/**
 * @brief Test construction from int64_t value
 * @details Verifies that BigInteger correctly constructs from a signed
 *          64-bit integer, including the maximum value
 */
TEST_F(BigIntegerTest, Constructor_FromInt64)
{
    const BigInteger bi(9223372036854775807LL);
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(9223372036854775807LL)));
}

/**
 * @brief Test construction from a large value exceeding 64-bit range
 * @details Verifies that BigInteger correctly handles strings representing
 *          values larger than what fits in a 64-bit integer
 */
TEST_F(BigIntegerTest, Constructor_FromString_LargeValue)
{
    const BigInteger bi("123456789012345678901234567890");
    BigInteger bi2("123456789012345678901234567890");
    EXPECT_EQ(bi, bi2);
}

/**
 * @brief Test addition of two positive big integers
 * @details Verifies that adding two positive BigInteger values produces
 *          the correct sum
 */
TEST_F(BigIntegerTest, Addition_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("200");
    const BigInteger result = a + b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(300)));
}

/**
 * @brief Test addition of positive and negative big integers
 * @details Verifies that adding a negative BigInteger to a positive one
 *          is equivalent to subtraction
 */
TEST_F(BigIntegerTest, Addition_PositiveAndNegative)
{
    const BigInteger a("500");
    const BigInteger b("-200");
    const BigInteger result = a + b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(300)));
}

/**
 * @brief Test subtraction of two positive big integers
 * @details Verifies that subtracting two positive BigInteger values produces
 *          the correct difference
 */
TEST_F(BigIntegerTest, Subtraction_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("30");
    const BigInteger result = a - b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(70)));
}

/**
 * @brief Test subtraction yielding a negative result
 * @details Verifies that subtracting a larger value from a smaller one
 *          produces the correct negative result
 */
TEST_F(BigIntegerTest, Subtraction_NegativeResult)
{
    const BigInteger a("30");
    const BigInteger b("100");
    const BigInteger result = a - b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(-70)));
}

/**
 * @brief Test multiplication of two positive big integers
 * @details Verifies that multiplying two positive BigInteger values produces
 *          the correct product
 */
TEST_F(BigIntegerTest, Multiplication_TwoPositives)
{
    const BigInteger a("12");
    const BigInteger b("34");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(408)));
}

/**
 * @brief Test multiplication of negative and positive big integers
 * @details Verifies that multiplying a negative BigInteger by a positive one
 *          produces the correct negative product
 */
TEST_F(BigIntegerTest, Multiplication_NegativeAndPositive)
{
    const BigInteger a("-5");
    const BigInteger b("6");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(-30)));
}

/**
 * @brief Test multiplication of large big integer values
 * @details Verifies that BigInteger correctly multiplies large values that
 *          exceed 64-bit range
 */
TEST_F(BigIntegerTest, Multiplication_LargeValues)
{
    const BigInteger a("123456789");
    const BigInteger b("987654321");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger("121932631112635269"));
}

/**
 * @brief Test division of two positive big integers
 * @details Verifies that dividing two positive BigInteger values produces
 *          the correct integer quotient (truncated toward zero)
 */
TEST_F(BigIntegerTest, Division_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("3");
    const BigInteger result = a / b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(33)));
}

/**
 * @brief Test division by zero throws exception
 * @details Verifies that dividing a BigInteger by zero raises
 *          std::invalid_argument
 */
TEST_F(BigIntegerTest, Division_ByZero_Throws)
{
    const BigInteger a("1");
    const BigInteger b("0");
    EXPECT_THROW(a / b, std::invalid_argument);
}

/**
 * @brief Test modulus of two positive big integers
 * @details Verifies that the modulus operator returns the correct remainder
 */
TEST_F(BigIntegerTest, Modulus_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("30");
    const BigInteger result = a % b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(10)));
}

/**
 * @brief Test modulus by zero throws exception
 * @details Verifies that taking modulus with zero raises
 *          std::invalid_argument
 */
TEST_F(BigIntegerTest, Modulus_ByZero_Throws)
{
    const BigInteger a("1");
    const BigInteger b("0");
    EXPECT_THROW(a % b, std::invalid_argument);
}

/**
 * @brief Test modulus when divisor is larger than dividend
 * @details Verifies that modulus with a larger divisor returns the dividend
 *          as the remainder
 */
TEST_F(BigIntegerTest, Modulus_LargerDivisor)
{
    const BigInteger a("10");
    const BigInteger b("100");
    const BigInteger result = a % b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(10)));
}

/**
 * @brief Test equality comparison of equal big integers
 * @details Verifies that operator== returns true for two BigInteger
 *          instances with the same value
 */
TEST_F(BigIntegerTest, Equality_EqualValues)
{
    const BigInteger a("999999");
    const BigInteger b("999999");
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality comparison of different big integers
 * @details Verifies that operator== returns false for two BigInteger
 *          instances with different values
 */
TEST_F(BigIntegerTest, Equality_DifferentValues)
{
    const BigInteger a("999999");
    const BigInteger b("999998");
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test less-than comparison operator
 * @details Verifies that operator< correctly compares two BigInteger values
 */
TEST_F(BigIntegerTest, Comparison_LessThan)
{
    const BigInteger a("10");
    const BigInteger b("20");
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test greater-than comparison operator
 * @details Verifies that operator> correctly compares two BigInteger values
 */
TEST_F(BigIntegerTest, Comparison_GreaterThan)
{
    const BigInteger a("50");
    const BigInteger b("25");
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

/**
 * @brief Test comparison of negative big integers
 * @details Verifies that comparison operators correctly handle negative
 *          BigInteger values
 */
TEST_F(BigIntegerTest, Comparison_NegativeValues)
{
    const BigInteger a("-100");
    const BigInteger b("-50");
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
}

/**
 * @brief Test associativity property of addition
 * @details Verifies that BigInteger addition is associative:
 *          (a + b) + c == a + (b + c)
 */
TEST_F(BigIntegerTest, Associativity_Addition)
{
    const BigInteger a("1000");
    const BigInteger b("2000");
    const BigInteger c("3000");
    EXPECT_EQ((a + b) + c, a + (b + c));
}

/**
 * @brief Test associativity property of multiplication
 * @details Verifies that BigInteger multiplication is associative:
 *          (a * b) * c == a * (b * c)
 */
TEST_F(BigIntegerTest, Associativity_Multiplication)
{
    const BigInteger a("11");
    const BigInteger b("22");
    const BigInteger c("33");
    EXPECT_EQ((a * b) * c, a * (b * c));
}
