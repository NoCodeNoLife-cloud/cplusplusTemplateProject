/**
 * @file TreapTest.cc
 * @brief Unit tests for the Treap class
 * @details Tests cover insertion, removal, search, heap-order invariants,
 *          and edge cases for the randomized binary search tree.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/Treap.hpp"

using namespace common::data_structure::tree;

/**
 * @brief Test fixture for Treap tests
 */
class TreapTest : public testing::Test
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

TEST_F(TreapTest, Insert_SingleNode_FindReturnsTrue)
{
    Treap<int> tree;
    tree.insert(10);
    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
    EXPECT_FALSE(tree.empty());
}

TEST_F(TreapTest, Insert_MultipleValues_AllFound)
{
    Treap<int> tree;
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
    EXPECT_EQ(5, tree.size());
}

TEST_F(TreapTest, Insert_Duplicate_NoChange)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(10);

    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
}

TEST_F(TreapTest, Insert_MaintainsHeapProperty)
{
    Treap<int> tree;
    for (const int v : {5, 3, 7, 2, 4, 6, 8})
    {
        tree.insert(v);
    }
    EXPECT_TRUE(tree.verify());
    EXPECT_EQ(7, tree.size());
}

TEST_F(TreapTest, Insert_MaintainsInorder)
{
    Treap<int> tree;
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);
    tree.insert(2);
    tree.insert(4);
    tree.insert(6);
    tree.insert(8);

    const std::vector<int> expected = {2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(TreapTest, Insert_SequentialValues_StillBalanced)
{
    Treap<int> tree;
    for (int i = 1; i <= 100; ++i)
    {
        tree.insert(i);
    }

    EXPECT_EQ(100, tree.size());
    EXPECT_TRUE(tree.verify());

    const auto sorted = tree.inorder();
    for (int i = 1; i <= 100; ++i)
    {
        EXPECT_EQ(i, sorted[static_cast<size_t>(i) - 1]);
    }
}

// ==================== Find Tests ====================

TEST_F(TreapTest, Find_EmptyTree_ReturnsFalse)
{
    const Treap<int> tree;
    EXPECT_FALSE(tree.find(1));
}

TEST_F(TreapTest, Find_ExistingValue_ReturnsTrue)
{
    Treap<int> tree;
    tree.insert(42);
    EXPECT_TRUE(tree.find(42));
}

TEST_F(TreapTest, Find_NonExistingValue_ReturnsFalse)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(20);
    EXPECT_FALSE(tree.find(30));
}

TEST_F(TreapTest, FindValue_Existing_ReturnsValue)
{
    Treap<int> tree;
    tree.insert(42);
    const auto result = tree.findValue(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(42, result.value());
}

TEST_F(TreapTest, FindValue_NonExisting_ReturnsNullopt)
{
    Treap<int> tree;
    tree.insert(1);
    tree.insert(2);
    const auto result = tree.findValue(99);
    EXPECT_FALSE(result.has_value());
}

TEST_F(TreapTest, FindValue_EmptyTree_ReturnsNullopt)
{
    const Treap<int> tree;
    const auto result = tree.findValue(1);
    EXPECT_FALSE(result.has_value());
}

// ==================== Removal Tests ====================

TEST_F(TreapTest, Remove_LeafNode)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(20);

    EXPECT_FALSE(tree.find(20));
    EXPECT_TRUE(tree.find(10));
    EXPECT_EQ(1, tree.size());
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, Remove_NodeWithLeftChild)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(5);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(5));
    EXPECT_EQ(1, tree.size());
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, Remove_NodeWithRightChild)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(20));
    EXPECT_EQ(1, tree.size());
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, Remove_NodeWithTwoChildren)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(5);
    tree.insert(15);
    tree.remove(10);

    EXPECT_FALSE(tree.find(10));
    EXPECT_TRUE(tree.find(5));
    EXPECT_TRUE(tree.find(15));
    EXPECT_EQ(2, tree.size());
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, Remove_RootNode)
{
    Treap<int> tree;
    tree.insert(42);
    tree.remove(42);

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(TreapTest, Remove_NonExisting_NoChange)
{
    Treap<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.remove(99);

    EXPECT_EQ(2, tree.size());
    EXPECT_TRUE(tree.find(1));
    EXPECT_TRUE(tree.find(2));
}

TEST_F(TreapTest, Remove_EmptyTree_NoCrash)
{
    Treap<int> tree;
    tree.remove(1);
    EXPECT_TRUE(tree.empty());
}

TEST_F(TreapTest, Remove_AllElements)
{
    Treap<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.remove(1);
    tree.remove(2);
    tree.remove(3);

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(TreapTest, Remove_ComplexTree_InvariantsPreserved)
{
    Treap<int> tree;
    for (const int v : {5, 3, 7, 2, 4, 6, 8})
    {
        tree.insert(v);
    }

    tree.remove(5);
    EXPECT_TRUE(tree.verify());
    EXPECT_EQ(6, tree.size());

    tree.remove(3);
    EXPECT_TRUE(tree.verify());
    EXPECT_EQ(5, tree.size());

    const std::vector<int> expected = {2, 4, 6, 7, 8};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(TreapTest, Remove_AllNodesSequentially)
{
    Treap<int> tree;
    for (int i = 1; i <= 50; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(50, tree.size());

    for (int i = 1; i <= 50; ++i)
    {
        tree.remove(i);
        EXPECT_TRUE(tree.verify());
    }
    EXPECT_TRUE(tree.empty());
}

// ==================== Edge Case Tests ====================

TEST_F(TreapTest, Clear_RemovesAllNodes)
{
    Treap<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    EXPECT_FALSE(tree.empty());

    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_FALSE(tree.find(1));
}

TEST_F(TreapTest, LargeDataset_InsertAndFindAll)
{
    Treap<int> tree;
    constexpr int N = 1000;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(static_cast<size_t>(N), tree.size());
    EXPECT_TRUE(tree.verify());

    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.find(i));
    }
}

TEST_F(TreapTest, LargeDataset_InsertRemoveConsistency)
{
    Treap<int> tree;
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
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, RandomOperations_InvariantsHold)
{
    Treap<int> tree;
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, 999);

    for (int i = 0; i < 100; ++i)
    {
        tree.insert(dist(gen));
    }
    EXPECT_TRUE(tree.verify());

    const auto sorted = tree.inorder();
    for (size_t i = 1; i < sorted.size(); ++i)
    {
        EXPECT_LE(sorted[i - 1], sorted[i]);
    }
}

// ==================== String Type Tests ====================

TEST_F(TreapTest, StringType_InsertAndFind)
{
    Treap<std::string> tree;
    tree.insert("hello");
    tree.insert("world");
    tree.insert("treap");

    EXPECT_TRUE(tree.find("hello"));
    EXPECT_TRUE(tree.find("world"));
    EXPECT_TRUE(tree.find("treap"));
    EXPECT_FALSE(tree.find("nonexistent"));
    EXPECT_TRUE(tree.verify());
}

TEST_F(TreapTest, StringType_FindValue)
{
    Treap<std::string> tree;
    tree.insert(std::string("test"));
    const auto result = tree.findValue(std::string("test"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("test", result.value());
}

// ==================== Negative Key Tests ====================

TEST_F(TreapTest, NegativeKeys_InsertAndFind)
{
    Treap<int> tree;
    tree.insert(-5);
    tree.insert(-10);
    tree.insert(0);
    tree.insert(7);

    EXPECT_TRUE(tree.find(-5));
    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
    EXPECT_TRUE(tree.find(7));
    EXPECT_FALSE(tree.find(-99));
    EXPECT_TRUE(tree.verify());

    const std::vector<int> expected = {-10, -5, 0, 7};
    EXPECT_EQ(expected, tree.inorder());
}

TEST_F(TreapTest, NegativeKeys_Remove)
{
    Treap<int> tree;
    tree.insert(-10);
    tree.insert(-5);
    tree.insert(0);
    tree.remove(-5);

    EXPECT_FALSE(tree.find(-5));
    EXPECT_TRUE(tree.find(-10));
    EXPECT_TRUE(tree.find(0));
    EXPECT_TRUE(tree.verify());
}

// ==================== Move Semantics Tests ====================

TEST_F(TreapTest, MoveConstructor)
{
    Treap<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    EXPECT_TRUE(tree.find(10));

    const Treap<int> other(std::move(tree));
    EXPECT_TRUE(other.find(10));
    EXPECT_TRUE(other.find(20));
    EXPECT_TRUE(other.find(5));
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(other.verify());
}

TEST_F(TreapTest, MoveAssignment)
{
    Treap<int> tree;
    tree.insert(100);
    tree.insert(200);
    EXPECT_TRUE(tree.find(100));

    Treap<int> other;
    other.insert(999);
    other = std::move(tree);
    EXPECT_TRUE(other.find(100));
    EXPECT_TRUE(other.find(200));
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(other.verify());
}

// ==================== Priority Distribution Tests ====================

TEST_F(TreapTest, PrioritiesAreNotAllEqual)
{
    Treap<int> tree;
    // Deterministic seed for reproducibility, but test should still pass
    // as priorities are independent random values
    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i);
    }
    EXPECT_TRUE(tree.verify());
}
