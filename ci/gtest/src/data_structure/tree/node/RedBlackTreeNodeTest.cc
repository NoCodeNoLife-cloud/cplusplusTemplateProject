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

/**
 * @brief Test default color of a new node is Red
 * @details Verifies that a newly constructed red-black tree node has Color::Red
 */
TEST_F(RBTreeNodeTest, Constructor_DefaultColorIsRed)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getColor(), Color::Red);
}

/**
 * @brief Test that constructor sets the data value
 * @details Verifies that getData returns the value passed to the constructor
 */
TEST_F(RBTreeNodeTest, Constructor_DataIsSet)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getData(), 42);
}

/**
 * @brief Test that children and parent are null initially
 * @details Verifies that left, right, and parent pointers are all null
 */
TEST_F(RBTreeNodeTest, Constructor_ChildrenAreNull)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getLeft(), nullptr);
    EXPECT_EQ(node.getRight(), nullptr);
    EXPECT_EQ(node.getParent(), nullptr);
}

// ==================== GetData / SetData Tests ====================

/**
 * @brief Test setData updates the stored value
 * @details Verifies that setData changes the value returned by getData
 */
TEST_F(RBTreeNodeTest, SetData_UpdatesValue)
{
    RedBlackTreeNode<int> node(10);
    node.setData(100);
    EXPECT_EQ(node.getData(), 100);
}

/**
 * @brief Test setData with string type
 * @details Verifies that setData works for non-integer types
 */
TEST_F(RBTreeNodeTest, SetData_StringType)
{
    RedBlackTreeNode<std::string> node("hello");
    EXPECT_EQ(node.getData(), "hello");
    node.setData("world");
    EXPECT_EQ(node.getData(), "world");
}

/**
 * @brief Test const correctness of getData
 * @details Verifies that getData can be called on const nodes
 */
TEST_F(RBTreeNodeTest, GetData_ConstCorrectness)
{
    const RedBlackTreeNode<int> node(42);
    EXPECT_EQ(node.getData(), 42);
}

// ==================== Color Tests ====================

/**
 * @brief Test setting color to Red
 * @details Verifies that setColor(Color::Red) works correctly
 */
TEST_F(RBTreeNodeTest, SetColor_Red)
{
    RedBlackTreeNode<int> node(0);
    node.setColor(Color::Red);
    EXPECT_EQ(node.getColor(), Color::Red);
}

/**
 * @brief Test setting color to Black
 * @details Verifies that setColor(Color::Black) works correctly
 */
TEST_F(RBTreeNodeTest, SetColor_Black)
{
    RedBlackTreeNode<int> node(0);
    node.setColor(Color::Black);
    EXPECT_EQ(node.getColor(), Color::Black);
}

/**
 * @brief Test toggling between colors
 * @details Verifies that color can be changed multiple times
 */
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

/**
 * @brief Test setting the left child
 * @details Verifies that setLeft correctly links a child node
 */
TEST_F(RBTreeNodeTest, SetLeft_SetsChild)
{
    RedBlackTreeNode<int> parent(10);
    const auto child = std::make_shared<RedBlackTreeNode<int>>(5);
    parent.setLeft(child);
    EXPECT_EQ(parent.getLeft(), child);
    EXPECT_EQ(parent.getLeft()->getData(), 5);
}

/**
 * @brief Test setting left child to nullptr
 * @details Verifies that setLeft(nullptr) correctly clears the child
 */
TEST_F(RBTreeNodeTest, SetLeft_Nullptr)
{
    RedBlackTreeNode<int> parent(10);
    parent.setLeft(nullptr);
    EXPECT_EQ(parent.getLeft(), nullptr);
}

/**
 * @brief Test replacing an existing left child
 * @details Verifies that setLeft correctly replaces an existing child
 */
TEST_F(RBTreeNodeTest, SetLeft_ReplaceChild)
{
    RedBlackTreeNode<int> parent(10);
    parent.setLeft(std::make_shared<RedBlackTreeNode<int>>(5));
    const auto newChild = std::make_shared<RedBlackTreeNode<int>>(3);
    parent.setLeft(newChild);
    EXPECT_EQ(parent.getLeft(), newChild);
    EXPECT_EQ(parent.getLeft()->getData(), 3);
}

// ==================== Right Child Tests ====================

/**
 * @brief Test setting the right child
 * @details Verifies that setRight correctly links a child node
 */
TEST_F(RBTreeNodeTest, SetRight_SetsChild)
{
    RedBlackTreeNode<int> parent(10);
    auto child = std::make_shared<RedBlackTreeNode<int>>(15);
    parent.setRight(child);
    EXPECT_EQ(parent.getRight(), child);
    EXPECT_EQ(parent.getRight()->getData(), 15);
}

/**
 * @brief Test setting right child to nullptr
 * @details Verifies that setRight(nullptr) correctly clears the child
 */
TEST_F(RBTreeNodeTest, SetRight_Nullptr)
{
    RedBlackTreeNode<int> parent(10);
    parent.setRight(nullptr);
    EXPECT_EQ(parent.getRight(), nullptr);
}

/**
 * @brief Test replacing an existing right child
 * @details Verifies that setRight correctly replaces an existing child
 */
TEST_F(RBTreeNodeTest, SetRight_ReplaceChild)
{
    RedBlackTreeNode<int> parent(10);
    parent.setRight(std::make_shared<RedBlackTreeNode<int>>(15));
    const auto newChild = std::make_shared<RedBlackTreeNode<int>>(20);
    parent.setRight(newChild);
    EXPECT_EQ(parent.getRight(), newChild);
    EXPECT_EQ(parent.getRight()->getData(), 20);
}

// ==================== Parent Tests ====================

/**
 * @brief Test setting the parent pointer
 * @details Verifies that setParent correctly links a parent to a child
 */
TEST_F(RBTreeNodeTest, SetParent_SetsParent)
{
    RedBlackTreeNode<int> child(5);
    const auto parent = std::make_shared<RedBlackTreeNode<int>>(10);
    child.setParent(parent);
    EXPECT_EQ(child.getParent(), parent);
    EXPECT_EQ(child.getParent()->getData(), 10);
}

/**
 * @brief Test setting parent to nullptr
 * @details Verifies that setParent(nullptr) correctly clears the parent
 */
TEST_F(RBTreeNodeTest, SetParent_Nullptr)
{
    RedBlackTreeNode<int> child(5);
    child.setParent(nullptr);
    EXPECT_EQ(child.getParent(), nullptr);
}

/**
 * @brief Test updating the parent pointer
 * @details Verifies that parent can be reassigned to a different node
 */
TEST_F(RBTreeNodeTest, SetParent_Update)
{
    RedBlackTreeNode<int> child(5);
    child.setParent(std::make_shared<RedBlackTreeNode<int>>(10));
    const auto newParent = std::make_shared<RedBlackTreeNode<int>>(20);
    child.setParent(newParent);
    EXPECT_EQ(child.getParent(), newParent);
}

// ==================== Full Linkage Tests ====================

/**
 * @brief Test constructing a fully linked red-black tree
 * @details Verifies that parent-child relationships and colors are correctly set
 */
TEST_F(RBTreeNodeTest, FullLinkage_ConstructTree)
{
    const auto root = std::make_shared<RedBlackTreeNode<int>>(10);
    const auto left = std::make_shared<RedBlackTreeNode<int>>(5);
    const auto right = std::make_shared<RedBlackTreeNode<int>>(15);

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

/**
 * @brief Test node with double type
 * @details Verifies that RedBlackTreeNode works with floating-point data
 */
TEST_F(RBTreeNodeTest, DoubleType)
{
    RedBlackTreeNode<double> node(3.14);
    EXPECT_DOUBLE_EQ(node.getData(), 3.14);
    node.setData(2.71);
    EXPECT_DOUBLE_EQ(node.getData(), 2.71);
}

/**
 * @brief Test node with pointer type
 * @details Verifies that RedBlackTreeNode works with pointer data types
 */
TEST_F(RBTreeNodeTest, PointerType)
{
    int value = 42;
    const RedBlackTreeNode<int*> node(&value);
    EXPECT_EQ(node.getData(), &value);
    EXPECT_EQ(*node.getData(), 42);
}
