/**
 * @file BTreeTest.cc
 * @brief Unit tests for the BTree class
 * @details Tests cover insertion, search, removal, structural invariants,
 *          and edge cases for the B-tree data structure.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/BTree.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for BTree tests
 */
class BTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Insertion Tests ====================

TEST_F(BTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    BTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.contains(10));
    EXPECT_EQ(1, tree.size());
    EXPECT_FALSE(tree.empty());
}

TEST_F(BTreeTest, Insert_SequentialAscending_AllFound)
{
    BTree<int> tree;
    for (int32_t i = 1; i <= 50; ++i)
        tree.insert(i);
    EXPECT_EQ(50, tree.size());
    for (int32_t i = 1; i <= 50; ++i)
        EXPECT_TRUE(tree.contains(i));
}

TEST_F(BTreeTest, Insert_SequentialDescending_AllFound)
{
    BTree<int> tree;
    for (int32_t i = 50; i >= 1; --i)
        tree.insert(i);
    EXPECT_EQ(50, tree.size());
    for (int32_t i = 1; i <= 50; ++i)
        EXPECT_TRUE(tree.contains(i));
}

TEST_F(BTreeTest, Insert_TriggersSplit)
{
    BTree<int> tree;
    // Order=3 => max 5 keys per node, multiple splits expected
    for (int32_t i = 1; i <= 100; ++i)
        tree.insert(i);
    EXPECT_EQ(100, tree.size());
    EXPECT_GT(tree.height(), 1);
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, Insert_Duplicate_NoEffect)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(10);
    EXPECT_EQ(1, tree.size());
}

TEST_F(BTreeTest, Insert_RandomOrder_ProducesSortedInorder)
{
    BTree<int> tree;
    std::vector<int32_t> input = {7, 3, 9, 1, 5, 8, 10, 2, 4, 6};
    for (int32_t v : input)
        tree.insert(v);
    auto result = tree.inorder();
    ASSERT_EQ(input.size(), result.size());
    for (size_t i = 1; i < result.size(); ++i)
        EXPECT_LE(result[i - 1], result[i]);
}

TEST_F(BTreeTest, Insert_ManyElements_VerifyStruct)
{
    BTree<int32_t, std::less<int32_t>, 3> tree;
    for (int32_t i = 0; i < 200; ++i)
        tree.insert(i);
    EXPECT_TRUE(tree.verify());
}

// ==================== Contains/Search Tests ====================

TEST_F(BTreeTest, Contains_ExistingKey_ReturnsTrue)
{
    BTree<int> tree;
    tree.insert(42);
    tree.insert(17);
    EXPECT_TRUE(tree.contains(42));
    EXPECT_TRUE(tree.contains(17));
}

TEST_F(BTreeTest, Contains_NonExistingKey_ReturnsFalse)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    EXPECT_FALSE(tree.contains(15));
    EXPECT_FALSE(tree.contains(0));
}

TEST_F(BTreeTest, Contains_AfterSplit_Works)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 50; ++i)
        tree.insert(i);
    for (int32_t i = 0; i < 50; ++i)
        EXPECT_TRUE(tree.contains(i));
    EXPECT_FALSE(tree.contains(-1));
    EXPECT_FALSE(tree.contains(100));
}

TEST_F(BTreeTest, Contains_EmptyTree_ReturnsFalse)
{
    const BTree<int> tree;
    EXPECT_FALSE(tree.contains(1));
}

// ==================== Removal Tests ====================

TEST_F(BTreeTest, Remove_NonExisting_NoEffect)
{
    BTree<int> tree;
    tree.insert(10);
    EXPECT_FALSE(tree.remove(20));
    EXPECT_EQ(1, tree.size());
    EXPECT_TRUE(tree.contains(10));
}

TEST_F(BTreeTest, Remove_SingleElement_EmptiesTree)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.remove(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_FALSE(tree.contains(42));
}

TEST_F(BTreeTest, Remove_MultipleElements_AllRemoved)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 20; ++i)
        tree.insert(i);
    for (int32_t i = 0; i < 20; ++i)
        EXPECT_TRUE(tree.remove(i));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(BTreeTest, Remove_TriggersMerge)
{
    BTree<int> tree;
    for (int32_t i = 1; i <= 10; ++i)
        tree.insert(i);
    // Remove enough elements to trigger merges
    for (int32_t i = 1; i <= 8; ++i)
        tree.remove(i);
    EXPECT_TRUE(tree.verify());
    EXPECT_EQ(2, tree.size());
    EXPECT_TRUE(tree.contains(9));
    EXPECT_TRUE(tree.contains(10));
}

TEST_F(BTreeTest, Remove_Reorder_CheckInorder)
{
    BTree<int> tree;
    for (int32_t i = 1; i <= 10; ++i)
        tree.insert(i);
    tree.remove(5);
    tree.remove(7);
    std::vector<int32_t> expected = {1, 2, 3, 4, 6, 8, 9, 10};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(BTreeTest, Remove_ThenInsert_Works)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(10);
    EXPECT_FALSE(tree.contains(10));
    EXPECT_TRUE(tree.contains(20));
    tree.insert(10);
    EXPECT_TRUE(tree.contains(10));
    EXPECT_EQ(2, tree.size());
}

TEST_F(BTreeTest, Remove_AndVerifyInvariants)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 30; ++i)
        tree.insert(i);
    for (int32_t i = 5; i < 15; ++i)
        tree.remove(i);
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, Remove_AllLeavesThenInternal)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    // Remove all elements one by one
    for (int32_t i = 0; i < 100; ++i)
        EXPECT_TRUE(tree.remove(i));
    EXPECT_TRUE(tree.empty());
}

// ==================== Inorder Traversal Tests ====================

TEST_F(BTreeTest, Inorder_EmptyTree_ReturnsEmpty)
{
    BTree<int> tree;
    EXPECT_TRUE(tree.inorder().empty());
}

TEST_F(BTreeTest, Inorder_SingleElement)
{
    BTree<int> tree;
    tree.insert(42);
    std::vector<int32_t> expected = {42};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(BTreeTest, Inorder_SortedAfterInsert)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 20; ++i)
        tree.insert(19 - i); // reverse order
    auto result = tree.inorder();
    ASSERT_EQ(20, result.size());
    for (int32_t i = 0; i < 20; ++i)
        EXPECT_EQ(i, result[i]);
}

// ==================== Verification Tests ====================

TEST_F(BTreeTest, Verify_EmptyTree_ReturnsTrue)
{
    BTree<int> tree;
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, Verify_SingleNode_ReturnsTrue)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, Verify_AfterMultipleInserts_ReturnsTrue)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, Verify_AfterInsertDelete_ReturnsTrue)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 50; ++i)
        tree.insert(i);
    for (int32_t i = 10; i < 20; ++i)
        tree.remove(i);
    EXPECT_TRUE(tree.verify());
}

// ==================== Height Tests ====================

TEST_F(BTreeTest, Height_EmptyTree_Zero)
{
    BTree<int> tree;
    EXPECT_EQ(0, tree.height());
}

TEST_F(BTreeTest, Height_SingleNode_One)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_EQ(1, tree.height());
}

TEST_F(BTreeTest, Height_AfterInserts_Reasonable)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    int32_t h = tree.height();
    // Order=3 => max fanout = 2*Order = 6 => min height = ceil(log6(100)) ≈ 3
    EXPECT_GE(h, 1);
    EXPECT_LE(h, 6);
}

// ==================== Edge Cases ====================

TEST_F(BTreeTest, LargeDataset_MinimumOrder)
{
    BTree<int32_t, std::less<int32_t>, 2> tree; // minimum order t=2 (2-3-4 tree)
    for (int32_t i = 0; i < 500; ++i)
        tree.insert(i);
    EXPECT_EQ(500, tree.size());
    EXPECT_TRUE(tree.verify());
    for (int32_t i = 0; i < 500; ++i)
        EXPECT_TRUE(tree.contains(i));
}

TEST_F(BTreeTest, Clear_EmptiesTree)
{
    BTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_TRUE(tree.verify());
}

TEST_F(BTreeTest, MoveConstructor_TransfersOwnership)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    EXPECT_EQ(2, tree.size());

    BTree<int> moved = std::move(tree);
    EXPECT_EQ(0, tree.size());
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(2, moved.size());
    EXPECT_TRUE(moved.contains(10));
    EXPECT_TRUE(moved.contains(20));
}

TEST_F(BTreeTest, MoveAssignment_TransfersOwnership)
{
    BTree<int> tree;
    tree.insert(1);
    BTree<int> other;
    other.insert(2);
    other = std::move(tree);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(1, other.size());
    EXPECT_TRUE(other.contains(1));
}

TEST_F(BTreeTest, Insert_RangeScanLike_VerifySorted)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    auto vec = tree.inorder();
    for (int32_t i = 0; i < 100; ++i)
        EXPECT_EQ(i, vec[i]);
}
