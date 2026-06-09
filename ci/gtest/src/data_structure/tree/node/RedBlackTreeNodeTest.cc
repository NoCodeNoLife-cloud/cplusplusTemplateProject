/**
 * @file RedBlackTreeNodeTest.cc
 * @brief Unit tests for the RedBlackTreeNode class
 * @details Tests cover construction, getters/setters, color management,
 *          and parent/child linkage for red-black tree nodes.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/node/RedBlackTreeNode.hpp"
#include "data_structure/base_type/Color.hpp"

using namespace common::data_structure;
using namespace common::data_structure::tree::node;

class RBTreeNodeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Constructor Tests ====================

TEST_F(RBTreeNodeTest, Constructor_DefaultColorIsRed)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getColor(), Color::Red);
}

TEST_F(RBTreeNodeTest, Constructor_DataIsSet)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getData(), 42);
}

TEST_F(RBTreeNodeTest, Constructor_ChildrenAreNull)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getLeft(), nullptr);
    EXPECT_EQ(node.getRight(), nullptr);
    EXPECT_EQ(node.getParent(), nullptr);
}

// ==================== GetData / SetData Tests ====================

TEST_F(RBTreeNodeTest, SetData_UpdatesValue)
{
    RedBlackTreeNode<int> node(10);
    node.setData(100);
    EXPECT_EQ(node.getData(), 100);
}

TEST_F(RBTreeNodeTest, SetData_StringType)
{
    RedBlackTreeNode<std::string> node("hello");
    EXPECT_EQ(node.getData(), "hello");
    node.setData("world");
    EXPECT_EQ(node.getData(), "world");
}

TEST_F(RBTreeNodeTest, GetData_ConstCorrectness)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getData(), 42);
}

// ==================== Color Tests ====================

TEST_F(RBTreeNodeTest, SetColor_Red)
{
    RedBlackTreeNode<int> node(0);
    node.setColor(Color::Red);
    EXPECT_EQ(node.getColor(), Color::Red);
}

TEST_F(RBTreeNodeTest, SetColor_Black)
{
    RedBlackTreeNode<int> node(0);
    node.setColor(Color::Black);
    EXPECT_EQ(node.getColor(), Color::Black);
}

TEST_F(RBTreeNodeTest, SetColor_Toggle)
{
    RedBlackTreeNode<int> node(0);
    node.setColor(Color::Red);
    EXPECT_EQ(node.getColor(), Color::Red);
    node.setColor(Color::Black);
    EXPECT_EQ(node.getColor(), Color::Black);
    node.setColor(Color::Red);
    EXPECT_EQ(node.getColor(), Color::Red);
}

// ==================== Left Child Tests ====================

TEST_F(RBTreeNodeTest, SetLeft_SetsChild)
{
    RedBlackTreeNode<int> parent(10);
    auto child = std::make_shared<RedBlackTreeNode<int>>(5);
    parent.setLeft(child);
    EXPECT_EQ(parent.getLeft(), child);
    EXPECT_EQ(parent.getLeft()->getData(), 5);
}

TEST_F(RBTreeNodeTest, SetLeft_Nullptr)
{
    RedBlackTreeNode<int> parent(10);
    parent.setLeft(nullptr);
    EXPECT_EQ(parent.getLeft(), nullptr);
}

TEST_F(RBTreeNodeTest, SetLeft_ReplaceChild)
{
    RedBlackTreeNode<int> parent(10);
    parent.setLeft(std::make_shared<RedBlackTreeNode<int>>(5));
    auto newChild = std::make_shared<RedBlackTreeNode<int>>(3);
    parent.setLeft(newChild);
    EXPECT_EQ(parent.getLeft(), newChild);
    EXPECT_EQ(parent.getLeft()->getData(), 3);
}

// ==================== Right Child Tests ====================

TEST_F(RBTreeNodeTest, SetRight_SetsChild)
{
    RedBlackTreeNode<int> parent(10);
    auto child = std::make_shared<RedBlackTreeNode<int>>(15);
    parent.setRight(child);
    EXPECT_EQ(parent.getRight(), child);
    EXPECT_EQ(parent.getRight()->getData(), 15);
}

TEST_F(RBTreeNodeTest, SetRight_Nullptr)
{
    RedBlackTreeNode<int> parent(10);
    parent.setRight(nullptr);
    EXPECT_EQ(parent.getRight(), nullptr);
}

TEST_F(RBTreeNodeTest, SetRight_ReplaceChild)
{
    RedBlackTreeNode<int> parent(10);
    parent.setRight(std::make_shared<RedBlackTreeNode<int>>(15));
    auto newChild = std::make_shared<RedBlackTreeNode<int>>(20);
    parent.setRight(newChild);
    EXPECT_EQ(parent.getRight(), newChild);
    EXPECT_EQ(parent.getRight()->getData(), 20);
}

// ==================== Parent Tests ====================

TEST_F(RBTreeNodeTest, SetParent_SetsParent)
{
    RedBlackTreeNode<int> child(5);
    auto parent = std::make_shared<RedBlackTreeNode<int>>(10);
    child.setParent(parent);
    EXPECT_EQ(child.getParent(), parent);
    EXPECT_EQ(child.getParent()->getData(), 10);
}

TEST_F(RBTreeNodeTest, SetParent_Nullptr)
{
    RedBlackTreeNode<int> child(5);
    child.setParent(nullptr);
    EXPECT_EQ(child.getParent(), nullptr);
}

TEST_F(RBTreeNodeTest, SetParent_Update)
{
    RedBlackTreeNode<int> child(5);
    child.setParent(std::make_shared<RedBlackTreeNode<int>>(10));
    auto newParent = std::make_shared<RedBlackTreeNode<int>>(20);
    child.setParent(newParent);
    EXPECT_EQ(child.getParent(), newParent);
}

// ==================== Full Linkage Tests ====================

TEST_F(RBTreeNodeTest, FullLinkage_ConstructTree)
{
    auto root = std::make_shared<RedBlackTreeNode<int>>(10);
    auto left = std::make_shared<RedBlackTreeNode<int>>(5);
    auto right = std::make_shared<RedBlackTreeNode<int>>(15);

    root->setLeft(left);
    root->setRight(right);
    left->setParent(root);
    right->setParent(root);

    root->setColor(Color::Black);
    left->setColor(Color::Red);
    right->setColor(Color::Red);

    EXPECT_EQ(root->getData(), 10);
    EXPECT_EQ(root->getColor(), Color::Black);
    EXPECT_EQ(root->getLeft()->getData(), 5);
    EXPECT_EQ(root->getLeft()->getColor(), Color::Red);
    EXPECT_EQ(root->getLeft()->getParent(), root);
    EXPECT_EQ(root->getRight()->getData(), 15);
    EXPECT_EQ(root->getRight()->getColor(), Color::Red);
    EXPECT_EQ(root->getRight()->getParent(), root);
}

// ==================== Different Types ====================

TEST_F(RBTreeNodeTest, DoubleType)
{
    RedBlackTreeNode<double> node(3.14);
    EXPECT_DOUBLE_EQ(node.getData(), 3.14);
    node.setData(2.71);
    EXPECT_DOUBLE_EQ(node.getData(), 2.71);
}

TEST_F(RBTreeNodeTest, PointerType)
{
    int value = 42;
    RedBlackTreeNode<int*> node(&value);
    EXPECT_EQ(node.getData(), &value);
    EXPECT_EQ(*node.getData(), 42);
}
