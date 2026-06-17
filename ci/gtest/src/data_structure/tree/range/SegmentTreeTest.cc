/**
 * @file SegmentTreeTest.cc
 * @brief Unit tests for the SegmentTree class
 * @details Tests cover constructors, point update, range query,
 *          custom merge operations, edge cases, and type compatibility.
 */

#include <functional>
#include <string>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/tree/range/SegmentTree.hpp"

using namespace common::data_structure::tree::range;

namespace
{
    struct MaxOp
    {
        template <typename T>
        constexpr T operator()(const T& a, const T& b) const noexcept
        {
            return a > b ? a : b;
        }
    };

    struct MinOp
    {
        template <typename T>
        constexpr T operator()(const T& a, const T& b) const noexcept
        {
            return a < b ? a : b;
        }
    };
}

class SegmentTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Constructor Tests ====================

/**
 * @brief Test default constructor creates empty segment tree
 * @details Verifies that a default-constructed SegmentTree is empty with zero size
 */
TEST_F(SegmentTreeTest, DefaultConstructor_EmptyTree)
{
    const SegmentTree<int> st;
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

/**
 * @brief Test size constructor creates tree with correct size
 * @details Verifies that constructing with a size creates a non-empty tree
 *          with all elements initialized to zero
 */
TEST_F(SegmentTreeTest, SizeConstructor_CorrectSize)
{
    const SegmentTree<int> st(10);
    EXPECT_FALSE(st.empty());
    EXPECT_EQ(st.size(), 10);
    EXPECT_EQ(st.query(0, 10), 0);
}

/**
 * @brief Test size constructor with zero size
 * @details Verifies that constructing with size zero creates an empty tree
 */
TEST_F(SegmentTreeTest, SizeConstructor_ZeroSize)
{
    const SegmentTree<int> st(0);
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

/**
 * @brief Test size and init value constructor
 * @details Verifies that constructing with size and initial value initializes
 *          all elements to that value
 */
TEST_F(SegmentTreeTest, SizeInitValueConstructor_AllInitialized)
{
    const SegmentTree<int> st(5, 3);
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
    EXPECT_EQ(st.at(0), 3);
    EXPECT_EQ(st.at(4), 3);
}

/**
 * @brief Test iterator range constructor from vector
 * @details Verifies that constructing from an iterator range correctly
 *          initializes the tree with the given elements
 */
TEST_F(SegmentTreeTest, IteratorConstructor_FromVector)
{
    const std::vector<int> data = {1, 2, 3, 4, 5};
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
}

/**
 * @brief Test iterator constructor with empty range
 * @details Verifies that constructing from an empty range creates an empty tree
 */
TEST_F(SegmentTreeTest, IteratorConstructor_EmptyRange)
{
    const std::vector<int> data;
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_TRUE(st.empty());
}

/**
 * @brief Test initializer list constructor
 * @details Verifies that constructing from an initializer list works correctly
 */
TEST_F(SegmentTreeTest, InitializerListConstructor)
{
    const SegmentTree<int> st = {1, 2, 3, 4, 5};
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
}

/**
 * @brief Test initializer list constructor with empty list
 * @details Verifies that constructing from an empty initializer list creates an empty tree
 */
TEST_F(SegmentTreeTest, InitializerListConstructor_Empty)
{
    const SegmentTree<int> st = {};
    EXPECT_TRUE(st.empty());
}

// ==================== Point Update Tests ====================

/**
 * @brief Test updating a single element
 * @details Verifies that update changes the element value and range query reflects it
 */
TEST_F(SegmentTreeTest, Update_SingleElement)
{
    SegmentTree<int> st(1);
    st.update(0, 5);
    EXPECT_EQ(st.query(0, 1), 5);
    EXPECT_EQ(st.at(0), 5);
}

/**
 * @brief Test updating multiple elements
 * @details Verifies that multiple point updates accumulate correctly in range queries
 */
TEST_F(SegmentTreeTest, Update_MultipleElements)
{
    SegmentTree<int> st(5);
    st.update(0, 1);
    st.update(1, 2);
    st.update(2, 3);
    st.update(3, 4);
    st.update(4, 5);
    EXPECT_EQ(st.query(0, 5), 15);
}

/**
 * @brief Test update overwrites previous value
 * @details Verifies that updating the same position multiple times overwrites
 */
TEST_F(SegmentTreeTest, Update_OverwriteValue)
{
    SegmentTree<int> st(3);
    st.update(1, 10);
    st.update(1, 20);
    st.update(1, 30);
    EXPECT_EQ(st.at(1), 30);
    EXPECT_EQ(st.query(0, 3), 30);
}

/**
 * @brief Test update with out-of-bounds index throws
 * @details Verifies that update throws std::out_of_range for invalid indices
 */
TEST_F(SegmentTreeTest, Update_OutOfBounds_Throws)
{
    SegmentTree<int> st(3);
    EXPECT_THROW(st.update(3, 5), std::out_of_range);
    EXPECT_THROW(st.update(100, 5), std::out_of_range);
}

/**
 * @brief Test update on an empty tree throws
 * @details Verifies that update throws std::out_of_range when tree is empty
 */
TEST_F(SegmentTreeTest, Update_EmptyTree_Throws)
{
    SegmentTree<int> st;
    EXPECT_THROW(st.update(0, 5), std::out_of_range);
}

// ==================== Range Query Tests ====================

/**
 * @brief Test range query over the full range
 * @details Verifies that query(0, n) returns the total sum of all elements
 */
TEST_F(SegmentTreeTest, Query_FullRange)
{
    SegmentTree<int> st(5);
    for (int i = 0; i < 5; ++i) st.update(i, i + 1);
    EXPECT_EQ(st.query(0, 5), 15);
}

/**
 * @brief Test range query over sub-ranges
 * @details Verifies that query over various sub-ranges returns correct partial sums
 */
TEST_F(SegmentTreeTest, Query_SubRange)
{
    SegmentTree<int> st(5);
    for (int i = 0; i < 5; ++i) st.update(i, i + 1);
    EXPECT_EQ(st.query(1, 4), 9);
    EXPECT_EQ(st.query(0, 3), 6);
    EXPECT_EQ(st.query(2, 5), 12);
}

/**
 * @brief Test range query over a single element
 * @details Verifies that query(i, i+1) returns the value at position i
 */
TEST_F(SegmentTreeTest, Query_SingleElement)
{
    SegmentTree<int> st(5);
    st.update(2, 10);
    EXPECT_EQ(st.query(2, 3), 10);
}

/**
 * @brief Test range query with invalid range throws
 * @details Verifies that query throws std::out_of_range when left >= right
 */
TEST_F(SegmentTreeTest, Query_InvalidRange_Throws)
{
    const SegmentTree<int> st(5);
    EXPECT_THROW(st.query(3, 2), std::out_of_range);
    EXPECT_THROW(st.query(3, 3), std::out_of_range);
}

/**
 * @brief Test range query with out-of-bounds range throws
 * @details Verifies that query throws std::out_of_range when range exceeds tree size
 */
TEST_F(SegmentTreeTest, Query_OutOfBounds_Throws)
{
    const SegmentTree<int> st(3);
    EXPECT_THROW(st.query(0, 4), std::out_of_range);
}

/**
 * @brief Test range query on an empty tree throws
 * @details Verifies that query throws std::out_of_range when tree is empty
 */
TEST_F(SegmentTreeTest, Query_EmptyTree_Throws)
{
    const SegmentTree<int> st;
    EXPECT_THROW(st.query(0, 1), std::out_of_range);
}

// ==================== At Tests ====================

/**
 * @brief Test at element access
 * @details Verifies that at() returns the correct values at various positions
 */
TEST_F(SegmentTreeTest, At_AccessElements)
{
    SegmentTree<int> st(5);
    st.update(0, 10);
    st.update(1, 20);
    st.update(2, 30);
    EXPECT_EQ(st.at(0), 10);
    EXPECT_EQ(st.at(1), 20);
    EXPECT_EQ(st.at(2), 30);
    EXPECT_EQ(st.at(3), 0);
    EXPECT_EQ(st.at(4), 0);
}

/**
 * @brief Test at with out-of-bounds index throws
 * @details Verifies that at() throws std::out_of_range for invalid indices
 */
TEST_F(SegmentTreeTest, At_OutOfBounds_Throws)
{
    const SegmentTree<int> st(3);
    EXPECT_THROW(st.at(3), std::out_of_range);
    EXPECT_THROW(st.at(100), std::out_of_range);
}

/**
 * @brief Test at on an empty tree throws
 * @details Verifies that at() throws std::out_of_range when tree is empty
 */
TEST_F(SegmentTreeTest, At_EmptyTree_Throws)
{
    const SegmentTree<int> st;
    EXPECT_THROW(st.at(0), std::out_of_range);
}

// ==================== Size & Empty Tests ====================

/**
 * @brief Test empty returns true for default-constructed tree
 * @details Verifies that a default-constructed SegmentTree is empty
 */
TEST_F(SegmentTreeTest, Empty_InitiallyEmpty)
{
    const SegmentTree<int> st;
    EXPECT_TRUE(st.empty());
}

/**
 * @brief Test empty returns false for non-empty tree
 * @details Verifies that a tree with elements is not empty
 */
TEST_F(SegmentTreeTest, Empty_NonEmpty)
{
    const SegmentTree<int> st(5);
    EXPECT_FALSE(st.empty());
}

/**
 * @brief Test size returns correct element count
 * @details Verifies that size() returns the number of elements for various sizes
 */
TEST_F(SegmentTreeTest, Size_CorrectCount)
{
    const SegmentTree<int> st;
    EXPECT_EQ(st.size(), 0);
    const SegmentTree<int> st2(10);
    EXPECT_EQ(st2.size(), 10);
    const SegmentTree<int> st3(100);
    EXPECT_EQ(st3.size(), 100);
}

// ==================== Swap Tests ====================

/**
 * @brief Test swap exchanges contents between two trees
 * @details Verifies that after swap, each tree holds the other's data
 */
TEST_F(SegmentTreeTest, Swap_ExchangesContents)
{
    SegmentTree<int> st1(3);
    st1.update(0, 1);
    st1.update(1, 2);
    st1.update(2, 3);

    SegmentTree<int> st2(2);
    st2.update(0, 10);
    st2.update(1, 20);

    st1.swap(st2);

    EXPECT_EQ(st1.size(), 2);
    EXPECT_EQ(st1.at(0), 10);
    EXPECT_EQ(st1.at(1), 20);

    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
}

/**
 * @brief Test swap with an empty tree
 * @details Verifies that swapping a non-empty tree with an empty tree works correctly
 */
TEST_F(SegmentTreeTest, Swap_WithEmpty)
{
    SegmentTree<int> st1(2);
    st1.update(0, 5);
    st1.update(1, 10);

    SegmentTree<int> st2;

    st1.swap(st2);

    EXPECT_TRUE(st1.empty());
    EXPECT_EQ(st2.size(), 2);
    EXPECT_EQ(st2.at(0), 5);
}

/**
 * @brief Test swap is symmetric
 * @details Verifies that swapping twice restores the original contents
 */
TEST_F(SegmentTreeTest, Swap_Symmetric)
{
    SegmentTree<int> st1(2);
    st1.update(0, 1);
    st1.update(1, 2);

    SegmentTree<int> st2(2);
    st2.update(0, 3);
    st2.update(1, 4);

    st1.swap(st2);
    st1.swap(st2);

    EXPECT_EQ(st1.at(0), 1);
    EXPECT_EQ(st1.at(1), 2);
    EXPECT_EQ(st2.at(0), 3);
    EXPECT_EQ(st2.at(1), 4);
}

// ==================== Custom Merge Operation Tests ====================

/**
 * @brief Test custom merge with max operation
 * @details Verifies that a segment tree using MaxOp correctly computes range maximums
 */
TEST_F(SegmentTreeTest, CustomMerge_MaxOp)
{
    SegmentTree<int, MaxOp> st(5);
    st.update(0, 1);
    st.update(1, 5);
    st.update(2, 3);
    st.update(3, 7);
    st.update(4, 2);
    EXPECT_EQ(st.query(0, 5), 7);
    EXPECT_EQ(st.query(0, 2), 5);
    EXPECT_EQ(st.query(2, 4), 7);
}

/**
 * @brief Test custom merge with min operation
 * @details Verifies that a segment tree using MinOp correctly computes range minimums
 */
TEST_F(SegmentTreeTest, CustomMerge_MinOp)
{
    SegmentTree<int, MinOp> st(5);
    st.update(0, 1);
    st.update(1, 5);
    st.update(2, 3);
    st.update(3, 7);
    st.update(4, 2);
    EXPECT_EQ(st.query(0, 5), 1);
    EXPECT_EQ(st.query(1, 3), 3);
    EXPECT_EQ(st.query(3, 5), 2);
}

/**
 * @brief Test custom merge with multiplication
 * @details Verifies that a segment tree using multiplication works correctly
 */
TEST_F(SegmentTreeTest, CustomMerge_Multiplies)
{
    SegmentTree<int, std::multiplies<>> st(3);
    st.update(0, 2);
    st.update(1, 3);
    st.update(2, 4);
    EXPECT_EQ(st.query(0, 3), 24);
    EXPECT_EQ(st.query(0, 2), 6);
}

/**
 * @brief Test custom merge with string concatenation
 * @details Verifies that a segment tree with string concatenation works correctly
 */
TEST_F(SegmentTreeTest, CustomMerge_StringConcat)
{
    SegmentTree<std::string, std::plus<>> st(3);
    st.update(0, "a");
    st.update(1, "b");
    st.update(2, "c");
    EXPECT_EQ(st.query(0, 3), "abc");
    EXPECT_EQ(st.query(1, 3), "bc");
}

// ==================== Copy & Move Tests ====================

/**
 * @brief Test copy constructor creates independent copy
 * @details Verifies that the copy constructor creates a deep copy that does not share data
 */
TEST_F(SegmentTreeTest, CopyConstructor_CopiesCorrectly)
{
    SegmentTree<int> st1(3);
    st1.update(0, 1);
    st1.update(1, 2);
    st1.update(2, 3);

    SegmentTree<int> st2(st1);
    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
    EXPECT_EQ(st2.query(0, 3), 6);

    st2.update(0, 10);
    EXPECT_EQ(st1.at(0), 1);
}

/**
 * @brief Test copy assignment creates independent copy
 * @details Verifies that copy assignment creates a deep copy with independent data
 */
TEST_F(SegmentTreeTest, CopyAssignment_CopiesCorrectly)
{
    SegmentTree<int> st1(3);
    st1.update(0, 1);
    st1.update(1, 2);
    st1.update(2, 3);

    SegmentTree<int> st2 = st1;
    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
    EXPECT_EQ(st2.query(0, 3), 6);

    st2.update(0, 10);
    EXPECT_EQ(st1.at(0), 1);
}

/**
 * @brief Test self-assignment via copy
 * @details Verifies that self-assignment works correctly without data corruption
 */
TEST_F(SegmentTreeTest, CopyAssignment_SelfAssignment)
{
    SegmentTree<int> st(3);
    st.update(0, 1);
    st.update(1, 2);
    st.update(2, 3);

    const auto& st_cref = st;
    st = st_cref;
    EXPECT_EQ(st.size(), 3);
    EXPECT_EQ(st.at(0), 1);
    EXPECT_EQ(st.at(1), 2);
    EXPECT_EQ(st.at(2), 3);
}

/**
 * @brief Test move constructor moves data correctly
 * @details Verifies that the move constructor transfers data to the new tree
 */
TEST_F(SegmentTreeTest, MoveConstructor_MovesCorrectly)
{
    SegmentTree<int> st1(3);
    st1.update(0, 1);
    st1.update(1, 2);
    st1.update(2, 3);

    const SegmentTree<int> st2(std::move(st1));
    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
    EXPECT_EQ(st2.query(0, 3), 6);
}

/**
 * @brief Test move assignment moves data correctly
 * @details Verifies that move assignment transfers data to the destination
 */
TEST_F(SegmentTreeTest, MoveAssignment_MovesCorrectly)
{
    SegmentTree<int> st1(3);
    st1.update(0, 1);
    st1.update(1, 2);
    st1.update(2, 3);

    const SegmentTree<int> st2(std::move(st1));
    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
    EXPECT_EQ(st2.query(0, 3), 6);
}

/**
 * @brief Test self-swap does not corrupt data
 * @details Verifies that swapping a tree with itself is safe
 */
TEST_F(SegmentTreeTest, Swap_SelfSwap)
{
    SegmentTree<int> st(3);
    st.update(0, 1);
    st.update(1, 2);
    st.update(2, 3);

    st.swap(st);
    EXPECT_EQ(st.size(), 3);
    EXPECT_EQ(st.at(0), 1);
    EXPECT_EQ(st.at(1), 2);
    EXPECT_EQ(st.at(2), 3);
    EXPECT_EQ(st.query(0, 3), 6);
}

// ==================== Single Element Edge Cases ====================

/**
 * @brief Test all operations on a single-element tree
 * @details Verifies that update, query, at, and out-of-bounds checks work
 *          correctly on a tree with one element
 */
TEST_F(SegmentTreeTest, SingleElement_AllOperations)
{
    SegmentTree<int> st(1);
    EXPECT_EQ(st.size(), 1);
    EXPECT_FALSE(st.empty());
    EXPECT_EQ(st.at(0), 0);
    EXPECT_EQ(st.query(0, 1), 0);

    st.update(0, 42);
    EXPECT_EQ(st.at(0), 42);
    EXPECT_EQ(st.query(0, 1), 42);

    EXPECT_THROW(st.update(1, 0), std::out_of_range);
    EXPECT_THROW(std::as_const(st).query(0, 2), std::out_of_range);
    EXPECT_THROW(std::as_const(st).at(1), std::out_of_range);
}

/**
 * @brief Test iterator construction with single element
 * @details Verifies that constructing from a single-element range works
 */
TEST_F(SegmentTreeTest, SingleElement_IteratorConstruct)
{
    const std::vector<int> data = {7};
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_EQ(st.size(), 1);
    EXPECT_EQ(st.at(0), 7);
    EXPECT_EQ(st.query(0, 1), 7);
}

/**
 * @brief Test initializer list construction with single element
 * @details Verifies that constructing with a single-element initializer list works
 */
TEST_F(SegmentTreeTest, SingleElement_InitListConstruct)
{
    const SegmentTree<int> st = {7};
    EXPECT_EQ(st.size(), 1);
    EXPECT_EQ(st.at(0), 7);
    EXPECT_EQ(st.query(0, 1), 7);
}

// ==================== Negative Values ====================

/**
 * @brief Test segment tree with negative values
 * @details Verifies that range queries with negative values compute correct sums
 */
TEST_F(SegmentTreeTest, NegativeValues_Correctness)
{
    SegmentTree<int> st(4);
    st.update(0, -5);
    st.update(1, -3);
    st.update(2, 0);
    st.update(3, 7);
    EXPECT_EQ(st.query(0, 4), -1);
    EXPECT_EQ(st.query(0, 2), -8);
    EXPECT_EQ(st.query(1, 3), -3);
    EXPECT_EQ(st.query(2, 4), 7);
}

/**
 * @brief Test negative values with custom min operation
 * @details Verifies that a segment tree with MinOp handles negative values correctly
 */
TEST_F(SegmentTreeTest, NegativeValues_WithCustomMinOp)
{
    SegmentTree<int, MinOp> st(4);
    st.update(0, -5);
    st.update(1, -3);
    st.update(2, 0);
    st.update(3, 7);
    EXPECT_EQ(st.query(0, 4), -5);
    EXPECT_EQ(st.query(0, 2), -5);
    EXPECT_EQ(st.query(1, 3), -3);
    EXPECT_EQ(st.query(2, 4), 0);
    EXPECT_EQ(st.query(3, 4), 7);
}

// ==================== Large Dataset ====================

/**
 * @brief Test correctness with a large dataset
 * @details Verifies that the segment tree handles 1000 elements correctly,
 *          including updates, range queries, and zeroing out even indices
 */
TEST_F(SegmentTreeTest, LargeDataset_Correctness)
{
    constexpr int kCount = 1000;
    SegmentTree<int> st(kCount);

    for (int i = 0; i < kCount; ++i)
    {
        st.update(i, i + 1);
    }

    EXPECT_EQ(st.query(0, kCount), kCount * (kCount + 1) / 2);

    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(st.at(i), i + 1);
    }

    for (int i = 0; i < kCount; i += 2)
    {
        st.update(i, 0);
    }

    int expected = 0;
    for (int i = 0; i < kCount; ++i)
    {
        if (i % 2 != 0)
        {
            expected += i + 1;
        }
    }
    EXPECT_EQ(st.query(0, kCount), expected);
}

// ==================== Different Types ====================

/**
 * @brief Test segment tree with double type
 * @details Verifies that the segment tree works correctly with double-precision values
 */
TEST_F(SegmentTreeTest, DoubleType_Correctness)
{
    SegmentTree<double> st(3);
    st.update(0, 1.5);
    st.update(1, 2.5);
    st.update(2, 3.0);
    EXPECT_DOUBLE_EQ(st.query(0, 3), 7.0);
    EXPECT_DOUBLE_EQ(st.at(1), 2.5);
}

/**
 * @brief Test segment tree with long long type
 * @details Verifies that the segment tree works correctly with large 64-bit integers
 */
TEST_F(SegmentTreeTest, LongLongType_Correctness)
{
    SegmentTree<long long> st(3);
    st.update(0, 10000000000LL);
    st.update(1, 20000000000LL);
    st.update(2, 30000000000LL);
    EXPECT_EQ(st.query(0, 3), 60000000000LL);
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

/**
 * @brief Test move constructor
 * @details Verifies that the move constructor transfers data correctly
 */
TEST_F(SegmentTreeTest, MoveConstructor)
{
    SegmentTree<int> st(4);
    st.update(0, 1);
    st.update(1, 2);
    st.update(2, 3);
    EXPECT_EQ(st.query(0, 4), 6);

    const SegmentTree<int> other(std::move(st));
    EXPECT_EQ(other.query(0, 4), 6);
}

/**
 * @brief Test move assignment
 * @details Verifies that move assignment transfers data correctly
 */
TEST_F(SegmentTreeTest, MoveAssignment)
{
    SegmentTree<int> st(3);
    st.update(0, 10);
    st.update(1, 20);
    EXPECT_EQ(st.query(0, 2), 30);

    SegmentTree<int> other(10);
    other = std::move(st);
    EXPECT_EQ(other.query(0, 2), 30);
}
