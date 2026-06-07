/**
 * @file BinarySearchTreeTest.cc
 * @brief Unit tests for the BinarySearchTree class
 * @details Tests cover insertion, removal, search, traversal,
 *          and edge cases for the binary search tree.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/BinarySearchTree.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for BinarySearchTree tests
 */
class BinarySearchTreeTest : public testing::Test
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

TEST_F(BinarySearchTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

TEST_F(BinarySearchTreeTest, Insert_MultipleValues_AllFound)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);

    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(70));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(60));
    EXPECT_TRUE(tree.find(80));
}

TEST_F(BinarySearchTreeTest, Insert_Duplicate_NoChange)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
}

TEST_F(BinarySearchTreeTest, Insert_LeftSkewed_StillFindsAll)
{
    BinarySearchTree<int> tree;
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

TEST_F(BinarySearchTreeTest, Insert_RightSkewed_StillFindsAll)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

// ==================== Find Tests ====================

TEST_F(BinarySearchTreeTest, Find_NonExisting_ReturnsFalse)
{
    const BinarySearchTree<int> tree;
    EXPECT_FALSE(tree.find(10));
}

TEST_F(BinarySearchTreeTest, Find_AfterInsert_FindsCorrectValue)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);

    EXPECT_TRUE(tree.find(5));
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_FALSE(tree.find(15));
}

// ==================== findValue Tests ====================

TEST_F(BinarySearchTreeTest, FindValue_ExistingValue_ReturnsValue)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);

    const auto result = tree.findValue(40);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 40);
}

TEST_F(BinarySearchTreeTest, FindValue_NonExisting_ReturnsNullopt)
{
    const BinarySearchTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

TEST_F(BinarySearchTreeTest, FindValue_EmptyTree_ReturnsNullopt)
{
    const BinarySearchTree<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

TEST_F(BinarySearchTreeTest, FindValue_RootNode_ReturnsValue)
{
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(150);

    const auto result = tree.findValue(100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

TEST_F(BinarySearchTreeTest, FindValue_LeftSubtree_ReturnsValue)
{
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(25);
    tree.insert(75);

    const auto result = tree.findValue(25);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 25);
}

TEST_F(BinarySearchTreeTest, FindValue_RightSubtree_ReturnsValue)
{
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(150);
    tree.insert(125);
    tree.insert(175);

    const auto result = tree.findValue(175);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 175);
}

TEST_F(BinarySearchTreeTest, FindValue_AfterRemoval_ReturnsNullopt)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    const auto result1 = tree.findValue(30);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), 30);

    tree.remove(30);

    const auto result2 = tree.findValue(30);
    EXPECT_FALSE(result2.has_value());
}

TEST_F(BinarySearchTreeTest, FindValue_ConsistencyWithFind)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    EXPECT_EQ(tree.find(20), tree.findValue(20).has_value());
    EXPECT_EQ(tree.find(999), tree.findValue(999).has_value());
}

TEST_F(BinarySearchTreeTest, FindValue_LargeDataset_ReturnsCorrectValue)
{
    BinarySearchTree<int> tree;

    for (int i = 0; i < 1000; ++i)
    {
        tree.insert(i);
    }

    const auto result1 = tree.findValue(0);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), 0);

    const auto result2 = tree.findValue(500);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), 500);

    const auto result3 = tree.findValue(999);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), 999);

    const auto result4 = tree.findValue(1000);
    EXPECT_FALSE(result4.has_value());
}

// ==================== Removal Tests ====================

TEST_F(BinarySearchTreeTest, Remove_LeafNode_Removed)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(70));
}

TEST_F(BinarySearchTreeTest, Remove_NodeWithOneChild_Removed)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(20);

    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(50));
}

TEST_F(BinarySearchTreeTest, Remove_NodeWithTwoChildren_Removed)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);

    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(70));
}

TEST_F(BinarySearchTreeTest, Remove_RootNode_Works)
{
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    tree.remove(50);
    EXPECT_FALSE(tree.find(50));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(70));
}

TEST_F(BinarySearchTreeTest, Remove_NonExisting_NoCrash)
{
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.remove(999);
    EXPECT_TRUE(tree.find(10));
}

TEST_F(BinarySearchTreeTest, Remove_FromEmptyTree_NoCrash)
{
    BinarySearchTree<int> tree;
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

TEST_F(BinarySearchTreeTest, Remove_AllElements_Empty)
{
    BinarySearchTree<int> tree;
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

// ==================== String Type ====================

TEST_F(BinarySearchTreeTest, StringType_StoresCorrectly)
{
    BinarySearchTree<std::string> tree;
    tree.insert("banana");
    tree.insert("apple");
    tree.insert("cherry");

    EXPECT_TRUE(tree.find("apple"));
    EXPECT_TRUE(tree.find("banana"));
    EXPECT_TRUE(tree.find("cherry"));
    EXPECT_FALSE(tree.find("grape"));
}

TEST_F(BinarySearchTreeTest, StringType_FindValue_ReturnsValue)
{
    BinarySearchTree<std::string> tree;
    tree.insert("hello");
    tree.insert("world");

    const auto result = tree.findValue("hello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "hello");
}

// ==================== Double Type ====================

TEST_F(BinarySearchTreeTest, DoubleType_StoresCorrectly)
{
    BinarySearchTree<double> tree;
    tree.insert(3.14);
    tree.insert(2.71);
    tree.insert(1.41);

    EXPECT_TRUE(tree.find(3.14));
    EXPECT_TRUE(tree.find(2.71));
    EXPECT_TRUE(tree.find(1.41));

    const auto result = tree.findValue(2.71);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 2.71);
}

// ==================== Negative Key Tests ====================

TEST_F(BinarySearchTreeTest, Insert_NegativeKeys_StoresCorrectly)
{
    BinarySearchTree<int> tree;
    tree.insert(-10);
    tree.insert(-20);
    tree.insert(-30);

    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(-20));
    EXPECT_TRUE(tree.find(-30));
    EXPECT_FALSE(tree.find(0));
}

TEST_F(BinarySearchTreeTest, FindValue_NegativeKeys_ReturnsValue)
{
    BinarySearchTree<int> tree;
    tree.insert(-5);
    tree.insert(-3);
    tree.insert(-7);

    const auto result = tree.findValue(-3);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), -3);
}

TEST_F(BinarySearchTreeTest, Remove_NegativeKeys_RemovesCorrectly)
{
    BinarySearchTree<int> tree;
    tree.insert(0);
    tree.insert(-10);
    tree.insert(10);

    tree.remove(-10);
    EXPECT_FALSE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
    EXPECT_TRUE(tree.find(10));
}

// ==================== Large Dataset ====================

TEST_F(BinarySearchTreeTest, Insert_LargeDataset_AllFound)
{
    BinarySearchTree<int> tree;
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

TEST_F(BinarySearchTreeTest, Remove_LargeDataset_Consistent)
{
    BinarySearchTree<int> tree;
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
