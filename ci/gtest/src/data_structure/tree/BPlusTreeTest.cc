/**
 * @file BPlusTreeTest.cc
 * @brief Unit tests for the BPlusTree class
 * @details Tests cover insertion, find, range scan, leaf list traversal,
 *          removal, structural invariants, and edge cases.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/BPlusTree.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for BPlusTree tests
 */
class BPlusTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Insertion & Find Tests ====================

/**
 * @brief Test inserting a single key-value pair
 * @details Verifies that after inserting one pair, find returns the expected value
 *          and size reflects the insertion
 */
TEST_F(BPlusTreeTest, Insert_SinglePair_FindReturnsValue)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "one");
    auto result = tree.find(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("one", *result);
    EXPECT_EQ(1, tree.size());
    EXPECT_FALSE(tree.empty());
}

/**
 * @brief Test inserting multiple key-value pairs
 * @details Verifies that all inserted pairs can be found and size is correct
 */
TEST_F(BPlusTreeTest, Insert_MultiplePairs_AllFound)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "one");
    tree.insert(2, "two");
    tree.insert(3, "three");
    EXPECT_EQ("one", *tree.find(1));
    EXPECT_EQ("two", *tree.find(2));
    EXPECT_EQ("three", *tree.find(3));
    EXPECT_EQ(3, tree.size());
}

/**
 * @brief Test inserting keys in sequential ascending order
 * @details Verifies that 50 sequentially ascending insertions all remain findable
 */
TEST_F(BPlusTreeTest, Insert_SequentialAscending_AllFound)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 1; i <= 50; ++i)
        tree.insert(i, std::to_string(i));
    EXPECT_EQ(50, tree.size());
    for (int32_t i = 1; i <= 50; ++i)
    {
        auto result = tree.find(i);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(std::to_string(i), *result);
    }
}

/**
 * @brief Test inserting a duplicate key updates the value
 * @details Verifies that inserting an existing key overwrites the old value
 *          without changing the size
 */
TEST_F(BPlusTreeTest, Insert_DuplicateKey_UpdatesValue)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "old");
    tree.insert(1, "new");
    EXPECT_EQ("new", *tree.find(1));
    EXPECT_EQ(1, tree.size());
}

/**
 * @brief Test insertion triggers leaf node split
 * @details Verifies that when a leaf exceeds its capacity, a split occurs
 *          and the tree's height increases
 */
TEST_F(BPlusTreeTest, Insert_TriggersLeafSplit)
{
    BPlusTree<int32_t, std::string, std::less<int32_t>, 2> tree; // t=2 => max 3 keys per leaf
    for (int32_t i = 1; i <= 20; ++i)
        tree.insert(i, std::to_string(i));
    EXPECT_EQ(20, tree.size());
    EXPECT_GT(tree.height(), 1);
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test inserting many elements and verifying all
 * @details Verifies that all 200 inserted elements can be found and
 *          structural invariants hold via verify()
 */
TEST_F(BPlusTreeTest, Insert_ManyElements_AllFound)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 0; i < 200; ++i)
        tree.insert(i, "val" + std::to_string(i));
    EXPECT_EQ(200, tree.size());
    EXPECT_TRUE(tree.verify());
    for (int32_t i = 0; i < 200; ++i)
    {
        auto result = tree.find(i);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("val" + std::to_string(i), *result);
    }
}

// ==================== Contains Tests ====================

/**
 * @brief Test contains with an existing key
 * @details Verifies that contains returns true for a key that was inserted
 */
TEST_F(BPlusTreeTest, Contains_ExistingKey_ReturnsTrue)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(42, "answer");
    EXPECT_TRUE(tree.contains(42));
}

/**
 * @brief Test contains with a non-existing key
 * @details Verifies that contains returns false for a key not in the tree
 */
TEST_F(BPlusTreeTest, Contains_NonExisting_ReturnsFalse)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(10, "ten");
    EXPECT_FALSE(tree.contains(20));
}

/**
 * @brief Test contains on an empty tree
 * @details Verifies that contains returns false when the tree is empty
 */
TEST_F(BPlusTreeTest, Contains_EmptyTree_ReturnsFalse)
{
    const BPlusTree<int32_t, std::string> tree;
    EXPECT_FALSE(tree.contains(1));
}

// ==================== Range Scan Tests ====================

/**
 * @brief Test range scan on an empty tree
 * @details Verifies that range scanning an empty tree returns an empty vector
 */
TEST_F(BPlusTreeTest, RangeScan_EmptyTree_ReturnsEmpty)
{
    const BPlusTree<int32_t, std::string> tree;
    auto result = tree.rangeScan(0, 10);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test range scan for a single point
 * @details Verifies that scanning a narrow range returns only the matching element
 */
TEST_F(BPlusTreeTest, RangeScan_SinglePoint_ReturnsThatElement)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(5, "five");
    tree.insert(10, "ten");
    auto result = tree.rangeScan(5, 6);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0].first);
    EXPECT_EQ("five", result[0].second);
}

/**
 * @brief Test range scan with a partial range
 * @details Verifies that scanning a sub-range returns only the elements within that range
 */
TEST_F(BPlusTreeTest, RangeScan_PartialRange_ReturnsCorrectElements)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 1; i <= 20; ++i)
        tree.insert(i, std::to_string(i));
    auto result = tree.rangeScan(5, 10);
    ASSERT_EQ(5, result.size());
    for (int32_t i = 0; i < 5; ++i)
        EXPECT_EQ(5 + i, result[i].first);
}

/**
 * @brief Test full range scan returns all elements sorted
 * @details Verifies that scanning the full key range returns all elements in sorted order
 */
TEST_F(BPlusTreeTest, RangeScan_FullRange_ReturnsAllSorted)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 0; i < 50; ++i)
        tree.insert(i, std::to_string(i));
    auto result = tree.rangeScan(0, 50);
    ASSERT_EQ(50, result.size());
    for (int32_t i = 0; i < 50; ++i)
        EXPECT_EQ(i, result[i].first);
}

/**
 * @brief Test range scan with no matching keys
 * @details Verifies that scanning a range with no elements returns an empty result
 */
TEST_F(BPlusTreeTest, RangeScan_NoMatch_ReturnsEmpty)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(20, "twenty");
    auto result = tree.rangeScan(30, 40);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test range scan end-exclusive behavior
 * @details Verifies that the end key is excluded from the range scan result
 */
TEST_F(BPlusTreeTest, RangeScan_EndExclusive_ExcludesEnd)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "a");
    tree.insert(2, "b");
    tree.insert(3, "c");
    auto result = tree.rangeScan(2, 3);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(2, result[0].first);
}

// ==================== Leaf List Tests ====================

/**
 * @brief Test inorder traversal from a single leaf
 * @details Verifies that inorder returns all elements when they fit in one leaf
 */
TEST_F(BPlusTreeTest, LeafList_SingleLeaf_AllElements)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "a");
    tree.insert(2, "b");
    auto inorder = tree.inorder();
    ASSERT_EQ(2, inorder.size());
    EXPECT_EQ(1, inorder[0].first);
    EXPECT_EQ(2, inorder[1].first);
}

/**
 * @brief Test inorder traversal after leaf splits
 * @details Verifies that the leaf list remains correctly linked after multiple splits
 */
TEST_F(BPlusTreeTest, LeafList_AfterSplits_LinkedCorrectly)
{
    BPlusTree<int32_t, std::string, std::less<int32_t>, 2> tree; // small degree => many splits
    for (int32_t i = 0; i < 30; ++i)
        tree.insert(i, std::to_string(i));
    auto inorder = tree.inorder();
    ASSERT_EQ(30, inorder.size());
    for (int32_t i = 0; i < 30; ++i)
        EXPECT_EQ(i, inorder[i].first);
}

// ==================== Removal Tests ====================

/**
 * @brief Test removing a non-existing key has no effect
 * @details Verifies that attempting to remove a key not in the tree
 *          returns false and leaves the tree unchanged
 */
TEST_F(BPlusTreeTest, Remove_NonExisting_NoEffect)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "one");
    EXPECT_FALSE(tree.remove(2));
    EXPECT_EQ(1, tree.size());
}

/**
 * @brief Test removing the single element empties the tree
 * @details Verifies that after removing the only element, the tree becomes empty
 */
TEST_F(BPlusTreeTest, Remove_SingleElement_EmptiesTree)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(42, "answer");
    EXPECT_TRUE(tree.remove(42));
    EXPECT_FALSE(tree.contains(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

/**
 * @brief Test removing multiple elements
 * @details Verifies that all 20 inserted elements can be removed sequentially
 */
TEST_F(BPlusTreeTest, Remove_MultipleElements_AllRemoved)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 0; i < 20; ++i)
        tree.insert(i, std::to_string(i));
    for (int32_t i = 0; i < 20; ++i)
        EXPECT_TRUE(tree.remove(i));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

/**
 * @brief Test inorder traversal correctness after removal
 * @details Verifies that after removing specific keys, the inorder traversal
 *          reflects the correct remaining elements
 */
TEST_F(BPlusTreeTest, Remove_CheckInorderAfterRemoval)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 1; i <= 10; ++i)
        tree.insert(i, std::to_string(i));
    tree.remove(5);
    tree.remove(7);
    auto inorder = tree.inorder();
    ASSERT_EQ(8, inorder.size());
    EXPECT_EQ(1, inorder[0].first);
    EXPECT_EQ(4, inorder[3].first);
    EXPECT_EQ(6, inorder[4].first);
    EXPECT_EQ(8, inorder[5].first);
}

/**
 * @brief Test remove then re-insert the same key
 * @details Verifies that after removing a key, re-inserting it works correctly
 *          with a new value
 */
TEST_F(BPlusTreeTest, Remove_ThenInsert_Works)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(20, "twenty");
    tree.remove(10);
    EXPECT_FALSE(tree.contains(10));
    EXPECT_TRUE(tree.contains(20));
    tree.insert(10, "new_ten");
    EXPECT_TRUE(tree.contains(10));
    EXPECT_EQ("new_ten", *tree.find(10));
    EXPECT_EQ(2, tree.size());
}

// ==================== Verification Tests ====================

/**
 * @brief Test verify on an empty tree
 * @details Verifies that an empty tree passes structural validation
 */
TEST_F(BPlusTreeTest, Verify_EmptyTree_ReturnsTrue)
{
    BPlusTree<int32_t, std::string> tree;
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test verify after multiple inserts
 * @details Verifies that the tree passes structural validation after insertions
 */
TEST_F(BPlusTreeTest, Verify_AfterInserts_ReturnsTrue)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 0; i < 50; ++i)
        tree.insert(i, std::to_string(i));
    EXPECT_TRUE(tree.verify());
}

/**
 * @brief Test verify after insert and delete operations
 * @details Verifies that the tree passes structural validation after mixed operations
 */
TEST_F(BPlusTreeTest, Verify_AfterInsertDelete_ReturnsTrue)
{
    BPlusTree<int32_t, std::string> tree;
    for (int32_t i = 0; i < 30; ++i)
        tree.insert(i, std::to_string(i));
    for (int32_t i = 5; i < 15; ++i)
        tree.remove(i);
    EXPECT_TRUE(tree.verify());
}

// ==================== Height Tests ====================

/**
 * @brief Test height of an empty tree
 * @details Verifies that an empty tree has height zero
 */
TEST_F(BPlusTreeTest, Height_EmptyTree_Zero)
{
    BPlusTree<int32_t, std::string> tree;
    EXPECT_EQ(0, tree.height());
}

/**
 * @brief Test height with a single node
 * @details Verifies that a tree with one element has height one
 */
TEST_F(BPlusTreeTest, Height_SingleNode_One)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(42, "x");
    EXPECT_EQ(1, tree.height());
}

/**
 * @brief Test height after multiple inserts
 * @details Verifies that the tree height stays within expected bounds
 *          after inserting 100 elements with a small degree
 */
TEST_F(BPlusTreeTest, Height_AfterInserts_Reasonable)
{
    BPlusTree<int32_t, std::string, std::less<int32_t>, 2> tree;
    for (int32_t i = 0; i < 100; ++i)
        tree.insert(i, std::to_string(i));
    int32_t h = tree.height();
    // t=2 => max fanout = 4 => min height ≈ ceil(log4(100)) ≈ 4
    EXPECT_GE(h, 1);
    EXPECT_LE(h, 6);
}

// ==================== Edge Cases ====================

/**
 * @brief Test clear empties the tree
 * @details Verifies that clear removes all elements and the tree passes validation
 */
TEST_F(BPlusTreeTest, Clear_EmptiesTree)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "a");
    tree.insert(2, "b");
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
TEST_F(BPlusTreeTest, MoveConstructor_TransfersOwnership)
{
    BPlusTree<int32_t, std::string> tree;
    tree.insert(1, "a");
    tree.insert(2, "b");
    auto moved = std::move(tree);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_EQ(2, moved.size());
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
}

/**
 * @brief Test move assignment transfers ownership
 * @details Verifies that move assignment transfers all elements
 *          and the source tree becomes empty
 */
TEST_F(BPlusTreeTest, MoveAssignment_TransfersOwnership)
{
    BPlusTree<int32_t, std::string> a;
    a.insert(1, "a");
    BPlusTree<int32_t, std::string> b;
    b.insert(2, "b");
    b = std::move(a);
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(1, b.size());
    EXPECT_TRUE(b.contains(1));
}

/**
 * @brief Test large dataset with minimum order
 * @details Verifies that the B+ tree handles 500 elements with minimum order (t=2)
 *          and all elements remain findable
 */
TEST_F(BPlusTreeTest, LargeDataset_MinimumOrder)
{
    BPlusTree<int32_t, std::string, std::less<int32_t>, 2> tree; // t=2
    for (int32_t i = 0; i < 500; ++i)
        tree.insert(i, std::to_string(i));
    EXPECT_TRUE(tree.verify());
    EXPECT_EQ(500, tree.size());
    for (int32_t i = 0; i < 500; ++i)
        EXPECT_TRUE(tree.contains(i));
}

/**
 * @brief Test range scan spanning multiple leaf nodes
 * @details Verifies that range scan correctly retrieves elements that span
 *          across multiple leaf nodes in the B+ tree
 */
TEST_F(BPlusTreeTest, RangeScan_SpansMultipleLeafNodes)
{
    BPlusTree<int32_t, std::string, std::less<int32_t>, 2> tree;
    for (int32_t i = 1; i <= 100; ++i)
        tree.insert(i, std::to_string(i));
    auto result = tree.rangeScan(40, 60);
    ASSERT_EQ(20, result.size());
    EXPECT_EQ(40, result.front().first);
    EXPECT_EQ(59, result.back().first);
}
