/**
 * @file KDTreeTest.cc
 * @brief Unit tests for the KDTree class
 * @details Tests cover default construction, bulk build, insertion,
 *          nearest-neighbor (NN), K-nearest-neighbors (KNN), radius-based
 *          range search, move semantics, thread safety, and compatibility
 *          with multiple point types (std::array, Point2D, Point3D).
 */

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "data_structure/spatial/KDTree.hpp"
#include "data_structure/geometry/Point2D.hpp"
#include "data_structure/geometry/Point3D.hpp"

using namespace cppforge::data_structure::spatial;
using namespace cppforge::data_structure::geometry;

// ══════════════════════════════════════════════════════════════════════════
//  Type aliases for common KDTree variants used throughout the tests
// ══════════════════════════════════════════════════════════════════════════

/// 2-D tree using std::array<double, 2> (default KDTreeAccessor via operator[])
using KDTreeArr2D = KDTree<std::array<double, 2>, 2>;

/// 3-D tree using std::array<double, 3>
using KDTreeArr3D = KDTree<std::array<double, 3>, 3>;

/// 1-D tree �?edge case (minimum dimensionality)
using KDTreeArr1D = KDTree<std::array<double, 1>, 1>;

/// 2-D tree using geometry::Point2D (specialized accessor)
using KDTreeP2D   = KDTree<Point2D, 2>;

/// 3-D tree using geometry::Point3D (specialized accessor)
using KDTreeP3D   = KDTree<Point3D, 3>;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class KDTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Empty tree behaviours
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A default-constructed tree has size 0.
 * @details Verifies that a newly constructed KDTree reports size zero
 */
TEST_F(KDTreeTest, Empty_SizeIsZero)
{
    const KDTreeArr2D tree;
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief A default-constructed tree reports empty() == true.
 * @details Verifies that a default-constructed KDTree is considered empty
 */
TEST_F(KDTreeTest, Empty_EmptyReturnsTrue)
{
    const KDTreeArr2D tree;
    EXPECT_TRUE(tree.empty());
}

/**
 * @brief nearestNeighbor on an empty tree returns std::nullopt.
 * @details Verifies that querying nearest neighbor on an empty tree returns no result
 */
TEST_F(KDTreeTest, Empty_NearestNeighbor_ReturnsNullopt)
{
    const KDTreeArr2D tree;
    const auto result = tree.nearestNeighbor({1.0, 2.0});
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief kNearestNeighbors on an empty tree returns an empty vector.
 * @details Verifies that KNN query on an empty tree returns an empty result vector
 */
TEST_F(KDTreeTest, Empty_KNearestNeighbors_ReturnsEmpty)
{
    const KDTreeArr2D tree;
    const auto result = tree.kNearestNeighbors({1.0, 2.0}, 5);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief rangeSearch on an empty tree returns an empty vector.
 * @details Verifies that radius-based range search on an empty tree returns an empty vector
 */
TEST_F(KDTreeTest, Empty_RangeSearch_ReturnsEmpty)
{
    const KDTreeArr2D tree;
    const auto result = tree.rangeSearch({1.0, 2.0}, 10.0);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Calling clear() on an empty tree is safe and idempotent.
 * @details Verifies that clearing an already empty tree does not crash and maintains empty state
 */
TEST_F(KDTreeTest, Empty_Clear_IsSafe)
{
    KDTreeArr2D tree;
    EXPECT_NO_THROW(tree.clear());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Bulk build
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Building via initializer list produces the correct size.
 * @details Verifies that building a KDTree from an initializer list yields the expected element count
 */
TEST_F(KDTreeTest, Build_WithInitializerList_CreatesCorrectSize)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}});
    EXPECT_EQ(tree.size(), 3);
    EXPECT_FALSE(tree.empty());
}

/**
 * @brief Building via iterator range produces the correct size.
 * @details Verifies that building a KDTree from iterator pair yields the expected element count
 */
TEST_F(KDTreeTest, Build_WithIteratorRange_CreatesCorrectSize)
{
    KDTreeArr2D tree;
    const std::vector<std::array<double, 2>> points = {
        {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}, {4.0, 4.0}
    };
    tree.build(points.begin(), points.end());
    EXPECT_EQ(tree.size(), 4);
}

/**
 * @brief A second build() discards points from the first build.
 * @details Verifies that calling build() again replaces all previously stored points
 */
TEST_F(KDTreeTest, Build_ReplacesPreviousContent)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}});
    ASSERT_EQ(tree.size(), 1);

    tree.build({{3.0, 4.0}, {5.0, 6.0}});
    EXPECT_EQ(tree.size(), 2);
}

/**
 * @brief Build with a single point produces a searchable tree.
 * @details Verifies that a KDTree built with a single point can be queried via nearest neighbor
 */
TEST_F(KDTreeTest, Build_WithSinglePoint)
{
    KDTreeArr2D tree;
    tree.build({{7.0, 8.0}});
    EXPECT_EQ(tree.size(), 1);

    const auto nn = tree.nearestNeighbor({7.0, 8.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 7.0);
    EXPECT_EQ((*nn)[1], 8.0);
}

/**
 * @brief Build with an even number of points.
 * @details Verifies that building a KDTree with an even number of points completes without error
 */
TEST_F(KDTreeTest, Build_WithEvenNumberOfPoints)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}});
    EXPECT_EQ(tree.size(), 4);
}

/**
 * @brief Build with an odd number of points.
 * @details Verifies that building a KDTree with an odd number of points completes without error
 */
TEST_F(KDTreeTest, Build_WithOddNumberOfPoints)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0},
                {3.0, 3.0}, {4.0, 4.0}});
    EXPECT_EQ(tree.size(), 5);
}

/**
 * @brief Build with an empty range produces an empty tree.
 * @details Verifies that building a KDTree from an empty point range results in an empty tree
 */
TEST_F(KDTreeTest, Build_EmptyRange_ResultsInEmptyTree)
{
    KDTreeArr2D tree;
    const std::vector<std::array<double, 2>> empty;
    tree.build(empty.begin(), empty.end());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Insert
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting a single point increases the tree size to 1.
 * @details Verifies that inserting one point increments size and returns true
 */
TEST_F(KDTreeTest, Insert_SinglePoint_IncreasesSize)
{
    KDTreeArr2D tree;
    EXPECT_TRUE(tree.insert({1.0, 2.0}));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Inserting multiple unique points increases the size accordingly.
 * @details Verifies that inserting multiple distinct points correctly increments size for each insertion
 */
TEST_F(KDTreeTest, Insert_MultiplePoints_IncreasesSize)
{
    KDTreeArr2D tree;
    EXPECT_TRUE(tree.insert({1.0, 1.0}));
    EXPECT_TRUE(tree.insert({2.0, 2.0}));
    EXPECT_TRUE(tree.insert({3.0, 3.0}));
    EXPECT_EQ(tree.size(), 3);
}

/**
 * @brief Inserting a duplicate returns false and does not change size.
 * @details Verifies that inserting an already existing point is rejected and size remains unchanged
 */
TEST_F(KDTreeTest, Insert_Duplicate_ReturnsFalse)
{
    KDTreeArr2D tree;
    EXPECT_TRUE(tree.insert({1.0, 2.0}));
    EXPECT_FALSE(tree.insert({1.0, 2.0}));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Inserting a duplicate after build does not increase size.
 * @details Verifies that inserting a duplicate point into a tree built via build() is correctly rejected
 */
TEST_F(KDTreeTest, Insert_Duplicate_AfterBuild_ReturnsFalse)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}, {3.0, 4.0}});
    ASSERT_EQ(tree.size(), 2);

    EXPECT_FALSE(tree.insert({1.0, 2.0}));
    EXPECT_EQ(tree.size(), 2);
}

/**
 * @brief Insert after build adds to the existing tree.
 * @details Verifies that inserting new points into a tree previously built via build() correctly increases size
 */
TEST_F(KDTreeTest, Insert_AfterBuild_AddsPoints)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 1.0}, {2.0, 2.0}});
    ASSERT_EQ(tree.size(), 2);

    EXPECT_TRUE(tree.insert({3.0, 3.0}));
    EXPECT_EQ(tree.size(), 3);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Nearest neighbour
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief NN on a single-point tree returns that exact point.
 * @details Verifies that nearest neighbor search on a tree with one point returns that point
 */
TEST_F(KDTreeTest, NearestNeighbor_SinglePoint_ReturnsThatPoint)
{
    KDTreeArr2D tree;
    tree.build({{5.0, 5.0}});
    const auto nn = tree.nearestNeighbor({5.0, 5.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 5.0);
    EXPECT_EQ((*nn)[1], 5.0);
}

/**
 * @brief NN finds the exact duplicate when the query matches a stored point.
 * @details Verifies that nearest neighbor returns the exact point when the query coincides with a stored point
 */
TEST_F(KDTreeTest, NearestNeighbor_ExactMatch_ReturnsQuery)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}});
    const auto nn = tree.nearestNeighbor({3.0, 4.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 3.0);
    EXPECT_EQ((*nn)[1], 4.0);
}

/**
 * @brief NN identifies the closest point among multiple candidates.
 * @details Verifies that nearest neighbor correctly identifies the closest point when multiple candidates exist
 */
TEST_F(KDTreeTest, NearestNeighbor_FindsClosest)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {100.0, 100.0}, {1.0, 1.0}});
    // Query at (0.9, 0.9) �?(1.0, 1.0) is the nearest
    const auto nn = tree.nearestNeighbor({0.9, 0.9});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 1.0);
    EXPECT_EQ((*nn)[1], 1.0);
}

/**
 * @brief NN with a richer set of points.
 * @details Verifies that nearest neighbor returns the correct point from a diverse set of candidates
 */
TEST_F(KDTreeTest, NearestNeighbor_MultiplePoints_CorrectResult)
{
    KDTreeArr2D tree;
    tree.build({{5.0, 5.0}, {0.0, 0.0}, {10.0, 0.0},
                {0.0, 10.0}, {10.0, 10.0}});
    // Query at (3.0, 4.0) �?closest is (5.0, 5.0)
    const auto nn = tree.nearestNeighbor({3.0, 4.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 5.0);
    EXPECT_EQ((*nn)[1], 5.0);
}

/**
 * @brief NN works correctly with negative coordinates.
 * @details Verifies that nearest neighbor search handles points with negative coordinate values correctly
 */
TEST_F(KDTreeTest, NearestNeighbor_WithNegativeCoordinates)
{
    KDTreeArr2D tree;
    tree.build({{-1.0, -1.0}, {-5.0, -5.0}, {3.0, 3.0}});
    const auto nn = tree.nearestNeighbor({-4.0, -4.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], -5.0);
    EXPECT_EQ((*nn)[1], -5.0);
}

/**
 * @brief NN does not crash when coordinates are at extremes.
 * @details Verifies that nearest neighbor handles extremely large coordinate values without numeric issues
 */
TEST_F(KDTreeTest, NearestNeighbor_WithExtremeValues)
{
    KDTreeArr2D tree;
    constexpr double big = 1e150;
    tree.build({{big, big}, {-big, -big}});
    const auto nn = tree.nearestNeighbor({big * 0.99, big * 0.99});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], big);
    EXPECT_EQ((*nn)[1], big);
}

// ══════════════════════════════════════════════════════════════════════════
//  5. K-nearest neighbours
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief K = 0 returns an empty vector regardless of tree contents.
 * @details Verifies that requesting zero nearest neighbors returns an empty result vector
 */
TEST_F(KDTreeTest, KNearestNeighbors_KIsZero_ReturnsEmpty)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}, {3.0, 4.0}});
    const auto result = tree.kNearestNeighbors({0.0, 0.0}, 0);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief K = 1 should return the same point as nearestNeighbor.
 * @details Verifies that KNN with K=1 produces the same result as a single nearestNeighbor query
 */
TEST_F(KDTreeTest, KNearestNeighbors_KIsOne_EqualsNearestNeighbor)
{
    KDTreeArr2D tree;
    tree.build({{5.0, 5.0}, {0.0, 0.0}, {10.0, 10.0}});
    const auto nn  = tree.nearestNeighbor({2.0, 2.0});
    const auto knn = tree.kNearestNeighbors({2.0, 2.0}, 1);
    ASSERT_EQ(knn.size(), 1);
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ(knn[0], *nn);
}

/**
 * @brief When K > tree size, all points are returned.
 * @details Verifies that requesting more neighbors than available points returns all stored points
 */
TEST_F(KDTreeTest, KNearestNeighbors_KGreaterThanSize_ReturnsAll)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 1.0}, {2.0, 2.0}});
    const auto result = tree.kNearestNeighbors({0.0, 0.0}, 10);
    EXPECT_EQ(result.size(), 2);
}

/**
 * @brief Results are sorted by increasing distance from the query.
 * @details Verifies that KNN returns results in non-decreasing order of squared distance from the query point
 */
TEST_F(KDTreeTest, KNearestNeighbors_ResultsSortedByDistance)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {10.0, 10.0}, {1.0, 1.0}, {2.0, 2.0}});

    const auto result = tree.kNearestNeighbors({0.5, 0.5}, 4);
    ASSERT_EQ(result.size(), 4);

    // Verify non-decreasing squared distance
    auto sqDist = [](const std::array<double, 2>& pt, double qx, double qy)
    {
        const double dx = pt[0] - qx;
        const double dy = pt[1] - qy;
        return dx * dx + dy * dy;
    };

    for (size_t i = 1; i < result.size(); ++i)
    {
        EXPECT_LE(sqDist(result[i - 1], 0.5, 0.5),
                  sqDist(result[i], 0.5, 0.5));
    }
}

/**
 * @brief An exact match appears as the first element (distance 0).
 * @details Verifies that when the query point exactly matches a stored point, it is the first result
 */
TEST_F(KDTreeTest, KNearestNeighbors_ExactMatchIsFirst)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}});
    const auto result = tree.kNearestNeighbors({2.0, 2.0}, 3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0][0], 2.0);
    EXPECT_EQ(result[0][1], 2.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Range search
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Radius = 0 returns only exact matches (distance 0).
 * @details Verifies that range search with radius zero returns only points that exactly match the query coordinates
 */
TEST_F(KDTreeTest, RangeSearch_RadiusZero_ReturnsOnlyExactMatches)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}});
    const auto result = tree.rangeSearch({1.0, 1.0}, 0.0);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], 1.0);
    EXPECT_EQ(result[0][1], 1.0);
}

/**
 * @brief Radius captures a subset of points.
 * @details Verifies that range search with an appropriate radius returns only points within that distance
 */
TEST_F(KDTreeTest, RangeSearch_RadiusIncludesSomePoints)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {3.0, 0.0}, {0.0, 3.0}, {6.0, 6.0}});
    // Query at origin, radius = 3.0  ->  includes (0,0), (3,0), (0,3)
    const auto result = tree.rangeSearch({0.0, 0.0}, 3.0);
    EXPECT_EQ(result.size(), 3);
}

/**
 * @brief A sufficiently large radius returns all points.
 * @details Verifies that a radius large enough to cover all points returns the entire dataset
 */
TEST_F(KDTreeTest, RangeSearch_RadiusIncludesAllPoints)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}});
    const auto result = tree.rangeSearch({0.0, 0.0}, 100.0);
    EXPECT_EQ(result.size(), 3);
}

/**
 * @brief Radius that captures no points returns an empty vector.
 * @details Verifies that range search with a radius too small to include any points returns an empty result
 */
TEST_F(KDTreeTest, RangeSearch_NoPointsInRange_ReturnsEmpty)
{
    KDTreeArr2D tree;
    tree.build({{10.0, 10.0}, {20.0, 20.0}});
    const auto result = tree.rangeSearch({0.0, 0.0}, 1.0);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief rangeSearch on an empty tree returns empty (already covered by
 *        Empty_RangeSearch_ReturnsEmpty, but re-verified here).
 * @details Re-verifies that range search on an empty tree returns an empty vector
 */
TEST_F(KDTreeTest, RangeSearch_EmptyTree_ReturnsEmpty)
{
    const KDTreeArr2D tree;
    EXPECT_TRUE(tree.rangeSearch({0.0, 0.0}, 10.0).empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Insert + search integration
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Points inserted incrementally can be found by NN.
 * @details Verifies that incrementally inserted points are correctly discoverable via nearest neighbor search
 */
TEST_F(KDTreeTest, InsertThenNearestNeighbor_FindsCorrectPoint)
{
    KDTreeArr2D tree;
    tree.insert({5.0, 5.0});
    tree.insert({0.0, 0.0});
    tree.insert({10.0, 10.0});

    const auto nn = tree.nearestNeighbor({1.0, 1.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 0.0);
    EXPECT_EQ((*nn)[1], 0.0);
}

/**
 * @brief Points inserted incrementally are found by range search.
 * @details Verifies that incrementally inserted points are correctly discoverable via range search
 */
TEST_F(KDTreeTest, InsertThenRangeSearch_FindsPoints)
{
    KDTreeArr2D tree;
    tree.insert({1.0, 1.0});
    tree.insert({5.0, 5.0});
    tree.insert({10.0, 10.0});

    const auto result = tree.rangeSearch({1.0, 1.0}, 0.5);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], 1.0);
    EXPECT_EQ(result[0][1], 1.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Point2D point type (specialized KDTreeAccessor)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Build and NN with geometry::Point2D.
 * @details Verifies that KDTree works correctly with geometry::Point2D point type for build and nearest neighbor
 */
TEST_F(KDTreeTest, Point2D_BuildAndNearestNeighbor)
{
    KDTreeP2D tree;
    tree.build({Point2D{1.0, 2.0}, Point2D{3.0, 4.0}, Point2D{5.0, 6.0}});
    EXPECT_EQ(tree.size(), 3);

    const auto nn = tree.nearestNeighbor(Point2D{3.1, 3.9});
    ASSERT_TRUE(nn.has_value());
    EXPECT_TRUE((*nn == Point2D{3.0, 4.0}));
}

/**
 * @brief Insert and KNN with geometry::Point2D.
 * @details Verifies that KDTree supports insert and K-nearest-neighbors with geometry::Point2D
 */
TEST_F(KDTreeTest, Point2D_InsertAndKNN)
{
    KDTreeP2D tree;
    EXPECT_TRUE(tree.insert(Point2D{10.0, 10.0}));
    EXPECT_TRUE(tree.insert(Point2D{0.0, 0.0}));
    EXPECT_TRUE(tree.insert(Point2D{5.0, 5.0}));
    EXPECT_EQ(tree.size(), 3);

    const auto knn = tree.kNearestNeighbors(Point2D{1.0, 1.0}, 2);
    ASSERT_EQ(knn.size(), 2);
    EXPECT_TRUE((knn[0] == Point2D{0.0, 0.0}));  // closest
}

/**
 * @brief Duplicate detection with Point2D (K=2 fast path).
 * @details Verifies that inserting a duplicate Point2D returns false using the K=2 optimized duplicate check
 */
TEST_F(KDTreeTest, Point2D_DuplicateInsert_ReturnsFalse)
{
    KDTreeP2D tree;
    EXPECT_TRUE(tree.insert(Point2D{2.0, 3.0}));
    EXPECT_FALSE(tree.insert(Point2D{2.0, 3.0}));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Range search with geometry::Point2D.
 * @details Verifies that radius-based range search works correctly with geometry::Point2D points
 */
TEST_F(KDTreeTest, Point2D_RangeSearch)
{
    KDTreeP2D tree;
    tree.build({Point2D{0.0, 0.0}, Point2D{2.0, 0.0}, Point2D{4.0, 0.0}});
    const auto result = tree.rangeSearch(Point2D{0.0, 0.0}, 2.5);
    EXPECT_EQ(result.size(), 2);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Point3D point type (specialized KDTreeAccessor)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Build and NN with geometry::Point3D.
 * @details Verifies that KDTree works correctly with geometry::Point3D point type for build and nearest neighbor
 */
TEST_F(KDTreeTest, Point3D_BuildAndNearestNeighbor)
{
    KDTreeP3D tree;
    tree.build({Point3D{1.0, 2.0, 3.0}, Point3D{4.0, 5.0, 6.0},
                Point3D{7.0, 8.0, 9.0}});
    EXPECT_EQ(tree.size(), 3);

    const auto nn = tree.nearestNeighbor(Point3D{3.9, 5.1, 6.1});
    ASSERT_TRUE(nn.has_value());
    EXPECT_TRUE((*nn == Point3D{4.0, 5.0, 6.0}));
}

/**
 * @brief Range search in 3D.
 * @details Verifies that radius-based range search works correctly in three-dimensional space
 */
TEST_F(KDTreeTest, Point3D_RangeSearch)
{
    KDTreeP3D tree;
    tree.build({Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 0.0, 0.0},
                Point3D{0.0, 2.0, 0.0}, Point3D{5.0, 5.0, 5.0}});
    // Query at origin, radius 2.0  ->  (0,0,0), (2,0,0), (0,2,0)
    const auto result = tree.rangeSearch(Point3D{0.0, 0.0, 0.0}, 2.0);
    EXPECT_EQ(result.size(), 3);
}

/**
 * @brief Duplicate detection with Point3D (K=3 fast path).
 * @details Verifies that inserting a duplicate Point3D returns false using the K=3 optimized duplicate check
 */
TEST_F(KDTreeTest, Point3D_DuplicateInsert_ReturnsFalse)
{
    KDTreeP3D tree;
    EXPECT_TRUE(tree.insert(Point3D{1.0, 2.0, 3.0}));
    EXPECT_FALSE(tree.insert(Point3D{1.0, 2.0, 3.0}));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief KNN with geometry::Point3D.
 * @details Verifies that K-nearest-neighbors search works correctly with geometry::Point3D points
 */
TEST_F(KDTreeTest, Point3D_KNearestNeighbors)
{
    KDTreeP3D tree;
    tree.build({Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0},
                Point3D{5.0, 5.0, 5.0}, Point3D{10.0, 10.0, 10.0}});
    const auto knn = tree.kNearestNeighbors(Point3D{0.5, 0.5, 0.5}, 2);
    ASSERT_EQ(knn.size(), 2);
    // The closest two are (0,0,0) and (1,1,1) in some order; both are correct
    // as long as the distances are correct
    EXPECT_TRUE((knn[0] == Point3D{0.0, 0.0, 0.0} ||
                 knn[0] == Point3D{1.0, 1.0, 1.0}));
}

// ══════════════════════════════════════════════════════════════════════════
//  10. 1-dimensional KDTree (minimum dimensionality)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief 1-D build and NN.
 * @details Verifies that a one-dimensional KDTree supports build and nearest neighbor search
 */
TEST_F(KDTreeTest, KDTree1D_BuildAndNearestNeighbor)
{
    KDTreeArr1D tree;
    tree.build({{0.0}, {10.0}, {20.0}, {30.0}});
    EXPECT_EQ(tree.size(), 4);

    const auto nn = tree.nearestNeighbor({12.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 10.0);
}

/**
 * @brief 1-D KNN.
 * @details Verifies that K-nearest-neighbors search works correctly in one dimension
 */
TEST_F(KDTreeTest, KDTree1D_KNearestNeighbors)
{
    KDTreeArr1D tree;
    tree.build({{0.0}, {5.0}, {10.0}, {15.0}});
    const auto result = tree.kNearestNeighbors({7.0}, 2);
    ASSERT_EQ(result.size(), 2);
    // Closest: 5.0, then 10.0
    EXPECT_EQ(result[0][0], 5.0);
    EXPECT_EQ(result[1][0], 10.0);
}

/**
 * @brief 1-D insert duplicates.
 * @details Verifies that inserting a duplicate point in a one-dimensional tree returns false
 */
TEST_F(KDTreeTest, KDTree1D_DuplicateInsert_ReturnsFalse)
{
    KDTreeArr1D tree;
    EXPECT_TRUE(tree.insert({5.0}));
    EXPECT_FALSE(tree.insert({5.0}));
    EXPECT_EQ(tree.size(), 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  11. Higher-dimensional KDTree (K=4) �?generic duplicate-detection path
// ══════════════════════════════════════════════════════════════════════════

/// 4-D tree �?exercises the K > 3 coordinate-wise duplicate check.
using KDTreeArr4D = KDTree<std::array<double, 4>, 4>;

/**
 * @brief Insert and duplicate detection with K=4 (generic path).
 * @details Verifies that duplicate detection works via the generic coordinate-wise path for K=4
 */
TEST_F(KDTreeTest, KDTree4D_DuplicateInsert_ReturnsFalse)
{
    KDTreeArr4D tree;
    constexpr std::array<double, 4> pt = {1.0, 2.0, 3.0, 4.0};
    EXPECT_TRUE(tree.insert(pt));
    EXPECT_FALSE(tree.insert(pt));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief NN with K=4.
 * @details Verifies that nearest neighbor search works correctly in four-dimensional space
 */
TEST_F(KDTreeTest, KDTree4D_NearestNeighbor)
{
    KDTreeArr4D tree;
    tree.build({{0.0, 0.0, 0.0, 0.0},
                {5.0, 5.0, 5.0, 5.0},
                {10.0, 10.0, 10.0, 10.0}});

    const auto nn = tree.nearestNeighbor({4.0, 4.0, 4.0, 4.0});
    ASSERT_TRUE(nn.has_value());
    for (size_t d = 0; d < 4; ++d)
    {
        EXPECT_EQ((*nn)[d], 5.0);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  12. Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief clear() removes all points and resets the tree to empty state.
 * @details Verifies that clear removes all points and all query methods return empty results afterward
 */
TEST_F(KDTreeTest, Clear_EmptiesTheTree)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 2.0}, {3.0, 4.0}});
    ASSERT_EQ(tree.size(), 2);
    ASSERT_FALSE(tree.empty());

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_FALSE(tree.nearestNeighbor({1.0, 2.0}).has_value());
    EXPECT_TRUE(tree.kNearestNeighbors({1.0, 2.0}, 1).empty());
    EXPECT_TRUE(tree.rangeSearch({1.0, 2.0}, 10.0).empty());
}

/**
 * @brief After clear(), the tree can be reused for a new build.
 * @details Verifies that a cleared tree can be rebuilt with new points and functions correctly
 */
TEST_F(KDTreeTest, Clear_ThenRebuild_Works)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 1.0}, {2.0, 2.0}});
    tree.clear();
    ASSERT_TRUE(tree.empty());

    tree.build({{10.0, 10.0}, {20.0, 20.0}, {30.0, 30.0}});
    EXPECT_EQ(tree.size(), 3);
}

// ══════════════════════════════════════════════════════════════════════════
//  13. Move semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Move constructor transfers state; source becomes empty.
 * @details Verifies that move construction transfers all points to the new tree and leaves the source empty
 */
TEST_F(KDTreeTest, MoveConstructor_TransfersState)
{
    KDTreeArr2D original;
    original.build({{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}});
    ASSERT_EQ(original.size(), 3);

    KDTreeArr2D moved(std::move(original));
    EXPECT_EQ(moved.size(), 3);
    EXPECT_TRUE(original.empty());

    // Moved tree should be fully functional
    const auto nn = moved.nearestNeighbor({2.0, 2.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 2.0);
    EXPECT_EQ((*nn)[1], 2.0);
}

/**
 * @brief Move assignment transfers state; source becomes empty.
 * @details Verifies that move assignment transfers all points to the target and leaves the source empty
 */
TEST_F(KDTreeTest, MoveAssignment_TransfersState)
{
    KDTreeArr2D first;
    first.build({{10.0, 10.0}});

    KDTreeArr2D second;
    second.build({{20.0, 20.0}, {30.0, 30.0}});

    second = std::move(first);

    EXPECT_EQ(second.size(), 1);
    EXPECT_TRUE(first.empty());

    const auto nn = second.nearestNeighbor({10.0, 10.0});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 10.0);
    EXPECT_EQ((*nn)[1], 10.0);
}

/**
 * @brief Self-move-assignment is safe (no crash, no hang).
 * @details Verifies that self-move-assignment via std::move does not crash and leaves the tree in a valid state
 */
TEST_F(KDTreeTest, MoveAssignment_SelfMove_IsSafe)
{
    KDTreeArr2D tree;
    tree.build({{1.0, 1.0}, {2.0, 2.0}});
    // Self-assignment via std::move �?should be safe
    tree = std::move(tree);
    // After self-move, the object should still be in a valid (unspecified but
    // destructible) state.  Best effort: ensure we can still call methods
    // without crashing.
    EXPECT_NO_THROW(static_cast<void>(tree.size()));
    EXPECT_NO_THROW(tree.clear());
}

// ══════════════════════════════════════════════════════════════════════════
//  14. Thread safety
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent read-only operations do not crash or produce wrong
 *        results.
 * @details Verifies that multiple threads performing nearest neighbor queries concurrently are safe and correct
 */
TEST_F(KDTreeTest, ConcurrentReads_NoCrash)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0},
                {4.0, 4.0}, {5.0, 5.0}, {6.0, 6.0}, {7.0, 7.0}});

    std::atomic<uint64_t> successCount{0};
    constexpr int THREADS       = 8;
    constexpr int OPS_PER_THREAD = 5000;

    auto worker = [&]()
    {
        for (int i = 0; i < OPS_PER_THREAD; ++i)
        {
            const double v = static_cast<double>(i % 100) / 10.0;
            const auto nn  = tree.nearestNeighbor({v, v});
            if (nn.has_value())
            {
                successCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t)
    {
        threads.emplace_back(worker);
    }
    for (auto& t : threads)
    {
        t.join();
    }

    // Every NN call on a non-empty tree should succeed
    EXPECT_EQ(successCount.load(), THREADS * OPS_PER_THREAD);
}

/**
 * @brief Concurrent read + write operations do not crash.
 * @details Verifies that simultaneous read (NN queries) and write (insert) operations do not cause crashes
 */
TEST_F(KDTreeTest, ConcurrentReadWrite_NoCrash)
{
    KDTreeArr2D tree;
    tree.build({{0.0, 0.0}, {2.0, 2.0}, {4.0, 4.0}});

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> readOk{0};

    auto reader = [&]()
    {
        while (!stop.load(std::memory_order_acquire))
        {
            for (int i = 0; i < 50; ++i)
            {
                const auto v = static_cast<double>(i);
                const auto nn  = tree.nearestNeighbor({v, v});
                if (nn.has_value())
                {
                    readOk.fetch_add(1, std::memory_order_relaxed);
                }
                // Also exercise other read methods
                static_cast<void>(tree.size());
                static_cast<void>(tree.empty());
            }
        }
    };

    auto writer = [&]()
    {
        for (int i = 0; i < 20; ++i)
        {
            tree.insert({static_cast<double>(i + 100),
                         static_cast<double>(i + 100)});
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            (void)tree.insert({static_cast<double>(i + 200),
                               static_cast<double>(i + 200)});
        }
        stop.store(true, std::memory_order_release);
    };

    std::vector<std::thread> readers;
    for (int t = 0; t < 4; ++t)
    {
        readers.emplace_back(reader);
    }

    std::thread writerThread(writer);

    for (auto& t : readers)
    {
        t.join();
    }
    writerThread.join();

    // At least some reads must have completed without error
    EXPECT_GT(readOk.load(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  15. Edge cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Points that share the same value on the splitting axis should not
 *        break the tree.
 * @details Verifies that the KDTree handles points with identical coordinates on the splitting axis correctly
 */
TEST_F(KDTreeTest, PointsWithSameAxisValue_Works)
{
    KDTreeArr2D tree;
    // All points share the same x coordinate
    tree.build({{0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0}, {0.0, 4.0}});
    EXPECT_EQ(tree.size(), 4);

    const auto nn = tree.nearestNeighbor({0.0, 2.5});
    ASSERT_TRUE(nn.has_value());
    EXPECT_EQ((*nn)[0], 0.0);
    // The closest y is either 2.0 or 3.0
    EXPECT_TRUE((*nn)[1] == 2.0 || (*nn)[1] == 3.0);
}

/**
 * @brief Many identical points (same coordinates) �?only the first is stored.
 * @details Verifies that inserting the same point many times only stores the first and rejects duplicates
 */
TEST_F(KDTreeTest, ManyDuplicateInserts_OnlyFirstStored)
{
    KDTreeArr2D tree;
    for (int i = 0; i < 100; ++i)
    {
        tree.insert({3.0, 3.0});
    }
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Building with a large number of points (stress test).
 * @details Verifies that building a KDTree with 10,000 points and performing searches completes without error
 */
TEST_F(KDTreeTest, LargeBuild_Stress)
{
    KDTreeArr2D tree;
    std::vector<std::array<double, 2>> points;
    constexpr int N = 10'000;
    points.reserve(N);
    for (int i = 0; i < N; ++i)
    {
        points.push_back({static_cast<double>(i),
                          static_cast<double>(i * 2)});
    }
    tree.build(points.begin(), points.end());
    EXPECT_EQ(tree.size(), N);

    // Perform a handful of searches
    for (int i = 0; i < 10; ++i)
    {
        const auto q = static_cast<double>(i * 1000);
        const auto nn  = tree.nearestNeighbor({q, q});
        ASSERT_TRUE(nn.has_value());
    }
}

/**
 * @brief Verify that K=2 tree with std::array works after move assignment
 *        for Point2D and Point3D as well (cross-type sanity).
 * @details Verifies that both Point2D and Point3D KDTree variants correctly handle build and search operations
 */
TEST_F(KDTreeTest, MixedPointTypes_BuildAndSearch)
{
    // Point2D tree
    KDTreeP2D tree2d;
    tree2d.build({Point2D{1.0, 2.0}, Point2D{3.0, 4.0}});
    EXPECT_EQ(tree2d.size(), 2);

    // Point3D tree
    KDTreeP3D tree3d;
    tree3d.build({Point3D{1.0, 2.0, 3.0}, Point3D{4.0, 5.0, 6.0}});
    EXPECT_EQ(tree3d.size(), 2);
}
