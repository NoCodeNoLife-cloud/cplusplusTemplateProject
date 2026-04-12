/**
 * @file BigDecimalTest.cc
 * @brief Unit tests for the BigDecimal class
 * @details Tests cover construction, arithmetic operations, and comparison functionality
 *          for high-precision decimal arithmetic.
 */

#include <gtest/gtest.h>
#include "base_type/BigDecimal.hpp"
#include <string>
#include <stdexcept>
#include <cmath>

using namespace common::base_type;

/**
 * @brief Test constructor from string with positive decimal
 * @details Verifies correct parsing of positive decimal string
 */
TEST(BigDecimalTest, Constructor_FromPositiveString) {
    const BigDecimal big_decimal{"123.456"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"123.456"});
}

/**
 * @brief Test constructor from string with negative decimal
 * @details Verifies correct parsing of negative decimal string
 */
TEST(BigDecimalTest, Constructor_FromNegativeString) {
    const BigDecimal big_decimal{"-789.012"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"-789.012"});
}

/**
 * @brief Test constructor from string with zero
 * @details Verifies correct parsing of zero string
 */
TEST(BigDecimalTest, Constructor_FromZeroString) {
    const BigDecimal big_decimal{"0.0"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"0.0"});
}

/**
 * @brief Test constructor from string with integer value
 * @details Verifies correct parsing of integer as decimal
 */
TEST(BigDecimalTest, Constructor_FromIntegerString) {
    const BigDecimal big_decimal{"100"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"100"});
}

/**
 * @brief Test constructor from string with very small decimal
 * @details Verifies correct parsing of very small decimal values
 */
TEST(BigDecimalTest, Constructor_FromVerySmallDecimal) {
    const BigDecimal big_decimal{"0.000000001"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"0.000000001"});
}

/**
 * @brief Test constructor from string with very large number
 * @details Verifies correct parsing of very large decimal values
 */
TEST(BigDecimalTest, Constructor_FromVeryLargeNumber) {
    const BigDecimal big_decimal{"999999999999.999999999"};
    
    EXPECT_EQ(big_decimal, BigDecimal{"999999999999.999999999"});
}

/**
 * @brief Test constructor from double with positive value
 * @details Verifies correct conversion from positive double
 */
TEST(BigDecimalTest, Constructor_FromPositiveDouble) {
    const BigDecimal big_decimal{3.14159};
    
    // Verify the value is approximately correct by checking it's close to expected
    const auto diff = big_decimal - BigDecimal{"3.14159"};
    const auto abs_diff = diff < BigDecimal{"0.0"} ? BigDecimal{"0.0"} - diff : diff;
    EXPECT_TRUE(abs_diff < BigDecimal{"0.00001"});
}

/**
 * @brief Test constructor from double with negative value
 * @details Verifies correct conversion from negative double
 */
TEST(BigDecimalTest, Constructor_FromNegativeDouble) {
    const BigDecimal big_decimal{-2.71828};
    
    EXPECT_TRUE(big_decimal < BigDecimal{"0.0"});
}

/**
 * @brief Test constructor from double with zero
 * @details Verifies correct conversion from zero double
 */
TEST(BigDecimalTest, Constructor_FromZeroDouble) {
    const BigDecimal big_decimal{0.0};
    
    EXPECT_EQ(big_decimal, BigDecimal{"0.0"});
}

/**
 * @brief Test addition operator with positive numbers
 * @details Verifies correct addition of two positive BigDecimals
 */
TEST(BigDecimalTest, Addition_PositiveNumbers) {
    const BigDecimal a{"10.5"};
    const BigDecimal b{"20.3"};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigDecimal{"30.8"});
}

/**
 * @brief Test addition operator with negative numbers
 * @details Verifies correct addition involving negative numbers
 */
TEST(BigDecimalTest, Addition_NegativeNumbers) {
    const BigDecimal a{"-15.5"};
    const BigDecimal b{"-10.2"};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigDecimal{"-25.7"});
}

/**
 * @brief Test addition operator with mixed signs
 * @details Verifies correct addition of positive and negative numbers
 */
TEST(BigDecimalTest, Addition_MixedSigns) {
    const BigDecimal a{"50.0"};
    const BigDecimal b{"-20.5"};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, BigDecimal{"29.5"});
}

/**
 * @brief Test addition with zero
 * @details Verifies that adding zero doesn't change the value
 */
TEST(BigDecimalTest, Addition_WithZero) {
    const BigDecimal a{"123.456"};
    const BigDecimal b{"0.0"};
    
    const auto result = a + b;
    
    EXPECT_EQ(result, a);
}

/**
 * @brief Test subtraction operator with positive result
 * @details Verifies correct subtraction when result is positive
 */
TEST(BigDecimalTest, Subtraction_PositiveResult) {
    const BigDecimal a{"100.5"};
    const BigDecimal b{"30.2"};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigDecimal{"70.3"});
}

/**
 * @brief Test subtraction operator with negative result
 * @details Verifies correct subtraction when result is negative
 */
TEST(BigDecimalTest, Subtraction_NegativeResult) {
    const BigDecimal a{"30.2"};
    const BigDecimal b{"100.5"};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigDecimal{"-70.3"});
}

/**
 * @brief Test subtraction with zero
 * @details Verifies that subtracting zero doesn't change the value
 */
TEST(BigDecimalTest, Subtraction_ByZero) {
    const BigDecimal a{"456.789"};
    const BigDecimal b{"0.0"};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, a);
}

/**
 * @brief Test subtraction resulting in zero
 * @details Verifies that subtracting equal values yields zero
 */
TEST(BigDecimalTest, Subtraction_EqualValues) {
    const BigDecimal a{"99.99"};
    const BigDecimal b{"99.99"};
    
    const auto result = a - b;
    
    EXPECT_EQ(result, BigDecimal{"0.0"});
}

/**
 * @brief Test multiplication operator with positive numbers
 * @details Verifies correct multiplication of positive BigDecimals
 */
TEST(BigDecimalTest, Multiplication_PositiveNumbers) {
    const BigDecimal a{"2.5"};
    const BigDecimal b{"4.0"};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigDecimal{"10.0"});
}

/**
 * @brief Test multiplication operator with negative numbers
 * @details Verifies correct multiplication involving negative numbers
 */
TEST(BigDecimalTest, Multiplication_NegativeNumbers) {
    const BigDecimal a{"-3.0"};
    const BigDecimal b{"5.0"};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigDecimal{"-15.0"});
}

/**
 * @brief Test multiplication by zero
 * @details Verifies that multiplication by zero yields zero
 */
TEST(BigDecimalTest, Multiplication_ByZero) {
    const BigDecimal a{"123.456"};
    const BigDecimal b{"0.0"};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigDecimal{"0.0"});
}

/**
 * @brief Test multiplication with decimal precision
 * @details Verifies correct handling of decimal places in multiplication
 */
TEST(BigDecimalTest, Multiplication_DecimalPrecision) {
    const BigDecimal a{"0.1"};
    const BigDecimal b{"0.2"};
    
    const auto result = a * b;
    
    EXPECT_EQ(result, BigDecimal{"0.02"});
}

/**
 * @brief Test division operator with exact division
 * @details Verifies correct division when result is exact
 */
TEST(BigDecimalTest, Division_ExactDivision) {
    const BigDecimal a{"10.0"};
    const BigDecimal b{"2.0"};
    
    const auto result = a / b;
    
    EXPECT_EQ(result, BigDecimal{"5.0"});
}

/**
 * @brief Test division operator with decimal result
 * @details Verifies correct division with decimal quotient
 */
TEST(BigDecimalTest, Division_DecimalResult) {
    const BigDecimal a{"10.0"};
    const BigDecimal b{"3.0"};
    
    const auto result = a / b;
    
    // Check if result is approximately 3.333...
    const auto expected = BigDecimal{"3.33333333333333333333"};
    const auto diff = result - expected;
    
    EXPECT_TRUE(diff < BigDecimal{"0.0000000001"} && diff > BigDecimal{"-0.0000000001"});
}

/**
 * @brief Test division by zero throws exception
 * @details Verifies that division by zero throws std::invalid_argument
 */
TEST(BigDecimalTest, Division_ByZero_ThrowsException) {
    const BigDecimal a{"100.0"};
    const BigDecimal b{"0.0"};
    
    EXPECT_THROW(static_cast<void>(a / b), std::invalid_argument);
}

/**
 * @brief Test division with negative numbers
 * @details Verifies correct division involving negative numbers
 */
TEST(BigDecimalTest, Division_NegativeNumbers) {
    const BigDecimal a{"-20.0"};
    const BigDecimal b{"4.0"};
    
    const auto result = a / b;
    
    EXPECT_EQ(result, BigDecimal{"-5.0"});
}

/**
 * @brief Test equality operator for equal values
 * @details Verifies that equal BigDecimals compare as equal
 */
TEST(BigDecimalTest, Equality_EqualValues) {
    const BigDecimal a{"123.456"};
    const BigDecimal b{"123.456"};
    
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality operator for different values
 * @details Verifies that different BigDecimals compare as not equal
 */
TEST(BigDecimalTest, Equality_DifferentValues) {
    const BigDecimal a{"123.456"};
    const BigDecimal b{"123.457"};
    
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test three-way comparison - less than
 * @details Verifies correct comparison when first operand is less
 */
TEST(BigDecimalTest, Comparison_LessThan) {
    const BigDecimal a{"10.5"};
    const BigDecimal b{"20.3"};
    
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test three-way comparison - greater than
 * @details Verifies correct comparison when first operand is greater
 */
TEST(BigDecimalTest, Comparison_GreaterThan) {
    const BigDecimal a{"30.7"};
    const BigDecimal b{"20.3"};
    
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

/**
 * @brief Test three-way comparison - equal
 * @details Verifies correct comparison when operands are equal
 */
TEST(BigDecimalTest, Comparison_Equal) {
    const BigDecimal a{"25.5"};
    const BigDecimal b{"25.5"};
    
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test three-way comparison with negative numbers
 * @details Verifies correct comparison involving negative numbers
 */
TEST(BigDecimalTest, Comparison_NegativeNumbers) {
    const BigDecimal a{"-10.5"};
    const BigDecimal b{"10.5"};
    
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(a > b);
}

/**
 * @brief Test three-way comparison with very small differences
 * @details Verifies correct comparison with minimal value differences
 */
TEST(BigDecimalTest, Comparison_SmallDifferences) {
    const BigDecimal a{"1.000000001"};
    const BigDecimal b{"1.000000002"};
    
    EXPECT_TRUE(a < b);
}

/**
 * @brief Test chained arithmetic operations
 * @details Verifies that multiple operations can be chained correctly
 */
TEST(BigDecimalTest, ChainedOperations) {
    const BigDecimal a{"10.0"};
    const BigDecimal b{"5.0"};
    const BigDecimal c{"2.0"};
    
    const auto result = (a + b) * c;
    
    EXPECT_EQ(result, BigDecimal{"30.0"});
}

/**
 * @brief Test operations preserve immutability
 * @details Verifies that original operands are not modified by operations
 */
TEST(BigDecimalTest, Operations_PreserveImmutability) {
    const BigDecimal a{"100.0"};
    const BigDecimal b{"50.0"};
    
    const auto sum = a + b;
    const auto diff = a - b;
    const auto product = a * b;
    const auto quotient = a / b;
    
    EXPECT_EQ(a, BigDecimal{"100.0"});
    EXPECT_EQ(b, BigDecimal{"50.0"});
    EXPECT_EQ(sum, BigDecimal{"150.0"});
    EXPECT_EQ(diff, BigDecimal{"50.0"});
    EXPECT_EQ(product, BigDecimal{"5000.0"});
    EXPECT_EQ(quotient, BigDecimal{"2.0"});
}

/**
 * @brief Test high precision arithmetic
 * @details Verifies that BigDecimal maintains precision in calculations
 */
TEST(BigDecimalTest, HighPrecision_Arithmetic) {
    const BigDecimal a{"0.1"};
    const BigDecimal b{"0.2"};
    
    const auto result = a + b;
    
    // Should be exactly 0.3, not affected by floating point errors
    EXPECT_EQ(result, BigDecimal{"0.3"});
}

/**
 * @brief Test complex arithmetic expression
 * @details Verifies correctness of complex multi-step calculations
 */
TEST(BigDecimalTest, ComplexExpression) {
    const BigDecimal price{"19.99"};
    const BigDecimal quantity{"3"};
    const BigDecimal tax_rate{"0.08"};
    
    const auto subtotal = price * quantity;
    const auto tax = subtotal * tax_rate;
    const auto total = subtotal + tax;
    
    // Verify intermediate results
    EXPECT_EQ(subtotal, BigDecimal{"59.97"});
    
    // Tax should be approximately 4.7976
    const auto expected_tax = BigDecimal{"4.7976"};
    const auto tax_diff = tax - expected_tax;
    EXPECT_TRUE(tax_diff < BigDecimal{"0.0001"} && tax_diff > BigDecimal{"-0.0001"});
}

/**
 * @brief Test comparison consistency with arithmetic
 * @details Verifies that comparison operators are consistent with arithmetic
 */
TEST(BigDecimalTest, Comparison_ConsistencyWithArithmetic) {
    const BigDecimal a{"10.0"};
    const BigDecimal b{"5.0"};
    
    const auto diff = a - b;
    
    EXPECT_TRUE(diff > BigDecimal{"0.0"});
    EXPECT_TRUE(a > b);
}
