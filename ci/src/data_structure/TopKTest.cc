/**
 * @file TopKTest.cc
 * @brief Unit tests for the TopK class
 * @details Tests cover core top-K largest numbers tracking functionality including
 *          adding numbers, retrieving top-K results, size management, and edge cases.
 */

#include <gtest/gtest.h>
#include "data_structure/TopK.hpp"
#include <stdexcept>
#include <algorithm>
#include <vector>

using namespace common::data_structure;

/**
 * @brief Test constructor with valid k value
 * @details Verifies that positive k values are accepted
 */
TEST(TopKTest, Constructor_ValidK) {
    EXPECT_NO_THROW(TopK topK(1));
    EXPECT_NO_THROW(TopK topK(10));
    EXPECT_NO_THROW(TopK topK(100));
}

/**
 * @brief Test constructor with invalid k value throws exception
 * @details Verifies proper error handling for non-positive k values
 */
TEST(TopKTest, Constructor_InvalidK_ThrowsException) {
    EXPECT_THROW(TopK topK(0), std::invalid_argument);
    EXPECT_THROW(TopK topK(-1), std::invalid_argument);
    EXPECT_THROW(TopK topK(-100), std::invalid_argument);
}

/**
 * @brief Test add maintains top K largest numbers
 * @details Verifies that only the K largest numbers are kept
 */
TEST(TopKTest, Add_MaintainsTopKLargest) {
    TopK topK(3);
    
    // Add numbers: 1, 2, 3, 4, 5
    topK.add(1);
    topK.add(2);
    topK.add(3);
    topK.add(4);
    topK.add(5);
    
    const auto result = topK.getTopK();
    
    // Should contain top 3 largest: 3, 4, 5 (in min-heap order)
    EXPECT_EQ(result.size(), 3);
    
    // Sort to verify correct values
    std::vector<int32_t> sorted(result.begin(), result.end());
    std::sort(sorted.begin(), sorted.end());
    
    EXPECT_EQ(sorted[0], 3);
    EXPECT_EQ(sorted[1], 4);
    EXPECT_EQ(sorted[2], 5);
}

/**
 * @brief Test add with fewer than K elements
 * @details Verifies behavior when less than K numbers have been added
 */
TEST(TopKTest, Add_FewerThanKElements) {
    TopK topK(5);
    
    topK.add(10);
    topK.add(20);
    topK.add(30);
    
    const auto result = topK.getTopK();
    
    // Should contain all 3 elements
    EXPECT_EQ(result.size(), 3);
    
    std::vector<int32_t> sorted(result.begin(), result.end());
    std::sort(sorted.begin(), sorted.end());
    
    EXPECT_EQ(sorted[0], 10);
    EXPECT_EQ(sorted[1], 20);
    EXPECT_EQ(sorted[2], 30);
}

/**
 * @brief Test add with duplicate numbers
 * @details Verifies that duplicates are handled correctly
 */
TEST(TopKTest, Add_DuplicateNumbers) {
    TopK topK(3);
    
    topK.add(5);
    topK.add(5);
    topK.add(5);
    topK.add(5);
    topK.add(5);
    
    const auto result = topK.getTopK();
    
    // All duplicates should be kept if they're in top K
    EXPECT_EQ(result.size(), 3);
    
    for (const auto num : result) {
        EXPECT_EQ(num, 5);
    }
}

/**
 * @brief Test add with negative numbers
 * @details Verifies that negative numbers are handled correctly
 */
TEST(TopKTest, Add_NegativeNumbers) {
    TopK topK(3);
    
    topK.add(-10);
    topK.add(-5);
    topK.add(-20);
    topK.add(-1);
    topK.add(-15);
    
    const auto result = topK.getTopK();
    
    // Top 3 largest (least negative): -5, -1, -10
    EXPECT_EQ(result.size(), 3);
    
    std::vector<int32_t> sorted(result.begin(), result.end());
    std::sort(sorted.begin(), sorted.end());
    
    EXPECT_EQ(sorted[0], -10);
    EXPECT_EQ(sorted[1], -5);
    EXPECT_EQ(sorted[2], -1);
}

/**
 * @brief Test add with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign numbers
 */
TEST(TopKTest, Add_MixedSignNumbers) {
    TopK topK(4);
    
    topK.add(-100);
    topK.add(50);
    topK.add(-50);
    topK.add(100);
    topK.add(0);
    topK.add(25);
    
    const auto result = topK.getTopK();
    
    // Top 4 largest: 0, 25, 50, 100
    EXPECT_EQ(result.size(), 4);
    
    std::vector<int32_t> sorted(result.begin(), result.end());
    std::sort(sorted.begin(), sorted.end());
    
    EXPECT_EQ(sorted[0], 0);
    EXPECT_EQ(sorted[1], 25);
    EXPECT_EQ(sorted[2], 50);
    EXPECT_EQ(sorted[3], 100);
}

/**
 * @brief Test getTopK returns elements in min-heap order
 * @details Verifies that getTopK returns elements from smallest to largest
 */
TEST(TopKTest, GetTopK_MinHeapOrder) {
    TopK topK(5);
    
    // Add in random order
    topK.add(30);
    topK.add(10);
    topK.add(50);
    topK.add(20);
    topK.add(40);
    
    const auto result = topK.getTopK();
    
    // Min-heap should return in ascending order
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[1], 20);
    EXPECT_EQ(result[2], 30);
    EXPECT_EQ(result[3], 40);
    EXPECT_EQ(result[4], 50);
}

/**
 * @brief Test getTopK preserves heap state
 * @details Verifies that calling getTopK doesn't destroy the heap
 */
TEST(TopKTest, GetTopK_PreservesHeapState) {
    TopK topK(3);
    
    topK.add(10);
    topK.add(20);
    topK.add(30);
    
    // First call
    const auto result1 = topK.getTopK();
    EXPECT_EQ(result1.size(), 3);
    
    // Second call should return same results
    const auto result2 = topK.getTopK();
    EXPECT_EQ(result2.size(), 3);
    
    // Verify both results are identical
    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test getTopK after adding more elements
 * @details Verifies that heap updates correctly after retrieval
 */
TEST(TopKTest, GetTopK_AfterAddingMoreElements) {
    TopK topK(3);
    
    topK.add(10);
    topK.add(20);
    topK.add(30);
    
    const auto result1 = topK.getTopK();
    EXPECT_EQ(result1.size(), 3);
    
    // Add larger numbers
    topK.add(40);
    topK.add(50);
    
    const auto result2 = topK.getTopK();
    
    // Should now contain 30, 40, 50
    EXPECT_EQ(result2.size(), 3);
    
    std::vector<int32_t> sorted(result2.begin(), result2.end());
    std::sort(sorted.begin(), sorted.end());
    
    EXPECT_EQ(sorted[0], 30);
    EXPECT_EQ(sorted[1], 40);
    EXPECT_EQ(sorted[2], 50);
}

/**
 * @brief Test size returns correct count
 * @details Verifies that size() returns accurate element count
 */
TEST(TopKTest, Size_CorrectCount) {
    TopK topK(5);
    
    EXPECT_EQ(topK.size(), 0);
    
    topK.add(10);
    EXPECT_EQ(topK.size(), 1);
    
    topK.add(20);
    EXPECT_EQ(topK.size(), 2);
    
    topK.add(30);
    EXPECT_EQ(topK.size(), 3);
    
    // Adding more shouldn't exceed K
    topK.add(40);
    topK.add(50);
    topK.add(60);
    EXPECT_EQ(topK.size(), 5);
}

/**
 * @brief Test size does not exceed K
 * @details Verifies that heap size is capped at K
 */
TEST(TopKTest, Size_DoesNotExceedK) {
    TopK topK(3);
    
    for (int i = 0; i < 100; ++i) {
        topK.add(i);
    }
    
    EXPECT_EQ(topK.size(), 3);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies that empty() accurately reflects heap state
 */
TEST(TopKTest, Empty_CorrectState) {
    TopK topK(5);
    
    EXPECT_TRUE(topK.empty());
    
    topK.add(10);
    EXPECT_FALSE(topK.empty());
    
    const auto result = topK.getTopK();
    EXPECT_FALSE(topK.empty()); // getTopK preserves state
}

/**
 * @brief Test empty after construction
 * @details Verifies that newly constructed TopK is empty
 */
TEST(TopKTest, Empty_AfterConstruction) {
    TopK topK(10);
    EXPECT_TRUE(topK.empty());
    EXPECT_EQ(topK.size(), 0);
}

/**
 * @brief Test with K=1 (edge case)
 * @details Verifies behavior with minimum valid K value
 */
TEST(TopKTest, EdgeCase_KEqualsOne) {
    TopK topK(1);
    
    topK.add(10);
    topK.add(5);
    topK.add(20);
    topK.add(15);
    
    const auto result = topK.getTopK();
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 20); // Largest number
}

/**
 * @brief Test with large K value
 * @details Verifies behavior with large K value
 */
TEST(TopKTest, EdgeCase_LargeK) {
    TopK topK(1000);
    
    for (int i = 0; i < 500; ++i) {
        topK.add(i);
    }
    
    EXPECT_EQ(topK.size(), 500);
    
    const auto result = topK.getTopK();
    EXPECT_EQ(result.size(), 500);
}

/**
 * @brief Test sequential addition maintains ordering
 * @details Verifies that adding numbers in sequence works correctly
 */
TEST(TopKTest, SequentialAddition_MaintainsOrdering) {
    TopK topK(5);
    
    // Add in ascending order
    for (int i = 1; i <= 10; ++i) {
        topK.add(i);
    }
    
    const auto result = topK.getTopK();
    
    // Should contain 6, 7, 8, 9, 10
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 6);
    EXPECT_EQ(result[1], 7);
    EXPECT_EQ(result[2], 8);
    EXPECT_EQ(result[3], 9);
    EXPECT_EQ(result[4], 10);
}

/**
 * @brief Test sequential addition in descending order
 * @details Verifies that adding numbers in descending order works correctly
 */
TEST(TopKTest, SequentialAddition_DescendingOrder) {
    TopK topK(5);
    
    // Add in descending order
    for (int i = 10; i >= 1; --i) {
        topK.add(i);
    }
    
    const auto result = topK.getTopK();
    
    // Should still contain 6, 7, 8, 9, 10
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 6);
    EXPECT_EQ(result[1], 7);
    EXPECT_EQ(result[2], 8);
    EXPECT_EQ(result[3], 9);
    EXPECT_EQ(result[4], 10);
}

/**
 * @brief Test multiple getTopK calls consistency
 * @details Verifies that multiple retrievals return consistent results
 */
TEST(TopKTest, MultipleGetTopK_Consistency) {
    TopK topK(4);
    
    topK.add(100);
    topK.add(200);
    topK.add(300);
    topK.add(400);
    topK.add(50);
    
    const auto result1 = topK.getTopK();
    const auto result2 = topK.getTopK();
    const auto result3 = topK.getTopK();
    
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result2, result3);
    EXPECT_EQ(result1.size(), 4);
}
