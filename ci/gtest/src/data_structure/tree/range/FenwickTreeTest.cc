/**
 * @file FenwickTreeTest.cc
 * @brief Unit tests for the FenwickTree class
 * @details Tests cover point update, prefix sum, range sum,
 *          constructors, edge cases, and type compatibility.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/tree/range/FenwickTree.hpp>

using namespace cppforge::data_structure::tree::range;

class FenwickTreeTest : public testing::Test
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
 * @brief Test default constructor creates empty Fenwick tree
 * @details Verifies that a default-constructed FenwickTree is empty with zero size
 */
TEST_F(FenwickTreeTest, DefaultConstructor_EmptyTree)
{
    const FenwickTree<int> ft;
    EXPECT_TRUE(ft.empty());
    EXPECT_EQ(ft.size(), 0);
}

/**
 * @brief Test size constructor creates tree with correct size
 * @details Verifies that constructing with a size creates a non-empty tree
 *          with all elements initialized to zero
 */
TEST_F(FenwickTreeTest, SizeConstructor_CorrectSize)
{
    const FenwickTree<int> ft(10);
    EXPECT_FALSE(ft.empty());
    EXPECT_EQ(ft.size(), 10);
    EXPECT_EQ(ft.sum(10), 0);
}

/**
 * @brief Test size constructor with zero size
 * @details Verifies that constructing with size zero creates an empty tree
 */
TEST_F(FenwickTreeTest, SizeConstructor_ZeroSize)
{
    const FenwickTree<int> ft(0);
    EXPECT_TRUE(ft.empty());
    EXPECT_EQ(ft.size(), 0);
}

/**
 * @brief Test size and init value constructor
 * @details Verifies that constructing with size and initial value initializes
 *          all elements to that value
 */
TEST_F(FenwickTreeTest, SizeInitValueConstructor_AllInitialized)
{
    const FenwickTree<int> ft(5, 3);
    EXPECT_EQ(ft.size(), 5);
    EXPECT_EQ(ft.sum(5), 15);
    EXPECT_EQ(ft.at(0), 3);
    EXPECT_EQ(ft.at(4), 3);
}

/**
 * @brief Test iterator range constructor from vector
 * @details Verifies that constructing from an iterator range correctly
 *          initializes the tree with the given elements
 */
TEST_F(FenwickTreeTest, IteratorConstructor_FromVector)
{
    const std::vector<int> data = {1, 2, 3, 4, 5};
    const FenwickTree<int> ft(data.begin(), data.end());
    EXPECT_EQ(ft.size(), 5);
    EXPECT_EQ(ft.sum(5), 15);
}

/**
 * @brief Test iterator constructor with empty range
 * @details Verifies that constructing from an empty range creates an empty tree
 */
TEST_F(FenwickTreeTest, IteratorConstructor_EmptyRange)
{
    const std::vector<int> data;
    const FenwickTree<int> ft(data.begin(), data.end());
    EXPECT_TRUE(ft.empty());
}

/**
 * @brief Test initializer list constructor
 * @details Verifies that constructing from an initializer list works correctly
 */
TEST_F(FenwickTreeTest, InitializerListConstructor)
{
    const FenwickTree<int> ft = {1, 2, 3, 4, 5};
    EXPECT_EQ(ft.size(), 5);
    EXPECT_EQ(ft.sum(5), 15);
}

/**
 * @brief Test initializer list constructor with empty list
 * @details Verifies that constructing from an empty initializer list creates an empty tree
 */
TEST_F(FenwickTreeTest, InitializerListConstructor_Empty)
{
    const FenwickTree<int> ft = {};
    EXPECT_TRUE(ft.empty());
}

// ==================== Point Update Tests ====================

/**
 * @brief Test adding value to a single element
 * @details Verifies that add updates the element and the prefix sum correctly
 */
TEST_F(FenwickTreeTest, Add_SingleElement)
{
    FenwickTree<int> ft(1);
    ft.add(0, 5);
    EXPECT_EQ(ft.sum(1), 5);
    EXPECT_EQ(ft.at(0), 5);
}

/**
 * @brief Test adding values to multiple elements
 * @details Verifies that multiple point updates accumulate correctly
 */
TEST_F(FenwickTreeTest, Add_MultipleUpdates)
{
    FenwickTree<int> ft(5);
    ft.add(0, 1);
    ft.add(1, 2);
    ft.add(2, 3);
    ft.add(3, 4);
    ft.add(4, 5);
    EXPECT_EQ(ft.sum(5), 15);
}

/**
 * @brief Test add is accumulative at the same position
 * @details Verifies that multiple adds at the same index accumulate correctly
 */
TEST_F(FenwickTreeTest, Add_Accumulative)
{
    FenwickTree<int> ft(3);
    ft.add(1, 10);
    ft.add(1, 20);
    ft.add(1, 30);
    EXPECT_EQ(ft.at(1), 60);
    EXPECT_EQ(ft.sum(3), 60);
}

/**
 * @brief Test adding a negative delta
 * @details Verifies that add works correctly with negative values
 */
TEST_F(FenwickTreeTest, Add_NegativeDelta)
{
    FenwickTree<int> ft(3);
    ft.add(0, 10);
    ft.add(1, 20);
    ft.add(2, 30);
    ft.add(1, -10);
    EXPECT_EQ(ft.at(1), 10);
    EXPECT_EQ(ft.sum(3), 50);
}

/**
 * @brief Test add with out-of-bounds index throws
 * @details Verifies that add throws std::out_of_range for invalid indices
 */
TEST_F(FenwickTreeTest, Add_OutOfBounds_Throws)
{
    FenwickTree<int> ft(3);
    EXPECT_THROW(ft.add(3, 5), std::out_of_range);
    EXPECT_THROW(ft.add(100, 5), std::out_of_range);
}

/**
 * @brief Test add on an empty tree throws
 * @details Verifies that add throws std::out_of_range when tree is empty
 */
TEST_F(FenwickTreeTest, Add_EmptyTree_Throws)
{
    FenwickTree<int> ft;
    EXPECT_THROW(ft.add(0, 5), std::out_of_range);
}

// ==================== Prefix Sum Tests ====================

/**
 * @brief Test prefix sum correctness
 * @details Verifies that prefix sums at various positions are computed correctly
 */
TEST_F(FenwickTreeTest, Sum_PrefixCorrectness)
{
    FenwickTree<int> ft(5);
    ft.add(0, 1);
    ft.add(1, 2);
    ft.add(2, 3);
    ft.add(3, 4);
    ft.add(4, 5);

    EXPECT_EQ(ft.sum(0), 0);
    EXPECT_EQ(ft.sum(1), 1);
    EXPECT_EQ(ft.sum(2), 3);
    EXPECT_EQ(ft.sum(3), 6);
    EXPECT_EQ(ft.sum(4), 10);
    EXPECT_EQ(ft.sum(5), 15);
}

/**
 * @brief Test prefix sum with out-of-bounds position throws
 * @details Verifies that sum throws std::out_of_range for invalid positions
 */
TEST_F(FenwickTreeTest, Sum_PosOutOfBounds_Throws)
{
    const FenwickTree<int> ft(3);
    EXPECT_THROW(static_cast<void>(ft.sum(4)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(ft.sum(100)), std::out_of_range);
}

/**
 * @brief Test prefix sum on an empty tree throws
 * @details Verifies that sum throws std::out_of_range when tree is empty
 */
TEST_F(FenwickTreeTest, Sum_EmptyTree_Throws)
{
    const FenwickTree<int> ft;
    EXPECT_THROW(static_cast<void>(ft.sum(1)), std::out_of_range);
}

/**
 * @brief Test prefix sum at position zero returns zero
 * @details Verifies that sum(0) always returns zero
 */
TEST_F(FenwickTreeTest, Sum_ZeroPos_ReturnsZero)
{
    const FenwickTree<int> ft(5);
    EXPECT_EQ(ft.sum(0), 0);
}

// ==================== Range Sum Tests ====================

/**
 * @brief Test range sum over the full range
 * @details Verifies that sum(0, n) returns the total sum of all elements
 */
TEST_F(FenwickTreeTest, RangeSum_FullRange)
{
    FenwickTree<int> ft(5);
    for (int i = 0; i < 5; ++i) ft.add(i, i + 1);
    EXPECT_EQ(ft.sum(0, 5), 15);
}

/**
 * @brief Test range sum over sub-ranges
 * @details Verifies that sum over various sub-ranges returns correct partial sums
 */
TEST_F(FenwickTreeTest, RangeSum_SubRange)
{
    FenwickTree<int> ft(5);
    for (int i = 0; i < 5; ++i) ft.add(i, i + 1);
    EXPECT_EQ(ft.sum(1, 4), 9);
    EXPECT_EQ(ft.sum(0, 3), 6);
    EXPECT_EQ(ft.sum(2, 5), 12);
}

/**
 * @brief Test range sum over a single element
 * @details Verifies that sum(i, i+1) returns the value at position i
 */
TEST_F(FenwickTreeTest, RangeSum_SingleElement)
{
    FenwickTree<int> ft(5);
    ft.add(2, 10);
    EXPECT_EQ(ft.sum(2, 3), 10);
}

/**
 * @brief Test range sum with invalid range throws
 * @details Verifies that sum throws std::out_of_range when left >= right
 */
TEST_F(FenwickTreeTest, RangeSum_EmptyRange_Throws)
{
    const FenwickTree<int> ft(5);
    EXPECT_THROW(static_cast<void>(ft.sum(3, 2)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(ft.sum(3, 3)), std::out_of_range);
}

/**
 * @brief Test range sum with out-of-bounds range throws
 * @details Verifies that sum throws std::out_of_range when range exceeds tree size
 */
TEST_F(FenwickTreeTest, RangeSum_OutOfBounds_Throws)
{
    const FenwickTree<int> ft(3);
    EXPECT_THROW(static_cast<void>(ft.sum(0, 4)), std::out_of_range);
}

// ==================== At Tests ====================

/**
 * @brief Test at element access
 * @details Verifies that at() returns the correct values at various positions
 */
TEST_F(FenwickTreeTest, At_AccessElements)
{
    FenwickTree<int> ft(5);
    ft.add(0, 10);
    ft.add(1, 20);
    ft.add(2, 30);
    EXPECT_EQ(ft.at(0), 10);
    EXPECT_EQ(ft.at(1), 20);
    EXPECT_EQ(ft.at(2), 30);
    EXPECT_EQ(ft.at(3), 0);
    EXPECT_EQ(ft.at(4), 0);
}

/**
 * @brief Test at with out-of-bounds index throws
 * @details Verifies that at() throws std::out_of_range for invalid indices
 */
TEST_F(FenwickTreeTest, At_OutOfBounds_Throws)
{
    const FenwickTree<int> ft(3);
    EXPECT_THROW(static_cast<void>(ft.at(3)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(ft.at(100)), std::out_of_range);
}

/**
 * @brief Test at on an empty tree throws
 * @details Verifies that at() throws std::out_of_range when tree is empty
 */
TEST_F(FenwickTreeTest, At_EmptyTree_Throws)
{
    const FenwickTree<int> ft;
    EXPECT_THROW(static_cast<void>(ft.at(0)), std::out_of_range);
}

// ==================== Size & Empty Tests ====================

/**
 * @brief Test empty returns true for default-constructed tree
 * @details Verifies that a default-constructed FenwickTree is empty
 */
TEST_F(FenwickTreeTest, Empty_InitiallyEmpty)
{
    const FenwickTree<int> ft;
    EXPECT_TRUE(ft.empty());
}

/**
 * @brief Test empty returns false for non-empty tree
 * @details Verifies that a tree with elements is not empty
 */
TEST_F(FenwickTreeTest, Empty_NonEmpty)
{
    const FenwickTree<int> ft(5);
    EXPECT_FALSE(ft.empty());
}

/**
 * @brief Test size returns correct element count
 * @details Verifies that size() returns the number of elements for various sizes
 */
TEST_F(FenwickTreeTest, Size_CorrectCount)
{
    const FenwickTree<int> ft;
    EXPECT_EQ(ft.size(), 0);
    const FenwickTree<int> ft2(10);
    EXPECT_EQ(ft2.size(), 10);
    const FenwickTree<int> ft3(100);
    EXPECT_EQ(ft3.size(), 100);
}

// ==================== Swap Tests ====================

/**
 * @brief Test swap exchanges contents between two trees
 * @details Verifies that after swap, each tree holds the other's data
 */
TEST_F(FenwickTreeTest, Swap_ExchangesContents)
{
    FenwickTree<int> ft1(3);
    ft1.add(0, 1);
    ft1.add(1, 2);
    ft1.add(2, 3);

    FenwickTree<int> ft2(2);
    ft2.add(0, 10);
    ft2.add(1, 20);

    ft1.swap(ft2);

    EXPECT_EQ(ft1.size(), 2);
    EXPECT_EQ(ft1.at(0), 10);
    EXPECT_EQ(ft1.at(1), 20);

    EXPECT_EQ(ft2.size(), 3);
    EXPECT_EQ(ft2.at(0), 1);
    EXPECT_EQ(ft2.at(1), 2);
    EXPECT_EQ(ft2.at(2), 3);
}

/**
 * @brief Test swap with an empty tree
 * @details Verifies that swapping a non-empty tree with an empty tree works correctly
 */
TEST_F(FenwickTreeTest, Swap_WithEmpty)
{
    FenwickTree<int> ft1(2);
    ft1.add(0, 5);
    ft1.add(1, 10);

    FenwickTree<int> ft2;

    ft1.swap(ft2);

    EXPECT_TRUE(ft1.empty());
    EXPECT_EQ(ft2.size(), 2);
    EXPECT_EQ(ft2.at(0), 5);
}

/**
 * @brief Test swap is symmetric
 * @details Verifies that swapping twice restores the original contents
 */
TEST_F(FenwickTreeTest, Swap_Symmetric)
{
    FenwickTree<int> ft1(2);
    ft1.add(0, 1);
    ft1.add(1, 2);

    FenwickTree<int> ft2(2);
    ft2.add(0, 3);
    ft2.add(1, 4);

    ft1.swap(ft2);
    ft1.swap(ft2);

    EXPECT_EQ(ft1.at(0), 1);
    EXPECT_EQ(ft1.at(1), 2);
    EXPECT_EQ(ft2.at(0), 3);
    EXPECT_EQ(ft2.at(1), 4);
}

// ==================== Large Dataset ====================

/**
 * @brief Test correctness with a large dataset
 * @details Verifies that the Fenwick tree handles 1000 elements correctly,
 *          including point updates, prefix sums, and zeroing out even indices
 */
TEST_F(FenwickTreeTest, LargeDataset_Correctness)
{
    constexpr int kCount = 1000;
    FenwickTree<int> ft(kCount);

    for (int i = 0; i < kCount; ++i)
    {
        ft.add(i, i + 1);
    }

    EXPECT_EQ(ft.sum(kCount), kCount * (kCount + 1) / 2);

    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(ft.at(i), i + 1);
    }

    for (int i = 0; i < kCount; i += 2)
    {
        ft.add(i, -ft.at(i));
    }

    int expected = 0;
    for (int i = 0; i < kCount; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_EQ(ft.at(i), 0);
        }
        else
        {
            EXPECT_EQ(ft.at(i), i + 1);
            expected += i + 1;
        }
    }
    EXPECT_EQ(ft.sum(kCount), expected);
}

// ==================== Different Types ====================

/**
 * @brief Test Fenwick tree with double type
 * @details Verifies that the Fenwick tree works correctly with double-precision values
 */
TEST_F(FenwickTreeTest, DoubleType_Correctness)
{
    FenwickTree<double> ft(3);
    ft.add(0, 1.5);
    ft.add(1, 2.5);
    ft.add(2, 3.0);
    EXPECT_DOUBLE_EQ(ft.sum(3), 7.0);
    EXPECT_DOUBLE_EQ(ft.at(1), 2.5);
}

/**
 * @brief Test Fenwick tree with long long type
 * @details Verifies that the Fenwick tree works correctly with large 64-bit integers
 */
TEST_F(FenwickTreeTest, LongLongType_Correctness)
{
    FenwickTree<long long> ft(3);
    ft.add(0, 10000000000LL);
    ft.add(1, 20000000000LL);
    ft.add(2, 30000000000LL);
    EXPECT_EQ(ft.sum(3), 60000000000LL);
}

/**
 * @brief Test Fenwick tree with float type
 * @details Verifies that the Fenwick tree works correctly with single-precision floats
 */
TEST_F(FenwickTreeTest, FloatType_Correctness)
{
    FenwickTree<float> ft(3);
    ft.add(0, 1.0f);
    ft.add(1, 2.0f);
    ft.add(2, 3.0f);
    EXPECT_FLOAT_EQ(ft.sum(3), 6.0f);
}
