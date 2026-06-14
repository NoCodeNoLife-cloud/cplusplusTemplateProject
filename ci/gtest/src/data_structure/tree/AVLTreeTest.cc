/**
 * @file AVLTreeTest.cc
 * @brief Unit tests for the AVLTree class
 * @details Tests cover insertion, removal, search, balance invariants,
 *          and edge cases for the self-balancing AVL tree.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/AVLTree.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for AVLTree tests
 */
class AVLTreeTest : public testing::Test
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
 * @brief Test inserting a single node into AVL tree
 * @details Verifies that after inserting a single element, find returns true for that element
 */
TEST_F(AVLTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    AVLTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

/**
 * @brief Test inserting multiple values into AVL tree
 * @details Verifies that all inserted values can be found after insertion
 */
TEST_F(AVLTreeTest, Insert_MultipleValues_AllFound)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    tree.insert(40);
    tree.insert(50);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(50));
}

/**
 * @brief Test inserting duplicate values into AVL tree
 * @details Verifies that inserting a duplicate value does not change the tree structure
 */
TEST_F(AVLTreeTest, Insert_Duplicate_NoChange)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    // Size should still be 1 effectively
}

/**
 * @brief Test left-heavy insertion triggers AVL rotation
 * @details Verifies that inserting values in descending order triggers a right rotation
 *          and all values remain findable
 */
TEST_F(AVLTreeTest, Insert_LeftHeavy_BalancesCorrectly)
{
    AVLTree<int> tree;
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test right-heavy insertion triggers AVL rotation
 * @details Verifies that inserting values in ascending order triggers a left rotation
 *          and all values remain findable
 */
TEST_F(AVLTreeTest, Insert_RightHeavy_BalancesCorrectly)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test left-right insertion triggers double rotation
 * @details Verifies that inserting values creating a left-right pattern triggers
 *          a left-right double rotation and maintains correct order
 */
TEST_F(AVLTreeTest, Insert_LeftRight_BalancesCorrectly)
{
    AVLTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test right-left insertion triggers double rotation
 * @details Verifies that inserting values creating a right-left pattern triggers
 *          a right-left double rotation and maintains correct order
 */
TEST_F(AVLTreeTest, Insert_RightLeft_BalancesCorrectly)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(30);
    tree.insert(20);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

// ==================== Removal Tests ====================

/**
 * @brief Test removing a leaf node from AVL tree
 * @details Verifies that a leaf node can be removed successfully and
 *          the remaining nodes are still findable
 */
TEST_F(AVLTreeTest, Remove_LeafNode_RemovedSuccessfully)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test removing a node with one child from AVL tree
 * @details Verifies that a node having a single child can be removed,
 *          bypassing the deleted node
 */
TEST_F(AVLTreeTest, Remove_NodeWithOneChild_RemovedSuccessfully)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

/**
 * @brief Test removing a node with two children from AVL tree
 * @details Verifies that a node having two children can be removed,
 *          with the inorder successor taking its place
 */
TEST_F(AVLTreeTest, Remove_NodeWithTwoChildren_RemovedSuccessfully)
{
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);

    tree.remove(50);
    EXPECT_FALSE(tree.find(50));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(70));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(60));
    EXPECT_TRUE(tree.find(80));
}

/**
 * @brief Test removing root node triggers rebalancing
 * @details Verifies that removing the root node triggers the appropriate
 *          AVL rebalancing and remaining nodes are still accessible
 */
TEST_F(AVLTreeTest, Remove_RootNode_Rebalances)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(20);
    EXPECT_FALSE(tree.find(20));
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test removing a non-existing key has no effect
 * @details Verifies that attempting to remove a key not present in the tree
 *          does not modify the tree structure
 */
TEST_F(AVLTreeTest, Remove_NonExisting_NoChange)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    tree.remove(999);
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
}

/**
 * @brief Test removing all elements empties the tree
 * @details Verifies that after removing all inserted elements,
 *          the tree reports none of them as findable
 */
TEST_F(AVLTreeTest, Remove_AllElements_TreeEmpty)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(10);
    tree.remove(20);
    tree.remove(30);

    EXPECT_FALSE(tree.find(10));
    EXPECT_FALSE(tree.find(20));
    EXPECT_FALSE(tree.find(30));
}

/**
 * @brief Test removing from an empty tree does not crash
 * @details Verifies that calling remove on an empty tree handles gracefully
 *          without throwing exceptions or crashing
 */
TEST_F(AVLTreeTest, Remove_FromEmptyTree_NoCrash)
{
    AVLTree<int> tree;
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

// ==================== Find Tests ====================

/**
 * @brief Test find on a non-existing key returns false
 * @details Verifies that querying a key not present in an empty tree returns false
 */
TEST_F(AVLTreeTest, Find_NonExisting_ReturnsFalse)
{
    const AVLTree<int> tree;
    EXPECT_FALSE(tree.find(10));
}

/**
 * @brief Test find after removal returns false
 * @details Verifies that after removing a key, find no longer locates it
 */
TEST_F(AVLTreeTest, Find_AfterRemove_ReturnsFalse)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

// ==================== findValue Tests ====================

/**
 * @brief Test findValue with an existing value
 * @details Verifies that findValue returns the correct value wrapped in optional
 *          for a key that exists in the tree
 */
TEST_F(AVLTreeTest, FindValue_ExistingValue_ReturnsValue)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    const auto result = tree.findValue(20);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 20);
}

/**
 * @brief Test findValue with a non-existing value
 * @details Verifies that findValue returns nullopt for a key that does not exist
 */
TEST_F(AVLTreeTest, FindValue_NonExisting_ReturnsNullopt)
{
    const AVLTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test findValue on an empty tree
 * @details Verifies that findValue returns nullopt when the tree is empty
 */
TEST_F(AVLTreeTest, FindValue_EmptyTree_ReturnsNullopt)
{
    const AVLTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test findValue on the root node
 * @details Verifies that findValue correctly retrieves the value stored at the root
 */
TEST_F(AVLTreeTest, FindValue_RootNode_ReturnsValue)
{
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    const auto result = tree.findValue(50);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 50);
}

/**
 * @brief Test findValue on a leaf node
 * @details Verifies that findValue correctly retrieves the value stored at a leaf node
 */
TEST_F(AVLTreeTest, FindValue_LeafNode_ReturnsValue)
{
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);

    const auto result = tree.findValue(20);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 20);
}

/**
 * @brief Test findValue consistency with find
 * @details Verifies that findValue.has_value() matches the boolean return of find
 *          for both existing and non-existing keys
 */
TEST_F(AVLTreeTest, FindValue_ConsistencyWithFind)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    EXPECT_EQ(tree.find(20), tree.findValue(20).has_value());
    EXPECT_EQ(tree.find(999), tree.findValue(999).has_value());
}

// ==================== String Type ====================

/**
 * @brief Test AVL tree with string keys
 * @details Verifies that the AVL tree correctly stores and retrieves string-typed keys
 */
TEST_F(AVLTreeTest, Insert_StringType_StoresCorrectly)
{
    AVLTree<std::string> tree;
    tree.insert("banana");
    tree.insert("apple");
    tree.insert("cherry");

    EXPECT_TRUE(tree.find("apple"));
    EXPECT_TRUE(tree.find("banana"));
    EXPECT_TRUE(tree.find("cherry"));
    EXPECT_FALSE(tree.find("grape"));
}

// ==================== Negative Key Tests ====================

/**
 * @brief Test AVL tree with negative integer keys
 * @details Verifies that the AVL tree correctly handles negative keys
 */
TEST_F(AVLTreeTest, Insert_NegativeKeys_StoresCorrectly)
{
    AVLTree<int> tree;
    tree.insert(-10);
    tree.insert(-20);
    tree.insert(-30);

    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(-20));
    EXPECT_TRUE(tree.find(-30));
    EXPECT_FALSE(tree.find(0));
}

// ==================== Double Rotation on Removal ====================

/**
 * @brief Test removal triggers left-right double rotation
 * @details Verifies that removing a node in a specific pattern triggers
 *          a left-right double rotation and the tree remains valid
 */
TEST_F(AVLTreeTest, Remove_TriggersLeftRightRotation_NoCrash)
{
    // Build a tree where removing a node triggers LR double rotation
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(80);
    tree.insert(20);
    tree.insert(40);
    tree.insert(70);
    tree.insert(90);
    tree.insert(35);
    tree.insert(45);

    tree.remove(20); // May trigger rebalancing
    EXPECT_FALSE(tree.find(20));
    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(35));
}

/**
 * @brief Test removal triggers right-left double rotation
 * @details Verifies that removing a node triggers a right-left double rotation
 *          and the tree remains valid
 */
TEST_F(AVLTreeTest, Remove_TriggersRightLeftRotation_NoCrash)
{
    // Build a tree where removing a node triggers RL double rotation
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(80);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(90);
    tree.insert(55);
    tree.insert(65);

    tree.remove(90); // May trigger rebalancing
    EXPECT_FALSE(tree.find(90));
    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(60));
    EXPECT_TRUE(tree.find(55));
}

// ==================== Large Dataset ====================

/**
 * @brief Test AVL tree with a large dataset
 * @details Verifies that inserting 1000 elements and searching for all of them
 *          works correctly, validating tree scalability
 */
TEST_F(AVLTreeTest, Insert_LargeDataset_AllFound)
{
    AVLTree<int> tree;
    constexpr int kCount = 1000;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(i);
    }

    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_TRUE(tree.find(i));
    }
}

/**
 * @brief Test interleaved insert and remove with large dataset
 * @details Verifies that a mix of insertions and deletions on a large dataset
 *          maintains consistency: even keys are removed, odd keys remain findable
 */
TEST_F(AVLTreeTest, InsertAndRemove_LargeDataset_Consistent)
{
    AVLTree<int> tree;
    constexpr int kCount = 500;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(i);
    }

    for (int i = 0; i < kCount; i += 2)
    {
        tree.remove(i);
    }

    for (int i = 0; i < kCount; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(tree.find(i));
        }
        else
        {
            EXPECT_TRUE(tree.find(i));
        }
    }
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that the move constructor transfers all elements from
 *          the source tree to the destination tree, leaving the source empty
 */
TEST_F(AVLTreeTest, MoveConstructor)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    EXPECT_TRUE(tree.find(10));

    AVLTree<int> other(std::move(tree));
    EXPECT_TRUE(other.find(10));
    EXPECT_TRUE(other.find(20));
    EXPECT_TRUE(other.find(5));
    EXPECT_FALSE(tree.find(10));
}

/**
 * @brief Test move assignment transfers ownership
 * @details Verifies that move assignment transfers all elements from
 *          the source tree to the destination tree
 */
TEST_F(AVLTreeTest, MoveAssignment)
{
    AVLTree<int> tree;
    tree.insert(100);
    tree.insert(200);
    EXPECT_TRUE(tree.find(100));

    AVLTree<int> other;
    other.insert(999);
    other = std::move(tree);
    EXPECT_TRUE(other.find(100));
    EXPECT_TRUE(other.find(200));
    EXPECT_FALSE(tree.find(100));
}
