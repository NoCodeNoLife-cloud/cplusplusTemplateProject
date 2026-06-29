/**
 * @file BinaryTreeTest.cc
 * @brief Unit tests for the BinaryTree class
 * @details Tests cover construction, accessors, traversal orders,
 *          tree properties, and edge cases.
 */

#include <memory>
#include <gtest/gtest.h>

#include <cppforge/data_structure/tree/core/BinaryTree.hpp>

using namespace cppforge::data_structure::tree::core;

/**
 * @brief Test fixture for BinaryTree tests
 */
class BinaryTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Build tree:
        //         1
        //        / \
        //       2   3
        //      / \
        //     4   5
        auto n4 = std::make_unique<BinaryTree<int>>(4);
        auto n5 = std::make_unique<BinaryTree<int>>(5);
        auto n2 = std::make_unique<BinaryTree<int>>(2);
        n2->setLeft(std::move(n4));
        n2->setRight(std::move(n5));
        auto n3 = std::make_unique<BinaryTree<int>>(3);
        root_ = std::make_unique<BinaryTree<int>>(1);
        root_->setLeft(std::move(n2));
        root_->setRight(std::move(n3));
    }

    void TearDown() override
    {
    }

    std::unique_ptr<BinaryTree<int>> root_{};
};

// ==================== Construction ====================

/**
 * @brief Test default constructor creates an empty node
 * @details Verifies that a default-constructed BinaryTree node is a leaf with size 1 and height 0
 */
TEST_F(BinaryTreeTest, DefaultConstructor_CreatesEmptyNode)
{
    const BinaryTree<int> node;
    EXPECT_TRUE(node.isLeaf());
    EXPECT_EQ(node.size(), 1);
    EXPECT_EQ(node.height(), 0);
}

/**
 * @brief Test value constructor sets node data
 * @details Verifies that constructing a node with a value sets data correctly
 */
TEST_F(BinaryTreeTest, ValueConstructor_SetsData)
{
    const BinaryTree<int> node(42);
    EXPECT_EQ(node.data(), 42);
}

/**
 * @brief Test value constructor allows mutable access
 * @details Verifies that data() returns a mutable reference that can be modified
 */
TEST_F(BinaryTreeTest, ValueConstructor_MutableAccess)
{
    BinaryTree<int> node(42);
    node.data() = 100;
    EXPECT_EQ(node.data(), 100);
}

// ==================== Accessors ====================

/**
 * @brief Test data accessor returns correct value
 * @details Verifies that data() returns the value stored at the root node
 */
TEST_F(BinaryTreeTest, Data_ReturnsCorrectValue)
{
    EXPECT_EQ(root_->data(), 1);
}

/**
 * @brief Test left child accessor
 * @details Verifies that left() returns the left child node
 */
TEST_F(BinaryTreeTest, Left_ReturnsLeftChild)
{
    ASSERT_NE(root_->left(), nullptr);
    EXPECT_EQ(root_->left()->data(), 2);
}

/**
 * @brief Test right child accessor
 * @details Verifies that right() returns the right child node
 */
TEST_F(BinaryTreeTest, Right_ReturnsRightChild)
{
    ASSERT_NE(root_->right(), nullptr);
    EXPECT_EQ(root_->right()->data(), 3);
}

/**
 * @brief Test left child is null for leaf node
 * @details Verifies that a leaf node has no left child
 */
TEST_F(BinaryTreeTest, Left_NullForLeaf)
{
    const auto leaf = std::make_unique<BinaryTree<int>>(99);
    EXPECT_EQ(leaf->left(), nullptr);
}

/**
 * @brief Test right child is null for leaf node
 * @details Verifies that a leaf node has no right child
 */
TEST_F(BinaryTreeTest, Right_NullForLeaf)
{
    const auto leaf = std::make_unique<BinaryTree<int>>(99);
    EXPECT_EQ(leaf->right(), nullptr);
}

// ==================== Traversals ====================

/**
 * @brief Test preorder traversal
 * @details Verifies that preorder returns nodes in root-left-right order
 */
TEST_F(BinaryTreeTest, Preorder_ReturnsCorrectOrder)
{
    const auto result = root_->preorder();
    const std::vector<int> expected{1, 2, 4, 5, 3};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test inorder traversal
 * @details Verifies that inorder returns nodes in left-root-right order
 */
TEST_F(BinaryTreeTest, Inorder_ReturnsCorrectOrder)
{
    const auto result = root_->inorder();
    const std::vector<int> expected{4, 2, 5, 1, 3};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test postorder traversal
 * @details Verifies that postorder returns nodes in left-right-root order
 */
TEST_F(BinaryTreeTest, Postorder_ReturnsCorrectOrder)
{
    const auto result = root_->postorder();
    const std::vector<int> expected{4, 5, 2, 3, 1};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test level-order traversal
 * @details Verifies that levelOrder returns nodes level by level
 */
TEST_F(BinaryTreeTest, LevelOrder_ReturnsCorrectOrder)
{
    const auto result = root_->levelOrder();
    const std::vector<int> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test preorder on a single node
 * @details Verifies that preorder traversal of a single node returns just that node
 */
TEST_F(BinaryTreeTest, Preorder_SingleNode_ReturnsSingle)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    const std::vector<int> expected{42};
    EXPECT_EQ(single->preorder(), expected);
}

/**
 * @brief Test inorder on a single node
 * @details Verifies that inorder traversal of a single node returns just that node
 */
TEST_F(BinaryTreeTest, Inorder_SingleNode_ReturnsSingle)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    const std::vector<int> expected{42};
    EXPECT_EQ(single->inorder(), expected);
}

/**
 * @brief Test postorder on a single node
 * @details Verifies that postorder traversal of a single node returns just that node
 */
TEST_F(BinaryTreeTest, Postorder_SingleNode_ReturnsSingle)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    const std::vector<int> expected{42};
    EXPECT_EQ(single->postorder(), expected);
}

/**
 * @brief Test level-order on a single node
 * @details Verifies that level-order traversal of a single node returns just that node
 */
TEST_F(BinaryTreeTest, LevelOrder_SingleNode_ReturnsSingle)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    const std::vector<int> expected{42};
    EXPECT_EQ(single->levelOrder(), expected);
}

// ==================== Tree Properties ====================

/**
 * @brief Test isLeaf on root with children
 * @details Verifies that the root of a multi-node tree is not a leaf
 */
TEST_F(BinaryTreeTest, IsLeaf_RootWithChildren_ReturnsFalse)
{
    EXPECT_FALSE(root_->isLeaf());
}

/**
 * @brief Test isLeaf on a leaf node
 * @details Verifies that a node with no children is identified as a leaf
 */
TEST_F(BinaryTreeTest, IsLeaf_LeafNode_ReturnsTrue)
{
    ASSERT_NE(root_->left()->left(), nullptr);
    EXPECT_TRUE(root_->left()->left()->isLeaf());
}

/**
 * @brief Test height returns correct value
 * @details Verifies that tree height (number of edges on longest path) is correct
 */
TEST_F(BinaryTreeTest, Height_ReturnsCorrectHeight)
{
    // Tree height (edges): 1->2->4 = 2
    EXPECT_EQ(root_->height(), 2);
}

/**
 * @brief Test height of a single node
 * @details Verifies that a single node has height 0
 */
TEST_F(BinaryTreeTest, Height_SingleNode_ReturnsZero)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    EXPECT_EQ(single->height(), 0);
}

/**
 * @brief Test size returns correct count
 * @details Verifies that size returns the total number of nodes in the tree
 */
TEST_F(BinaryTreeTest, Size_ReturnsCorrectCount)
{
    EXPECT_EQ(root_->size(), 5);
}

/**
 * @brief Test size of a single node
 * @details Verifies that a single node has size 1
 */
TEST_F(BinaryTreeTest, Size_SingleNode_ReturnsOne)
{
    const auto single = std::make_unique<BinaryTree<int>>(42);
    EXPECT_EQ(single->size(), 1);
}

// ==================== Clear ====================

/**
 * @brief Test clear removes children
 * @details Verifies that clear() removes all child pointers from a node
 */
TEST_F(BinaryTreeTest, Clear_RemovesChildren)
{
    ASSERT_NE(root_->left(), nullptr);
    root_->clear();
    EXPECT_EQ(root_->left(), nullptr);
    EXPECT_EQ(root_->right(), nullptr);
    EXPECT_TRUE(root_->isLeaf());
}

/**
 * @brief Test clear does not affect node data
 * @details Verifies that clear() removes children but preserves the node's own data
 */
TEST_F(BinaryTreeTest, Clear_DoesNotAffectData)
{
    root_->clear();
    EXPECT_EQ(root_->data(), 1);
}

// ==================== Set Children ====================

/**
 * @brief Test setLeft takes ownership
 * @details Verifies that setLeft transfers ownership of the child node
 */
TEST_F(BinaryTreeTest, SetLeft_TakesOwnership)
{
    auto child = std::make_unique<BinaryTree<int>>(99);
    auto* child_ptr = child.get();
    root_->setLeft(std::move(child));
    EXPECT_EQ(root_->left(), child_ptr);
    EXPECT_EQ(root_->left()->data(), 99);
}

/**
 * @brief Test setRight takes ownership
 * @details Verifies that setRight transfers ownership of the child node
 */
TEST_F(BinaryTreeTest, SetRight_TakesOwnership)
{
    auto child = std::make_unique<BinaryTree<int>>(88);
    auto* child_ptr = child.get();
    root_->setRight(std::move(child));
    EXPECT_EQ(root_->right(), child_ptr);
    EXPECT_EQ(root_->right()->data(), 88);
}

// ==================== Edge Cases ====================

/**
 * @brief Test traversals of a right-skewed tree
 * @details Verifies that all traversal orders work correctly on a degenerate
 *          tree with only right children
 */
TEST_F(BinaryTreeTest, RightSkewedTree_Traversals)
{
    // 1 -> 2 -> 3 (only right children)
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setRight(std::make_unique<BinaryTree<int>>(2));
    root->right()->setRight(std::make_unique<BinaryTree<int>>(3));

    EXPECT_EQ(root->preorder(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(root->inorder(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(root->postorder(), (std::vector<int>{3, 2, 1}));
    EXPECT_EQ(root->levelOrder(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(root->height(), 2);
    EXPECT_EQ(root->size(), 3);
}

/**
 * @brief Test traversals of a left-skewed tree
 * @details Verifies that all traversal orders work correctly on a degenerate
 *          tree with only left children
 */
TEST_F(BinaryTreeTest, LeftSkewedTree_Traversals)
{
    // 3 -> 2 -> 1 (only left children)
    const auto root = std::make_unique<BinaryTree<int>>(3);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(1));

    EXPECT_EQ(root->preorder(), (std::vector<int>{3, 2, 1}));
    EXPECT_EQ(root->inorder(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(root->postorder(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(root->levelOrder(), (std::vector<int>{3, 2, 1}));
    EXPECT_EQ(root->height(), 2);
    EXPECT_EQ(root->size(), 3);
}

/**
 * @brief Test clear on a single node does not crash
 * @details Verifies that clearing a leaf node is safe
 */
TEST_F(BinaryTreeTest, Clear_SingleNode_NoCrash)
{
    const auto node = std::make_unique<BinaryTree<int>>(42);
    EXPECT_NO_THROW(node->clear());
    EXPECT_TRUE(node->isLeaf());
}

/**
 * @brief Test clear called twice does not crash
 * @details Verifies that calling clear() multiple times is safe
 */
TEST_F(BinaryTreeTest, Clear_Twice_NoCrash)
{
    ASSERT_NE(root_->left(), nullptr);
    root_->clear();
    root_->clear();
    EXPECT_EQ(root_->left(), nullptr);
    EXPECT_EQ(root_->right(), nullptr);
}

/**
 * @brief Test setLeft replaces existing child
 * @details Verifies that setting a new left child replaces the old one
 */
TEST_F(BinaryTreeTest, SetLeft_ReplacesExistingChild)
{
    auto old_child = std::make_unique<BinaryTree<int>>(1);
    root_->setLeft(std::move(old_child));
    ASSERT_NE(root_->left(), nullptr);
    EXPECT_EQ(root_->left()->data(), 1);

    auto new_child = std::make_unique<BinaryTree<int>>(99);
    root_->setLeft(std::move(new_child));
    EXPECT_EQ(root_->left()->data(), 99);
}

/**
 * @brief Test setRight replaces existing child
 * @details Verifies that setting a new right child replaces the old one
 */
TEST_F(BinaryTreeTest, SetRight_ReplacesExistingChild)
{
    auto old_child = std::make_unique<BinaryTree<int>>(1);
    root_->setRight(std::move(old_child));
    ASSERT_NE(root_->right(), nullptr);
    EXPECT_EQ(root_->right()->data(), 1);

    auto new_child = std::make_unique<BinaryTree<int>>(88);
    root_->setRight(std::move(new_child));
    EXPECT_EQ(root_->right()->data(), 88);
}

/**
 * @brief Test level-order on a right-skewed tree
 * @details Verifies level-order traversal for a degenerate right-skewed tree
 */
TEST_F(BinaryTreeTest, LevelOrder_RightSkewed)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setRight(std::make_unique<BinaryTree<int>>(2));
    root->right()->setRight(std::make_unique<BinaryTree<int>>(3));
    EXPECT_EQ(root->levelOrder(), (std::vector<int>{1, 2, 3}));
}

/**
 * @brief Test level-order on a left-skewed tree
 * @details Verifies level-order traversal for a degenerate left-skewed tree
 */
TEST_F(BinaryTreeTest, LevelOrder_LeftSkewed)
{
    const auto root = std::make_unique<BinaryTree<int>>(3);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(1));
    EXPECT_EQ(root->levelOrder(), (std::vector<int>{3, 2, 1}));
}

/**
 * @brief Test BinaryTree with string type
 * @details Verifies that BinaryTree works correctly with std::string data
 */
TEST_F(BinaryTreeTest, StringType_StoresCorrectly)
{
    const auto root = std::make_unique<BinaryTree<std::string>>("hello");
    root->setLeft(std::make_unique<BinaryTree<std::string>>("world"));
    root->setRight(std::make_unique<BinaryTree<std::string>>("foo"));

    const auto pre = root->preorder();
    const std::vector<std::string> expected{"hello", "world", "foo"};
    EXPECT_EQ(pre, expected);
    EXPECT_EQ(root->size(), 3);
}

/**
 * @brief Test BinaryTree with double type
 * @details Verifies that BinaryTree works correctly with double-precision data
 */
TEST_F(BinaryTreeTest, DoubleType_StoresCorrectly)
{
    const auto root = std::make_unique<BinaryTree<double>>(3.14);
    root->setLeft(std::make_unique<BinaryTree<double>>(2.71));

    EXPECT_DOUBLE_EQ(root->data(), 3.14);
    EXPECT_DOUBLE_EQ(root->left()->data(), 2.71);
    EXPECT_EQ(root->size(), 2);
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

/**
 * @brief Test move constructor
 * @details Verifies that the move constructor transfers data and children
 *          to the new tree node
 */
TEST_F(BinaryTreeTest, MoveConstructor)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(20));

    BinaryTree<int> other(std::move(*root));
    EXPECT_EQ(other.data(), 10);
    EXPECT_EQ(other.left()->data(), 20);
    EXPECT_EQ(other.size(), 2);
}

/**
 * @brief Test move assignment
 * @details Verifies that move assignment transfers data and children
 *          to the destination tree node
 */
TEST_F(BinaryTreeTest, MoveAssignment)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(20));

    const auto other = std::make_unique<BinaryTree<int>>(0);
    *other = std::move(*root);
    EXPECT_EQ(other->data(), 10);
    EXPECT_EQ(other->left()->data(), 20);
    EXPECT_EQ(other->size(), 2);
}
