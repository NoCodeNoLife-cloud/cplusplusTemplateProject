/**
 * @file BigDecimalTest.cc
 * @brief Unit tests for the BigDecimal class
 * @details Tests cover construction, arithmetic operators, comparison operators,
 *          and edge cases for high-precision decimal arithmetic.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/BigDecimal.hpp"

using namespace common::data_structure::base_type;

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

TEST_F(BigDecimalTest, Constructor_FromString_ValidValue)
{
    const BigDecimal bd("123.456");
    EXPECT_EQ(bd, BigDecimal("123.456"));
}

TEST_F(BigDecimalTest, Constructor_FromString_Zero)
{
    const BigDecimal bd("0");
    EXPECT_EQ(bd, BigDecimal("0"));
}

TEST_F(BigDecimalTest, Constructor_FromString_Negative)
{
    const BigDecimal bd("-42.5");
    EXPECT_EQ(bd, BigDecimal("-42.5"));
}

TEST_F(BigDecimalTest, Constructor_FromEmptyString_Throws)
{
    EXPECT_THROW(BigDecimal(""), std::invalid_argument);
}

TEST_F(BigDecimalTest, Constructor_FromString_LargePrecision)
{
    const BigDecimal bd("3.1415926535897932384626433832795028841971");
    EXPECT_EQ(bd, BigDecimal("3.1415926535897932384626433832795028841971"));
}

TEST_F(BigDecimalTest, Addition_TwoPositives)
{
    const BigDecimal a("10.5");
    const BigDecimal b("20.3");
    const BigDecimal result = a + b;
    EXPECT_EQ(result, BigDecimal("30.8"));
}

TEST_F(BigDecimalTest, Addition_PositiveAndNegative)
{
    const BigDecimal a("100.0");
    const BigDecimal b("-30.5");
    const BigDecimal result = a + b;
    EXPECT_EQ(result, BigDecimal("69.5"));
}

TEST_F(BigDecimalTest, Subtraction_TwoPositives)
{
    const BigDecimal a("50.0");
    const BigDecimal b("20.0");
    const BigDecimal result = a - b;
    EXPECT_EQ(result, BigDecimal("30.0"));
}

TEST_F(BigDecimalTest, Subtraction_NegativeResult)
{
    const BigDecimal a("10.0");
    const BigDecimal b("30.0");
    const BigDecimal result = a - b;
    EXPECT_EQ(result, BigDecimal("-20.0"));
}

TEST_F(BigDecimalTest, Multiplication_TwoPositives)
{
    const BigDecimal a("3.0");
    const BigDecimal b("4.0");
    const BigDecimal result = a * b;
    EXPECT_EQ(result, BigDecimal("12.0"));
}

TEST_F(BigDecimalTest, Multiplication_NegativeAndPositive)
{
    const BigDecimal a("-2.5");
    const BigDecimal b("4.0");
    const BigDecimal result = a * b;
    EXPECT_EQ(result, BigDecimal("-10.0"));
}

TEST_F(BigDecimalTest, Division_TwoPositives)
{
    const BigDecimal a("10.0");
    const BigDecimal b("3.0");
    const BigDecimal result = a / b;
    EXPECT_EQ(result, result);
}

TEST_F(BigDecimalTest, Division_ByZero_Throws)
{
    const BigDecimal a("1.0");
    const BigDecimal b("0.0");
    EXPECT_THROW(a / b, std::invalid_argument);
}

TEST_F(BigDecimalTest, Equality_EqualValues)
{
    const BigDecimal a("99.99");
    const BigDecimal b("99.99");
    EXPECT_TRUE(a == b);
}

TEST_F(BigDecimalTest, Equality_DifferentValues)
{
    const BigDecimal a("99.99");
    const BigDecimal b("99.98");
    EXPECT_FALSE(a == b);
}

TEST_F(BigDecimalTest, Comparison_LessThan)
{
    const BigDecimal a("10.0");
    const BigDecimal b("20.0");
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST_F(BigDecimalTest, Comparison_GreaterThan)
{
    const BigDecimal a("30.0");
    const BigDecimal b("15.0");
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

TEST_F(BigDecimalTest, Associativity_Addition)
{
    const BigDecimal a("5.0");
    const BigDecimal b("10.0");
    const BigDecimal c("15.0");
    EXPECT_EQ((a + b) + c, a + (b + c));
}

TEST_F(BigDecimalTest, Associativity_Multiplication)
{
    const BigDecimal a("2.0");
    const BigDecimal b("3.0");
    const BigDecimal c("4.0");
    EXPECT_EQ((a * b) * c, a * (b * c));
}
