#include <gtest/gtest.h>
#include <optional>

#include "src/data_structure/tree/AVLTree.hpp"
#include "src/data_structure/tree/BinarySearchTree.hpp"

using namespace common::data_structure::tree;

// ==================== AVLTree findValue Tests ====================

/**
 * @brief Test AVLTree findValue with existing value
 * @details Verifies that findValue returns the correct value when found
 */
TEST(AVLTreeTest, FindValue_ExistingValue_ReturnsValue) {
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    tree.insert(40);
    tree.insert(50);
    tree.insert(25);

    const auto result = tree.findValue(20);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 20);
}

/**
 * @brief Test AVLTree findValue with non-existing value
 * @details Verifies that findValue returns std::nullopt when value not found
 */
TEST(AVLTreeTest, FindValue_NonExistingValue_ReturnsNullopt) {
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    const auto result = tree.findValue(999);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test AVLTree findValue with empty tree
 * @details Verifies that findValue returns std::nullopt for empty tree
 */
TEST(AVLTreeTest, FindValue_EmptyTree_ReturnsNullopt) {
    const AVLTree<int> tree;

    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test AVLTree findValue with root node
 * @details Verifies that findValue works correctly for root node
 */
TEST(AVLTreeTest, FindValue_RootNode_ReturnsValue) {
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    const auto result = tree.findValue(50);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 50);
}

/**
 * @brief Test AVLTree findValue with leaf node
 * @details Verifies that findValue works correctly for leaf nodes
 */
TEST(AVLTreeTest, FindValue_LeafNode_ReturnsValue) {
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);

    const auto result = tree.findValue(20);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 20);
}

/**
 * @brief Test AVLTree findValue with string type
 * @details Verifies that findValue works with different types
 */
TEST(AVLTreeTest, FindValue_StringType_ReturnsValue) {
    AVLTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");

    const auto result = tree.findValue("banana");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "banana");
}

/**
 * @brief Test AVLTree findValue consistency with find
 * @details Verifies that findValue and find return consistent results
 */
TEST(AVLTreeTest, FindValue_ConsistencyWithFind) {
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    // Test existing value
    EXPECT_EQ(tree.find(20), tree.findValue(20).has_value());

    // Test non-existing value
    EXPECT_EQ(tree.find(999), tree.findValue(999).has_value());
}

// ==================== BinarySearchTree findValue Tests ====================

/**
 * @brief Test BinarySearchTree findValue with existing value
 * @details Verifies that findValue returns the correct value when found
 */
TEST(BinarySearchTreeTest, FindValue_ExistingValue_ReturnsValue) {
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);

    const auto result = tree.findValue(40);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 40);
}

/**
 * @brief Test BinarySearchTree findValue with non-existing value
 * @details Verifies that findValue returns std::nullopt when value not found
 */
TEST(BinarySearchTreeTest, FindValue_NonExistingValue_ReturnsNullopt) {
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    const auto result = tree.findValue(999);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test BinarySearchTree findValue with empty tree
 * @details Verifies that findValue returns std::nullopt for empty tree
 */
TEST(BinarySearchTreeTest, FindValue_EmptyTree_ReturnsNullopt) {
    const BinarySearchTree<int> tree;

    const auto result = tree.findValue(10);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test BinarySearchTree findValue with root node
 * @details Verifies that findValue works correctly for root node
 */
TEST(BinarySearchTreeTest, FindValue_RootNode_ReturnsValue) {
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(150);

    const auto result = tree.findValue(100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

/**
 * @brief Test BinarySearchTree findValue with left subtree
 * @details Verifies that findValue works correctly for nodes in left subtree
 */
TEST(BinarySearchTreeTest, FindValue_LeftSubtree_ReturnsValue) {
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(50);
    tree.insert(25);
    tree.insert(75);

    const auto result = tree.findValue(25);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 25);
}

/**
 * @brief Test BinarySearchTree findValue with right subtree
 * @details Verifies that findValue works correctly for nodes in right subtree
 */
TEST(BinarySearchTreeTest, FindValue_RightSubtree_ReturnsValue) {
    BinarySearchTree<int> tree;
    tree.insert(100);
    tree.insert(150);
    tree.insert(125);
    tree.insert(175);

    const auto result = tree.findValue(175);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 175);
}

/**
 * @brief Test BinarySearchTree findValue with double type
 * @details Verifies that findValue works with floating point types
 */
TEST(BinarySearchTreeTest, FindValue_DoubleType_ReturnsValue) {
    BinarySearchTree<double> tree;
    tree.insert(3.14);
    tree.insert(2.71);
    tree.insert(1.41);

    const auto result = tree.findValue(2.71);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 2.71);
}

/**
 * @brief Test BinarySearchTree findValue consistency with find
 * @details Verifies that findValue and find return consistent results
 */
TEST(BinarySearchTreeTest, FindValue_ConsistencyWithFind) {
    BinarySearchTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    // Test existing value
    EXPECT_EQ(tree.find(20), tree.findValue(20).has_value());

    // Test non-existing value
    EXPECT_EQ(tree.find(999), tree.findValue(999).has_value());
}

/**
 * @brief Test BinarySearchTree findValue after removal
 * @details Verifies that findValue correctly reflects tree state after removal
 */
TEST(BinarySearchTreeTest, FindValue_AfterRemoval_ReturnsNullopt) {
    BinarySearchTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);

    // Verify value exists
    const auto result1 = tree.findValue(30);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), 30);

    // Remove the value
    tree.remove(30);

    // Verify value no longer exists
    const auto result2 = tree.findValue(30);
    EXPECT_FALSE(result2.has_value());
}

/**
 * @brief Test BinarySearchTree findValue with large dataset
 * @details Verifies that findValue works efficiently with many elements
 */
TEST(BinarySearchTreeTest, FindValue_LargeDataset_ReturnsCorrectValue) {
    BinarySearchTree<int> tree;

    // Insert 1000 elements
    for (int i = 0; i < 1000; ++i) {
        tree.insert(i);
    }

    // Test finding various values
    const auto result1 = tree.findValue(0);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), 0);

    const auto result2 = tree.findValue(500);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), 500);

    const auto result3 = tree.findValue(999);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), 999);

    // Test non-existing value
    const auto result4 = tree.findValue(1000);
    EXPECT_FALSE(result4.has_value());
}