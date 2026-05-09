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
 * @brief Test default constructor (unbounded heap)
 * @details Verifies that TopK can be created without capacity limit
 */
TEST(TopKTest, Constructor_Default_Unbounded) {
    EXPECT_NO_THROW(TopK topK);
}

/**
 * @brief Test constructor with valid max_capacity value
 * @details Verifies that non-negative max_capacity values are accepted
 */
TEST(TopKTest, Constructor_ValidMaxCapacity) {
    EXPECT_NO_THROW(TopK topK(0)); // Unbounded
    EXPECT_NO_THROW(TopK topK(1));
    EXPECT_NO_THROW(TopK topK(10));
    EXPECT_NO_THROW(TopK topK(100));
}

/**
 * @brief Test constructor with invalid max_capacity value throws exception
 * @details Verifies proper error handling for negative max_capacity values
 */
TEST(TopKTest, Constructor_InvalidMaxCapacity_ThrowsException) {
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
    const TopK topK(10);
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

/**
 * @brief Test getTopK is non-destructive and heap can continue to be used
 * @details Verifies that after calling getTopK(), the heap state is preserved
 *          and new elements can still be added correctly
 */
TEST(TopKTest, GetTopK_NonDestructive_HeapUsableAfterRetrieval) {
    TopK topK(3);

    // Add initial elements
    topK.add(10);
    topK.add(20);
    topK.add(30);

    // First retrieval
    const auto result1 = topK.getTopK();
    EXPECT_EQ(result1.size(), 3);
    EXPECT_EQ(topK.size(), 3); // Size should remain unchanged

    // Add more elements after retrieval
    topK.add(40);
    topK.add(50);

    // Second retrieval should reflect new state
    const auto result2 = topK.getTopK();
    EXPECT_EQ(result2.size(), 3);
    EXPECT_EQ(topK.size(), 3); // Size should still be 3

    // Verify the results are different (heap was updated)
    EXPECT_NE(result1, result2);

    // Verify second result contains the largest 3 numbers
    std::vector<int32_t> sorted(result2.begin(), result2.end());
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 30);
    EXPECT_EQ(sorted[1], 40);
    EXPECT_EQ(sorted[2], 50);
}

/**
 * @brief Test getTopK with descending order parameter
 * @details Verifies that getTopK(false) returns elements in descending order
 */
TEST(TopKTest, GetTopK_DescendingOrder_ReturnsLargestFirst) {
    TopK topK(5);

    // Add in random order
    topK.add(30);
    topK.add(10);
    topK.add(50);
    topK.add(20);
    topK.add(40);

    const auto result = topK.getTopK(0, false); // All elements, descending order

    // Should return in descending order (largest to smallest)
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 50);
    EXPECT_EQ(result[1], 40);
    EXPECT_EQ(result[2], 30);
    EXPECT_EQ(result[3], 20);
    EXPECT_EQ(result[4], 10);
}

/**
 * @brief Test getTopK with ascending order parameter (explicit)
 * @details Verifies that getTopK(true) returns elements in ascending order
 */
TEST(TopKTest, GetTopK_AscendingOrderExplicit_ReturnsSmallestFirst) {
    TopK topK(5);

    topK.add(30);
    topK.add(10);
    topK.add(50);
    topK.add(20);
    topK.add(40);

    const auto result = topK.getTopK(0, true); // All elements, ascending order (explicit)

    // Should return in ascending order (smallest to largest)
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[1], 20);
    EXPECT_EQ(result[2], 30);
    EXPECT_EQ(result[3], 40);
    EXPECT_EQ(result[4], 50);
}

/**
 * @brief Test getTopK default parameter is ascending
 * @details Verifies that getTopK() without parameter defaults to ascending order
 */
TEST(TopKTest, GetTopK_DefaultParameter_IsAscending) {
    TopK topK(5);

    topK.add(30);
    topK.add(10);
    topK.add(50);
    topK.add(20);
    topK.add(40);

    const auto result_default = topK.getTopK();         // Default parameter
    const auto result_explicit = topK.getTopK(0, true); // Explicit ascending

    // Both should be identical (ascending order)
    EXPECT_EQ(result_default, result_explicit);
    EXPECT_EQ(result_default[0], 10);
    EXPECT_EQ(result_default[4], 50);
}

/**
 * @brief Test getTopK descending order preserves heap state
 * @details Verifies that using descending order doesn't affect heap state
 */
TEST(TopKTest, GetTopK_DescendingOrder_PreservesHeapState) {
    TopK topK(3);

    topK.add(10);
    topK.add(20);
    topK.add(30);

    // Get in descending order
    const auto result_desc = topK.getTopK(0, false);
    EXPECT_EQ(result_desc.size(), 3);
    EXPECT_EQ(result_desc[0], 30);
    EXPECT_EQ(result_desc[1], 20);
    EXPECT_EQ(result_desc[2], 10);

    // Heap state should be preserved
    EXPECT_EQ(topK.size(), 3);

    // Can still get in ascending order
    const auto result_asc = topK.getTopK(0, true);
    EXPECT_EQ(result_asc[0], 10);
    EXPECT_EQ(result_asc[1], 20);
    EXPECT_EQ(result_asc[2], 30);
}

/**
 * @brief Test getTopK with dynamic count parameter
 * @details Verifies that getTopK can return fewer elements than heap size
 */
TEST(TopKTest, GetTopK_DynamicCount_ReturnsSpecifiedNumberOfElements) {
    TopK topK(10); // Maintain top 10

    // Add 10 numbers
    for (int i = 1; i <= 10; ++i) {
        topK.add(i);
    }

    EXPECT_EQ(topK.size(), 10);

    // Get top 3 (largest 3, ascending order)
    const auto top3 = topK.getTopK(3);
    EXPECT_EQ(top3.size(), 3);
    EXPECT_EQ(top3[0], 8); // Smallest of the top 3
    EXPECT_EQ(top3[1], 9);
    EXPECT_EQ(top3[2], 10); // Largest of the top 3

    // Get top 5 (largest 5, ascending order)
    const auto top5 = topK.getTopK(5);
    EXPECT_EQ(top5.size(), 5);
    EXPECT_EQ(top5[0], 6);  // Smallest of the top 5
    EXPECT_EQ(top5[4], 10); // Largest of the top 5

    // Get all (count > heap size)
    const auto topAll = topK.getTopK(100);
    EXPECT_EQ(topAll.size(), 10);
}

/**
 * @brief Test getTopK with dynamic count and descending order
 * @details Verifies that count and order parameters work together correctly
 */
TEST(TopKTest, GetTopK_DynamicCountWithDescending_ReturnsLargestFirst) {
    TopK topK(10);

    for (int i = 1; i <= 10; ++i) {
        topK.add(i);
    }

    // Get top 3 in descending order (largest 3)
    const auto top3_desc = topK.getTopK(3, false);
    EXPECT_EQ(top3_desc.size(), 3);
    EXPECT_EQ(top3_desc[0], 10); // Largest
    EXPECT_EQ(top3_desc[1], 9);
    EXPECT_EQ(top3_desc[2], 8); // 3rd largest

    // Get top 5 in descending order
    const auto top5_desc = topK.getTopK(5, false);
    EXPECT_EQ(top5_desc.size(), 5);
    EXPECT_EQ(top5_desc[0], 10);
    EXPECT_EQ(top5_desc[4], 6);
}

/**
 * @brief Test getTopK with count=0 returns all elements
 * @details Verifies that count=0 returns all elements
 */
TEST(TopKTest, GetTopK_ZeroCount_ReturnsAllElements) {
    TopK topK(10);

    for (int i = 1; i <= 7; ++i) {
        topK.add(i);
    }

    // count=0 should return all
    const auto result_zero = topK.getTopK(0);
    EXPECT_EQ(result_zero.size(), 7);

    // Default parameter (count=0) should return all
    const auto result_default = topK.getTopK();
    EXPECT_EQ(result_default.size(), 7);
}

/**
 * @brief Test getTopK with negative count throws exception
 * @details Verifies proper error handling for negative count values
 */
TEST(TopKTest, GetTopK_NegativeCount_ThrowsException) {
    TopK topK(10);

    for (int i = 1; i <= 7; ++i) {
        topK.add(i);
    }

    // Negative count should throw std::invalid_argument
    EXPECT_THROW(topK.getTopK(-1), std::invalid_argument);
    EXPECT_THROW(topK.getTopK(-100), std::invalid_argument);
    EXPECT_THROW(topK.getTopK(-1, true), std::invalid_argument);
    EXPECT_THROW(topK.getTopK(-1, false), std::invalid_argument);
}

/**
 * @brief Test unbounded TopK (default constructor)
 * @details Verifies that TopK without capacity limit can grow indefinitely
 */
TEST(TopKTest, Unbounded_TopK_GrowsIndefinitely) {
    TopK topK; // No capacity limit

    // Add 100 numbers
    for (int i = 1; i <= 100; ++i) {
        topK.add(i);
    }

    // Should contain all 100 numbers
    EXPECT_EQ(topK.size(), 100);

    // Get top 10
    const auto top10 = topK.getTopK(10, false); // Descending
    EXPECT_EQ(top10.size(), 10);
    EXPECT_EQ(top10[0], 100); // Largest
    EXPECT_EQ(top10[9], 91);
}

/**
 * @brief Test unbounded TopK with getTopK dynamic count
 * @details Verifies that unbounded TopK works correctly with dynamic count parameter
 */
TEST(TopKTest, Unbounded_TopK_DynamicCountWorks) {
    TopK topK; // Unbounded

    for (int i = 1; i <= 50; ++i) {
        topK.add(i);
    }

    EXPECT_EQ(topK.size(), 50);

    // Get different counts
    const auto top5 = topK.getTopK(5, false);
    EXPECT_EQ(top5.size(), 5);
    EXPECT_EQ(top5[0], 50);

    const auto top20 = topK.getTopK(20, false);
    EXPECT_EQ(top20.size(), 20);
    EXPECT_EQ(top20[0], 50);

    // Get all
    const auto all = topK.getTopK();
    EXPECT_EQ(all.size(), 50);
}

/**
 * @brief Test bounded vs unbounded TopK behavior difference
 * @details Compares behavior of bounded and unbounded TopK
 */
TEST(TopKTest, BoundedVsUnbounded_BehaviorComparison) {
    TopK bounded(3);
    TopK unbounded;

    // Add same numbers to both
    for (int i = 1; i <= 10; ++i) {
        bounded.add(i);
        unbounded.add(i);
    }

    // Bounded should only keep top 3
    EXPECT_EQ(bounded.size(), 3);
    const auto bounded_result = bounded.getTopK(3, false);
    EXPECT_EQ(bounded_result[0], 10);
    EXPECT_EQ(bounded_result[2], 8);

    // Unbounded should keep all 10
    EXPECT_EQ(unbounded.size(), 10);
    const auto unbounded_result = unbounded.getTopK(10, false);
    EXPECT_EQ(unbounded_result[0], 10);
    EXPECT_EQ(unbounded_result[9], 1);
}