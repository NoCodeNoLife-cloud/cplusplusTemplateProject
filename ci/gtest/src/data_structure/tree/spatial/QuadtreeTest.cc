/**
 * @file QuadtreeTest.cc
 * @brief Unit tests for the Quadtree class
 * @details Tests cover insertion, spatial query, boundary checks, subdivision,
 *          type variations, copy behavior, and edge cases.
 */

#include <algorithm>
#include <string>
#include <gtest/gtest.h>

#include "data_structure/tree/spatial/Quadtree.hpp"

using namespace common::data_structure::tree::spatial;

/**
 * @brief Test fixture for Quadtree tests
 */
class QuadtreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Basic Operations ====================

/**
 * @brief Test quadtree initial state
 * @details Verifies that a newly created quadtree is empty and has zero size
 */
TEST_F(QuadtreeTest, InitialState_EmptyAndZeroSize)
{
    const Quadtree<int> tree(0, 0, 100);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief Test inserting a single point
 * @details Verifies that after insertion the tree is not empty and size is one
 */
TEST_F(QuadtreeTest, Insert_SinglePoint_UpdatesSize)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 42);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Test inserting multiple points at different locations
 * @details Verifies that multiple insertions update the size correctly
 */
TEST_F(QuadtreeTest, Insert_MultiplePoints_TracksSize)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(-30, 40), 2);
    tree.insert(Point(50, -60), 3);
    tree.insert(Point(-80, -90), 4);

    EXPECT_EQ(tree.size(), 4);
}

// ==================== Subdivision ====================

/**
 * @brief Test that inserting points in the same quadrant triggers subdivision
 * @details When multiple points fall into the same region, the quadtree
 *          subdivides to accommodate them
 */
TEST_F(QuadtreeTest, Subdivide_SameQuadrant_StoresAllPoints)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(30, 40), 2);
    tree.insert(Point(50, 60), 3);

    EXPECT_EQ(tree.size(), 3);
}

// ==================== Query Range ====================

/**
 * @brief Test query range with all points in range
 * @details Verifies that querying a region containing all points returns all of them
 */
TEST_F(QuadtreeTest, QueryRange_ContainsAllPoints_ReturnsAll)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(-30, 40), 2);
    tree.insert(Point(50, -60), 3);

    const auto results = tree.queryRange(Point(0, 0), 150);
    EXPECT_EQ(results.size(), 3);
}

/**
 * @brief Test query range with no points in range
 * @details Verifies that querying a distant region returns an empty result
 */
TEST_F(QuadtreeTest, QueryRange_NoPointsInRange_ReturnsEmpty)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(-30, 40), 2);

    const auto results = tree.queryRange(Point(500, 500), 10);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Test query range with partial overlap
 * @details Verifies that only the points within the query region are returned
 */
TEST_F(QuadtreeTest, QueryRange_PartialOverlap_ReturnsSubset)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 10), 1);
    tree.insert(Point(-80, -80), 2);
    tree.insert(Point(80, 80), 3);

    // Query only the top-right quadrant
    const auto results = tree.queryRange(Point(50, 50), 60);
    ASSERT_EQ(results.size(), 2);
    std::vector<int> actual_values;
    for (const auto& [_, value] : results)
    {
        actual_values.push_back(value);
    }
    EXPECT_NE(std::find(actual_values.begin(), actual_values.end(), 1), actual_values.end());
    EXPECT_NE(std::find(actual_values.begin(), actual_values.end(), 3), actual_values.end());
}

/**
 * @brief Test query range on empty tree
 * @details Verifies that querying an empty tree returns an empty result
 */
TEST_F(QuadtreeTest, QueryRange_EmptyTree_ReturnsEmpty)
{
    const Quadtree<int> tree(0, 0, 100);
    const auto results = tree.queryRange(Point(0, 0), 50);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Test query range after subdivision
 * @details Verifies that range queries work correctly when points are deep in the tree
 */
TEST_F(QuadtreeTest, QueryRange_AfterSubdivision_FindsPoints)
{
    Quadtree<int> tree(0, 0, 100);
    // Insert many points clustered in the top-right quadrant to force subdivision
    tree.insert(Point(10, 10), 1);
    tree.insert(Point(20, 20), 2);
    tree.insert(Point(30, 30), 3);
    tree.insert(Point(40, 40), 4);

    const auto results = tree.queryRange(Point(25, 25), 50);
    EXPECT_EQ(results.size(), 4);
}

// ==================== Clear ====================

/**
 * @brief Test clearing the quadtree
 * @details Verifies that clear removes all points and resets to empty state
 */
TEST_F(QuadtreeTest, Clear_AfterInsertions_ResetsToEmpty)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(-30, 40), 2);
    tree.insert(Point(50, -60), 3);

    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);

    // Verify re-insertion works after clear
    tree.insert(Point(0, 0), 99);
    EXPECT_EQ(tree.size(), 1);
}

// ==================== Boundary ====================

/**
 * @brief Test inserting a point outside the boundary
 * @details Verifies that inserting a point outside the quadtree boundary
 *          throws a runtime_error
 */
TEST_F(QuadtreeTest, Insert_PointOutsideBoundary_ThrowsException)
{
    Quadtree<int> tree(0, 0, 100);
    EXPECT_THROW(tree.insert(Point(200, 200), 1), std::runtime_error);
    EXPECT_TRUE(tree.empty());
}

// ==================== Type Variations ====================

/**
 * @brief Test quadtree with string values
 * @details Verifies that the quadtree works with non-numeric types
 */
TEST_F(QuadtreeTest, Insert_StringType_StoresCorrectly)
{
    Quadtree<std::string> tree(0, 0, 100);
    tree.insert(Point(10, 20), "apple");
    tree.insert(Point(-30, 40), "banana");

    const auto results = tree.queryRange(Point(0, 0), 150);
    EXPECT_EQ(results.size(), 2);

    std::vector<std::string> values;
    for (const auto& [_, value] : results)
    {
        values.push_back(value);
    }
    EXPECT_NE(std::find(values.begin(), values.end(), "apple"), values.end());
    EXPECT_NE(std::find(values.begin(), values.end(), "banana"), values.end());
}

/**
 * @brief Test quadtree with double precision
 * @details Verifies that floating point coordinates are handled correctly
 */
TEST_F(QuadtreeTest, Insert_DoublePrecision_StoresCorrectly)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(3.14159, 2.71828), 42);

    const auto results = tree.queryRange(Point(0, 0), 150);
    ASSERT_EQ(results.size(), 1);
    EXPECT_DOUBLE_EQ(results[0].first.x_, 3.14159);
    EXPECT_DOUBLE_EQ(results[0].first.y_, 2.71828);
    EXPECT_EQ(results[0].second, 42);
}

/**
 * @brief Test query with zero-width range
 * @details Verifies that a zero-width range query returns only exact match
 */
TEST_F(QuadtreeTest, QueryRange_ZeroWidth_FindsExactMatch)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 20), 1);
    tree.insert(Point(11, 21), 2);

    const auto results = tree.queryRange(Point(10, 20), 0);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].second, 1);
}

/**
 * @brief Test deep subdivision through many insertions
 * @details Inserts many points in the same small area to force deep recursion
 */
TEST_F(QuadtreeTest, DeepSubdivision_ManyPoints_SameRegion)
{
    Quadtree<int> tree(0, 0, 100);
    constexpr int kCount = 50;

    for (int i = 1; i <= kCount; ++i)
    {
        tree.insert(Point(static_cast<double>(i), static_cast<double>(i)), i);
    }

    EXPECT_EQ(tree.size(), kCount);

    const auto results = tree.queryRange(Point(25, 25), 30);
    EXPECT_EQ(results.size(), kCount);
}

// ==================== Edge Cases ====================

/**
 * @brief Test inserting at the exact center
 * @details Verifies that a point at the center is stored correctly
 */
TEST_F(QuadtreeTest, Insert_CenterPoint_StoresCorrectly)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(0, 0), 1);

    const auto results = tree.queryRange(Point(0, 0), 10);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].second, 1);
}

/**
 * @brief Test inserting points on boundary edges
 * @details Verifies that points exactly on the boundary are accepted
 */
TEST_F(QuadtreeTest, Insert_OnBoundaryEdge_AcceptsPoint)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(100, 50), 1);   // On right edge (x = cx + hd)
    tree.insert(Point(-100, 50), 2);  // On left edge (x = cx - hd)
    tree.insert(Point(50, 100), 3);   // On top edge (y = cy + hd)
    tree.insert(Point(50, -100), 4);  // On bottom edge (y = cy - hd)

    EXPECT_EQ(tree.size(), 4);

    const auto results = tree.queryRange(Point(0, 0), 150);
    EXPECT_EQ(results.size(), 4);
}

/**
 * @brief Test large number of insertions
 * @details Verifies that the quadtree can handle many points
 */
TEST_F(QuadtreeTest, Insert_LargeDataset_MaintainsCorrectSize)
{
    Quadtree<int> tree(0, 0, 1000);
    constexpr int kCount = 500;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(Point(static_cast<double>(i), static_cast<double>(i)), i);
    }

    EXPECT_EQ(tree.size(), kCount);

    const auto results = tree.queryRange(Point(0, 0), 1500);
    EXPECT_EQ(results.size(), kCount);
}

/**
 * @brief Test multiple query ranges
 * @details Verifies that multiple sequential queries return consistent results
 */
TEST_F(QuadtreeTest, QueryRange_MultipleQueries_ConsistentResults)
{
    Quadtree<int> tree(0, 0, 100);
    tree.insert(Point(10, 10), 1);
    tree.insert(Point(50, 50), 2);
    tree.insert(Point(-50, -50), 3);

    // Query top-right
    const auto r1 = tree.queryRange(Point(50, 50), 60);
    EXPECT_EQ(r1.size(), 2);

    // Query bottom-left
    const auto r2 = tree.queryRange(Point(-50, -50), 60);
    EXPECT_EQ(r2.size(), 2);

    // Query everything
    const auto r3 = tree.queryRange(Point(0, 0), 150);
    EXPECT_EQ(r3.size(), 3);
}
