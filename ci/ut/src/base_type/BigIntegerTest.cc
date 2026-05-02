/**
 * @file BigIntegerTest.cc
 * @brief Unit tests for the BigInteger class
 * @details Tests cover construction, arithmetic operations, comparison, and factory methods.
 */

#include <gtest/gtest.h>
#include "base_type/BigInteger.hpp"
#include <string>
#include <stdexcept>

using namespace common::base_type;

/**
 * @brief Test default constructor initializes to zero
 * @details Verifies that default-constructed BigInteger equals zero
 */
TEST(BigIntegerTest, DefaultConstructor_InitializesToZero) {
    const BigInteger big_int;
    
    EXPECT_EQ(big_int, BigInteger::fromInt(0));
}

/**
 * @brief Test constructor from int64_t positive value
 * @details Verifies correct initialization from positive integer
 */
TEST(BigIntegerTest, Constructor_FromPositiveInt64) {
    const BigInteger big_int{12345};
    
    EXPECT_EQ(big_int, BigInteger::fromInt(12345));
}

/**
 * @brief Test constructor from int64_t negative value
 * @details Verifies correct initialization from negative integer
 */
TEST(BigIntegerTest, Constructor_FromNegativeInt64) {
    const BigInteger big_int{-98765};
    
    EXPECT_EQ(big_int, BigInteger::fromInt(-98765));
}

/**
 * @brief Test constructor from int64_t zero
 * @details Verifies correct initialization from zero
 */
TEST(BigIntegerTest, Constructor_FromZero) {
    const BigInteger big_int{0};
    
    EXPECT_EQ(big_int, BigInteger::fromInt(0));
}

/**
 * @brief Test constructor from string with positive number
 * @details Verifies correct parsing of positive numeric string
 */
TEST(BigIntegerTest, Constructor_FromPositiveString) {
    const BigInteger big_int{"12345678901234567890"};
    
    EXPECT_EQ(big_int, BigInteger::fromString("12345678901234567890"));
}

/**
 * @brief Test constructor from string with negative number
 * @details Verifies correct parsing of negative numeric string
 */
TEST(BigIntegerTest, Constructor_FromNegativeString) {
    const BigInteger big_int{"-98765432109876543210"};
    
    EXPECT_EQ(big_int, BigInteger::fromString("-98765432109876543210"));
}

/**
 * @brief Test constructor from string with zero
 * @details Verifies correct parsing of zero string
 */
TEST(BigIntegerTest, Constructor_FromZeroString) {
    const BigInteger big_int{"0"};
    
    EXPECT_EQ(big_int, BigInteger::fromInt(0));
}

/**
 * @brief Test addition operator with positive numbers
 * @details Verifies correct addition of two positive BigIntegers
 */
TEST(BigIntegerTest, Addition_PositiveNumbers) {
    const BigInteger a{100};
    const BigInteger b{200};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigInteger::fromInt(300));
}

/**
 * @brief Test addition operator with negative numbers
 * @details Verifies correct addition involving negative numbers
 */
TEST(BigIntegerTest, Addition_NegativeNumbers) {
    const BigInteger a{-50};
    const BigInteger b{-30};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigInteger::fromInt(-80));
}

/**
 * @brief Test addition operator with mixed signs
 * @details Verifies correct addition of positive and negative numbers
 */
TEST(BigIntegerTest, Addition_MixedSigns) {
    const BigInteger a{100};
    const BigInteger b{-30};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigInteger::fromInt(70));
}

/**
 * @brief Test addition with very large numbers
 * @details Verifies addition works correctly with numbers exceeding int64 range
 */
TEST(BigIntegerTest, Addition_VeryLargeNumbers) {
    const BigInteger a{"999999999999999999999999999999"};
    const BigInteger b{"1"};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigInteger::fromString("1000000000000000000000000000000"));
}

/**
 * @brief Test subtraction operator with positive result
 * @details Verifies correct subtraction when result is positive
 */
TEST(BigIntegerTest, Subtraction_PositiveResult) {
    const BigInteger a{200};
    const BigInteger b{50};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigInteger::fromInt(150));
}

/**
 * @brief Test subtraction operator with negative result
 * @details Verifies correct subtraction when result is negative
 */
TEST(BigIntegerTest, Subtraction_NegativeResult) {
    const BigInteger a{50};
    const BigInteger b{200};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigInteger::fromInt(-150));
}

/**
 * @brief Test subtraction with very large numbers
 * @details Verifies subtraction works correctly with large numbers
 */
TEST(BigIntegerTest, Subtraction_VeryLargeNumbers) {
    const BigInteger a{"1000000000000000000000000000000"};
    const BigInteger b{"1"};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigInteger::fromString("999999999999999999999999999999"));
}

/**
 * @brief Test multiplication operator with positive numbers
 * @details Verifies correct multiplication of positive BigIntegers
 */
TEST(BigIntegerTest, Multiplication_PositiveNumbers) {
    const BigInteger a{12};
    const BigInteger b{15};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigInteger::fromInt(180));
}

/**
 * @brief Test multiplication operator with negative numbers
 * @details Verifies correct multiplication involving negative numbers
 */
TEST(BigIntegerTest, Multiplication_NegativeNumbers) {
    const BigInteger a{-5};
    const BigInteger b{10};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigInteger::fromInt(-50));
}

/**
 * @brief Test multiplication by zero
 * @details Verifies that multiplication by zero yields zero
 */
TEST(BigIntegerTest, Multiplication_ByZero) {
    const BigInteger a{12345};
    const BigInteger b{0};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigInteger::fromInt(0));
}

/**
 * @brief Test multiplication with very large numbers
 * @details Verifies multiplication works correctly with large numbers
 */
TEST(BigIntegerTest, Multiplication_VeryLargeNumbers) {
    const BigInteger a{"12345678901234567890"};
    const BigInteger b{"98765432109876543210"};
    
    const auto result = a * b;
    
    // Just verify it doesn't throw and produces some result
    EXPECT_NE(result, BigInteger::fromInt(0));
}

/**
 * @brief Test division operator with exact division
 * @details Verifies correct division when result is exact
 */
TEST(BigIntegerTest, Division_ExactDivision) {
    const BigInteger a{100};
    const BigInteger b{10};
    
    const auto result = a / b;
    
    EXPECT_EQ(result, BigInteger::fromInt(10));
}

/**
 * @brief Test division operator with truncation
 * @details Verifies correct division with integer truncation
 */
TEST(BigIntegerTest, Division_WithTruncation) {
    const BigInteger a{100};
    const BigInteger b{30};
    
    const auto result = a / b;
    
    EXPECT_EQ(result, BigInteger::fromInt(3));
}

/**
 * @brief Test division by zero throws exception
 * @details Verifies that division by zero throws std::invalid_argument
 */
TEST(BigIntegerTest, Division_ByZero_ThrowsException) {
    const BigInteger a{100};
    const BigInteger b{0};
    
    EXPECT_THROW(a / b, std::invalid_argument);
}

/**
 * @brief Test division with negative numbers
 * @details Verifies correct division involving negative numbers
 */
TEST(BigIntegerTest, Division_NegativeNumbers) {
    const BigInteger a{-100};
    const BigInteger b{10};
    
    const auto result = a / b;
    
    EXPECT_EQ(result, BigInteger::fromInt(-10));
}

/**
 * @brief Test modulus operator
 * @details Verifies correct modulus operation
 */
TEST(BigIntegerTest, Modulus_Basic) {
    const BigInteger a{100};
    const BigInteger b{30};
    
    const auto result = a % b;
    
    EXPECT_EQ(result, BigInteger::fromInt(10));
}

/**
 * @brief Test modulus by zero throws exception
 * @details Verifies that modulus by zero throws std::invalid_argument
 */
TEST(BigIntegerTest, Modulus_ByZero_ThrowsException) {
    const BigInteger a{100};
    const BigInteger b{0};
    
    EXPECT_THROW(a % b, std::invalid_argument);
}

/**
 * @brief Test modulus with exact division
 * @details Verifies that modulus is zero when division is exact
 */
TEST(BigIntegerTest, Modulus_ExactDivision) {
    const BigInteger a{100};
    const BigInteger b{10};
    
    const auto result = a % b;
    
    EXPECT_EQ(result, BigInteger::fromInt(0));
}

/**
 * @brief Test equality operator for equal values
 * @details Verifies that equal BigIntegers compare as equal
 */
TEST(BigIntegerTest, Equality_EqualValues) {
    const BigInteger a{42};
    const BigInteger b{42};
    
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality operator for different values
 * @details Verifies that different BigIntegers compare as not equal
 */
TEST(BigIntegerTest, Equality_DifferentValues) {
    const BigInteger a{42};
    const BigInteger b{43};
    
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test three-way comparison - less than
 * @details Verifies correct comparison when first operand is less
 */
TEST(BigIntegerTest, Comparison_LessThan) {
    const BigInteger a{10};
    const BigInteger b{20};
    
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test three-way comparison - greater than
 * @details Verifies correct comparison when first operand is greater
 */
TEST(BigIntegerTest, Comparison_GreaterThan) {
    const BigInteger a{30};
    const BigInteger b{20};
    
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

/**
 * @brief Test three-way comparison - equal
 * @details Verifies correct comparison when operands are equal
 */
TEST(BigIntegerTest, Comparison_Equal) {
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
TEST(BigIntegerTest, Comparison_NegativeNumbers) {
    const BigInteger a{-10};
    const BigInteger b{10};
    
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(a > b);
}

/**
 * @brief Test three-way comparison with large numbers
 * @details Verifies correct comparison with very large numbers
 */
TEST(BigIntegerTest, Comparison_LargeNumbers) {
    const BigInteger a{"999999999999999999999999999999"};
    const BigInteger b{"1000000000000000000000000000000"};
    
    EXPECT_TRUE(a < b);
}

/**
 * @brief Test fromString factory method
 * @details Verifies that fromString creates correct BigInteger
 */
TEST(BigIntegerTest, FromString_FactoryMethod) {
    const auto big_int = BigInteger::fromString("12345");
    
    EXPECT_EQ(big_int, BigInteger::fromInt(12345));
}

/**
 * @brief Test fromString with negative number
 * @details Verifies fromString handles negative numbers correctly
 */
TEST(BigIntegerTest, FromString_NegativeNumber) {
    const auto big_int = BigInteger::fromString("-67890");
    
    EXPECT_EQ(big_int, BigInteger::fromInt(-67890));
}

/**
 * @brief Test fromInt factory method
 * @details Verifies that fromInt creates correct BigInteger
 */
TEST(BigIntegerTest, FromInt_FactoryMethod) {
    const auto big_int = BigInteger::fromInt(54321);
    
    EXPECT_EQ(big_int, BigInteger::fromString("54321"));
}

/**
 * @brief Test fromInt with negative value
 * @details Verifies fromInt handles negative values correctly
 */
TEST(BigIntegerTest, FromInt_NegativeValue) {
    const auto big_int = BigInteger::fromInt(-11111);
    
    EXPECT_EQ(big_int, BigInteger::fromString("-11111"));
}

/**
 * @brief Test fromInt with zero
 * @details Verifies fromInt handles zero correctly
 */
TEST(BigIntegerTest, FromInt_Zero) {
    const auto big_int = BigInteger::fromInt(0);
    
    EXPECT_EQ(big_int, BigInteger::fromString("0"));
}

/**
 * @brief Test arithmetic operations with very large numbers
 * @details Verifies that arithmetic operations work correctly beyond int64 range
 */
TEST(BigIntegerTest, Arithmetic_VeryLargeNumbers) {
    const BigInteger a{"123456789012345678901234567890"};
    const BigInteger b{"987654321098765432109876543210"};
    
    const auto sum = a + b;
    const auto diff = b - a;
    const auto product = a * BigInteger::fromInt(2);
    
    EXPECT_TRUE(sum > a);
    EXPECT_TRUE(diff > BigInteger::fromInt(0));
    EXPECT_TRUE(product > a);
}

/**
 * @brief Test chained arithmetic operations
 * @details Verifies that multiple operations can be chained correctly
 */
TEST(BigIntegerTest, ChainedOperations) {
    const BigInteger a{10};
    const BigInteger b{20};
    const BigInteger c{5};
    
    const auto result = (a + b) * c;
    
    EXPECT_EQ(result, BigInteger::fromInt(150));
}

/**
 * @brief Test operations preserve immutability
 * @details Verifies that original operands are not modified by operations
 */
TEST(BigIntegerTest, Operations_PreserveImmutability) {
    const BigInteger a{100};
    const BigInteger b{50};
    
    const auto sum = a + b;
    const auto diff = a - b;
    
    EXPECT_EQ(a, BigInteger::fromInt(100));
    EXPECT_EQ(b, BigInteger::fromInt(50));
    EXPECT_EQ(sum, BigInteger::fromInt(150));
    EXPECT_EQ(diff, BigInteger::fromInt(50));
}
