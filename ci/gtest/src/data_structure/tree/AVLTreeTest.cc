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

TEST_F(AVLTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    AVLTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

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

TEST_F(AVLTreeTest, Insert_Duplicate_NoChange)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    // Size should still be 1 effectively
}

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

TEST_F(AVLTreeTest, Remove_NodeWithOneChild_RemovedSuccessfully)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

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

TEST_F(AVLTreeTest, Remove_NonExisting_NoChange)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    tree.remove(999);
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
}

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

TEST_F(AVLTreeTest, Remove_FromEmptyTree_NoCrash)
{
    AVLTree<int> tree;
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

// ==================== Find Tests ====================

TEST_F(AVLTreeTest, Find_NonExisting_ReturnsFalse)
{
    const AVLTree<int> tree;
    EXPECT_FALSE(tree.find(10));
}

TEST_F(AVLTreeTest, Find_AfterRemove_ReturnsFalse)
{
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

// ==================== findValue Tests ====================

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

TEST_F(AVLTreeTest, FindValue_NonExisting_ReturnsNullopt)
{
    const AVLTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AVLTreeTest, FindValue_EmptyTree_ReturnsNullopt)
{
    const AVLTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

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
