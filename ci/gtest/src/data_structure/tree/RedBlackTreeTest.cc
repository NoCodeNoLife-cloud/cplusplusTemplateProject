/**
 * @file RedBlackTreeTest.cc
 * @brief Unit tests for the RedBlackTree class and RedBlackTreeNode
 * @details Tests cover insertion, color invariants, node accessors,
 *          rotation correctness, and property enforcement.
 */

#include <algorithm>
#include <memory>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/tree/RedBlackTree.hpp"

using namespace common::data_structure::tree;
using namespace common::data_structure;

/**
 * @brief Test fixture for RedBlackTree tests
 */
class RedBlackTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test fixture for RedBlackTreeNode tests
 */
class RedBlackTreeNodeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== RedBlackTreeNode Tests ====================

TEST_F(RedBlackTreeNodeTest, Constructor_SetsData)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getData(), 42);
}

TEST_F(RedBlackTreeNodeTest, Constructor_DefaultColorIsRed)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getColor(), Color::Red);
}

TEST_F(RedBlackTreeNodeTest, SetData_UpdatesValue)
{
    RedBlackTreeNode<int> node(10);
    node.setData(99);
    EXPECT_EQ(node.getData(), 99);
}

TEST_F(RedBlackTreeNodeTest, SetColor_UpdatesColor)
{
    RedBlackTreeNode<int> node(10);
    node.setColor(Color::Black);
    EXPECT_EQ(node.getColor(), Color::Black);
    node.setColor(Color::Red);
    EXPECT_EQ(node.getColor(), Color::Red);
}

TEST_F(RedBlackTreeNodeTest, LeftRight_NullByDefault)
{
    const RedBlackTreeNode<int> node(10);
    EXPECT_EQ(node.getLeft(), nullptr);
    EXPECT_EQ(node.getRight(), nullptr);
    EXPECT_EQ(node.getParent(), nullptr);
}

TEST_F(RedBlackTreeNodeTest, SetLeftRightParent_UpdatesPointers)
{
    auto n1 = std::make_shared<RedBlackTreeNode<int>>(1);
    auto n2 = std::make_shared<RedBlackTreeNode<int>>(2);
    auto n3 = std::make_shared<RedBlackTreeNode<int>>(3);

    n1->setLeft(n2);
    n1->setRight(n3);
    n2->setParent(n1);
    n3->setParent(n1);

    EXPECT_EQ(n1->getLeft(), n2);
    EXPECT_EQ(n1->getRight(), n3);
    EXPECT_EQ(n2->getParent(), n1);
    EXPECT_EQ(n3->getParent(), n1);
}

TEST_F(RedBlackTreeNodeTest, SetLeft_ReplacesChild)
{
    auto n1 = std::make_shared<RedBlackTreeNode<int>>(1);
    n1->setLeft(std::make_shared<RedBlackTreeNode<int>>(2));
    ASSERT_NE(n1->getLeft(), nullptr);
    EXPECT_EQ(n1->getLeft()->getData(), 2);

    auto n3 = std::make_shared<RedBlackTreeNode<int>>(3);
    n1->setLeft(n3);
    EXPECT_EQ(n1->getLeft()->getData(), 3);
}

// ==================== RedBlackTree Insertion Tests ====================

TEST_F(RedBlackTreeTest, Insert_SingleNode_RootIsBlack)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 10);
    EXPECT_EQ(root->getColor(), Color::Black);
}

TEST_F(RedBlackTreeTest, Insert_TwoNodes_MaintainsOrder)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(5);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 10);
    ASSERT_NE(root->getLeft(), nullptr);
    EXPECT_EQ(root->getLeft()->getData(), 5);
}

TEST_F(RedBlackTreeTest, Insert_Duplicate_GoesToRight)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(10);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    // Duplicate goes to right subtree
    ASSERT_NE(root->getRight(), nullptr);
    EXPECT_EQ(root->getRight()->getData(), 10);
}

TEST_F(RedBlackTreeTest, Insert_LeftLeftCase_RotatesCorrectly)
{
    // Insert 30, 20, 10 should trigger right rotation
    RedBlackTree<int> tree;
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
    EXPECT_EQ(root->getColor(), Color::Black);
    ASSERT_NE(root->getLeft(), nullptr);
    EXPECT_EQ(root->getLeft()->getData(), 10);
    EXPECT_EQ(root->getLeft()->getColor(), Color::Red);
    ASSERT_NE(root->getRight(), nullptr);
    EXPECT_EQ(root->getRight()->getData(), 30);
    EXPECT_EQ(root->getRight()->getColor(), Color::Red);
}

TEST_F(RedBlackTreeTest, Insert_RightRightCase_RotatesCorrectly)
{
    // Insert 10, 20, 30 should trigger left rotation
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
    EXPECT_EQ(root->getColor(), Color::Black);
    ASSERT_NE(root->getLeft(), nullptr);
    EXPECT_EQ(root->getLeft()->getData(), 10);
    EXPECT_EQ(root->getLeft()->getColor(), Color::Red);
    ASSERT_NE(root->getRight(), nullptr);
    EXPECT_EQ(root->getRight()->getData(), 30);
    EXPECT_EQ(root->getRight()->getColor(), Color::Red);
}

TEST_F(RedBlackTreeTest, Insert_LeftRightCase_RotatesCorrectly)
{
    // Insert 30, 10, 20 should trigger left-right rotation
    RedBlackTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
    EXPECT_EQ(root->getColor(), Color::Black);
    ASSERT_NE(root->getLeft(), nullptr);
    EXPECT_EQ(root->getLeft()->getData(), 10);
    EXPECT_EQ(root->getLeft()->getColor(), Color::Red);
    ASSERT_NE(root->getRight(), nullptr);
    EXPECT_EQ(root->getRight()->getData(), 30);
    EXPECT_EQ(root->getRight()->getColor(), Color::Red);
}

TEST_F(RedBlackTreeTest, Insert_RightLeftCase_RotatesCorrectly)
{
    // Insert 10, 30, 20 should trigger right-left rotation
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(30);
    tree.insert(20);

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
    EXPECT_EQ(root->getColor(), Color::Black);
    ASSERT_NE(root->getLeft(), nullptr);
    EXPECT_EQ(root->getLeft()->getData(), 10);
    EXPECT_EQ(root->getLeft()->getColor(), Color::Red);
    ASSERT_NE(root->getRight(), nullptr);
    EXPECT_EQ(root->getRight()->getData(), 30);
    EXPECT_EQ(root->getRight()->getColor(), Color::Red);
}

// ==================== Color Invariant Tests ====================

TEST_F(RedBlackTreeTest, RootRemainsBlack_AfterMultipleInsertions)
{
    RedBlackTree<int> tree;
    for (int i = 1; i <= 100; ++i)
    {
        tree.insert(i);
    }

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getColor(), Color::Black);
}

TEST_F(RedBlackTreeTest, NoConsecutiveRedNodes_AfterInsertion)
{
    RedBlackTree<int> tree;
    // Insert values that force various rebalancing scenarios
    const int values[] = {10, 20, 30, 40, 50, 5, 15, 25, 35, 45};
    for (const int v : values)
    {
        tree.insert(v);
    }

    // Helper lambda to check no red node has a red child
    const auto checkNoRedRed = [](const auto& self, const std::shared_ptr<RedBlackTreeNode<int>>& node) -> bool
    {
        if (!node) return true;
        if (node->getColor() == Color::Red)
        {
            if (node->getLeft() && node->getLeft()->getColor() == Color::Red) return false;
            if (node->getRight() && node->getRight()->getColor() == Color::Red) return false;
        }
        return self(self, node->getLeft()) && self(self, node->getRight());
    };

    EXPECT_TRUE(checkNoRedRed(checkNoRedRed, tree.getRoot()));
}

TEST_F(RedBlackTreeTest, BlackHeight_ConsistentAcrossPaths)
{
    RedBlackTree<int> tree;
    for (int i = 1; i <= 50; ++i)
    {
        tree.insert(i);
    }

    // Helper lambda to verify black-height consistency
    const auto checkBlackHeight = [](const auto& self, const std::shared_ptr<RedBlackTreeNode<int>>& node) -> std::pair<bool, int>
    {
        if (!node) return {true, 1};
        const auto [leftOk, leftBh] = self(self, node->getLeft());
        const auto [rightOk, rightBh] = self(self, node->getRight());
        if (!leftOk || !rightOk || leftBh != rightBh) return {false, 0};
        const int add = (node->getColor() == Color::Black) ? 1 : 0;
        return {true, leftBh + add};
    };

    const auto [ok, bh] = checkBlackHeight(checkBlackHeight, tree.getRoot());
    EXPECT_TRUE(ok);
    EXPECT_GT(bh, 0);
}

// ==================== Remove Tests ====================

TEST_F(RedBlackTreeTest, Remove_LeafNode_Removed)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    tree.remove(10);
    // Verify by in-order traversal: should only have 20
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
}

TEST_F(RedBlackTreeTest, Remove_NonExisting_NoCrash)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.remove(999);
    EXPECT_NE(tree.getRoot(), nullptr);
}

TEST_F(RedBlackTreeTest, Remove_FromEmptyTree_NoCrash)
{
    RedBlackTree<int> tree;
    EXPECT_NO_THROW(tree.remove(10));
    EXPECT_EQ(tree.getRoot(), nullptr);
}

TEST_F(RedBlackTreeTest, Remove_AllElements_TreeEmpty)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    tree.remove(10);
    tree.remove(20);
    tree.remove(30);
    EXPECT_EQ(tree.getRoot(), nullptr);
}

TEST_F(RedBlackTreeTest, Remove_RootWithSingleLeftChild_Works)
{
    RedBlackTree<int> tree;
    tree.insert(20);
    tree.insert(10);

    tree.remove(20);
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 10);
    EXPECT_EQ(root->getColor(), Color::Black);
}

TEST_F(RedBlackTreeTest, Remove_RootWithSingleRightChild_Works)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    tree.remove(10);
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getData(), 20);
    EXPECT_EQ(root->getColor(), Color::Black);
}

TEST_F(RedBlackTreeTest, Remove_NodeWithTwoChildren_Works)
{
    RedBlackTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);

    tree.remove(30);
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getColor(), Color::Black);

    // Verify all remaining values are reachable via in-order traversal
    std::vector<int> values;
    const auto collect = [](const auto& self, const std::shared_ptr<RedBlackTreeNode<int>>& node,
                            std::vector<int>& out) -> void
    {
        if (!node) return;
        self(self, node->getLeft(), out);
        out.push_back(node->getData());
        self(self, node->getRight(), out);
    };
    collect(collect, root, values);

    const std::vector<int> expected{20, 40, 50, 70};
    EXPECT_EQ(values, expected);
}

TEST_F(RedBlackTreeTest, Remove_LargeDataset_RemainingNodesReachable)
{
    RedBlackTree<int> tree;
    constexpr int kCount = 100;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(i);
    }

    // Remove half
    for (int i = 0; i < kCount; i += 2)
    {
        tree.remove(i);
    }

    // Verify root remains black
    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getColor(), Color::Black);

    // Verify remaining nodes are reachable
    std::vector<int> values;
    const auto collect = [](const auto& self, const std::shared_ptr<RedBlackTreeNode<int>>& node,
                            std::vector<int>& out) -> void
    {
        if (!node) return;
        self(self, node->getLeft(), out);
        out.push_back(node->getData());
        self(self, node->getRight(), out);
    };
    collect(collect, root, values);

    // Should contain all odd numbers from 1 to 99
    ASSERT_EQ(values.size(), kCount / 2);
    for (size_t i = 0; i < values.size(); ++i)
    {
        EXPECT_EQ(values[i], static_cast<int>(i) * 2 + 1);
    }
}

// ==================== Large Dataset ====================

TEST_F(RedBlackTreeTest, Insert_LargeDataset_AllNodesReachable)
{
    RedBlackTree<int> tree;
    constexpr int kCount = 1000;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(i);
    }

    const auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getColor(), Color::Black);

    // Verify all values are reachable via in-order traversal
    std::vector<int> values;
    const auto collect = [](const auto& self, const std::shared_ptr<RedBlackTreeNode<int>>& node,
                            std::vector<int>& out) -> void
    {
        if (!node) return;
        self(self, node->getLeft(), out);
        out.push_back(node->getData());
        self(self, node->getRight(), out);
    };
    collect(collect, root, values);

    ASSERT_EQ(values.size(), kCount);
    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(values[i], i);
    }
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST_F(RedBlackTreeTest, MoveConstructor)
{
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    auto root = tree.getRoot();
    ASSERT_NE(root, nullptr);

    RedBlackTree<int> other(std::move(tree));
    ASSERT_NE(other.getRoot(), nullptr);
    EXPECT_EQ(tree.getRoot(), nullptr);
}

TEST_F(RedBlackTreeTest, MoveAssignment)
{
    RedBlackTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    ASSERT_NE(tree.getRoot(), nullptr);

    RedBlackTree<int> other;
    other.insert(99);
    other = std::move(tree);
    ASSERT_NE(other.getRoot(), nullptr);
    EXPECT_EQ(tree.getRoot(), nullptr);
}
