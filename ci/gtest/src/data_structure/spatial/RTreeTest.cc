/**
 * @file RTreeTest.cc
 * @brief Unit tests for R*-Tree spatial index and its BoundingBox dependency
 * @details Tests cover the BoundingBox geometric primitives, RTree insertion,
 *          range search, node splitting, deletion, K-nearest-neighbour queries,
 *          move semantics, edge cases, and basic thread safety.
 */

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "data_structure/spatial/RTree.hpp"

using namespace common::data_structure::spatial;

// ══════════════════════════════════════════════════════════════════════════
//  Type aliases for common variants used throughout the tests
// ══════════════════════════════════════════════════════════════════════════

/// 2-D bounding box with double precision (default instantiation).
using Box = BoundingBox<double, 2>;

/// 3-D bounding box with double precision.
using Box3 = BoundingBox<double, 3>;

/// Default RTree: 2-D, MaxEntries = 10, kMinEntries = 4, kReinsertCount = 3.
using Tree = RTree<int, 2, 10>;

/// Small-capacity tree for testing split behaviour: MaxEntries = 5,
/// kMinEntries = 2, kReinsertCount = 1.
using SmallTree = RTree<int, 2, 5>;

/// A 3-D RTree with default capacity.
using Tree3D = RTree<int, 3, 10>;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class RTreeTest : public testing::Test
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
//  1. BoundingBox — geometric primitives (dependency validation)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed BoundingBox has zero-initialised corners.
 * @details Both min and max should be zero arrays after default construction.
 */
TEST_F(RTreeTest, BoundingBox_DefaultConstructor)
{
    const Box box;
    for (size_t d = 0; d < 2; ++d)
    {
        EXPECT_EQ(box.min(d), 0.0);
        EXPECT_EQ(box.max(d), 0.0);
    }
}

/**
 * @brief Explicit min/max constructor stores the supplied corners correctly.
 * @details Verifies that the values passed to the constructor are returned
 *          by the corresponding accessors.
 */
TEST_F(RTreeTest, BoundingBox_ExplicitMinMax)
{
    const Box box({1.0, 2.0}, {5.0, 8.0});
    EXPECT_EQ(box.min(0), 1.0);
    EXPECT_EQ(box.min(1), 2.0);
    EXPECT_EQ(box.max(0), 5.0);
    EXPECT_EQ(box.max(1), 8.0);
}

/**
 * @brief Point constructor creates a degenerate box where min == max.
 * @details A box constructed from a single point collapses both corners
 *          to that point.
 */
TEST_F(RTreeTest, BoundingBox_PointConstructor)
{
    const Box box({3.0, 4.0});
    EXPECT_EQ(box.min(0), 3.0);
    EXPECT_EQ(box.min(1), 4.0);
    EXPECT_EQ(box.max(0), 3.0);
    EXPECT_EQ(box.max(1), 4.0);
}

/**
 * @brief contains(point) correctly identifies interior, boundary, and
 *        exterior points.
 * @details Tests a point clearly inside, a point on the box boundary,
 *          and a point outside the box.
 */
TEST_F(RTreeTest, BoundingBox_ContainsPoint)
{
    const Box box({1.0, 2.0}, {5.0, 8.0});
    EXPECT_TRUE(box.contains(std::array{3.0, 4.0}));       // inside
    EXPECT_TRUE(box.contains(std::array{1.0, 2.0}));       // on min corner
    EXPECT_TRUE(box.contains(std::array{5.0, 8.0}));       // on max corner
    EXPECT_TRUE(box.contains(std::array{1.0, 8.0}));       // on edge
    EXPECT_FALSE(box.contains(std::array{0.0, 4.0}));      // left
    EXPECT_FALSE(box.contains(std::array{6.0, 4.0}));      // right
    EXPECT_FALSE(box.contains(std::array{3.0, 1.0}));      // below
    EXPECT_FALSE(box.contains(std::array{3.0, 9.0}));      // above
}

/**
 * @brief contains(box) correctly detects full containment.
 * @details A smaller box fully inside a larger box is contained;
 *          a partially overlapping box is not.
 */
TEST_F(RTreeTest, BoundingBox_ContainsBox)
{
    const Box outer({1.0, 2.0}, {5.0, 8.0});
    const Box inner({2.0, 3.0}, {4.0, 7.0});
    const Box partial({3.0, 1.0}, {6.0, 5.0});
    const Box disjoint({10.0, 10.0}, {12.0, 12.0});

    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(outer.contains(partial));
    EXPECT_FALSE(outer.contains(disjoint));
}

/**
 * @brief intersects detects partial overlap, full containment, and
 *        disjoint boxes correctly.
 * @details Two boxes that share any volume intersect; two boxes that
 *          do not overlap in any dimension do not intersect.
 */
TEST_F(RTreeTest, BoundingBox_Intersects)
{
    const Box a({1.0, 2.0}, {5.0, 8.0});
    const Box overlapping({3.0, 4.0}, {7.0, 10.0});
    const Box inside({2.0, 3.0}, {4.0, 7.0});
    const Box disjoint({10.0, 10.0}, {12.0, 12.0});

    EXPECT_TRUE(a.intersects(overlapping));   // partial overlap
    EXPECT_TRUE(a.intersects(inside));        // contains
    EXPECT_TRUE(inside.intersects(a));        // symmetric
    EXPECT_FALSE(a.intersects(disjoint));     // no overlap
}

/**
 * @brief area computes the 2-D extent correctly.
 * @details For box ({1,2},{3,4}), area = (3-1)*(4-2) = 4.
 */
TEST_F(RTreeTest, BoundingBox_Area)
{
    const Box box({1.0, 2.0}, {3.0, 4.0});
    EXPECT_DOUBLE_EQ(box.area(), 4.0);
}

/**
 * @brief margin computes the sum of side lengths correctly.
 * @details For box ({1,2},{3,4}), margin = (3-1)+(4-2) = 4.
 */
TEST_F(RTreeTest, BoundingBox_Margin)
{
    const Box box({1.0, 2.0}, {3.0, 4.0});
    EXPECT_DOUBLE_EQ(box.margin(), 4.0);
}

/**
 * @brief enlarge extends the box to enclose another box.
 * @details After enlarging, the box should contain the argument.
 */
TEST_F(RTreeTest, BoundingBox_Enlarge)
{
    Box a({1.0, 2.0}, {3.0, 4.0});
    const Box b({0.0, 1.0}, {5.0, 6.0});
    a.enlarge(b);
    EXPECT_TRUE(a.contains(b));
    EXPECT_EQ(a.min(0), 0.0);
    EXPECT_EQ(a.min(1), 1.0);
    EXPECT_EQ(a.max(0), 5.0);
    EXPECT_EQ(a.max(1), 6.0);
}

/**
 * @brief combined returns the minimal enclosing box without modifying
 *        the original.
 * @details The original box should remain unchanged after combined().
 */
TEST_F(RTreeTest, BoundingBox_Combined)
{
    const Box a({1.0, 2.0}, {3.0, 4.0});
    const Box b({0.0, 1.0}, {5.0, 6.0});
    const Box c = a.combined(b);

    EXPECT_TRUE(c.contains(a));
    EXPECT_TRUE(c.contains(b));
    // a is unchanged
    EXPECT_EQ(a.min(0), 1.0);
    EXPECT_EQ(a.max(0), 3.0);
}

/**
 * @brief enlargement equals combinedArea( other ) - area().
 * @details Verifies the convenience method matches the definition.
 */
TEST_F(RTreeTest, BoundingBox_Enlargement)
{
    const Box a({1.0, 2.0}, {3.0, 4.0});
    const Box b({2.0, 3.0}, {5.0, 6.0});
    const double expectedEnl = a.combinedArea(b) - a.area();
    EXPECT_DOUBLE_EQ(a.enlargement(b), expectedEnl);
}

/**
 * @brief minDistSquared returns 0 when the query point is inside the box.
 * @details A point inside the box has zero minimum distance.
 */
TEST_F(RTreeTest, BoundingBox_MinDistSquared_Inside)
{
    const Box box({1.0, 2.0}, {5.0, 8.0});
    EXPECT_DOUBLE_EQ(box.minDistSquared(std::array{3.0, 4.0}), 0.0);
    EXPECT_DOUBLE_EQ(box.minDistSquared(std::array{1.0, 2.0}), 0.0);  // on edge
}

/**
 * @brief minDistSquared computes correct squared distance for an exterior
 *        point.
 * @details For point (6,9) outside box ({1,2},{5,8}):
 *          dx = 6-5 = 1, dy = 9-8 = 1, dist² = 2.
 */
TEST_F(RTreeTest, BoundingBox_MinDistSquared_Outside)
{
    const Box box({1.0, 2.0}, {5.0, 8.0});
    const double expected = (6.0 - 5.0) * (6.0 - 5.0) + (9.0 - 8.0) * (9.0 - 8.0);
    EXPECT_DOUBLE_EQ(box.minDistSquared(std::array{6.0, 9.0}), expected);
}

/**
 * @brief center returns the geometric centre of the box.
 * @details For box ({1,2},{3,4}), centre = ((1+3)/2, (2+4)/2) = (2, 3).
 */
TEST_F(RTreeTest, BoundingBox_Center)
{
    const Box box({1.0, 2.0}, {3.0, 4.0});
    const auto c = box.center();
    EXPECT_DOUBLE_EQ(c[0], 2.0);
    EXPECT_DOUBLE_EQ(c[1], 3.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Basic insertion and tree state
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A default-constructed empty tree reports size=0, empty()=true,
 *        and height()=0.
 * @details Verifies the initial state of a freshly created RTree.
 */
TEST_F(RTreeTest, Empty_Tree_SizeZero_EmptyTrue)
{
    const Tree tree;
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.height(), 0);
}

/**
 * @brief Inserting a single item yields size=1 and the item is
 *        discoverable via search.
 * @details Verifies the simplest possible insertion scenario.
 */
TEST_F(RTreeTest, Insert_SingleItem)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 42);
    EXPECT_EQ(tree.size(), 1);
    EXPECT_FALSE(tree.empty());

    const auto results = tree.search(Box({0.0, 0.0}, {3.0, 3.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 42);
}

/**
 * @brief Inserting up to MaxEntries items (below capacity) works and all
 *        items remain discoverable.
 * @details Uses a tree with MaxEntries=10 and inserts 9 items.
 */
TEST_F(RTreeTest, Insert_MultipleItems_BelowCapacity)
{
    Tree tree;
    constexpr int kCount = 9;
    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 3);
        tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), i);
    }

    EXPECT_EQ(tree.size(), kCount);
    EXPECT_FALSE(tree.empty());

    // A large query box covers all items.
    const auto results = tree.search(Box({0.0, 0.0}, {30.0, 2.0}));
    ASSERT_EQ(results.size(), static_cast<size_t>(kCount));

    // Verify all IDs are present (order may vary).
    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
    }
}

/**
 * @brief Multiple items sharing the same bounding box are all stored
 *        and returned by search.
 * @details RTree is a multi-map — it does not deduplicate entries.
 */
TEST_F(RTreeTest, Insert_ItemsWithSameBBox)
{
    Tree tree;
    const Box sharedBox({0.0, 0.0}, {1.0, 1.0});
    tree.insert(sharedBox, 10);
    tree.insert(sharedBox, 20);
    tree.insert(sharedBox, 30);

    EXPECT_EQ(tree.size(), 3);

    const auto results = tree.search(Box({-1.0, -1.0}, {2.0, 2.0}));
    ASSERT_EQ(results.size(), 3);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 10);
    EXPECT_EQ(sorted[1], 20);
    EXPECT_EQ(sorted[2], 30);
}

/**
 * @brief Insert using move semantics stores the data correctly.
 * @details The rvalue-reference overload of insert() should work
 *          identically to the copy overload.
 */
TEST_F(RTreeTest, Insert_MoveSemantics)
{
    Tree tree;
    std::string src("hello");
    // Use a tree with std::string data type.
    RTree<std::string, 2, 10> strTree;
    strTree.insert(Box({1.0, 1.0}, {2.0, 2.0}), std::move(src));

    EXPECT_EQ(strTree.size(), 1);

    const auto results = strTree.search(Box({0.0, 0.0}, {3.0, 3.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "hello");
}

/**
 * @brief RTree works correctly in 3 dimensions.
 * @details Constructs a 3-D tree, inserts an item, and retrieves it
 *          via search using a 3-D bounding box.
 */
TEST_F(RTreeTest, Insert_3D_RTree)
{
    Tree3D tree;
    const Box3 box3({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0});
    tree.insert(box3, 99);

    EXPECT_EQ(tree.size(), 1);

    const auto results = tree.search(Box3({0.0, 0.0, 0.0}, {5.0, 5.0, 6.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 99);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Range search
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Searching an empty tree returns an empty vector.
 * @details No crash or error — just an empty result.
 */
TEST_F(RTreeTest, Search_EmptyTree_ReturnsEmpty)
{
    const Tree tree;
    const auto results = tree.search(Box({0.0, 0.0}, {10.0, 10.0}));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Searching with the exact same box as inserted finds the item.
 * @details An exact box match is also an intersection.
 */
TEST_F(RTreeTest, Search_ExactMatch)
{
    Tree tree;
    tree.insert(Box({2.0, 3.0}, {4.0, 5.0}), 77);

    const auto results = tree.search(Box({2.0, 3.0}, {4.0, 5.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 77);
}

/**
 * @brief A query box that partially overlaps the item's box finds it.
 * @details Intersection does not require full containment.
 */
TEST_F(RTreeTest, Search_PartialOverlap)
{
    Tree tree;
    tree.insert(Box({5.0, 5.0}, {10.0, 10.0}), 33);

    // Query only covers the top-right portion of the item's box.
    const auto results = tree.search(Box({8.0, 8.0}, {12.0, 12.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 33);
}

/**
 * @brief A query box that does not intersect any stored box returns empty.
 * @details Verifies that non-overlapping queries correctly produce no results.
 */
TEST_F(RTreeTest, Search_NoMatch)
{
    Tree tree;
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 1);

    const auto results = tree.search(Box({10.0, 10.0}, {20.0, 20.0}));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief A large query box covering all stored items returns every item.
 * @details Verifies full-coverage search returns the entire dataset.
 */
TEST_F(RTreeTest, Search_FullCoverage)
{
    Tree tree;
    for (int i = 0; i < 8; ++i)
    {
        const double x = static_cast<double>(i * 3);
        tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), i);
    }

    const auto results = tree.search(Box({-10.0, -10.0}, {100.0, 100.0}));
    ASSERT_EQ(results.size(), 8);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
    }
}

/**
 * @brief Search works correctly with a non-default MaxEntries parameter.
 * @details Uses a SmallTree (MaxEntries=5) to verify the template
 *          handles different capacity values.
 */
TEST_F(RTreeTest, Search_NonDefaultMaxEntries)
{
    SmallTree tree;
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 10);
    tree.insert(Box({2.0, 0.0}, {3.0, 1.0}), 20);

    const auto results = tree.search(Box({-1.0, -1.0}, {4.0, 2.0}));
    ASSERT_EQ(results.size(), 2);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 10);
    EXPECT_EQ(sorted[1], 20);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Node splitting
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting MaxEntries+1 items (all spatially distinct) triggers
 *        a node split; all items remain searchable.
 * @details With MaxEntries=10, inserting 11 items causes the root leaf
 *          to overflow and split.  After the split, the tree should have
 *          height >= 2 and all items are accessible.
 */
TEST_F(RTreeTest, Split_TriggeredAtCapacity)
{
    Tree tree;
    constexpr int kCount = 11;
    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 3);
        tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), i);
    }

    EXPECT_EQ(tree.size(), kCount);
    EXPECT_GE(tree.height(), 2);  // split should have occurred

    // All items must be discoverable.
    const auto results = tree.search(Box({-10.0, -10.0}, {100.0, 100.0}));
    ASSERT_EQ(results.size(), static_cast<size_t>(kCount));

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
    }
}

/**
 * @brief Inserting a large number of items (50) triggers multiple splits
 *        and produces a multi-level tree.
 * @details After many splits the tree height should exceed 1, and every
 *          inserted item must still be found by search.
 */
TEST_F(RTreeTest, Split_MultipleSplits)
{
    Tree tree;
    constexpr int kRows = 5;
    constexpr int kCols = 10;
    constexpr int kCount = kRows * kCols;

    for (int row = 0; row < kRows; ++row)
    {
        for (int col = 0; col < kCols; ++col)
        {
            const int id = row * kCols + col;
            const double x = static_cast<double>(col * 3);
            const double y = static_cast<double>(row * 3);
            tree.insert(Box({x, y}, {x + 1.0, y + 1.0}), id);
        }
    }

    EXPECT_EQ(tree.size(), kCount);
    EXPECT_GE(tree.height(), 2);  // multiple splits → height > 1

    // Large query covering all items.
    const auto results = tree.search(Box({-1.0, -1.0}, {31.0, 16.0}));
    ASSERT_EQ(results.size(), static_cast<size_t>(kCount));

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
    }
}

/**
 * @brief After splitting, each item's individual bounding box search
 *        returns exactly that item (no false positives when boxes are
 *        non-overlapping).
 * @details Uses non-overlapping boxes so that searching with the exact
 *          box yields exactly one result.
 */
TEST_F(RTreeTest, Split_DataIntegrity)
{
    Tree tree;
    constexpr int kCount = 15;  // enough to force multiple splits
    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 4);
        tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), i);
    }

    EXPECT_EQ(tree.size(), kCount);

    // Each item should be individually findable via its exact bounding box.
    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 4);
        const Box itemBox({x, 0.0}, {x + 1.0, 1.0});
        const auto results = tree.search(itemBox);
        ASSERT_FALSE(results.empty()) << "Item " << i << " not found";

        bool found = false;
        for (const int val : results)
        {
            if (val == i)
            {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Item " << i << " missing from search results";
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Deletion
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Removing an existing entry returns true and decrements the size.
 * @details Verifies the basic remove-contract.
 */
TEST_F(RTreeTest, Remove_ExistingEntry)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 42);
    ASSERT_EQ(tree.size(), 1);

    const bool removed = tree.remove(Box({1.0, 1.0}, {2.0, 2.0}), 42);
    EXPECT_TRUE(removed);
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

/**
 * @brief Removing a non-existent entry returns false and does not change
 *        the tree.
 * @details The data value not found or the box not matching both should
 *          result in a false return.
 */
TEST_F(RTreeTest, Remove_NonExistingEntry)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 42);

    const bool removed = tree.remove(Box({10.0, 10.0}, {20.0, 20.0}), 99);
    EXPECT_FALSE(removed);
    EXPECT_EQ(tree.size(), 1);  // unchanged
}

/**
 * @brief Removing all entries one by one eventually yields an empty tree.
 * @details Inserts 5 items (well below MaxEntries so no internal
 *          underflow complications) and removes each, verifying
 *          size decreases monotonically.
 */
TEST_F(RTreeTest, Remove_AllEntries)
{
    Tree tree;
    constexpr int kCount = 5;
    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 3);
        tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), i);
    }
    ASSERT_EQ(tree.size(), kCount);

    for (int i = 0; i < kCount; ++i)
    {
        const double x = static_cast<double>(i * 3);
        const bool removed = tree.remove(Box({x, 0.0}, {x + 1.0, 1.0}), i);
        EXPECT_TRUE(removed);
        EXPECT_EQ(tree.size(), static_cast<size_t>(kCount - 1 - i));
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief After removal, a search for the deleted entry's box no longer
 *        contains that entry.
 * @details Other entries remain unaffected.
 */
TEST_F(RTreeTest, Remove_ThenSearch)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 10);
    tree.insert(Box({10.0, 10.0}, {11.0, 11.0}), 20);

    ASSERT_TRUE(tree.remove(Box({1.0, 1.0}, {2.0, 2.0}), 10));

    // Search for the removed item's box should be empty.
    const auto resRemoved = tree.search(Box({1.0, 1.0}, {2.0, 2.0}));
    EXPECT_TRUE(resRemoved.empty());

    // The other item should still be findable.
    const auto resRemaining = tree.search(Box({10.0, 10.0}, {11.0, 11.0}));
    ASSERT_EQ(resRemaining.size(), 1);
    EXPECT_EQ(resRemaining[0], 20);
}

/**
 * @brief Remove works with a non-default MaxEntries tree (SmallTree).
 * @details Verifies that the remove logic handles different node sizes.
 */
TEST_F(RTreeTest, Remove_FromNonDefaultTree)
{
    SmallTree tree;
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 100);
    tree.insert(Box({2.0, 0.0}, {3.0, 1.0}), 200);

    EXPECT_TRUE(tree.remove(Box({0.0, 0.0}, {1.0, 1.0}), 100));
    EXPECT_EQ(tree.size(), 1);

    const auto results = tree.search(Box({-10.0, -10.0}, {10.0, 10.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 200);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. K-nearest-neighbour search
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief nearestNeighbor on an empty tree returns an empty vector.
 * @details k is irrelevant when the tree has no entries.
 */
TEST_F(RTreeTest, NearestNeighbor_EmptyTree)
{
    const Tree tree;
    const auto result = tree.nearestNeighbor(std::array{1.0, 1.0}, 5);
    EXPECT_TRUE(result.empty());
}

/**
 * @brief nearestNeighbor with a single item returns that item with
 *        distance 0 (query point inside the item's box).
 * @details Distance is Euclidean to the box interior.
 */
TEST_F(RTreeTest, NearestNeighbor_SingleItem)
{
    Tree tree;
    tree.insert(Box({0.0, 0.0}, {2.0, 2.0}), 42);

    const auto result = tree.nearestNeighbor(std::array{1.0, 1.0}, 1);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].first, 42);
    EXPECT_DOUBLE_EQ(result[0].second, 0.0);
}

/**
 * @brief Requesting k=1 returns the single closest entry.
 * @details Verifies that the nearest of multiple candidates is found.
 */
TEST_F(RTreeTest, NearestNeighbor_KEqualsOne)
{
    Tree tree;
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 10);
    tree.insert(Box({100.0, 100.0}, {101.0, 101.0}), 20);

    // Query near the first box.
    const auto result = tree.nearestNeighbor(std::array{0.5, 0.5}, 1);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].first, 10);
}

/**
 * @brief Requesting k=3 returns three results sorted by increasing
 *        distance.
 * @details Verifies ordering and correct element selection.
 */
TEST_F(RTreeTest, NearestNeighbor_KMultiple)
{
    Tree tree;
    // Items at increasing distances from origin.
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 1);     // closest
    tree.insert(Box({4.0, 4.0}, {5.0, 5.0}), 2);     // medium
    tree.insert(Box({9.0, 9.0}, {10.0, 10.0}), 3);   // farthest

    const auto result = tree.nearestNeighbor(std::array{0.0, 0.0}, 3);
    ASSERT_EQ(result.size(), 3);

    // Results must be in increasing distance order.
    EXPECT_LE(result[0].second, result[1].second);
    EXPECT_LE(result[1].second, result[2].second);

    EXPECT_EQ(result[0].first, 1);
    EXPECT_EQ(result[1].first, 2);
    EXPECT_EQ(result[2].first, 3);
}

/**
 * @brief When the query point falls inside a box, the distance for that
 *        entry is 0 and it appears first.
 * @details minDistSquared inside a box is always 0.
 */
TEST_F(RTreeTest, NearestNeighbor_PointInsideBox)
{
    Tree tree;
    tree.insert(Box({0.0, 0.0}, {5.0, 5.0}), 100);
    tree.insert(Box({10.0, 10.0}, {15.0, 15.0}), 200);

    // Query point (3,3) is inside the first box → distance 0.
    const auto result = tree.nearestNeighbor(std::array{3.0, 3.0}, 2);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].first, 100);
    EXPECT_DOUBLE_EQ(result[0].second, 0.0);
}

/**
 * @brief When k exceeds the total number of entries, all entries are
 *        returned (in distance order).
 * @details The result size equals tree size, not k.
 */
TEST_F(RTreeTest, NearestNeighbor_KGreaterThanSize)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 10);
    tree.insert(Box({3.0, 3.0}, {4.0, 4.0}), 20);

    const auto result = tree.nearestNeighbor(std::array{0.0, 0.0}, 100);
    ASSERT_EQ(result.size(), 2);
    // Both entries returned, closest first.
    EXPECT_EQ(result[0].first, 10);
    EXPECT_EQ(result[1].first, 20);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Boundary & edge cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief clear() on a non-empty tree removes all entries.
 * @details After clear, size=0, empty()=true, and searches return empty.
 */
TEST_F(RTreeTest, Clear_NonEmptyTree)
{
    Tree tree;
    tree.insert(Box({1.0, 1.0}, {2.0, 2.0}), 1);
    tree.insert(Box({3.0, 3.0}, {4.0, 4.0}), 2);
    tree.insert(Box({5.0, 5.0}, {6.0, 6.0}), 3);
    ASSERT_EQ(tree.size(), 3);

    tree.clear();

    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(tree.search(Box({0.0, 0.0}, {10.0, 10.0})).empty());
}

/**
 * @brief clear() on an already empty tree is safe and idempotent.
 * @details No crash, no error, state remains empty.
 */
TEST_F(RTreeTest, Clear_EmptyTree)
{
    Tree tree;
    EXPECT_NO_THROW(tree.clear());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

/**
 * @brief Move constructor transfers all state to the new tree; the
 *        source becomes empty.
 * @details After moving, the target has the items, the source is empty,
 *          and the target is fully functional.
 */
TEST_F(RTreeTest, MoveConstructor_TransfersState)
{
    Tree original;
    original.insert(Box({1.0, 1.0}, {2.0, 2.0}), 10);
    original.insert(Box({3.0, 3.0}, {4.0, 4.0}), 20);
    ASSERT_EQ(original.size(), 2);

    Tree moved(std::move(original));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(original.empty());

    // Moved tree is functional.
    const auto results = moved.search(Box({0.0, 0.0}, {5.0, 5.0}));
    ASSERT_EQ(results.size(), 2);
}

/**
 * @brief Move assignment transfers state; the source becomes empty.
 * @details Verifies the move-assignment operator.
 */
TEST_F(RTreeTest, MoveAssignment_TransfersState)
{
    Tree source;
    source.insert(Box({1.0, 1.0}, {2.0, 2.0}), 100);

    Tree dest;
    dest.insert(Box({9.0, 9.0}, {10.0, 10.0}), 200);

    dest = std::move(source);

    EXPECT_EQ(dest.size(), 1);
    EXPECT_TRUE(source.empty());

    const auto results = dest.search(Box({0.0, 0.0}, {3.0, 3.0}));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 100);
}

/**
 * @brief Insert → Remove → Insert round-trip works correctly.
 * @details After a cycle of insertions and removals, the tree continues
 *          to function and has the expected state.
 */
TEST_F(RTreeTest, Insert_Remove_Insert_Roundtrip)
{
    Tree tree;

    // Phase 1: insert.
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 1);
    tree.insert(Box({2.0, 2.0}, {3.0, 3.0}), 2);
    tree.insert(Box({4.0, 4.0}, {5.0, 5.0}), 3);
    ASSERT_EQ(tree.size(), 3);

    // Phase 2: remove all.
    EXPECT_TRUE(tree.remove(Box({0.0, 0.0}, {1.0, 1.0}), 1));
    EXPECT_TRUE(tree.remove(Box({2.0, 2.0}, {3.0, 3.0}), 2));
    EXPECT_TRUE(tree.remove(Box({4.0, 4.0}, {5.0, 5.0}), 3));
    EXPECT_TRUE(tree.empty());

    // Phase 3: insert again.
    tree.insert(Box({10.0, 10.0}, {11.0, 11.0}), 77);
    tree.insert(Box({12.0, 12.0}, {13.0, 13.0}), 88);

    EXPECT_EQ(tree.size(), 2);

    const auto results = tree.search(Box({9.0, 9.0}, {14.0, 14.0}));
    ASSERT_EQ(results.size(), 2);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 77);
    EXPECT_EQ(sorted[1], 88);
}

/**
 * @brief Large-scale insertion (100 items) followed by multiple searches
 *        — stress test for structural integrity.
 * @details Inserts items on a 10×10 grid, then runs 10 search queries
 *          on various regions.  Every search must complete without error
 *          and the full-coverage query must return all 100 items.
 */
TEST_F(RTreeTest, LargeScale_InsertThenSearch)
{
    Tree tree;
    constexpr int kGridSize = 10;
    constexpr int kCount = kGridSize * kGridSize;

    for (int row = 0; row < kGridSize; ++row)
    {
        for (int col = 0; col < kGridSize; ++col)
        {
            const int id = row * kGridSize + col;
            const double x = static_cast<double>(col * 3);
            const double y = static_cast<double>(row * 3);
            tree.insert(Box({x, y}, {x + 1.0, y + 1.0}), id);
        }
    }

    ASSERT_EQ(tree.size(), kCount);

    // Full coverage.
    {
        const auto all = tree.search(Box({-10.0, -10.0}, {100.0, 100.0}));
        ASSERT_EQ(all.size(), kCount);
        std::vector<int> sorted = all;
        std::sort(sorted.begin(), sorted.end());
        for (int i = 0; i < kCount; ++i)
        {
            EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
        }
    }

    // 10 region queries scattered across the grid.
    const std::array<Box, 10> queries = {{
        Box({-1.0, -1.0}, {5.0, 5.0}),     // top-left quadrant
        Box({15.0, -1.0}, {25.0, 5.0}),     // top-middle
        Box({25.0, -1.0}, {35.0, 10.0}),    // top-right
        Box({-1.0, 10.0}, {10.0, 20.0}),    // middle-left
        Box({10.0, 10.0}, {20.0, 20.0}),    // centre
        Box({20.0, 10.0}, {35.0, 20.0}),    // middle-right
        Box({-1.0, 20.0}, {10.0, 30.0}),    // bottom-left
        Box({10.0, 20.0}, {20.0, 30.0}),    // bottom-centre
        Box({20.0, 20.0}, {35.0, 30.0}),    // bottom-right
        Box({0.0, 0.0}, {30.0, 30.0}),      // large centre
    }};

    for (const auto& q : queries)
    {
        const auto results = tree.search(q);
        // Every query should intersect at least one item.
        EXPECT_FALSE(results.empty()) << "Query box (" << q.min(0) << ","
                                      << q.min(1) << ") returned no results";
        // All results must be valid IDs.
        for (const int val : results)
        {
            EXPECT_GE(val, 0);
            EXPECT_LT(val, kCount);
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Thread safety (basic level)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent insertions from multiple threads are safe and
 *        produce the correct final count.
 * @details Two threads each insert 25 non-overlapping items.  After
 *          joining, the tree should contain exactly 50 items.
 */
TEST_F(RTreeTest, ConcurrentInsert)
{
    Tree tree;
    constexpr int kItemsPerThread = 25;
    constexpr int kNumThreads = 2;
    constexpr int kTotal = kItemsPerThread * kNumThreads;

    auto worker = [&tree](int threadId)
    {
        for (int i = 0; i < kItemsPerThread; ++i)
        {
            const int id = threadId * kItemsPerThread + i;
            const double x = static_cast<double>(id * 4);
            tree.insert(Box({x, 0.0}, {x + 1.0, 1.0}), id);
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < kNumThreads; ++t)
    {
        threads.emplace_back(worker, t);
    }
    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(tree.size(), kTotal);

    // Verify all inserted items are searchable.
    const auto results = tree.search(Box({-10.0, -10.0}, {200.0, 200.0}));
    ASSERT_EQ(results.size(), kTotal);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < kTotal; ++i)
    {
        EXPECT_EQ(sorted[static_cast<size_t>(i)], i);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  9. k = 0 edge case for nearestNeighbor
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Requesting k = 0 returns an empty vector even when the tree
 *        contains items.
 * @details Zero neighbours is a valid request and should not error.
 */
TEST_F(RTreeTest, NearestNeighbor_KIsZero_ReturnsEmpty)
{
    Tree tree;
    tree.insert(Box({0.0, 0.0}, {1.0, 1.0}), 42);
    const auto result = tree.nearestNeighbor(std::array{0.0, 0.0}, 0);
    EXPECT_TRUE(result.empty());
}
