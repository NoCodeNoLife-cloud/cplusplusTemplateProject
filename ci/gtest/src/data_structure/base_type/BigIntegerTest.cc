/**
 * @file BigIntegerTest.cc
 * @brief Unit tests for the BigInteger class
 * @details Tests cover construction, arithmetic operators, modulus, comparison operators,
 *          and edge cases for big integer arithmetic.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/BigInteger.hpp"

using namespace common::data_structure::base_type;

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

TEST_F(BigIntegerTest, DefaultConstructor_Zero)
{
    const BigInteger bi;
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(0)));
}

TEST_F(BigIntegerTest, Constructor_FromString_ValidValue)
{
    const BigInteger bi("123456789");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(123456789)));
}

TEST_F(BigIntegerTest, Constructor_FromString_Negative)
{
    const BigInteger bi("-42");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(-42)));
}

TEST_F(BigIntegerTest, Constructor_FromString_Zero)
{
    const BigInteger bi("0");
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(0)));
}

TEST_F(BigIntegerTest, Constructor_FromEmptyString_Throws)
{
    EXPECT_THROW(BigInteger(""), std::invalid_argument);
}

TEST_F(BigIntegerTest, Constructor_FromInt64)
{
    const BigInteger bi(9223372036854775807LL);
    EXPECT_EQ(bi, BigInteger(static_cast<int64_t>(9223372036854775807LL)));
}

TEST_F(BigIntegerTest, Constructor_FromString_LargeValue)
{
    const BigInteger bi("123456789012345678901234567890");
    BigInteger bi2("123456789012345678901234567890");
    EXPECT_EQ(bi, bi2);
}

TEST_F(BigIntegerTest, Addition_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("200");
    const BigInteger result = a + b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(300)));
}

TEST_F(BigIntegerTest, Addition_PositiveAndNegative)
{
    const BigInteger a("500");
    const BigInteger b("-200");
    const BigInteger result = a + b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(300)));
}

TEST_F(BigIntegerTest, Subtraction_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("30");
    const BigInteger result = a - b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(70)));
}

TEST_F(BigIntegerTest, Subtraction_NegativeResult)
{
    const BigInteger a("30");
    const BigInteger b("100");
    const BigInteger result = a - b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(-70)));
}

TEST_F(BigIntegerTest, Multiplication_TwoPositives)
{
    const BigInteger a("12");
    const BigInteger b("34");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(408)));
}

TEST_F(BigIntegerTest, Multiplication_NegativeAndPositive)
{
    const BigInteger a("-5");
    const BigInteger b("6");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(-30)));
}

TEST_F(BigIntegerTest, Multiplication_LargeValues)
{
    const BigInteger a("123456789");
    const BigInteger b("987654321");
    const BigInteger result = a * b;
    EXPECT_EQ(result, BigInteger("121932631112635269"));
}

TEST_F(BigIntegerTest, Division_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("3");
    const BigInteger result = a / b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(33)));
}

TEST_F(BigIntegerTest, Division_ByZero_Throws)
{
    const BigInteger a("1");
    const BigInteger b("0");
    EXPECT_THROW(a / b, std::invalid_argument);
}

TEST_F(BigIntegerTest, Modulus_TwoPositives)
{
    const BigInteger a("100");
    const BigInteger b("30");
    const BigInteger result = a % b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(10)));
}

TEST_F(BigIntegerTest, Modulus_ByZero_Throws)
{
    const BigInteger a("1");
    const BigInteger b("0");
    EXPECT_THROW(a % b, std::invalid_argument);
}

TEST_F(BigIntegerTest, Modulus_LargerDivisor)
{
    const BigInteger a("10");
    const BigInteger b("100");
    const BigInteger result = a % b;
    EXPECT_EQ(result, BigInteger(static_cast<int64_t>(10)));
}

TEST_F(BigIntegerTest, Equality_EqualValues)
{
    const BigInteger a("999999");
    const BigInteger b("999999");
    EXPECT_TRUE(a == b);
}

TEST_F(BigIntegerTest, Equality_DifferentValues)
{
    const BigInteger a("999999");
    const BigInteger b("999998");
    EXPECT_FALSE(a == b);
}

TEST_F(BigIntegerTest, Comparison_LessThan)
{
    const BigInteger a("10");
    const BigInteger b("20");
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST_F(BigIntegerTest, Comparison_GreaterThan)
{
    const BigInteger a("50");
    const BigInteger b("25");
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

TEST_F(BigIntegerTest, Comparison_NegativeValues)
{
    const BigInteger a("-100");
    const BigInteger b("-50");
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
}

TEST_F(BigIntegerTest, Associativity_Addition)
{
    const BigInteger a("1000");
    const BigInteger b("2000");
    const BigInteger c("3000");
    EXPECT_EQ((a + b) + c, a + (b + c));
}

TEST_F(BigIntegerTest, Associativity_Multiplication)
{
    const BigInteger a("11");
    const BigInteger b("22");
    const BigInteger c("33");
    EXPECT_EQ((a * b) * c, a * (b * c));
}
