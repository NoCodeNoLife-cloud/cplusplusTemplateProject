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

#include "data_structure/tree/SegmentTree.hpp"

using namespace common::data_structure::tree;

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

TEST_F(SegmentTreeTest, DefaultConstructor_EmptyTree)
{
    const SegmentTree<int> st;
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

TEST_F(SegmentTreeTest, SizeConstructor_CorrectSize)
{
    const SegmentTree<int> st(10);
    EXPECT_FALSE(st.empty());
    EXPECT_EQ(st.size(), 10);
    EXPECT_EQ(st.query(0, 10), 0);
}

TEST_F(SegmentTreeTest, SizeConstructor_ZeroSize)
{
    const SegmentTree<int> st(0);
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

TEST_F(SegmentTreeTest, SizeInitValueConstructor_AllInitialized)
{
    const SegmentTree<int> st(5, 3);
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
    EXPECT_EQ(st.at(0), 3);
    EXPECT_EQ(st.at(4), 3);
}

TEST_F(SegmentTreeTest, IteratorConstructor_FromVector)
{
    const std::vector<int> data = {1, 2, 3, 4, 5};
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
}

TEST_F(SegmentTreeTest, IteratorConstructor_EmptyRange)
{
    const std::vector<int> data;
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_TRUE(st.empty());
}

TEST_F(SegmentTreeTest, InitializerListConstructor)
{
    const SegmentTree<int> st = {1, 2, 3, 4, 5};
    EXPECT_EQ(st.size(), 5);
    EXPECT_EQ(st.query(0, 5), 15);
}

TEST_F(SegmentTreeTest, InitializerListConstructor_Empty)
{
    const SegmentTree<int> st = {};
    EXPECT_TRUE(st.empty());
}

// ==================== Point Update Tests ====================

TEST_F(SegmentTreeTest, Update_SingleElement)
{
    SegmentTree<int> st(1);
    st.update(0, 5);
    EXPECT_EQ(st.query(0, 1), 5);
    EXPECT_EQ(st.at(0), 5);
}

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

TEST_F(SegmentTreeTest, Update_OverwriteValue)
{
    SegmentTree<int> st(3);
    st.update(1, 10);
    st.update(1, 20);
    st.update(1, 30);
    EXPECT_EQ(st.at(1), 30);
    EXPECT_EQ(st.query(0, 3), 30);
}

TEST_F(SegmentTreeTest, Update_OutOfBounds_Throws)
{
    SegmentTree<int> st(3);
    EXPECT_THROW(st.update(3, 5), std::out_of_range);
    EXPECT_THROW(st.update(100, 5), std::out_of_range);
}

TEST_F(SegmentTreeTest, Update_EmptyTree_Throws)
{
    SegmentTree<int> st;
    EXPECT_THROW(st.update(0, 5), std::out_of_range);
}

// ==================== Range Query Tests ====================

TEST_F(SegmentTreeTest, Query_FullRange)
{
    SegmentTree<int> st(5);
    for (int i = 0; i < 5; ++i) st.update(i, i + 1);
    EXPECT_EQ(st.query(0, 5), 15);
}

TEST_F(SegmentTreeTest, Query_SubRange)
{
    SegmentTree<int> st(5);
    for (int i = 0; i < 5; ++i) st.update(i, i + 1);
    EXPECT_EQ(st.query(1, 4), 9);
    EXPECT_EQ(st.query(0, 3), 6);
    EXPECT_EQ(st.query(2, 5), 12);
}

TEST_F(SegmentTreeTest, Query_SingleElement)
{
    SegmentTree<int> st(5);
    st.update(2, 10);
    EXPECT_EQ(st.query(2, 3), 10);
}

TEST_F(SegmentTreeTest, Query_InvalidRange_Throws)
{
    const SegmentTree<int> st(5);
    EXPECT_THROW(st.query(3, 2), std::out_of_range);
    EXPECT_THROW(st.query(3, 3), std::out_of_range);
}

TEST_F(SegmentTreeTest, Query_OutOfBounds_Throws)
{
    const SegmentTree<int> st(3);
    EXPECT_THROW(st.query(0, 4), std::out_of_range);
}

TEST_F(SegmentTreeTest, Query_EmptyTree_Throws)
{
    const SegmentTree<int> st;
    EXPECT_THROW(st.query(0, 1), std::out_of_range);
}

// ==================== At Tests ====================

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

TEST_F(SegmentTreeTest, At_OutOfBounds_Throws)
{
    const SegmentTree<int> st(3);
    EXPECT_THROW(st.at(3), std::out_of_range);
    EXPECT_THROW(st.at(100), std::out_of_range);
}

TEST_F(SegmentTreeTest, At_EmptyTree_Throws)
{
    const SegmentTree<int> st;
    EXPECT_THROW(st.at(0), std::out_of_range);
}

// ==================== Size & Empty Tests ====================

TEST_F(SegmentTreeTest, Empty_InitiallyEmpty)
{
    const SegmentTree<int> st;
    EXPECT_TRUE(st.empty());
}

TEST_F(SegmentTreeTest, Empty_NonEmpty)
{
    const SegmentTree<int> st(5);
    EXPECT_FALSE(st.empty());
}

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

TEST_F(SegmentTreeTest, CustomMerge_Multiplies)
{
    SegmentTree<int, std::multiplies<>> st(3);
    st.update(0, 2);
    st.update(1, 3);
    st.update(2, 4);
    EXPECT_EQ(st.query(0, 3), 24);
    EXPECT_EQ(st.query(0, 2), 6);
}

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

TEST_F(SegmentTreeTest, SingleElement_IteratorConstruct)
{
    const std::vector<int> data = {7};
    const SegmentTree<int> st(data.begin(), data.end());
    EXPECT_EQ(st.size(), 1);
    EXPECT_EQ(st.at(0), 7);
    EXPECT_EQ(st.query(0, 1), 7);
}

TEST_F(SegmentTreeTest, SingleElement_InitListConstruct)
{
    const SegmentTree<int> st = {7};
    EXPECT_EQ(st.size(), 1);
    EXPECT_EQ(st.at(0), 7);
    EXPECT_EQ(st.query(0, 1), 7);
}

// ==================== Negative Values ====================

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

TEST_F(SegmentTreeTest, DoubleType_Correctness)
{
    SegmentTree<double> st(3);
    st.update(0, 1.5);
    st.update(1, 2.5);
    st.update(2, 3.0);
    EXPECT_DOUBLE_EQ(st.query(0, 3), 7.0);
    EXPECT_DOUBLE_EQ(st.at(1), 2.5);
}

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
