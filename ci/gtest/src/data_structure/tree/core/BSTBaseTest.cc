/**
 * @file BSTBaseTest.cc
 * @brief Unit tests for the BSTBase CRTP base class
 * @details Tests cover insert, remove, find, and findValue operations
 *          through a minimal concrete derived class.
 */

#include <memory>
#include <optional>
#include <utility>
#include <gtest/gtest.h>

#include <cppforge/data_structure/tree/core/BSTBase.hpp>

using namespace cppforge::data_structure::tree::core;

/**
 * @brief Minimal concrete derived class for testing BSTBase.
 * @details Uses the default no-op postInsert/postRemove hooks.
 */
template <typename T>
class TestBST : public BSTBase<T, TestBST<T>>
{
};

class BSTBaseTest : public testing::Test
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
 * @brief Test inserting a single node into BST
 * @details Verifies that after inserting a single element, find returns true for that element
 */
TEST_F(BSTBaseTest, Insert_SingleNode_FindReturnsTrue)
{
    TestBST<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

/**
 * @brief Test inserting multiple values into BST
 * @details Verifies that all inserted values can be found after insertion
 */
TEST_F(BSTBaseTest, Insert_MultipleValues_AllFound)
{
    TestBST<int> tree;
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

/**
 * @brief Test inserting duplicate values into BST
 * @details Verifies that inserting a duplicate value does not change the tree structure
 */
TEST_F(BSTBaseTest, Insert_Duplicate_NoChange)
{
    TestBST<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

/**
 * @brief Test left-skewed insertion into BST
 * @details Verifies that inserting values in descending order creates a valid BST
 *          where all values are findable
 */
TEST_F(BSTBaseTest, Insert_LeftSkewed_AllFound)
{
    TestBST<int> tree;
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

/**
 * @brief Test right-skewed insertion into BST
 * @details Verifies that inserting values in ascending order creates a valid BST
 *          where all values are findable
 */
TEST_F(BSTBaseTest, Insert_RightSkewed_AllFound)
{
    TestBST<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    EXPECT_TRUE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(30));
}

// ==================== Find Tests ====================

/**
 * @brief Test find with a non-existing key returns false
 * @details Verifies that querying a key not present in an empty tree returns false
 */
TEST_F(BSTBaseTest, Find_NonExisting_ReturnsFalse)
{
    const TestBST<int> tree;
    EXPECT_FALSE(tree.find(10));
}

/**
 * @brief Test find on an empty tree returns false
 * @details Verifies that querying any key in an empty tree returns false
 */
TEST_F(BSTBaseTest, Find_EmptyTree_ReturnsFalse)
{
    const TestBST<int> tree;
    EXPECT_FALSE(tree.find(0));
}

/**
 * @brief Test find after insertion returns true
 * @details Verifies that after inserting a key, find locates it correctly
 */
TEST_F(BSTBaseTest, Find_AfterInsert_ReturnsTrue)
{
    TestBST<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.find(42));
}

// ==================== FindValue Tests ====================

/**
 * @brief Test findValue with an existing value
 * @details Verifies that findValue returns the correct value for an existing key
 */
TEST_F(BSTBaseTest, FindValue_Existing_ReturnsValue)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    const auto result = tree.findValue(30);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 30);
}

/**
 * @brief Test findValue with a non-existing value
 * @details Verifies that findValue returns nullopt for a key not in the tree
 */
TEST_F(BSTBaseTest, FindValue_NonExisting_ReturnsNullopt)
{
    const TestBST<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test findValue on the root node
 * @details Verifies that findValue correctly retrieves the value stored at the root
 */
TEST_F(BSTBaseTest, FindValue_RootNode)
{
    TestBST<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(150);
    const auto result = tree.findValue(100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

/**
 * @brief Test findValue in the left subtree
 * @details Verifies that findValue works for nodes in the left subtree
 */
TEST_F(BSTBaseTest, FindValue_LeftSubtree)
{
    TestBST<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(25);
    tree.insert(75);
    const auto result = tree.findValue(25);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 25);
}

/**
 * @brief Test findValue in the right subtree
 * @details Verifies that findValue works for nodes in the right subtree
 */
TEST_F(BSTBaseTest, FindValue_RightSubtree)
{
    TestBST<int> tree;
    tree.insert(100);
    tree.insert(150);
    tree.insert(125);
    tree.insert(175);
    const auto result = tree.findValue(175);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 175);
}

/**
 * @brief Test findValue after a key has been removed
 * @details Verifies that findValue returns nullopt for a key that was removed
 */
TEST_F(BSTBaseTest, FindValue_AfterRemoval_ReturnsNullopt)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.remove(30);
    const auto result = tree.findValue(30);
    EXPECT_FALSE(result.has_value());
}

// ==================== Removal Tests ====================

/**
 * @brief Test removing a leaf node
 * @details Verifies that a leaf node can be removed without affecting other nodes
 */
TEST_F(BSTBaseTest, Remove_LeafNode_Removed)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(50));
    EXPECT_TRUE(tree.find(70));
}

/**
 * @brief Test removing a node with only a left child
 * @details Verifies that a node with one left child is bypassed correctly
 */
TEST_F(BSTBaseTest, Remove_NodeWithOneLeftChild_Removed)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(20);
    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(20));
    EXPECT_TRUE(tree.find(50));
}

/**
 * @brief Test removing a node with only a right child
 * @details Verifies that a node with one right child is bypassed correctly
 */
TEST_F(BSTBaseTest, Remove_NodeWithOneRightChild_Removed)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(40);
    tree.remove(30);
    EXPECT_FALSE(tree.find(30));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(50));
}

/**
 * @brief Test removing a node with two children
 * @details Verifies that a node with two children is replaced by its inorder successor
 */
TEST_F(BSTBaseTest, Remove_NodeWithTwoChildren_Removed)
{
    TestBST<int> tree;
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

/**
 * @brief Test removing the root node
 * @details Verifies that the root node can be removed and the tree remains valid
 */
TEST_F(BSTBaseTest, Remove_RootNode)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.remove(50);
    EXPECT_FALSE(tree.find(50));
    EXPECT_TRUE(tree.find(30));
    EXPECT_TRUE(tree.find(70));
}

/**
 * @brief Test removing a non-existing key does not crash
 * @details Verifies that attempting to remove a key not in the tree is a no-op
 */
TEST_F(BSTBaseTest, Remove_NonExisting_NoCrash)
{
    TestBST<int> tree;
    tree.insert(10);
    tree.remove(999);
    EXPECT_TRUE(tree.find(10));
}

/**
 * @brief Test removing from an empty tree does not crash
 * @details Verifies that calling remove on an empty tree handles gracefully
 */
TEST_F(BSTBaseTest, Remove_FromEmptyTree_NoCrash)
{
    TestBST<int> tree;
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

/**
 * @brief Test removing all elements empties the tree
 * @details Verifies that after removing all inserted elements, the tree is empty
 */
TEST_F(BSTBaseTest, Remove_AllElements_Empty)
{
    TestBST<int> tree;
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
 * @brief Test removing root that has no left child
 * @details Verifies that removing a root with only right children works correctly
 */
TEST_F(BSTBaseTest, Remove_RootWithNoLeftChild)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(60);
    tree.insert(70);
    tree.remove(50);
    EXPECT_FALSE(tree.find(50));
    EXPECT_TRUE(tree.find(60));
    EXPECT_TRUE(tree.find(70));
}

/**
 * @brief Test removing root that has no right child
 * @details Verifies that removing a root with only left children works correctly
 */
TEST_F(BSTBaseTest, Remove_RootWithNoRightChild)
{
    TestBST<int> tree;
    tree.insert(50);
    tree.insert(40);
    tree.insert(30);
    tree.remove(50);
    EXPECT_FALSE(tree.find(50));
    EXPECT_TRUE(tree.find(40));
    EXPECT_TRUE(tree.find(30));
}

// ==================== String Type ====================

/**
 * @brief Test BST with string keys
 * @details Verifies that the BST correctly stores and retrieves string-typed keys
 */
TEST_F(BSTBaseTest, StringType_StoresCorrectly)
{
    TestBST<std::string> tree;
    tree.insert("banana");
    tree.insert("apple");
    tree.insert("cherry");
    EXPECT_TRUE(tree.find("apple"));
    EXPECT_TRUE(tree.find("banana"));
    EXPECT_TRUE(tree.find("cherry"));
    EXPECT_FALSE(tree.find("grape"));
}

/**
 * @brief Test findValue with string keys
 * @details Verifies that findValue works correctly with string-typed data
 */
TEST_F(BSTBaseTest, StringType_FindValue)
{
    TestBST<std::string> tree;
    tree.insert("hello");
    tree.insert("world");
    const auto result = tree.findValue("hello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "hello");
}

// ==================== Double Type ====================

/**
 * @brief Test BST with double-precision keys
 * @details Verifies that the BST correctly handles floating-point keys
 */
TEST_F(BSTBaseTest, DoubleType_StoresCorrectly)
{
    TestBST<double> tree;
    tree.insert(3.14);
    tree.insert(2.71);
    tree.insert(1.41);
    EXPECT_TRUE(tree.find(3.14));
    EXPECT_TRUE(tree.find(2.71));
    const auto result = tree.findValue(2.71);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 2.71);
}

// ==================== Negative Key Tests ====================

/**
 * @brief Test BST with negative integer keys
 * @details Verifies that the BST correctly handles negative keys
 */
TEST_F(BSTBaseTest, NegativeKeys_StoresCorrectly)
{
    TestBST<int> tree;
    tree.insert(-10);
    tree.insert(-20);
    tree.insert(-30);
    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(-20));
    EXPECT_TRUE(tree.find(-30));
    EXPECT_FALSE(tree.find(0));
}

/**
 * @brief Test removing negative keys
 * @details Verifies that removal works correctly with negative keys
 */
TEST_F(BSTBaseTest, Remove_NegativeKeys)
{
    TestBST<int> tree;
    tree.insert(0);
    tree.insert(-10);
    tree.insert(10);
    tree.remove(-10);
    EXPECT_FALSE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
    EXPECT_TRUE(tree.find(10));
}

// ==================== Large Dataset ====================

/**
 * @brief Test large dataset insertion
 * @details Verifies that inserting 500 elements and searching for all of them works correctly
 */
TEST_F(BSTBaseTest, Insert_LargeDataset_AllFound)
{
    TestBST<int> tree;
    constexpr int kCount = 500;
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
 * @brief Test large dataset removal consistency
 * @details Verifies that removing even elements from 500 elements leaves odd elements findable
 */
TEST_F(BSTBaseTest, Remove_LargeDataset_Consistent)
{
    TestBST<int> tree;
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
