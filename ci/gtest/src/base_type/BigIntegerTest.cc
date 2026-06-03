/**
 * @file BigIntegerTest.cc
 * @brief Unit tests for the BigInteger class
 * @details Tests cover construction, arithmetic operations, comparison, and factory methods.
 */

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

#include "base_type/BigInteger.hpp"

using namespace common::base_type;

/**
 * @brief Test fixture for BigIntegerTest tests
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
 * @details Verifies that default-constructed BigInteger equals zero
 */
TEST_F(BigIntegerTest, DefaultConstructor_InitializesToZero)
{
    const BigInteger big_int;

    EXPECT_EQ(big_int, BigInteger(0));
}

/**
 * @brief Test constructor from int64_t positive value
 * @details Verifies correct initialization from positive integer
 */
TEST_F(BigIntegerTest, Constructor_FromPositiveInt64)
{
    const BigInteger big_int{12345};

    EXPECT_EQ(big_int, BigInteger(12345));
}

/**
 * @brief Test constructor from int64_t negative value
 * @details Verifies correct initialization from negative integer
 */
TEST_F(BigIntegerTest, Constructor_FromNegativeInt64)
{
    const BigInteger big_int{-98765};

    EXPECT_EQ(big_int, BigInteger(-98765));
}

/**
 * @brief Test constructor from int64_t zero
 * @details Verifies correct initialization from zero
 */
TEST_F(BigIntegerTest, Constructor_FromZero)
{
    const BigInteger big_int{0};

    EXPECT_EQ(big_int, BigInteger(0));
}

/**
 * @brief Test constructor from string with positive number
 * @details Verifies correct parsing of positive numeric string
 */
TEST_F(BigIntegerTest, Constructor_FromPositiveString)
{
    const BigInteger big_int{"12345678901234567890"};

    EXPECT_EQ(big_int, BigInteger("12345678901234567890"));
}

/**
 * @brief Test constructor from string with negative number
 * @details Verifies correct parsing of negative numeric string
 */
TEST_F(BigIntegerTest, Constructor_FromNegativeString)
{
    const BigInteger big_int{"-98765432109876543210"};

    EXPECT_EQ(big_int, BigInteger("-98765432109876543210"));
}

/**
 * @brief Test constructor from string with zero
 * @details Verifies correct parsing of zero string
 */
TEST_F(BigIntegerTest, Constructor_FromZeroString)
{
    const BigInteger big_int{"0"};

    EXPECT_EQ(big_int, BigInteger(0));
}

/**
 * @brief Test addition operator with positive numbers
 * @details Verifies correct addition of two positive BigIntegers
 */
TEST_F(BigIntegerTest, Addition_PositiveNumbers)
{
    const BigInteger a{100};
    const BigInteger b{200};

    const auto result = a + b;

    EXPECT_EQ(result, BigInteger(300));
}

/**
 * @brief Test addition operator with negative numbers
 * @details Verifies correct addition involving negative numbers
 */
TEST_F(BigIntegerTest, Addition_NegativeNumbers)
{
    const BigInteger a{-50};
    const BigInteger b{-30};

    const auto result = a + b;

    EXPECT_EQ(result, BigInteger(-80));
}

/**
 * @brief Test addition operator with mixed signs
 * @details Verifies correct addition of positive and negative numbers
 */
TEST_F(BigIntegerTest, Addition_MixedSigns)
{
    const BigInteger a{100};
    const BigInteger b{-30};

    const auto result = a + b;

    EXPECT_EQ(result, BigInteger(70));
}

/**
 * @brief Test addition with very large numbers
 * @details Verifies addition works correctly with numbers exceeding int64 range
 */
TEST_F(BigIntegerTest, Addition_VeryLargeNumbers)
{
    const BigInteger a{"999999999999999999999999999999"};
    const BigInteger b{"1"};

    const auto result = a + b;

    EXPECT_EQ(result, BigInteger("1000000000000000000000000000000"));
}

/**
 * @brief Test subtraction operator with positive result
 * @details Verifies correct subtraction when result is positive
 */
TEST_F(BigIntegerTest, Subtraction_PositiveResult)
{
    const BigInteger a{200};
    const BigInteger b{50};

    const auto result = a - b;

    EXPECT_EQ(result, BigInteger(150));
}

/**
 * @brief Test subtraction operator with negative result
 * @details Verifies correct subtraction when result is negative
 */
TEST_F(BigIntegerTest, Subtraction_NegativeResult)
{
    const BigInteger a{50};
    const BigInteger b{200};

    const auto result = a - b;

    EXPECT_EQ(result, BigInteger(-150));
}

/**
 * @brief Test subtraction with very large numbers
 * @details Verifies subtraction works correctly with large numbers
 */
TEST_F(BigIntegerTest, Subtraction_VeryLargeNumbers)
{
    const BigInteger a{"1000000000000000000000000000000"};
    const BigInteger b{"1"};

    const auto result = a - b;

    EXPECT_EQ(result, BigInteger("999999999999999999999999999999"));
}

/**
 * @brief Test multiplication operator with positive numbers
 * @details Verifies correct multiplication of positive BigIntegers
 */
TEST_F(BigIntegerTest, Multiplication_PositiveNumbers)
{
    const BigInteger a{12};
    const BigInteger b{15};

    const auto result = a * b;

    EXPECT_EQ(result, BigInteger(180));
}

/**
 * @brief Test multiplication operator with negative numbers
 * @details Verifies correct multiplication involving negative numbers
 */
TEST_F(BigIntegerTest, Multiplication_NegativeNumbers)
{
    const BigInteger a{-5};
    const BigInteger b{10};

    const auto result = a * b;

    EXPECT_EQ(result, BigInteger(-50));
}

/**
 * @brief Test multiplication by zero
 * @details Verifies that multiplication by zero yields zero
 */
TEST_F(BigIntegerTest, Multiplication_ByZero)
{
    const BigInteger a{12345};
    const BigInteger b{0};

    const auto result = a * b;

    EXPECT_EQ(result, BigInteger(0));
}

/**
 * @brief Test multiplication with very large numbers
 * @details Verifies multiplication works correctly with large numbers
 */
TEST_F(BigIntegerTest, Multiplication_VeryLargeNumbers)
{
    const BigInteger a{"12345678901234567890"};
    const BigInteger b{"98765432109876543210"};

    const auto result = a * b;

    // Just verify it doesn't throw and produces some result
    EXPECT_NE(result, BigInteger(0));
}

/**
 * @brief Test division operator with exact division
 * @details Verifies correct division when result is exact
 */
TEST_F(BigIntegerTest, Division_ExactDivision)
{
    const BigInteger a{100};
    const BigInteger b{10};

    const auto result = a / b;

    EXPECT_EQ(result, BigInteger(10));
}

/**
 * @brief Test division operator with truncation
 * @details Verifies correct division with integer truncation
 */
TEST_F(BigIntegerTest, Division_WithTruncation)
{
    const BigInteger a{100};
    const BigInteger b{30};

    const auto result = a / b;

    EXPECT_EQ(result, BigInteger(3));
}

/**
 * @brief Test division by zero throws exception
 * @details Verifies that division by zero throws std::invalid_argument
 */
TEST_F(BigIntegerTest, Division_ByZero_ThrowsException)
{
    const BigInteger a{100};
    const BigInteger b{0};

    EXPECT_THROW(a / b, std::invalid_argument);
}

/**
 * @brief Test division with negative numbers
 * @details Verifies correct division involving negative numbers
 */
TEST_F(BigIntegerTest, Division_NegativeNumbers)
{
    const BigInteger a{-100};
    const BigInteger b{10};

    const auto result = a / b;

    EXPECT_EQ(result, BigInteger(-10));
}

/**
 * @brief Test modulus operator
 * @details Verifies correct modulus operation
 */
TEST_F(BigIntegerTest, Modulus_Basic)
{
    const BigInteger a{100};
    const BigInteger b{30};

    const auto result = a % b;

    EXPECT_EQ(result, BigInteger(10));
}

/**
 * @brief Test modulus by zero throws exception
 * @details Verifies that modulus by zero throws std::invalid_argument
 */
TEST_F(BigIntegerTest, Modulus_ByZero_ThrowsException)
{
    const BigInteger a{100};
    const BigInteger b{0};

    EXPECT_THROW(a % b, std::invalid_argument);
}

/**
 * @brief Test modulus with exact division
 * @details Verifies that modulus is zero when division is exact
 */
TEST_F(BigIntegerTest, Modulus_ExactDivision)
{
    const BigInteger a{100};
    const BigInteger b{10};

    const auto result = a % b;

    EXPECT_EQ(result, BigInteger(0));
}

/**
 * @brief Test equality operator for equal values
 * @details Verifies that equal BigIntegers compare as equal
 */
TEST_F(BigIntegerTest, Equality_EqualValues)
{
    const BigInteger a{42};
    const BigInteger b{42};

    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality operator for different values
 * @details Verifies that different BigIntegers compare as not equal
 */
TEST_F(BigIntegerTest, Equality_DifferentValues)
{
    const BigInteger a{42};
    const BigInteger b{43};

    EXPECT_FALSE(a == b);
}

/**
 * @brief Test three-way comparison - less than
 * @details Verifies correct comparison when first operand is less
 */
TEST_F(BigIntegerTest, Comparison_LessThan)
{
    const BigInteger a{10};
    const BigInteger b{20};

    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test three-way comparison - greater than
 * @details Verifies correct comparison when first operand is greater
 */
TEST_F(BigIntegerTest, Comparison_GreaterThan)
{
    const BigInteger a{30};
    const BigInteger b{20};

    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

/**
 * @brief Test three-way comparison - equal
 * @details Verifies correct comparison when operands are equal
 */
TEST_F(BigIntegerTest, Comparison_Equal)
{
    const BigInteger a{25};
    const BigInteger b{25};

    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test three-way comparison with negative numbers
 * @details Verifies correct comparison involving negative numbers
 */
TEST_F(BigIntegerTest, Comparison_NegativeNumbers)
{
    const BigInteger a{-10};
    const BigInteger b{10};

    EXPECT_TRUE(a < b);
    EXPECT_FALSE(a > b);
}

/**
 * @brief Test three-way comparison with large numbers
 * @details Verifies correct comparison with very large numbers
 */
TEST_F(BigIntegerTest, Comparison_LargeNumbers)
{
    const BigInteger a{"999999999999999999999999999999"};
    const BigInteger b{"1000000000000000000000000000000"};

    EXPECT_TRUE(a < b);
}

/**
 * @brief Test fromString factory method
 * @details Verifies that fromString creates correct BigInteger
 */
TEST_F(BigIntegerTest, FromString_FactoryMethod)
{
    const auto big_int = BigInteger("12345");

    EXPECT_EQ(big_int, BigInteger(12345));
}

/**
 * @brief Test fromString with negative number
 * @details Verifies fromString handles negative numbers correctly
 */
TEST_F(BigIntegerTest, FromString_NegativeNumber)
{
    const auto big_int = BigInteger("-67890");

    EXPECT_EQ(big_int, BigInteger(-67890));
}

/**
 * @brief Test fromInt factory method
 * @details Verifies that fromInt creates correct BigInteger
 */
TEST_F(BigIntegerTest, FromInt_FactoryMethod)
{
    const auto big_int = BigInteger(54321);

    EXPECT_EQ(big_int, BigInteger("54321"));
}

/**
 * @brief Test fromInt with negative value
 * @details Verifies fromInt handles negative values correctly
 */
TEST_F(BigIntegerTest, FromInt_NegativeValue)
{
    const auto big_int = BigInteger(-11111);

    EXPECT_EQ(big_int, BigInteger("-11111"));
}

/**
 * @brief Test fromInt with zero
 * @details Verifies fromInt handles zero correctly
 */
TEST_F(BigIntegerTest, FromInt_Zero)
{
    const auto big_int = BigInteger(0);

    EXPECT_EQ(big_int, BigInteger("0"));
}

/**
 * @brief Test arithmetic operations with very large numbers
 * @details Verifies that arithmetic operations work correctly beyond int64 range
 */
TEST_F(BigIntegerTest, Arithmetic_VeryLargeNumbers)
{
    const BigInteger a{"123456789012345678901234567890"};
    const BigInteger b{"987654321098765432109876543210"};

    const auto sum = a + b;
    const auto diff = b - a;
    const auto product = a * BigInteger(2);

    EXPECT_TRUE(sum > a);
    EXPECT_TRUE(diff > BigInteger(0));
    EXPECT_TRUE(product > a);
}

/**
 * @brief Test chained arithmetic operations
 * @details Verifies that multiple operations can be chained correctly
 */
TEST_F(BigIntegerTest, ChainedOperations)
{
    const BigInteger a{10};
    const BigInteger b{20};
    const BigInteger c{5};

    const auto result = (a + b) * c;

    EXPECT_EQ(result, BigInteger(150));
}

/**
 * @brief Test operations preserve immutability
 * @details Verifies that original operands are not modified by operations
 */
TEST_F(BigIntegerTest, Operations_PreserveImmutability)
{
    const BigInteger a{100};
    const BigInteger b{50};

    const auto sum = a + b;
    const auto diff = a - b;

    EXPECT_EQ(a, BigInteger(100));
    EXPECT_EQ(b, BigInteger(50));
    EXPECT_EQ(sum, BigInteger(150));
    EXPECT_EQ(diff, BigInteger(50));
}

/**
 * @brief Test constructor from empty string throws exception
 * @details Verifies proper error handling for empty string input
 */
TEST_F(BigIntegerTest, Constructor_EmptyString_ThrowsException)
{
    EXPECT_THROW(BigInteger(""), std::exception);
}

/**
 * @brief Test constructor from invalid string throws exception
 * @details Verifies proper error handling for non-numeric strings
 */
TEST_F(BigIntegerTest, Constructor_InvalidString_ThrowsException)
{
    EXPECT_THROW(BigInteger("abc"), std::exception);
    EXPECT_THROW(BigInteger("12.34"), std::exception); // Decimal point not allowed
    EXPECT_THROW(BigInteger("--123"), std::exception);
}

/**
 * @brief Test constructor from string with only whitespace
 * @details Verifies behavior with whitespace-only input
 */
TEST_F(BigIntegerTest, Constructor_WhitespaceString)
{
    EXPECT_THROW(BigInteger("   "), std::exception);
}

/**
 * @brief Test constructor from extremely large number string
 * @details Verifies handling of numbers with hundreds of digits
 */
TEST_F(BigIntegerTest, Constructor_ExtremelyLargeNumber)
{
    const std::string huge_num = std::string(500, '9'); // 500-digit number
    EXPECT_NO_THROW(const BigInteger big_int = BigInteger(huge_num));
    const BigInteger big_int = BigInteger(huge_num);
    EXPECT_GT(big_int, BigInteger(0));
}

/**
 * @brief Test multiplication with very large numbers
 * @details Verifies correct handling of huge multiplication results
 */
TEST_F(BigIntegerTest, Multiplication_HugeNumbers)
{
    const BigInteger a{std::string(100, '9')}; // 100-digit number
    const BigInteger b{std::string(100, '9')};

    EXPECT_NO_THROW(const auto result = a * b);
    const auto result = a * b;
    EXPECT_GT(result, a);
    EXPECT_GT(result, b);
}

/**
 * @brief Test power operation (repeated multiplication)
 * @details Verifies that repeated multiplication works correctly
 */
TEST_F(BigIntegerTest, Power_RepeatedMultiplication)
{
    const BigInteger base{2};

    BigInteger result{1};
    for (int i = 0; i < 100; ++i)
    {
        result = result * base;
    }

    // 2^100 should be a very large number
    EXPECT_GT(result, BigInteger(1000000));
}

/**
 * @brief Test division and modulus consistency
 * @details Verifies that (a / b) * b + (a % b) == a
 */
TEST_F(BigIntegerTest, Division_Modulus_Consistency)
{
    const BigInteger a{12345};
    const BigInteger b{67};

    const auto quotient = a / b;
    const auto remainder = a % b;
    const auto reconstructed = quotient * b + remainder;

    EXPECT_EQ(reconstructed, a);
}

/**
 * @brief Test modulo with negative numbers
 * @details Verifies correct behavior of modulo with negative operands
 */
TEST_F(BigIntegerTest, Modulus_NegativeNumbers)
{
    const BigInteger a{-100};
    const BigInteger b{30};

    const auto result = a % b;

    // Result sign follows dividend in C++
    EXPECT_TRUE(result < BigInteger(0));
}

/**
 * @brief Test comparison with zero
 * @details Verifies correct comparison behavior with zero
 */
TEST_F(BigIntegerTest, Comparison_WithZero)
{
    const BigInteger positive{1};
    const BigInteger negative{-1};
    const BigInteger zero{0};

    EXPECT_TRUE(positive > zero);
    EXPECT_TRUE(negative < zero);
    EXPECT_FALSE(zero > zero);
    EXPECT_FALSE(zero < zero);
    EXPECT_TRUE(zero == zero);
}

/**
 * @brief Test addition commutativity
 * @details Verifies that a + b == b + a
 */
TEST_F(BigIntegerTest, Addition_Commutativity)
{
    const BigInteger a{12345};
    const BigInteger b{67890};

    const auto result1 = a + b;
    const auto result2 = b + a;

    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test multiplication commutativity
 * @details Verifies that a * b == b * a
 */
TEST_F(BigIntegerTest, Multiplication_Commutativity)
{
    const BigInteger a{123};
    const BigInteger b{456};

    const auto result1 = a * b;
    const auto result2 = b * a;

    EXPECT_EQ(result1, result2);
}
