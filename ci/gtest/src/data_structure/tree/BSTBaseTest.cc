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

#include "data_structure/tree/BSTBase.hpp"

using namespace common::data_structure::tree;

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

TEST_F(BSTBaseTest, Insert_SingleNode_FindReturnsTrue)
{
    TestBST<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

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

TEST_F(BSTBaseTest, Insert_Duplicate_NoChange)
{
    TestBST<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
}

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

TEST_F(BSTBaseTest, Find_NonExisting_ReturnsFalse)
{
    const TestBST<int> tree;
    EXPECT_FALSE(tree.find(10));
}

TEST_F(BSTBaseTest, Find_EmptyTree_ReturnsFalse)
{
    const TestBST<int> tree;
    EXPECT_FALSE(tree.find(0));
}

TEST_F(BSTBaseTest, Find_AfterInsert_ReturnsTrue)
{
    TestBST<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.find(42));
}

// ==================== FindValue Tests ====================

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

TEST_F(BSTBaseTest, FindValue_NonExisting_ReturnsNullopt)
{
    const TestBST<int> tree;
    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

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

TEST_F(BSTBaseTest, Remove_NonExisting_NoCrash)
{
    TestBST<int> tree;
    tree.insert(10);
    tree.remove(999);
    EXPECT_TRUE(tree.find(10));
}

TEST_F(BSTBaseTest, Remove_FromEmptyTree_NoCrash)
{
    TestBST<int> tree;
    tree.remove(10);
    EXPECT_FALSE(tree.find(10));
}

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
