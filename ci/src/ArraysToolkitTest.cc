#include <gtest/gtest.h>
#include "toolkit/ArraysToolkit.hpp"
#include <vector>
#include <string>

using namespace common::toolkit;

// Test asList functionality
TEST(ArraysToolkitTest, AsList_BasicConversion) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::asList(arr, 5);
    
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[4], 5);
}

TEST(ArraysToolkitTest, AsList_EmptyArray) {
    auto result = ArraysToolkit::asList<int>(nullptr, 0);
    EXPECT_TRUE(result.empty());
}

TEST(ArraysToolkitTest, AsList_NullPointerWithSize_ThrowsException) {
    EXPECT_THROW(ArraysToolkit::asList<int>(nullptr, 5), std::invalid_argument);
}

// Test binarySearch functionality
TEST(ArraysToolkitTest, BinarySearch_KeyFound) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 5);
    EXPECT_EQ(index, 2);
}

TEST(ArraysToolkitTest, BinarySearch_KeyNotFound) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 6);
    EXPECT_EQ(index, -1);
}

TEST(ArraysToolkitTest, BinarySearch_FirstElement) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 1);
    EXPECT_EQ(index, 0);
}

TEST(ArraysToolkitTest, BinarySearch_LastElement) {
    int arr[] = {1, 3, 5, 7, 9};
    auto index = ArraysToolkit::binarySearch(arr, 5, 9);
    EXPECT_EQ(index, 4);
}

TEST(ArraysToolkitTest, BinarySearch_WithRange_KeyFound) {
    int arr[] = {1, 3, 5, 7, 9, 11, 13};
    auto index = ArraysToolkit::binarySearch(arr, 2, 5, 7);
    EXPECT_EQ(index, 3);
}

TEST(ArraysToolkitTest, BinarySearch_WithRange_InvalidRange_ThrowsException) {
    int arr[] = {1, 3, 5, 7, 9};
    EXPECT_THROW(ArraysToolkit::binarySearch(arr, 3, 2, 5), std::out_of_range);
}

// Test copyOf functionality
TEST(ArraysToolkitTest, CopyOf_SameLength) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOf(arr, 5, 5);
    
    EXPECT_EQ(result.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(result[i], arr[i]);
    }
}

TEST(ArraysToolkitTest, CopyOf_LongerLength) {
    int arr[] = {1, 2, 3};
    auto result = ArraysToolkit::copyOf(arr, 3, 5);
    
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 0); // Default initialized
    EXPECT_EQ(result[4], 0);
}

TEST(ArraysToolkitTest, CopyOf_ShorterLength) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOf(arr, 5, 3);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
}

// Test copyOfRange functionality
TEST(ArraysToolkitTest, CopyOfRange_BasicRange) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::copyOfRange(arr, 1, 4);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[2], 4);
}

TEST(ArraysToolkitTest, CopyOfRange_FullRange) {
    int arr[] = {1, 2, 3};
    auto result = ArraysToolkit::copyOfRange(arr, 0, 3);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[2], 3);
}

TEST(ArraysToolkitTest, CopyOfRange_InvalidRange_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::copyOfRange(arr, 3, 2), std::out_of_range);
}

// Test equals functionality
TEST(ArraysToolkitTest, Equals_IdenticalArrays) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    EXPECT_TRUE(ArraysToolkit::equals(a, 3, b, 3));
}

TEST(ArraysToolkitTest, Equals_DifferentContent) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    EXPECT_FALSE(ArraysToolkit::equals(a, 3, b, 3));
}

TEST(ArraysToolkitTest, Equals_DifferentSize) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2};
    EXPECT_FALSE(ArraysToolkit::equals(a, 3, b, 2));
}

TEST(ArraysToolkitTest, Equals_EmptyArrays) {
    EXPECT_TRUE(ArraysToolkit::equals<int>(nullptr, 0, nullptr, 0));
}

// Test fill functionality
TEST(ArraysToolkitTest, Fill_BasicFill) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::fill(arr, 5, 0);
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(arr[i], 0);
    }
}

TEST(ArraysToolkitTest, Fill_EmptyArray) {
    // Should not throw or crash
    EXPECT_NO_THROW(ArraysToolkit::fill<int>(nullptr, 0, 5));
}

// Test sort functionality
TEST(ArraysToolkitTest, Sort_UnsortedArray) {
    int arr[] = {5, 2, 8, 1, 9};
    ArraysToolkit::sort(arr, 5);
    
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 5);
    EXPECT_EQ(arr[3], 8);
    EXPECT_EQ(arr[4], 9);
}

TEST(ArraysToolkitTest, Sort_AlreadySorted) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::sort(arr, 5);
    
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[4], 5);
}

TEST(ArraysToolkitTest, Sort_WithRange) {
    int arr[] = {5, 3, 1, 4, 2};
    ArraysToolkit::sort(arr, 1, 4);
    
    EXPECT_EQ(arr[0], 5);      // Unchanged
    EXPECT_EQ(arr[1], 1);      // Sorted range
    EXPECT_EQ(arr[2], 3);
    EXPECT_EQ(arr[3], 4);
    EXPECT_EQ(arr[4], 2);      // Unchanged
}

TEST(ArraysToolkitTest, Sort_InvalidRange_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::sort(arr, 3, 2), std::out_of_range);
}

// Test toString functionality
TEST(ArraysToolkitTest, ToString_BasicArray) {
    int arr[] = {1, 2, 3};
    auto str = ArraysToolkit::toString(arr, 3);
    EXPECT_EQ(str, "[1, 2, 3]");
}

TEST(ArraysToolkitTest, ToString_EmptyArray) {
    auto str = ArraysToolkit::toString<int>(nullptr, 0);
    EXPECT_EQ(str, "[]");
}

TEST(ArraysToolkitTest, ToString_SingleElement) {
    int arr[] = {42};
    auto str = ArraysToolkit::toString(arr, 1);
    EXPECT_EQ(str, "[42]");
}

TEST(ArraysToolkitTest, ToString_StringArray) {
    std::string arr[] = {"hello", "world"};
    auto str = ArraysToolkit::toString(arr, 2);
    EXPECT_EQ(str, "[hello, world]");
}
