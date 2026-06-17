/**
 * @file BTreeTest.cc
 * @brief Unit tests for the BTree class
 * @details Tests cover insertion, search, removal, structural invariants,
 *          and edge cases for the B-tree data structure.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/multiway/BTree.hpp"

using namespace common::data_structure::tree::multiway;

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

/**
 * @brief Test inserting a single node into B-tree
 * @details Verifies that after inserting a single element, contains returns true
 *          and size reflects the insertion
 */
TEST_F(BTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    BTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.contains(10));
    EXPECT_EQ(1, tree.size());
    EXPECT_FALSE(tree.empty());
}

/**
 * @brief Test inserting keys in sequential ascending order
 * @details Verifies that all 50 sequentially ascending keys remain findable
 */
TEST_F(BTreeTest, Insert_SequentialAscending_AllFound)
{
    BTree<int> tree;
    for (int32_t i = 1; i <= 50; ++i)
        tree.insert(i);
    EXPECT_EQ(50, tree.size());
    for (int32_t i = 1; i <= 50; ++i)
        EXPECT_TRUE(tree.contains(i));
}

/**
 * @brief Test inserting keys in sequential descending order
 * @details Verifies that all 50 sequentially descending keys remain findable
 */
TEST_F(BTreeTest, Insert_SequentialDescending_AllFound)
{
    BTree<int> tree;
    for (int32_t i = 50; i >= 1; --i)
        tree.insert(i);
    EXPECT_EQ(50, tree.size());
    for (int32_t i = 1; i <= 50; ++i)
        EXPECT_TRUE(tree.contains(i));
}

/**
 * @brief Test insertion triggers node split
 * @details Verifies that inserting enough elements triggers node splits
 *          and the tree maintains structural integrity
 */
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

/**
 * @brief Test inserting duplicate keys has no effect
 * @details Verifies that inserting a duplicate key does not increase the tree size
 */
TEST_F(BTreeTest, Insert_Duplicate_NoEffect)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(10);
    EXPECT_EQ(1, tree.size());
}

/**
 * @brief Test random order insertion produces sorted inorder
 * @details Verifies that regardless of insertion order, inorder traversal
 *          returns elements in sorted order
 */
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

/**
 * @brief Test structural verification after many insertions
 * @details Verifies that inserting 200 elements passes structural validation
 */
TEST_F(BTreeTest, Insert_ManyElements_VerifyStruct)
{
    BTree<int32_t, std::less<int32_t>, 3> tree;
    for (int32_t i = 0; i < 200; ++i)
        tree.insert(i);
    EXPECT_TRUE(tree.verify());
}

// ==================== Contains/Search Tests ====================

/**
 * @brief Test contains with an existing key
 * @details Verifies that contains returns true for keys that were inserted
 */
TEST_F(BTreeTest, Contains_ExistingKey_ReturnsTrue)
{
    BTree<int> tree;
    tree.insert(42);
    tree.insert(17);
    EXPECT_TRUE(tree.contains(42));
    EXPECT_TRUE(tree.contains(17));
}

/**
 * @brief Test contains with a non-existing key
 * @details Verifies that contains returns false for keys not in the tree
 */
TEST_F(BTreeTest, Contains_NonExistingKey_ReturnsFalse)
{
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    EXPECT_FALSE(tree.contains(15));
    EXPECT_FALSE(tree.contains(0));
}

/**
 * @brief Test contains works correctly after node splits
 * @details Verifies that contains still works for all keys after
 *          the tree has undergone multiple node splits
 */
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

/**
 * @brief Test contains on an empty tree
 * @details Verifies that contains returns false when the tree is empty
 */
TEST_F(BTreeTest, Contains_EmptyTree_ReturnsFalse)
{
    const BTree<int> tree;
    EXPECT_FALSE(tree.contains(1));
}

// ==================== Removal Tests ====================

/**
 * @brief Test removing a non-existing key has no effect
 * @details Verifies that attempting to remove a key not in the tree
 *          returns false and leaves the tree unchanged
 */
TEST_F(BTreeTest, Remove_NonExisting_NoEffect)
{
    BTree<int> tree;
    tree.insert(10);
    EXPECT_FALSE(tree.remove(20));
    EXPECT_EQ(1, tree.size());
    EXPECT_TRUE(tree.contains(10));
}

/**
 * @brief Test removing the single element empties the tree
 * @details Verifies that after removing the only element, the tree becomes empty
 */
TEST_F(BTreeTest, Remove_SingleElement_EmptiesTree)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.remove(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_FALSE(tree.contains(42));
}

/**
 * @brief Test removing multiple elements
 * @details Verifies that all 20 inserted elements can be removed sequentially
 */
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

/**
 * @brief Test removal triggers node merge
 * @details Verifies that removing enough elements triggers node merges
 *          and structural invariants are maintained
 */
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

/**
 * @brief Test inorder correctness after removal
 * @details Verifies that after removing specific keys, the inorder traversal
 *          reflects the correct remaining elements
 */
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

/**
 * @brief Test remove then re-insert the same key
 * @details Verifies that after removing a key, re-inserting it works correctly
 */
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

/**
 * @brief Test structural invariants after insert and delete
 * @details Verifies that mixed insert and delete operations maintain invariants
 */
TEST_F(BTreeTest, Remove_AndVerifyInvariants)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 30; ++i)
        tree.insert(i);
    for (int32_t i = 5; i < 15; ++i)
        tree.remove(i);
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test removing all leaves then internal nodes
 * @details Verifies that all 100 elements can be removed one by one
 *          and the tree ends up empty
 */
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

/**
 * @brief Test inorder on an empty tree
 * @details Verifies that inorder traversal of an empty tree returns an empty list
 */
TEST_F(BTreeTest, Inorder_EmptyTree_ReturnsEmpty)
{
    BTree<int> tree;
    EXPECT_TRUE(tree.inorder().empty());
}

/**
 * @brief Test inorder with a single element
 * @details Verifies that inorder traversal of a single-element tree returns that element
 */
TEST_F(BTreeTest, Inorder_SingleElement)
{
    BTree<int> tree;
    tree.insert(42);
    std::vector<int32_t> expected = {42};
    EXPECT_EQ(expected, tree.inorder());
}

/**
 * @brief Test inorder sorted after insert
 * @details Verifies that inserting in reverse order still produces sorted inorder output
 */
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

/**
 * @brief Test verify on an empty tree
 * @details Verifies that an empty tree passes structural validation
 */
TEST_F(BTreeTest, Verify_EmptyTree_ReturnsTrue)
{
    BTree<int> tree;
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test verify on a single node
 * @details Verifies that a single-node tree passes structural validation
 */
TEST_F(BTreeTest, Verify_SingleNode_ReturnsTrue)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test verify after multiple inserts
 * @details Verifies that the tree passes validation after many insertions
 */
TEST_F(BTreeTest, Verify_AfterMultipleInserts_ReturnsTrue)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test verify after insert and delete operations
 * @details Verifies that mixed operations maintain B-tree structural invariants
 */
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

/**
 * @brief Test height of an empty tree
 * @details Verifies that an empty tree has height zero
 */
TEST_F(BTreeTest, Height_EmptyTree_Zero)
{
    BTree<int> tree;
    EXPECT_EQ(0, tree.height());
}

/**
 * @brief Test height with a single node
 * @details Verifies that a tree with one element has height one
 */
TEST_F(BTreeTest, Height_SingleNode_One)
{
    BTree<int> tree;
    tree.insert(42);
    EXPECT_EQ(1, tree.height());
}

/**
 * @brief Test height after multiple inserts
 * @details Verifies that the tree height stays within expected bounds
 *          after inserting 100 elements
 */
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

/**
 * @brief Test large dataset with minimum order
 * @details Verifies that the B-tree handles 500 elements with minimum order t=2
 *          and all elements remain findable
 */
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

/**
 * @brief Test clear empties the tree
 * @details Verifies that clear removes all elements and the tree passes validation
 */
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

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that the move constructor transfers all elements
 *          and the source tree becomes empty
 */
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

/**
 * @brief Test move assignment transfers ownership
 * @details Verifies that move assignment transfers all elements
 *          and the source tree becomes empty
 */
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

/**
 * @brief Test sorted order after sequential insert
 * @details Verifies that inserting 0..99 produces an inorder list matching the sequence
 */
TEST_F(BTreeTest, Insert_RangeScanLike_VerifySorted)
{
    BTree<int> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i);
    auto vec = tree.inorder();
    for (int32_t i = 0; i < 100; ++i)
        EXPECT_EQ(i, vec[i]);
}
