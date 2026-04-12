/**
 * @file ArraysToolkitTest.cc
 * @brief Unit tests for the ArraysToolkit class
 * @details Tests cover array operations including conversion, search, copy, comparison,
 *          fill, sort, and string representation.
 */

#include <gtest/gtest.h>
#include "toolkit/ArraysToolkit.hpp"
#include <vector>
#include <string>

using namespace common::toolkit;

/**
 * @brief Test asList functionality with basic array conversion
 * @details Verifies that C-style arrays are correctly converted to std::vector
 */
TEST(ArraysToolkitTest, AsList_BasicConversion) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::asList(arr, 5);

    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[4], 5);
}

/**
 * @brief Test asList with empty array
 * @details Verifies handling of zero-length arrays
 */
TEST(ArraysToolkitTest, AsList_EmptyArray) {
    auto result = ArraysToolkit::asList<int>(nullptr, 0);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test asList with null pointer and non-zero size throws exception
 * @details Validates proper error handling for invalid input parameters
 */
TEST(ArraysToolkitTest, AsList_NullPointerWithSize_ThrowsException) {
    EXPECT_THROW(ArraysToolkit::asList<int>(nullptr, 5), std::invalid_argument);
}

/**
 * @brief Test binarySearch when key is found
 * @details Verifies correct index is returned for existing elements
 */
TEST(ArraysToolkitTest, BinarySearch_KeyFound) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 5);
    EXPECT_EQ(index, 2);
}

/**
 * @brief Test binarySearch when key is not found
 * @details Verifies -1 is returned for non-existent elements
 */
TEST(ArraysToolkitTest, BinarySearch_KeyNotFound) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 6);
    EXPECT_EQ(index, -1);
}

/**
 * @brief Test binarySearch for first element
 * @details Verifies correct handling of boundary condition at array start
 */
TEST(ArraysToolkitTest, BinarySearch_FirstElement) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 1);
    EXPECT_EQ(index, 0);
}

/**
 * @brief Test binarySearch for last element
 * @details Verifies correct handling of boundary condition at array end
 */
TEST(ArraysToolkitTest, BinarySearch_LastElement) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 9);
    EXPECT_EQ(index, 4);
}

/**
 * @brief Test binarySearch with custom range when key is found
 * @details Verifies search works correctly within specified sub-range
 */
TEST(ArraysToolkitTest, BinarySearch_WithRange_KeyFound) {
    int arr[] = {1, 3, 5, 7, 9, 11, 13};
    auto index = ArraysToolkit::binarySearch(arr, 2, 5, 7);
    EXPECT_EQ(index, 3);
}

/**
 * @brief Test binarySearch with invalid range throws exception
 * @details Validates proper error handling for out-of-bounds range parameters
 */
TEST(ArraysToolkitTest, BinarySearch_WithRange_InvalidRange_ThrowsException) {
    int arr[] = {1, 3, 5, 7, 9};
    EXPECT_THROW(ArraysToolkit::binarySearch(arr, 3, 2, 5), std::out_of_range);
}

/**
 * @brief Test copyOf with same length as original
 * @details Verifies exact duplication of array content
 */
TEST(ArraysToolkitTest, CopyOf_SameLength) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOf(arr, 5, 5);

    EXPECT_EQ(result.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(result[i], arr[i]);
    }
}

/**
 * @brief Test copyOf with longer length than original
 * @details Verifies array is padded with default-initialized values
 */
TEST(ArraysToolkitTest, CopyOf_LongerLength) {
    int arr[] = {1, 2, 3};
    auto result = ArraysToolkit::copyOf(arr, 3, 5);

    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 0); // Default initialized
    EXPECT_EQ(result[4], 0);
}

/**
 * @brief Test copyOf with shorter length than original
 * @details Verifies array is truncated to specified length
 */
TEST(ArraysToolkitTest, CopyOf_ShorterLength) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOf(arr, 5, 3);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
}

/**
 * @brief Test copyOfRange with basic range extraction
 * @details Verifies correct sub-array extraction from specified indices
 */
TEST(ArraysToolkitTest, CopyOfRange_BasicRange) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOfRange(arr, 1, 4);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[2], 4);
}

/**
 * @brief Test copyOfRange with full array range
 * @details Verifies complete array duplication using range method
 */
TEST(ArraysToolkitTest, CopyOfRange_FullRange) {
    int arr[] = {1, 2, 3};
    auto result = ArraysToolkit::copyOfRange(arr, 0, 3);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
}

/**
 * @brief Test copyOfRange with invalid range throws exception
 * @details Validates proper error handling when from > to
 */
TEST(ArraysToolkitTest, CopyOfRange_InvalidRange_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::copyOfRange(arr, 3, 2), std::out_of_range);
}

/**
 * @brief Test equals with identical arrays
 * @details Verifies content comparison returns true for matching arrays
 */
TEST(ArraysToolkitTest, Equals_IdenticalArrays) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    EXPECT_TRUE(ArraysToolkit::equals(a, 3, b, 3));
}

/**
 * @brief Test equals with different content
 * @details Verifies content comparison returns false for mismatched values
 */
TEST(ArraysToolkitTest, Equals_DifferentContent) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    EXPECT_FALSE(ArraysToolkit::equals(a, 3, b, 3));
}

/**
 * @brief Test equals with different sizes
 * @details Verifies size mismatch results in inequality
 */
TEST(ArraysToolkitTest, Equals_DifferentSize) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2};
    EXPECT_FALSE(ArraysToolkit::equals(a, 3, b, 2));
}

/**
 * @brief Test equals with empty arrays
 * @details Verifies two null/empty arrays are considered equal
 */
TEST(ArraysToolkitTest, Equals_EmptyArrays) {
    EXPECT_TRUE(ArraysToolkit::equals<int>(nullptr, 0, nullptr, 0));
}

/**
 * @brief Test fill with basic value assignment
 * @details Verifies all elements are set to specified value
 */
TEST(ArraysToolkitTest, Fill_BasicFill) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::fill(arr, 5, 0);

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(arr[i], 0);
    }
}

/**
 * @brief Test fill with empty array
 * @details Verifies no crash or exception on zero-length array
 */
TEST(ArraysToolkitTest, Fill_EmptyArray) {
    // Should not throw or crash
    EXPECT_NO_THROW(ArraysToolkit::fill<int>(nullptr, 0, 5));
}

/**
 * @brief Test sort with unsorted array
 * @details Verifies ascending order sorting works correctly
 */
TEST(ArraysToolkitTest, Sort_UnsortedArray) {
    int arr[] = {5, 2, 8, 1, 9};
    ArraysToolkit::sort(arr, 5);

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 5);
    EXPECT_EQ(arr[3], 8);
    EXPECT_EQ(arr[4], 9);
}

/**
 * @brief Test sort with already sorted array
 * @details Verifies idempotency of sort operation
 */
TEST(ArraysToolkitTest, Sort_AlreadySorted) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::sort(arr, 5);

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[4], 5);
}

/**
 * @brief Test sort with custom range
 * @details Verifies partial sorting within specified indices
 */
TEST(ArraysToolkitTest, Sort_WithRange) {
    int arr[] = {5, 3, 1, 4, 2};
    ArraysToolkit::sort(arr, 1, 4);

    EXPECT_EQ(arr[0], 5); // Unchanged
    EXPECT_EQ(arr[1], 1); // Sorted range
    EXPECT_EQ(arr[2], 3);
    EXPECT_EQ(arr[3], 4);
    EXPECT_EQ(arr[4], 2); // Unchanged
}

/**
 * @brief Test sort with invalid range throws exception
 * @details Validates proper error handling when from > to
 */
TEST(ArraysToolkitTest, Sort_InvalidRange_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::sort(arr, 3, 2), std::out_of_range);
}

/**
 * @brief Test toString with basic array
 * @details Verifies correct string representation format "[elem1, elem2, ...]"
 */
TEST(ArraysToolkitTest, ToString_BasicArray) {
    int arr[] = {1, 2, 3};
    auto str = ArraysToolkit::toString(arr, 3);
    EXPECT_EQ(str, "[1, 2, 3]");
}

/**
 * @brief Test toString with empty array
 * @details Verifies empty array produces "[]" string
 */
TEST(ArraysToolkitTest, ToString_EmptyArray) {
    auto str = ArraysToolkit::toString<int>(nullptr, 0);
    EXPECT_EQ(str, "[]");
}

/**
 * @brief Test toString with single element
 * @details Verifies single-element array formatting
 */
TEST(ArraysToolkitTest, ToString_SingleElement) {
    int arr[] = {42};
    auto str = ArraysToolkit::toString(arr, 1);
    EXPECT_EQ(str, "[42]");
}

/**
 * @brief Test toString with string array
 * @details Verifies non-numeric types are correctly formatted
 */
TEST(ArraysToolkitTest, ToString_StringArray) {
    std::string arr[] = {"hello", "world"};
    auto str = ArraysToolkit::toString(arr, 2);
    EXPECT_EQ(str, "[hello, world]");
}