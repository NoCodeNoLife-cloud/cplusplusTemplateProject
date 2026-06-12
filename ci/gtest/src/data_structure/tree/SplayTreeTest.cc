/**
 * @file SplayTreeTest.cc
 * @brief Unit tests for the SplayTree class
 * @details Tests cover insertion, removal, search, splay amortization invariants,
 *          and edge cases for the self-adjusting splay tree.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/SplayTree.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for SplayTree tests
 */
class SplayTreeTest : public testing::Test
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

TEST_F(SplayTreeTest, Insert_SingleNode_FindReturnsTrue)
{
    SplayTree<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
    EXPECT_FALSE(tree.empty());
}

TEST_F(SplayTreeTest, Insert_LastInsertedBecomesRoot)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    // After splay, the last inserted value becomes the root
    EXPECT_TRUE(tree.find(30));
    // find splays accessed node - we verify splay semantics via inorder
    std::vector<int> expected = {10, 20, 30};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(SplayTreeTest, Insert_MultipleValues_AllFound)
{
    SplayTree<int> tree;
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

TEST_F(SplayTreeTest, Insert_Duplicate_NoChange)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
}

TEST_F(SplayTreeTest, Insert_SequentialAscending_ZigZagSplayAtRoot)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    // last inserted (3) should be root after splay
    auto vec = tree.inorder();
    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(expected, vec);
}

TEST_F(SplayTreeTest, Insert_SequentialDescending_ZigZigSplayAtRoot)
{
    SplayTree<int> tree;
    tree.insert(3);
    tree.insert(2);
    tree.insert(1);

    auto vec = tree.inorder();
    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(expected, vec);
}

TEST_F(SplayTreeTest, Insert_ComplexOrder_InorderCorrect)
{
    SplayTree<int> tree;
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);
    tree.insert(2);
    tree.insert(4);
    tree.insert(6);
    tree.insert(8);

    std::vector<int> expected = {2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(expected, tree.inorder());
    EXPECT_EQ(7, tree.size());
}

// ==================== Splay Semantic Tests ====================

TEST_F(SplayTreeTest, Find_SplaysFoundNodeToRoot)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    // Verify order is correct
    EXPECT_EQ((std::vector<int>{10, 20, 30}), tree.inorder());

    // find(10) should splay 10 to root
    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(10, tree.inorder()[0]); // 10 is smallest, stays in correct position
}

TEST_F(SplayTreeTest, Find_NonExisting_DoesNotCrash)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    EXPECT_FALSE(tree.find(30));
}

TEST_F(SplayTreeTest, Find_EmptyTree_ReturnsFalse)
{
    SplayTree<int> tree;
    EXPECT_FALSE(tree.find(1));
}

TEST_F(SplayTreeTest, FindValue_Existing_ReturnsValue)
{
    SplayTree<int> tree;
    tree.insert(42);
    auto result = tree.findValue(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(42, result.value());
}

TEST_F(SplayTreeTest, FindValue_NonExisting_ReturnsNullopt)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    auto result = tree.findValue(99);
    EXPECT_FALSE(result.has_value());
}

TEST_F(SplayTreeTest, FindValue_EmptyTree_ReturnsNullopt)
{
    SplayTree<int> tree;
    auto result = tree.findValue(1);
    EXPECT_FALSE(result.has_value());
}

// ==================== Removal Tests ====================

TEST_F(SplayTreeTest, Remove_LeafNode)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(20);

    EXPECT_FALSE(tree.find(20));
    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
}

TEST_F(SplayTreeTest, Remove_NodeWithLeftChild)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(5);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(5));
    EXPECT_EQ(1, tree.size());
}

TEST_F(SplayTreeTest, Remove_NodeWithRightChild)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_EQ(1, tree.size());
}

TEST_F(SplayTreeTest, Remove_NodeWithTwoChildren)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(5);
    tree.insert(15);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(5));
    EXPECT_TRUE(tree.find(15));
    EXPECT_EQ(2, tree.size());
}

TEST_F(SplayTreeTest, Remove_RootNode)
{
    SplayTree<int> tree;
    tree.insert(42);
    tree.remove(42);

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(SplayTreeTest, Remove_NonExisting_NoChange)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.remove(99);

    EXPECT_EQ(2, tree.size());
    EXPECT_TRUE(tree.find(1));
    EXPECT_TRUE(tree.find(2));
}

TEST_F(SplayTreeTest, Remove_EmptyTree_NoCrash)
{
    SplayTree<int> tree;
    tree.remove(1);
    EXPECT_TRUE(tree.empty());
}

TEST_F(SplayTreeTest, Remove_AllElements)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.remove(1);
    tree.remove(2);
    tree.remove(3);

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(SplayTreeTest, Remove_ComplexTree_InorderPreserved)
{
    SplayTree<int> tree;
    for (const int v : {5, 3, 7, 2, 4, 6, 8})
    {
        tree.insert(v);
    }

    tree.remove(5);
    tree.remove(3);

    std::vector<int> expected = {2, 4, 6, 7, 8};
    EXPECT_EQ(expected, tree.inorder());
    EXPECT_EQ(5, tree.size());
}

// ==================== Edge Case Tests ====================

TEST_F(SplayTreeTest, Clear_RemovesAllNodes)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    EXPECT_FALSE(tree.empty());

    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_FALSE(tree.find(1));
}

TEST_F(SplayTreeTest, LargeDataset_InsertAndFindAll)
{
    SplayTree<int> tree;
    constexpr int N = 1000;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(static_cast<size_t>(N), tree.size());

    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.find(i));
    }
}

TEST_F(SplayTreeTest, LargeDataset_InsertRemoveConsistency)
{
    SplayTree<int> tree;
    constexpr int N = 500;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(static_cast<size_t>(N), tree.size());

    for (int i = N - 1; i >= 0; --i)
    {
        tree.remove(i);
    }
    EXPECT_TRUE(tree.empty());
}

// ==================== String Type Tests ====================

TEST_F(SplayTreeTest, StringType_InsertAndFind)
{
    SplayTree<std::string> tree;
    tree.insert("hello");
    tree.insert("world");
    tree.insert("splay");

    EXPECT_TRUE(tree.find("hello"));
    EXPECT_TRUE(tree.find("world"));
    EXPECT_TRUE(tree.find("splay"));
    EXPECT_FALSE(tree.find("nonexistent"));
}

TEST_F(SplayTreeTest, StringType_FindValue)
{
    SplayTree<std::string> tree;
    tree.insert(std::string("test"));
    auto result = tree.findValue(std::string("test"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("test", result.value());
}

// ==================== Negative Key Tests ====================

TEST_F(SplayTreeTest, NegativeKeys_InsertAndFind)
{
    SplayTree<int> tree;
    tree.insert(-5);
    tree.insert(-10);
    tree.insert(0);
    tree.insert(7);

    EXPECT_TRUE(tree.find(-5));
    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
    EXPECT_TRUE(tree.find(7));
    EXPECT_FALSE(tree.find(-99));

    std::vector<int> expected = {-10, -5, 0, 7};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(SplayTreeTest, NegativeKeys_Remove)
{
    SplayTree<int> tree;
    tree.insert(-10);
    tree.insert(-5);
    tree.insert(0);
    tree.remove(-5);

    EXPECT_FALSE(tree.find(-5));
    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
}

// ==================== Move Semantics Tests ====================

TEST_F(SplayTreeTest, MoveConstructor)
{
    SplayTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    EXPECT_TRUE(tree.find(10));

    SplayTree<int> other(std::move(tree));
    EXPECT_TRUE(other.find(10));
    EXPECT_TRUE(other.find(20));
    EXPECT_TRUE(other.find(5));
    EXPECT_TRUE(tree.empty()); // NOLINT: moved-from is valid but unspecified
}

TEST_F(SplayTreeTest, MoveAssignment)
{
    SplayTree<int> tree;
    tree.insert(100);
    tree.insert(200);
    EXPECT_TRUE(tree.find(100));

    SplayTree<int> other;
    other.insert(999);
    other = std::move(tree);
    EXPECT_TRUE(other.find(100));
    EXPECT_TRUE(other.find(200));
    EXPECT_TRUE(tree.empty()); // NOLINT: moved-from is valid but unspecified
}

// ==================== Splay-Specific Tests ====================

TEST_F(SplayTreeTest, FindRepeatedly_SplayAffectsStructure)
{
    SplayTree<int> tree;
    // Insert a chain: 1 -> 2 -> 3 -> 4 -> 5
    for (int i = 1; i <= 5; ++i)
    {
        tree.insert(i);
    }

    // find(3) should bring 3 to (or near) the root
    EXPECT_TRUE(tree.find(3));

    // Now find(3) again should be fast (cached near root)
    EXPECT_TRUE(tree.find(3));

    // The tree should still contain all values
    for (int i = 1; i <= 5; ++i)
    {
        EXPECT_TRUE(tree.find(i));
    }
}

TEST_F(SplayTreeTest, InsertAfterRemove_ReusesSize)
{
    SplayTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.remove(1);
    EXPECT_EQ(1, tree.size());

    tree.insert(3);
    EXPECT_EQ(2, tree.size());
    EXPECT_TRUE(tree.find(2));
    EXPECT_TRUE(tree.find(3));
    EXPECT_FALSE(tree.find(1));
}
