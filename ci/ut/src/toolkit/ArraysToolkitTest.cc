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

// ==================== New Feature Tests ====================

/**
 * @brief Test isEmpty with null array
 * @details Verifies null array is considered empty
 */
TEST(ArraysToolkitTest, IsEmpty_NullArray) {
    EXPECT_TRUE(ArraysToolkit::isEmpty<int>(nullptr, 0));
}

/**
 * @brief Test isEmpty with zero size
 * @details Verifies zero-size array is considered empty
 */
TEST(ArraysToolkitTest, IsEmpty_ZeroSize) {
    int arr[] = {1, 2, 3};
    EXPECT_TRUE(ArraysToolkit::isEmpty(arr, 0));
}

/**
 * @brief Test isEmpty with non-empty array
 * @details Verifies non-empty array returns false
 */
TEST(ArraysToolkitTest, IsEmpty_NonEmpty) {
    int arr[] = {1, 2, 3};
    EXPECT_FALSE(ArraysToolkit::isEmpty(arr, 3));
}

/**
 * @brief Test contains when element exists
 * @details Verifies contains returns true for existing element
 */
TEST(ArraysToolkitTest, Contains_ElementExists) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_TRUE(ArraysToolkit::contains(arr, 5, 3));
}

/**
 * @brief Test contains when element doesn't exist
 * @details Verifies contains returns false for non-existing element
 */
TEST(ArraysToolkitTest, Contains_ElementNotExists) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_FALSE(ArraysToolkit::contains(arr, 5, 6));
}

/**
 * @brief Test contains with empty array
 * @details Verifies contains returns false for empty array
 */
TEST(ArraysToolkitTest, Contains_EmptyArray) {
    EXPECT_FALSE(ArraysToolkit::contains<int>(nullptr, 0, 1));
}

/**
 * @brief Test linearSearch when key is found
 * @details Verifies correct index is returned
 */
TEST(ArraysToolkitTest, LinearSearch_KeyFound) {
    int arr[] = {5, 3, 8, 1, 9};
    auto index = ArraysToolkit::linearSearch(arr, 5, 8);
    EXPECT_EQ(index, 2);
}

/**
 * @brief Test linearSearch when key is not found
 * @details Verifies -1 is returned for non-existent element
 */
TEST(ArraysToolkitTest, LinearSearch_KeyNotFound) {
    int arr[] = {5, 3, 8, 1, 9};
    auto index = ArraysToolkit::linearSearch(arr, 5, 7);
    EXPECT_EQ(index, -1);
}

/**
 * @brief Test linearSearch for first occurrence
 * @details Verifies first occurrence is returned for duplicates
 */
TEST(ArraysToolkitTest, LinearSearch_FirstOccurrence) {
    int arr[] = {1, 2, 3, 2, 4};
    auto index = ArraysToolkit::linearSearch(arr, 5, 2);
    EXPECT_EQ(index, 1);
}

/**
 * @brief Test maxElement finds maximum value index
 * @details Verifies correct index of maximum element
 */
TEST(ArraysToolkitTest, MaxElement_FindsMaximum) {
    int arr[] = {5, 2, 8, 1, 9};
    auto index = ArraysToolkit::maxElement(arr, 5);
    EXPECT_EQ(index, 4);
    EXPECT_EQ(arr[index], 9);
}

/**
 * @brief Test maxElement with empty array
 * @details Verifies -1 is returned for empty array
 */
TEST(ArraysToolkitTest, MaxElement_EmptyArray) {
    auto index = ArraysToolkit::maxElement<int>(nullptr, 0);
    EXPECT_EQ(index, -1);
}

/**
 * @brief Test minElement finds minimum value index
 * @details Verifies correct index of minimum element
 */
TEST(ArraysToolkitTest, MinElement_FindsMinimum) {
    int arr[] = {5, 2, 8, 1, 9};
    auto index = ArraysToolkit::minElement(arr, 5);
    EXPECT_EQ(index, 3);
    EXPECT_EQ(arr[index], 1);
}

/**
 * @brief Test minElement with empty array
 * @details Verifies -1 is returned for empty array
 */
TEST(ArraysToolkitTest, MinElement_EmptyArray) {
    auto index = ArraysToolkit::minElement<int>(nullptr, 0);
    EXPECT_EQ(index, -1);
}

/**
 * @brief Test count occurrences of a value
 * @details Verifies correct count of element occurrences
 */
TEST(ArraysToolkitTest, Count_MultipleOccurrences) {
    int arr[] = {1, 2, 3, 2, 4, 2, 5};
    auto cnt = ArraysToolkit::count(arr, 7, 2);
    EXPECT_EQ(cnt, 3);
}

/**
 * @brief Test count when value doesn't exist
 * @details Verifies 0 is returned for non-existent value
 */
TEST(ArraysToolkitTest, Count_NoOccurrences) {
    int arr[] = {1, 2, 3, 4, 5};
    auto cnt = ArraysToolkit::count(arr, 5, 6);
    EXPECT_EQ(cnt, 0);
}

/**
 * @brief Test reverse entire array
 * @details Verifies array is reversed in place
 */
TEST(ArraysToolkitTest, Reverse_EntireArray) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::reverse(arr, 5);
    EXPECT_EQ(arr[0], 5);
    EXPECT_EQ(arr[4], 1);
}

/**
 * @brief Test reverse with range
 * @details Verifies partial array reversal
 */
TEST(ArraysToolkitTest, Reverse_WithRange) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::reverse(arr, 1, 4);
    EXPECT_EQ(arr[0], 1); // Unchanged
    EXPECT_EQ(arr[1], 4); // Reversed range
    EXPECT_EQ(arr[2], 3);
    EXPECT_EQ(arr[3], 2);
    EXPECT_EQ(arr[4], 5); // Unchanged
}

/**
 * @brief Test reverse with invalid range throws exception
 * @details Validates proper error handling
 */
TEST(ArraysToolkitTest, Reverse_InvalidRange_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::reverse(arr, 3, 2), std::out_of_range);
}

/**
 * @brief Test distinct removes duplicates
 * @details Verifies duplicate elements are removed and result is sorted
 */
TEST(ArraysToolkitTest, Distinct_RemovesDuplicates) {
    int arr[] = {3, 1, 2, 1, 3, 2, 4};
    auto result = ArraysToolkit::distinct(arr, 7);
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 4);
}

/**
 * @brief Test map transforms elements
 * @details Verifies transformation function is applied to all elements
 */
TEST(ArraysToolkitTest, Map_TransformElements) {
    int arr[] = {1, 2, 3, 4, 5};
    auto result = ArraysToolkit::map<int, int>(arr, 5, [](int x) { return x * 2; });
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[4], 10);
}

/**
 * @brief Test map with type conversion
 * @details Verifies map can convert between different types
 */
TEST(ArraysToolkitTest, Map_TypeConversion) {
    int arr[] = {1, 2, 3};
    auto result = ArraysToolkit::map<int, double>(arr, 3, [](int x) { return static_cast<double>(x) * 1.5; });
    ASSERT_EQ(result.size(), 3);
    EXPECT_DOUBLE_EQ(result[0], 1.5);
    EXPECT_DOUBLE_EQ(result[2], 4.5);
}

/**
 * @brief Test filter keeps matching elements
 * @details Verifies only elements satisfying predicate are kept
 */
TEST(ArraysToolkitTest, Filter_KeepsMatchingElements) {
    int arr[] = {1, 2, 3, 4, 5, 6};
    auto result = ArraysToolkit::filter(arr, 6, [](int x) { return x % 2 == 0; });
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[1], 4);
    EXPECT_EQ(result[2], 6);
}

/**
 * @brief Test filter with no matches
 * @details Verifies empty vector is returned when no elements match
 */
TEST(ArraysToolkitTest, Filter_NoMatches) {
    int arr[] = {1, 3, 5, 7};
    auto result = ArraysToolkit::filter(arr, 4, [](int x) { return x % 2 == 0; });
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test sum calculates total
 * @details Verifies sum of all elements is correct
 */
TEST(ArraysToolkitTest, Sum_CalculatesTotal) {
    int arr[] = {1, 2, 3, 4, 5};
    auto total = ArraysToolkit::sum(arr, 5);
    EXPECT_EQ(total, 15);
}

/**
 * @brief Test sum with empty array
 * @details Verifies sum of empty array is zero
 */
TEST(ArraysToolkitTest, Sum_EmptyArray) {
    auto total = ArraysToolkit::sum<int>(nullptr, 0);
    EXPECT_EQ(total, 0);
}

/**
 * @brief Test average calculates mean
 * @details Verifies average calculation is correct
 */
TEST(ArraysToolkitTest, Average_CalculatesMean) {
    int arr[] = {2, 4, 6, 8};
    auto avg = ArraysToolkit::average(arr, 4);
    EXPECT_DOUBLE_EQ(avg, 5.0);
}

/**
 * @brief Test average with empty array throws exception
 * @details Validates proper error handling
 */
TEST(ArraysToolkitTest, Average_EmptyArray_ThrowsException) {
    EXPECT_THROW(ArraysToolkit::average<int>(nullptr, 0), std::invalid_argument);
}

/**
 * @brief Test allMatch when all elements satisfy predicate
 * @details Verifies true is returned when condition holds for all
 */
TEST(ArraysToolkitTest, AllMatch_AllSatisfy) {
    int arr[] = {2, 4, 6, 8};
    EXPECT_TRUE(ArraysToolkit::allMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test allMatch when some elements don't satisfy
 * @details Verifies false is returned when condition fails for any
 */
TEST(ArraysToolkitTest, AllMatch_NotAllSatisfy) {
    int arr[] = {2, 3, 6, 8};
    EXPECT_FALSE(ArraysToolkit::allMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test anyMatch when at least one element satisfies
 * @details Verifies true is returned when condition holds for any
 */
TEST(ArraysToolkitTest, AnyMatch_AnySatisfies) {
    int arr[] = {1, 3, 5, 6};
    EXPECT_TRUE(ArraysToolkit::anyMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test anyMatch when no elements satisfy
 * @details Verifies false is returned when condition fails for all
 */
TEST(ArraysToolkitTest, AnyMatch_NoneSatisfies) {
    int arr[] = {1, 3, 5, 7};
    EXPECT_FALSE(ArraysToolkit::anyMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test noneMatch when no elements satisfy predicate
 * @details Verifies true is returned when condition fails for all
 */
TEST(ArraysToolkitTest, NoneMatch_NoneSatisfy) {
    int arr[] = {1, 3, 5, 7};
    EXPECT_TRUE(ArraysToolkit::noneMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test noneMatch when some elements satisfy
 * @details Verifies false is returned when condition holds for any
 */
TEST(ArraysToolkitTest, NoneMatch_SomeSatisfy) {
    int arr[] = {1, 2, 3, 5};
    EXPECT_FALSE(ArraysToolkit::noneMatch(arr, 4, [](int x) { return x % 2 == 0; }));
}

/**
 * @brief Test concat merges two arrays
 * @details Verifies both arrays are concatenated in order
 */
TEST(ArraysToolkitTest, Concat_MergesArrays) {
    int a[] = {1, 2, 3};
    int b[] = {4, 5, 6};
    auto result = ArraysToolkit::concat(a, 3, b, 3);
    ASSERT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[5], 6);
}

/**
 * @brief Test concat with empty first array
 * @details Verifies concatenation works with empty first array
 */
TEST(ArraysToolkitTest, Concat_EmptyFirstArray) {
    int b[] = {4, 5, 6};
    auto result = ArraysToolkit::concat<int>(nullptr, 0, b, 3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 4);
}

/**
 * @brief Test intersection finds common elements
 * @details Verifies intersection returns sorted unique common elements
 */
TEST(ArraysToolkitTest, Intersection_FindsCommonElements) {
    int a[] = {1, 2, 3, 4, 5};
    int b[] = {3, 4, 5, 6, 7};
    auto result = ArraysToolkit::intersection(a, 5, b, 5);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 3);
    EXPECT_EQ(result[1], 4);
    EXPECT_EQ(result[2], 5);
}

/**
 * @brief Test intersection with no common elements
 * @details Verifies empty vector is returned
 */
TEST(ArraysToolkitTest, Intersection_NoCommonElements) {
    int a[] = {1, 2, 3};
    int b[] = {4, 5, 6};
    auto result = ArraysToolkit::intersection(a, 3, b, 3);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test unionSet combines all unique elements
 * @details Verifies union returns sorted unique elements from both arrays
 */
TEST(ArraysToolkitTest, UnionSet_CombinesUniqueElements) {
    int a[] = {1, 2, 3};
    int b[] = {3, 4, 5};
    auto result = ArraysToolkit::unionSet(a, 3, b, 3);
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[4], 5);
}

/**
 * @brief Test difference finds elements in first but not second
 * @details Verifies difference returns sorted unique elements
 */
TEST(ArraysToolkitTest, Difference_FindsUniqueElements) {
    int a[] = {1, 2, 3, 4, 5};
    int b[] = {3, 4, 5, 6, 7};
    auto result = ArraysToolkit::difference(a, 5, b, 5);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
}

/**
 * @brief Test isSorted with sorted array
 * @details Verifies true is returned for sorted array
 */
TEST(ArraysToolkitTest, IsSorted_SortedArray) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_TRUE(ArraysToolkit::isSorted(arr, 5));
}

/**
 * @brief Test isSorted with unsorted array
 * @details Verifies false is returned for unsorted array
 */
TEST(ArraysToolkitTest, IsSorted_UnsortedArray) {
    int arr[] = {1, 3, 2, 4, 5};
    EXPECT_FALSE(ArraysToolkit::isSorted(arr, 5));
}

/**
 * @brief Test isSorted with single element
 * @details Verifies single element array is considered sorted
 */
TEST(ArraysToolkitTest, IsSorted_SingleElement) {
    int arr[] = {42};
    EXPECT_TRUE(ArraysToolkit::isSorted(arr, 1));
}

/**
 * @brief Test isSorted with empty array
 * @details Verifies empty array is considered sorted
 */
TEST(ArraysToolkitTest, IsSorted_EmptyArray) {
    EXPECT_TRUE(ArraysToolkit::isSorted<int>(nullptr, 0));
}

/**
 * @brief Test shuffle randomizes array order
 * @details Verifies shuffled array has same elements but potentially different order
 */
TEST(ArraysToolkitTest, Shuffle_RandomizesOrder) {
    int arr[] = {1, 2, 3, 4, 5};
    auto original = ArraysToolkit::asList(arr, 5);
    ArraysToolkit::shuffle(arr, 5);
    auto shuffled = ArraysToolkit::asList(arr, 5);
    
    // Verify same elements exist
    std::sort(shuffled.begin(), shuffled.end());
    EXPECT_EQ(original, shuffled);
}

/**
 * @brief Test rotate shifts elements right
 * @details Verifies elements are rotated by specified positions
 */
TEST(ArraysToolkitTest, Rotate_RightRotation) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::rotate(arr, 5, 2);
    EXPECT_EQ(arr[0], 4);
    EXPECT_EQ(arr[1], 5);
    EXPECT_EQ(arr[2], 1);
    EXPECT_EQ(arr[3], 2);
    EXPECT_EQ(arr[4], 3);
}

/**
 * @brief Test rotate shifts elements left (negative positions)
 * @details Verifies left rotation works correctly
 */
TEST(ArraysToolkitTest, Rotate_LeftRotation) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::rotate(arr, 5, -2);
    EXPECT_EQ(arr[0], 3);
    EXPECT_EQ(arr[1], 4);
    EXPECT_EQ(arr[2], 5);
    EXPECT_EQ(arr[3], 1);
    EXPECT_EQ(arr[4], 2);
}

/**
 * @brief Test rotate with zero positions
 * @details Verifies array remains unchanged
 */
TEST(ArraysToolkitTest, Rotate_ZeroPositions) {
    int arr[] = {1, 2, 3, 4, 5};
    ArraysToolkit::rotate(arr, 5, 0);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[4], 5);
}

/**
 * @brief Test topK gets largest K elements
 * @details Verifies top K elements are returned in descending order
 */
TEST(ArraysToolkitTest, TopK_GetsLargestElements) {
    int arr[] = {5, 3, 8, 1, 9, 2, 7};
    auto result = ArraysToolkit::topK(arr, 7, 3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 9);
    EXPECT_EQ(result[1], 8);
    EXPECT_EQ(result[2], 7);
}

/**
 * @brief Test topK with invalid k throws exception
 * @details Validates proper error handling
 */
TEST(ArraysToolkitTest, TopK_InvalidK_ThrowsException) {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_THROW(ArraysToolkit::topK(arr, 5, 0), std::invalid_argument);
    EXPECT_THROW(ArraysToolkit::topK(arr, 5, 6), std::invalid_argument);
}