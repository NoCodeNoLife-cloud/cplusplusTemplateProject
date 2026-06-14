/**
 * @file TreeNodeTest.cc
 * @brief Unit tests for TreeNode static utility methods
 * @details Tests cover findMin, findNode, height, and size methods
 *          with various tree shapes and configurations.
 */

#include <memory>
#include <gtest/gtest.h>

#include "data_structure/tree/node/TreeNode.hpp"

using namespace common::data_structure::tree;
using namespace common::data_structure::tree::node;

/**
 * @brief Test fixture for TreeNode tests
 */
class TreeNodeTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Build tree:
        //         10
        //        /  \
        //       5    15
        //      / \
        //     2   8
        auto n2 = std::make_unique<TreeNode<int>>(2);
        auto n8 = std::make_unique<TreeNode<int>>(8);
        auto n5 = std::make_unique<TreeNode<int>>(5);
        n5->left_ = std::move(n2);
        n5->right_ = std::move(n8);
        auto n15 = std::make_unique<TreeNode<int>>(15);
        root_ = std::make_unique<TreeNode<int>>(10);
        root_->left_ = std::move(n5);
        root_->right_ = std::move(n15);
    }

    void TearDown() override
    {
    }

    std::unique_ptr<TreeNode<int>> root_{};
};

// ==================== findMin ====================

/**
 * @brief Test findMin on a non-empty tree
 * @details Verifies that findMin returns the smallest value
 */
TEST_F(TreeNodeTest, FindMin_NonEmpty_ReturnsMinimum)
{
    const auto* minNode = TreeNode<int>::findMin(root_.get());
    ASSERT_NE(minNode, nullptr);
    EXPECT_EQ(minNode->data, 2);
}

/**
 * @brief Test findMin on a single node
 * @details Verifies that findMin returns the node itself
 */
TEST_F(TreeNodeTest, FindMin_SingleNode_ReturnsNode)
{
    const auto single = std::make_unique<TreeNode<int>>(42);
    const auto* minNode = TreeNode<int>::findMin(single.get());
    ASSERT_NE(minNode, nullptr);
    EXPECT_EQ(minNode->data, 42);
}

/**
 * @brief Test findMin on a right-skewed tree
 * @details Minimum is always the leftmost node
 */
TEST_F(TreeNodeTest, FindMin_RightSkewed_ReturnsRoot)
{
    // Tree: 1 -> 2 -> 3 (no left children)
    const auto root = std::make_unique<TreeNode<int>>(1);
    root->right_ = std::make_unique<TreeNode<int>>(2);
    root->right_->right_ = std::make_unique<TreeNode<int>>(3);

    const auto* minNode = TreeNode<int>::findMin(root.get());
    ASSERT_NE(minNode, nullptr);
    EXPECT_EQ(minNode->data, 1);
}

/**
 * @brief Test findMin on a left-skewed tree
 * @details Minimum is the leftmost leaf
 */
TEST_F(TreeNodeTest, FindMin_LeftSkewed_ReturnsLeftmost)
{
    // Tree: 3 -> 2 -> 1
    const auto root = std::make_unique<TreeNode<int>>(3);
    root->left_ = std::make_unique<TreeNode<int>>(2);
    root->left_->left_ = std::make_unique<TreeNode<int>>(1);

    const auto* minNode = TreeNode<int>::findMin(root.get());
    ASSERT_NE(minNode, nullptr);
    EXPECT_EQ(minNode->data, 1);

    auto* mutableMinNode = TreeNode<int>::findMin(root.get());
    ASSERT_NE(mutableMinNode, nullptr);
    EXPECT_EQ(mutableMinNode->data, 1);
}

/**
 * @brief Test findMin on a null pointer
 * @details Verifies that findMin returns nullptr
 */
TEST_F(TreeNodeTest, FindMin_Null_ReturnsNull)
{
    const auto* minNode = TreeNode<int>::findMin(static_cast<const TreeNode<int>*>(nullptr));
    EXPECT_EQ(minNode, nullptr);
}

// ==================== findNode (mutable overload) ====================

/**
 * @brief Test mutable findNode on an existing value
 * @details Verifies that findNode returns a modifiable pointer to the correct node
 */
TEST_F(TreeNodeTest, FindNode_Mutable_ExistingValue_ReturnsNode)
{
    auto* found = TreeNode<int>::findNode(root_.get(), 8);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->data, 8);
    // Verify modifiable
    found->data = 88;
    EXPECT_EQ(found->data, 88);
}

/**
 * @brief Test mutable findNode on a null root
 * @details Verifies that findNode returns nullptr for empty tree
 */
TEST_F(TreeNodeTest, FindNode_Mutable_Null_ReturnsNull)
{
    auto* found = TreeNode<int>::findNode(static_cast<TreeNode<int>*>(nullptr), 10);
    EXPECT_EQ(found, nullptr);
}

/**
 * @brief Test mutable findNode on a non-existing value
 * @details Verifies that findNode returns nullptr for missing value
 */
TEST_F(TreeNodeTest, FindNode_Mutable_NonExisting_ReturnsNull)
{
    auto* found = TreeNode<int>::findNode(root_.get(), 999);
    EXPECT_EQ(found, nullptr);
}

// ==================== findNode ====================

/**
 * @brief Test findNode with an existing value
 * @details Verifies that findNode locates the correct node
 */
TEST_F(TreeNodeTest, FindNode_ExistingValue_ReturnsNode)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), 8);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->data, 8);
}

/**
 * @brief Test findNode with root value
 * @details Verifies that findNode locates the root
 */
TEST_F(TreeNodeTest, FindNode_RootValue_ReturnsRoot)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), 10);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->data, 10);
}

/**
 * @brief Test findNode with a non-existing value
 * @details Verifies that findNode returns nullptr
 */
TEST_F(TreeNodeTest, FindNode_NonExistingValue_ReturnsNull)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), 999);
    EXPECT_EQ(found, nullptr);
}

/**
 * @brief Test findNode with null root
 * @details Verifies that findNode returns nullptr for empty tree
 */
TEST_F(TreeNodeTest, FindNode_Null_ReturnsNull)
{
    const auto* found = TreeNode<int>::findNode(static_cast<const TreeNode<int>*>(nullptr), 10);
    EXPECT_EQ(found, nullptr);
}

/**
 * @brief Test findNode with leaf value
 * @details Verifies that findNode works for leaf nodes
 */
TEST_F(TreeNodeTest, FindNode_LeafValue_ReturnsLeaf)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), 2);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->data, 2);
}

/**
 * @brief Test findNode with value less than any node
 * @details Verifies that findNode returns nullptr
 */
TEST_F(TreeNodeTest, FindNode_ValueLessThanAll_ReturnsNull)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), -5);
    EXPECT_EQ(found, nullptr);
}

/**
 * @brief Test findNode with value greater than any node
 * @details Verifies that findNode returns nullptr
 */
TEST_F(TreeNodeTest, FindNode_ValueGreaterThanAll_ReturnsNull)
{
    const auto* found = TreeNode<int>::findNode(root_.get(), 9999);
    EXPECT_EQ(found, nullptr);
}

/**
 * @brief Test findNode on left-skewed tree
 * @details Verifies correct node is found in a left-skewed tree
 */
TEST_F(TreeNodeTest, FindNode_LeftSkewed_FindsCorrectly)
{
    const auto root = std::make_unique<TreeNode<int>>(3);
    root->left_ = std::make_unique<TreeNode<int>>(2);
    root->left_->left_ = std::make_unique<TreeNode<int>>(1);

    EXPECT_NE(TreeNode<int>::findNode(root.get(), 1), nullptr);
    EXPECT_NE(TreeNode<int>::findNode(root.get(), 2), nullptr);
    EXPECT_NE(TreeNode<int>::findNode(root.get(), 3), nullptr);
    EXPECT_EQ(TreeNode<int>::findNode(root.get(), 4), nullptr);
}

// ==================== height ====================

/**
 * @brief Test height on a non-empty tree
 * @details For the tree: 10->5->2 (2 edges), right: 15 (1 edge), max = 2
 */
TEST_F(TreeNodeTest, Height_NonEmpty_ReturnsCorrectHeight)
{
    EXPECT_EQ(TreeNode<int>::height(root_.get()), 2);
}

/**
 * @brief Test height on a single node
 * @details A single node has height 0
 */
TEST_F(TreeNodeTest, Height_SingleNode_ReturnsZero)
{
    const auto single = std::make_unique<TreeNode<int>>(42);
    EXPECT_EQ(TreeNode<int>::height(single.get()), 0);
}

/**
 * @brief Test height on a null pointer
 * @details Null has height -1
 */
TEST_F(TreeNodeTest, Height_Null_ReturnsNegativeOne)
{
    EXPECT_EQ(TreeNode<int>::height(nullptr), -1);
}

/**
 * @brief Test height on a skewed tree
 * @details A chain of 4 nodes has height 3
 */
TEST_F(TreeNodeTest, Height_Skewed_ReturnsCorrectHeight)
{
    const auto root = std::make_unique<TreeNode<int>>(1);
    root->right_ = std::make_unique<TreeNode<int>>(2);
    root->right_->right_ = std::make_unique<TreeNode<int>>(3);
    root->right_->right_->right_ = std::make_unique<TreeNode<int>>(4);

    EXPECT_EQ(TreeNode<int>::height(root.get()), 3);
}

// ==================== size ====================

/**
 * @brief Test size on a non-empty tree
 * @details The test tree has 5 nodes: 10, 5, 15, 2, 8
 */
TEST_F(TreeNodeTest, Size_NonEmpty_ReturnsCorrectCount)
{
    EXPECT_EQ(TreeNode<int>::size(root_.get()), 5);
}

/**
 * @brief Test size on a single node
 * @details A single node has size 1
 */
TEST_F(TreeNodeTest, Size_SingleNode_ReturnsOne)
{
    const auto single = std::make_unique<TreeNode<int>>(42);
    EXPECT_EQ(TreeNode<int>::size(single.get()), 1);
}

/**
 * @brief Test size on a null pointer
 * @details Null has size 0
 */
TEST_F(TreeNodeTest, Size_Null_ReturnsZero)
{
    EXPECT_EQ(TreeNode<int>::size(nullptr), 0);
}

/**
 * @brief Test height on a left-skewed tree
 * @details A chain of 4 nodes on the left has height 3
 */
TEST_F(TreeNodeTest, Height_LeftSkewed_ReturnsCorrectHeight)
{
    const auto root = std::make_unique<TreeNode<int>>(4);
    root->left_ = std::make_unique<TreeNode<int>>(3);
    root->left_->left_ = std::make_unique<TreeNode<int>>(2);
    root->left_->left_->left_ = std::make_unique<TreeNode<int>>(1);

    EXPECT_EQ(TreeNode<int>::height(root.get()), 3);
}

/**
 * @brief Test size on a left-skewed tree
 * @details A chain of 4 nodes has size 4
 */
TEST_F(TreeNodeTest, Size_LeftSkewed_ReturnsCorrectCount)
{
    const auto root = std::make_unique<TreeNode<int>>(4);
    root->left_ = std::make_unique<TreeNode<int>>(3);
    root->left_->left_ = std::make_unique<TreeNode<int>>(2);
    root->left_->left_->left_ = std::make_unique<TreeNode<int>>(1);

    EXPECT_EQ(TreeNode<int>::size(root.get()), 4);
}

/**
 * @brief Test size on a skewed tree
 * @details A chain of 4 nodes has size 4
 */
TEST_F(TreeNodeTest, Size_Skewed_ReturnsCorrectCount)
{
    const auto root = std::make_unique<TreeNode<int>>(1);
    root->right_ = std::make_unique<TreeNode<int>>(2);
    root->right_->right_ = std::make_unique<TreeNode<int>>(3);

    EXPECT_EQ(TreeNode<int>::size(root.get()), 3);
}
