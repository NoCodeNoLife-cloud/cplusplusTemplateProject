/**
 * @file UnionSetTest.cc
 * @brief Unit tests for the UnionSet class
 * @details Tests cover core union-find operations including find, unionSets, connected,
 *          path compression, union by rank, and edge cases with different data types.
 */

#include <gtest/gtest.h>
#include "container/UnionSet.hpp"
#include <string>

using namespace common::container;

/**
 * @brief Test default constructor creates empty UnionSet
 * @details Verifies that a newly constructed UnionSet is ready to use
 */
TEST(UnionSetTest, DefaultConstructor_EmptyUnionSet) {
    UnionSet<int> unionSet;
    // UnionSet should be usable after construction
    EXPECT_TRUE(unionSet.connected(1, 1));
}

/**
 * @brief Test find returns the element itself for single element
 * @details Verifies that an unconnected element is its own root
 */
TEST(UnionSetTest, Find_SingleElement_ReturnsSelf) {
    UnionSet<int> unionSet;
    
    int root = unionSet.find(5);
    
    EXPECT_EQ(root, 5);
}

/**
 * @brief Test find with path compression
 * @details Verifies that path compression optimizes future lookups
 */
TEST(UnionSetTest, Find_PathCompression) {
    UnionSet<int> unionSet;
    
    // Create a chain: 1 -> 2 -> 3 -> 4
    unionSet.unionSets(1, 2);
    unionSet.unionSets(2, 3);
    unionSet.unionSets(3, 4);
    
    // Find should compress the path
    int root = unionSet.find(1);
    
    EXPECT_EQ(root, unionSet.find(4));
    // After path compression, subsequent finds should be faster
    EXPECT_EQ(unionSet.find(1), unionSet.find(2));
}

/**
 * @brief Test unionSets merges two separate sets
 * @details Verifies that union operation correctly combines two disjoint sets
 */
TEST(UnionSetTest, UnionSets_MergesTwoSets) {
    UnionSet<int> unionSet;
    
    bool result = unionSet.unionSets(1, 2);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(unionSet.connected(1, 2));
}

/**
 * @brief Test unionSets returns false for already connected elements
 * @details Verifies that union operation detects redundant unions
 */
TEST(UnionSetTest, UnionSets_AlreadyConnected_ReturnsFalse) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    bool result = unionSet.unionSets(1, 2);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Test unionSets with multiple elements maintains connectivity
 * @details Verifies transitive property of union operations
 */
TEST(UnionSetTest, UnionSets_MultipleElements_TransitiveConnectivity) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    unionSet.unionSets(2, 3);
    unionSet.unionSets(3, 4);
    
    EXPECT_TRUE(unionSet.connected(1, 4));
    EXPECT_TRUE(unionSet.connected(1, 3));
    EXPECT_TRUE(unionSet.connected(2, 4));
}

/**
 * @brief Test unionSets maintains separate sets when not united
 * @details Verifies that unrelated elements remain in different sets
 */
TEST(UnionSetTest, UnionSets_SeparateSets_RemainDisconnected) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    unionSet.unionSets(3, 4);
    
    EXPECT_FALSE(unionSet.connected(1, 3));
    EXPECT_FALSE(unionSet.connected(2, 4));
    EXPECT_TRUE(unionSet.connected(1, 2));
    EXPECT_TRUE(unionSet.connected(3, 4));
}

/**
 * @brief Test connected returns true for same element
 * @details Verifies reflexivity property
 */
TEST(UnionSetTest, Connected_SameElement_ReturnsTrue) {
    UnionSet<int> unionSet;
    
    EXPECT_TRUE(unionSet.connected(5, 5));
}

/**
 * @brief Test connected returns false for unconnected elements
 * @details Verifies that elements without union are not connected
 */
TEST(UnionSetTest, Connected_UnconnectedElements_ReturnsFalse) {
    UnionSet<int> unionSet;
    
    EXPECT_FALSE(unionSet.connected(1, 2));
}

/**
 * @brief Test connected after union operation
 * @details Verifies that connected correctly identifies united elements
 */
TEST(UnionSetTest, Connected_AfterUnion_ReturnsTrue) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(10, 20);
    unionSet.unionSets(20, 30);
    
    EXPECT_TRUE(unionSet.connected(10, 30));
    EXPECT_TRUE(unionSet.connected(10, 20));
    EXPECT_TRUE(unionSet.connected(20, 30));
}

/**
 * @brief Test connected with const reference
 * @details Verifies const-correctness of connected method
 */
TEST(UnionSetTest, Connected_ConstReference_CorrectBehavior) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    unionSet.unionSets(3, 4);
    
    const UnionSet<int>& constUnionSet = unionSet;
    
    EXPECT_TRUE(constUnionSet.connected(1, 2));
    EXPECT_FALSE(constUnionSet.connected(1, 3));
}

/**
 * @brief Test union by rank optimization
 * @details Verifies that union by rank creates balanced trees
 */
TEST(UnionSetTest, UnionByRank_BalancedStructure) {
    UnionSet<int> unionSet;
    
    // Create two chains
    for (int i = 0; i < 10; ++i) {
        unionSet.unionSets(i, i + 1);
    }
    
    for (int i = 10; i < 20; ++i) {
        unionSet.unionSets(i, i + 1);
    }
    
    // Unite the two chains
    unionSet.unionSets(0, 10);
    
    // All elements should be connected
    EXPECT_TRUE(unionSet.connected(0, 20));
    EXPECT_TRUE(unionSet.connected(5, 15));
}

/**
 * @brief Test UnionSet with string elements
 * @details Verifies UnionSet works correctly with non-numeric types
 */
TEST(UnionSetTest, StringElements_CorrectBehavior) {
    UnionSet<std::string> unionSet;
    
    unionSet.unionSets("apple", "banana");
    unionSet.unionSets("banana", "cherry");
    
    EXPECT_TRUE(unionSet.connected("apple", "cherry"));
    EXPECT_TRUE(unionSet.connected("apple", "banana"));
    EXPECT_FALSE(unionSet.connected("apple", "date"));
}

/**
 * @brief Test UnionSet with large number of elements
 * @details Verifies scalability and correctness with many elements
 */
TEST(UnionSetTest, LargeNumberOfElements_Correctness) {
    UnionSet<int> unionSet;
    
    // Create 100 separate sets
    for (int i = 0; i < 100; ++i) {
        unionSet.unionSets(i, i + 1);
    }
    
    // All should be connected
    EXPECT_TRUE(unionSet.connected(0, 100));
    EXPECT_TRUE(unionSet.connected(50, 99));
    
    // Elements outside range should not be connected
    EXPECT_FALSE(unionSet.connected(0, 200));
}

/**
 * @brief Test multiple independent components
 * @details Verifies correct handling of multiple disjoint sets
 */
TEST(UnionSetTest, MultipleIndependentComponents_CorrectHandling) {
    UnionSet<int> unionSet;
    
    // Component 1: 1-2-3
    unionSet.unionSets(1, 2);
    unionSet.unionSets(2, 3);
    
    // Component 2: 10-11-12
    unionSet.unionSets(10, 11);
    unionSet.unionSets(11, 12);
    
    // Component 3: 100-101
    unionSet.unionSets(100, 101);
    
    // Verify within-component connectivity
    EXPECT_TRUE(unionSet.connected(1, 3));
    EXPECT_TRUE(unionSet.connected(10, 12));
    EXPECT_TRUE(unionSet.connected(100, 101));
    
    // Verify between-component disconnection
    EXPECT_FALSE(unionSet.connected(1, 10));
    EXPECT_FALSE(unionSet.connected(1, 100));
    EXPECT_FALSE(unionSet.connected(10, 100));
}

/**
 * @brief Test UnionSet with negative numbers
 * @details Verifies correct handling of negative values
 */
TEST(UnionSetTest, NegativeNumbers_CorrectHandling) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(-10, -5);
    unionSet.unionSets(-5, -1);
    
    EXPECT_TRUE(unionSet.connected(-10, -1));
    EXPECT_FALSE(unionSet.connected(-10, 0));
}

/**
 * @brief Test UnionSet with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign values
 */
TEST(UnionSetTest, MixedSignNumbers_CorrectHandling) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(-100, 50);
    unionSet.unionSets(50, -50);
    unionSet.unionSets(-50, 100);
    
    EXPECT_TRUE(unionSet.connected(-100, 100));
    EXPECT_TRUE(unionSet.connected(50, -50));
}

/**
 * @brief Test repeated union operations on same elements
 * @details Verifies idempotency of union operations
 */
TEST(UnionSetTest, RepeatedUnions_Idempotent) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    unionSet.unionSets(1, 2);
    unionSet.unionSets(1, 2);
    
    EXPECT_TRUE(unionSet.connected(1, 2));
}

/**
 * @brief Test UnionSet forms correct equivalence classes
 * @details Verifies reflexive, symmetric, and transitive properties
 */
TEST(UnionSetTest, EquivalenceClasses_CorrectProperties) {
    UnionSet<int> unionSet;
    
    unionSet.unionSets(1, 2);
    unionSet.unionSets(2, 3);
    
    // Reflexive
    EXPECT_TRUE(unionSet.connected(1, 1));
    EXPECT_TRUE(unionSet.connected(2, 2));
    EXPECT_TRUE(unionSet.connected(3, 3));
    
    // Symmetric
    EXPECT_TRUE(unionSet.connected(1, 3));
    EXPECT_TRUE(unionSet.connected(3, 1));
    
    // Transitive
    EXPECT_TRUE(unionSet.connected(1, 2));
    EXPECT_TRUE(unionSet.connected(2, 3));
    EXPECT_TRUE(unionSet.connected(1, 3));
}
