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

/**
 * @brief Test height of a null tree
 * @details Verifies that height returns -1 for null pointer
 */
TEST_F(TreeToolkitTest, Height_NullTree_ReturnsNegativeOne)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
}

/**
 * @brief Test height of a single node
 * @details Verifies that height returns 0 for a single node
 */
TEST_F(TreeToolkitTest, Height_SingleNode_ReturnsZero)
{
    const BinaryTree<int> node(10);
    EXPECT_EQ(TreeToolkit::height(&node), 0);
}

/**
 * @brief Test height with two levels on the left
 * @details Verifies that height is 1 for a root with one left child
 */
TEST_F(TreeToolkitTest, Height_TwoLevelsLeft_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

/**
 * @brief Test height with two levels on the right
 * @details Verifies that height is 1 for a root with one right child
 */
TEST_F(TreeToolkitTest, Height_TwoLevelsRight_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

/**
 * @brief Test height with two levels on both sides
 * @details Verifies that height is 1 for a root with both children
 */
TEST_F(TreeToolkitTest, Height_TwoLevelsBoth_ReturnsOne)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::height(root.get()), 1);
}

/**
 * @brief Test height of a left chain with three levels
 * @details Verifies height is 2 for root->left->left chain
 */
TEST_F(TreeToolkitTest, Height_LeftChain_ThreeLevels)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->left()->setLeft(std::make_unique<BinaryTree<int>>(3));
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

/**
 * @brief Test height of a right chain with three levels
 * @details Verifies height is 2 for root->right->right chain
 */
TEST_F(TreeToolkitTest, Height_RightChain_ThreeLevels)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    root->right()->setRight(std::make_unique<BinaryTree<int>>(20));
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

/**
 * @brief Test height of a left-skewed tree of depth 5
 * @details Verifies that height correctly computes depth of a skewed left chain
 */
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

/**
 * @brief Test height of a right-skewed tree of depth 5
 * @details Verifies that height correctly computes depth of a skewed right chain
 */
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

/**
 * @brief Test height of a balanced two-level tree
 * @details Verifies height is 2 for a tree with two levels of children
 */
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

/**
 * @brief Test height on an asymmetric tree with left side deeper
 * @details Verifies that height picks the longest path (left subtree)
 */
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

/**
 * @brief Test height on a zigzag tree picks the longest path
 * @details Verifies that height follows the longest branch in a zigzag structure
 */
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

/**
 * @brief Test height with only a left grandchild chain
 * @details Verifies height is 3 for a chain of 4 nodes on the left
 */
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

/**
 * @brief Test that height terminates correctly at a leaf
 * @details Verifies that height does not count past leaf nodes
 */
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

/**
 * @brief Test size of a null tree
 * @details Verifies that size returns 0 for null pointer
 */
TEST_F(TreeToolkitTest, Size_NullTree_ReturnsZero)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

/**
 * @brief Test size of a single node
 * @details Verifies that size returns 1 for a single node
 */
TEST_F(TreeToolkitTest, Size_SingleNode_ReturnsOne)
{
    const BinaryTree<int> node(10);
    EXPECT_EQ(TreeToolkit::size(&node), 1);
}

/**
 * @brief Test size with a left child
 * @details Verifies size is 2 for root + left child
 */
TEST_F(TreeToolkitTest, Size_TwoNodes_LeftChild)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    EXPECT_EQ(TreeToolkit::size(root.get()), 2);
}

/**
 * @brief Test size with a right child
 * @details Verifies size is 2 for root + right child
 */
TEST_F(TreeToolkitTest, Size_TwoNodes_RightChild)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::size(root.get()), 2);
}

/**
 * @brief Test size with both children
 * @details Verifies size is 3 for root + both children
 */
TEST_F(TreeToolkitTest, Size_ThreeNodes_BothChildren)
{
    const auto root = std::make_unique<BinaryTree<int>>(10);
    root->setLeft(std::make_unique<BinaryTree<int>>(5));
    root->setRight(std::make_unique<BinaryTree<int>>(15));
    EXPECT_EQ(TreeToolkit::size(root.get()), 3);
}

/**
 * @brief Test size of a right chain of 11 nodes
 * @details Verifies that size correctly counts a deep right-skewed tree
 */
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

/**
 * @brief Test size of a full binary tree with 7 nodes
 * @details Verifies that size correctly counts a complete three-level tree
 */
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

/**
 * @brief Test size of an incomplete tree
 * @details Verifies correct size for a tree with missing children
 */
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

/**
 * @brief Test height and size on null pointer
 * @details Verifies that height returns -1 and size returns 0 for null
 */
TEST_F(TreeToolkitTest, NullPtr_HeightNegativeOne_SizeZero)
{
    const BinaryTree<int>* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

/**
 * @brief Test both height and size on a complex tree
 * @details Verifies that both metrics are computed correctly on the same tree
 */
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

/**
 * @brief Test height with a custom node type
 * @details Verifies that TreeToolkit works with non-BinaryTree node types
 */
TEST_F(TreeToolkitTest, CustomNodeType_Height)
{
    const auto root = std::make_unique<CustomNode>(1);
    root->left_ = std::make_unique<CustomNode>(2);
    root->left_->left_ = std::make_unique<CustomNode>(3);
    EXPECT_EQ(TreeToolkit::height(root.get()), 2);
}

/**
 * @brief Test size with a custom node type
 * @details Verifies that size calculation works for custom node types
 */
TEST_F(TreeToolkitTest, CustomNodeType_Size)
{
    const auto root = std::make_unique<CustomNode>(1);
    root->left_ = std::make_unique<CustomNode>(2);
    root->right_ = std::make_unique<CustomNode>(3);
    root->left_->right_ = std::make_unique<CustomNode>(4);
    EXPECT_EQ(TreeToolkit::size(root.get()), 4);
}

/**
 * @brief Test height of null custom node
 * @details Verifies that height returns -1 for null custom node
 */
TEST_F(TreeToolkitTest, CustomNodeType_NullHeight)
{
    const CustomNode* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::height(nullPtr), -1);
}

/**
 * @brief Test size of null custom node
 * @details Verifies that size returns 0 for null custom node
 */
TEST_F(TreeToolkitTest, CustomNodeType_NullSize)
{
    const CustomNode* nullPtr = nullptr;
    EXPECT_EQ(TreeToolkit::size(nullPtr), 0);
}

// ============================================================================
//  Consistency invariants
// ============================================================================

/**
 * @brief Test size invariant: size = 1 + leftSize + rightSize
 * @details Verifies the recursive size decomposition property
 */
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

/**
 * @brief Test that height is non-negative for non-null trees
 * @details Verifies the invariant that any non-null tree has height >= 0
 */
TEST_F(TreeToolkitTest, Invariant_HeightGeqZeroForNonNull)
{
    const auto root = std::make_unique<BinaryTree<int>>(1);
    root->setLeft(std::make_unique<BinaryTree<int>>(2));
    root->left()->setRight(std::make_unique<BinaryTree<int>>(3));

    EXPECT_GE(TreeToolkit::height(root.get()), 0);
    EXPECT_GE(TreeToolkit::height(root->left()), 0);
    EXPECT_GE(TreeToolkit::height(root->left()->right()), 0);
}

/**
 * @brief Test that a single node has size 1 and height 0
 * @details Verifies the base case invariants for a leaf node
 */
TEST_F(TreeToolkitTest, Invariant_SingleNode_SizeOneHeightZero)
{
    const BinaryTree<int> node(42);
    EXPECT_EQ(TreeToolkit::size(&node), 1);
    EXPECT_EQ(TreeToolkit::height(&node), 0);
}
