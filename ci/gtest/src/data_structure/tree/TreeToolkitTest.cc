/**
 * @file TreeToolkitTest.cc
 * @brief Unit tests for the TreeToolkit utility class
 * @details Tests cover height and size computation for various
 *          tree shapes (empty, single node, balanced, skewed, zigzag),
 *          template flexibility with custom node types, and consistency
 *          invariants (height along longest path, size = sum of subtree sizes + 1).
 */

#include <memory>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/tree/TreeToolkit.hpp"
#include "data_structure/tree/BinaryTree.hpp"

using namespace common::data_structure::tree;

class TreeToolkitTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ============================================================================
//  Height Tests
// ============================================================================

TEST_F(TreeToolkitTest, Height_NullTree_ReturnsNegativeOne)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
}

TEST_F(TreeToolkitTest, Height_SingleNode_ReturnsZero)
{
    const BinaryTree<int> node(10);
    EXPECT_EQ(TreeToolkit::height(&node), 0);
}

TEST_F(TreeToolkitTest, Height_TwoLevelsLeft_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

TEST_F(TreeToolkitTest, Height_TwoLevelsRight_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

TEST_F(TreeToolkitTest, Height_TwoLevelsBoth_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

TEST_F(TreeToolkitTest, Height_LeftChain_ThreeLevels)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(3));
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

TEST_F(TreeToolkitTest, Height_RightChain_ThreeLevels)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    root->right()->setRight(std::make_unique<BinaryTree<int>>(20));
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

TEST_F(TreeToolkitTest, Height_LeftSkewed_Depth5)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    auto* node = root.get();
    for (int i = 0; i < 5; ++i)
    {
        auto child = std::make_unique<BinaryTree<int>>(i);
        node->setLeft(std::move(child));
        node = node->left();
    }
    EXPECT_EQ(TreeToolkit::height(root.get()), 5);
}

TEST_F(TreeToolkitTest, Height_RightSkewed_Depth5)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    auto* node = root.get();
    for (int i = 0; i < 5; ++i)
    {
        auto child = std::make_unique<BinaryTree<int>>(i);
        node->setRight(std::move(child));
        node = node->right();
    }
    EXPECT_EQ(TreeToolkit::height(root.get()), 5);
}

TEST_F(TreeToolkitTest, Height_Balanced_TwoLevels)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(4));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(5));
    root->right()->setLeft(std::make_unique<BinaryTree<int>>(6));
    /*
     *        1
     *       / \
     *      2   3
     *     / \  /
     *    4   5 6
     * height = 2
     */
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

TEST_F(TreeToolkitTest, Height_Asymmetric_LeftDeeper)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(4));
    root->left()->left()->setLeft(std::make_unique<BinaryTree<int>>(5));
    /*
     *        1
     *       / \
     *      2   3
     *     /
     *    4
     *   /
     *  5
     * height = 3  (left subtree height = 2, right = 0)
     */
    EXPECT_EQ(TreeToolkit::height(root.get()), 3);
}

TEST_F(TreeToolkitTest, Height_Zigzag_PicksLongest)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->right()->setLeft(std::make_unique<BinaryTree<int>>(4));
    /*
     *    1
     *   /
     *  2
     *   \
     *    3
     *   /
     *  4
     * height = 3
     */
    EXPECT_EQ(TreeToolkit::height(root.get()), 3);
}

TEST_F(TreeToolkitTest, Height_OnlyLeftGrandchild)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(3));
    root->left()->left()->setLeft(std::make_unique<BinaryTree<int>>(4));
    /*
     *      1
     *     /
     *    2
     *   /
     *  3
     * /
     *4
     * height = 3
     */
    EXPECT_EQ(TreeToolkit::height(root.get()), 3);
}

TEST_F(TreeToolkitTest, Height_TerminatesAtLeaf)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(3));
    // leaf 3 has no children
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

// ============================================================================
//  Size Tests
// ============================================================================

TEST_F(TreeToolkitTest, Size_NullTree_ReturnsZero)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

TEST_F(TreeToolkitTest, Size_SingleNode_ReturnsOne)
{
    const BinaryTree<int> node(10);
    EXPECT_EQ(TreeToolkit::size(&node), 1);
}

TEST_F(TreeToolkitTest, Size_TwoNodes_LeftChild)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    EXPECT_EQ(TreeToolkit::size(root.get()), 2);
}

TEST_F(TreeToolkitTest, Size_TwoNodes_RightChild)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::size(root.get()), 2);
}

TEST_F(TreeToolkitTest, Size_ThreeNodes_BothChildren)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::size(root.get()), 3);
}

TEST_F(TreeToolkitTest, Size_RightChain)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    auto* node = root.get();
    for (int i = 0; i < 10; ++i)
    {
        auto child = std::make_unique<BinaryTree<int>>(i);
        node->setRight(std::move(child));
        node = node->right();
    }
    EXPECT_EQ(TreeToolkit::size(root.get()), 11);
}

TEST_F(TreeToolkitTest, Size_FullBinaryTree_SevenNodes)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(4));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(5));
    root->right()->setLeft(std::make_unique<BinaryTree<int>>(6));
    root->right()->setRight(std::make_unique<BinaryTree<int>>(7));
    /*
     *      1
     *     / \
     *    2   3
     *   / \ / \
     *  4  5 6  7
     * size = 7
     */
    EXPECT_EQ(TreeToolkit::size(root.get()), 7);
}

TEST_F(TreeToolkitTest, Size_IncompleteTree)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(4));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    /*
     *    1
     *   / \
     *  2   3
     *   \
     *    4
     * size = 4
     */
    EXPECT_EQ(TreeToolkit::size(root.get()), 4);
}

// ============================================================================
//  Combined Height + Size (same tree)
// ============================================================================

TEST_F(TreeToolkitTest, NullPtr_HeightNegativeOne_SizeZero)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

TEST_F(TreeToolkitTest, ComplexTree_HeightAndSize)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(4));
    root->left()->left()->setLeft(std::make_unique<BinaryTree<int>>(5));
    /*
     *        1
     *       / \
     *      2   3
     *     /
     *    4
     *   /
     *  5
     * height = 3, size = 5
     */
    EXPECT_EQ(TreeToolkit::height(root.get()), 3);
    EXPECT_EQ(TreeToolkit::size(root.get()), 5);
}

// ============================================================================
//  Template Flexibility — custom node type
// ============================================================================

namespace
{
    /// A minimal node type exposing left_ / right_ as unique_ptr members,
    /// used to verify that TreeToolkit works with non-BinaryTree node types.
    struct CustomNode
    {
        int data{};
        std::unique_ptr<CustomNode> left_;
        std::unique_ptr<CustomNode> right_;

        explicit CustomNode(int value) : data(value)
        {
        }
    };
}

TEST_F(TreeToolkitTest, CustomNodeType_Height)
{
    auto root = std::make_unique<CustomNode>(1);
    root->left_ = std::make_unique<CustomNode>(2);
    root->left_->left_ = std::make_unique<CustomNode>(3);
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

TEST_F(TreeToolkitTest, CustomNodeType_Size)
{
    auto root = std::make_unique<CustomNode>(1);
    root->left_ = std::make_unique<CustomNode>(2);
    root->right_ = std::make_unique<CustomNode>(3);
    root->left_->right_ = std::make_unique<CustomNode>(4);
    EXPECT_EQ(TreeToolkit::size(root.get()), 4);
}

TEST_F(TreeToolkitTest, CustomNodeType_NullHeight)
{
    const CustomNode* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
}

TEST_F(TreeToolkitTest, CustomNodeType_NullSize)
{
    const CustomNode* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

// ============================================================================
//  Consistency invariants
// ============================================================================

TEST_F(TreeToolkitTest, Invariant_SizeEqualsSubtreeSizesPlusOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->setRight(std::make_unique<BinaryTree<int>>(3));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(4));

    const size_t leftSize = TreeToolkit::size(root->left());
    const size_t rightSize = TreeToolkit::size(root->right());
    EXPECT_EQ(TreeToolkit::size(root.get()), 1 + leftSize + rightSize);
}

TEST_F(TreeToolkitTest, Invariant_HeightGeqZeroForNonNull)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(3));

    EXPECT_GE(TreeToolkit::height(root.get()), 0);
    EXPECT_GE(TreeToolkit::height(root->left()), 0);
    EXPECT_GE(TreeToolkit::height(root->left()->right()), 0);
}

TEST_F(TreeToolkitTest, Invariant_SingleNode_SizeOneHeightZero)
{
    const BinaryTree<int> node(42);
    EXPECT_EQ(TreeToolkit::size(&node), 1);
    EXPECT_EQ(TreeToolkit::height(&node), 0);
}
