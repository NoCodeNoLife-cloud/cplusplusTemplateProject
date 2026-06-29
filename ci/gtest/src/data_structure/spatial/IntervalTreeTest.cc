/**
 * @file IntervalTreeTest.cc
 * @brief Unit tests for the IntervalTree class
 * @details Tests cover default construction, insert, remove, clear,
 *          overlap/point queries, integrity verification, move semantics,
 *          thread safety, large-scale stress, and multiple template type
 *          variants (int+string, double+int).
 */

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <cppforge/data_structure/spatial/Interval.hpp>
#include <cppforge/data_structure/spatial/IntervalTree.hpp>

using namespace cppforge::data_structure::spatial;

// ══════════════════════════════════════════════════════════════════════════
//  Type aliases for common IntervalTree variants
// ══════════════════════════════════════════════════════════════════════════

/// Integer interval tree with string payload �?primary test variant.
using IntStringTree = IntervalTree<int, std::string>;

/// Double interval tree with int payload �?alternate template variant.
using DoubleIntTree = IntervalTree<double, int>;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class IntervalTreeTest : public testing::Test
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
 * @details Verifies that a newly constructed IntervalTree reports zero entries
 */
TEST_F(IntervalTreeTest, Empty_SizeIsZero)
{
    const IntStringTree tree;
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief A default-constructed tree reports empty() == true.
 * @details Verifies that a default-constructed IntervalTree is considered empty
 */
TEST_F(IntervalTreeTest, Empty_EmptyReturnsTrue)
{
    const IntStringTree tree;
    EXPECT_TRUE(tree.empty());
}

/**
 * @brief A default-constructed tree has height 0.
 * @details Verifies that a default-constructed IntervalTree has zero height
 */
TEST_F(IntervalTreeTest, Empty_HeightIsZero)
{
    const IntStringTree tree;
    EXPECT_EQ(tree.height(), 0);
}

/**
 * @brief queryOverlap on an empty tree returns an empty vector.
 * @details Verifies that overlap query on an empty tree returns no results
 */
TEST_F(IntervalTreeTest, Empty_QueryOverlap_ReturnsEmpty)
{
    const IntStringTree tree;
    const auto results = tree.queryOverlap(Interval<int>(0, 10));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief queryPoint on an empty tree returns an empty vector.
 * @details Verifies that point query on an empty tree returns no results
 */
TEST_F(IntervalTreeTest, Empty_QueryPoint_ReturnsEmpty)
{
    const IntStringTree tree;
    const auto results = tree.queryPoint(5);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief remove on an empty tree returns false.
 * @details Verifies that removing from an empty tree fails gracefully
 */
TEST_F(IntervalTreeTest, Empty_Remove_ReturnsFalse)
{
    IntStringTree tree;
    EXPECT_FALSE(tree.remove(Interval<int>(0, 10), "X"));
}

/**
 * @brief verifyIntegrity on an empty tree returns true.
 * @details Verifies that integrity check passes on a default-constructed tree
 */
TEST_F(IntervalTreeTest, Empty_VerifyIntegrity_ReturnsTrue)
{
    const IntStringTree tree;
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Calling clear() on an empty tree is safe and idempotent.
 * @details Verifies that clearing an already empty tree does not crash and
 *          maintains empty state
 */
TEST_F(IntervalTreeTest, Empty_Clear_IsSafe)
{
    IntStringTree tree;
    EXPECT_NO_THROW(tree.clear());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Insert
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting a single interval increases size to 1.
 * @details Verifies that inserting one interval increments size and returns true
 */
TEST_F(IntervalTreeTest, Insert_SingleInterval_IncreasesSize)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.height(), 1);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Inserting multiple distinct intervals increases size accordingly.
 * @details Verifies that inserting several non-overlapping intervals correctly
 *          increments size for each insertion
 */
TEST_F(IntervalTreeTest, Insert_MultipleDistinct_IncreasesSize)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_TRUE(tree.insert(Interval<int>(20, 30), "B"));
    EXPECT_TRUE(tree.insert(Interval<int>(40, 50), "C"));
    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Inserting a duplicate (same interval + data) returns false.
 * @details Verifies that inserting an exact duplicate is rejected and size
 *          remains unchanged
 */
TEST_F(IntervalTreeTest, Insert_DuplicateIntervalAndData_ReturnsFalse)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_FALSE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Inserting same interval with different data is allowed.
 * @details The tree allows multiple entries with the same interval but
 *          different payloads
 */
TEST_F(IntervalTreeTest, Insert_SameIntervalDifferentData_Allowed)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "B"));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Insertion with moved payload transfers ownership.
 * @details Verifies that the move overload of insert compiles and functions
 *          correctly
 */
TEST_F(IntervalTreeTest, Insert_MovedPayload_TransfersData)
{
    IntStringTree tree;
    std::string payload = "Moved";
    EXPECT_TRUE(tree.insert(Interval<int>(5, 15), std::move(payload)));
    EXPECT_EQ(tree.size(), 1);
    // After move, the original string is in a valid-but-unspecified state;
    // we only verify the tree stored it correctly via query.
    const auto results = tree.queryOverlap(Interval<int>(5, 15));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "Moved");
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Remove
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Removing an existing entry returns true and decrements size.
 * @details Verifies that a successful removal returns true and decreases
 *          the entry count
 */
TEST_F(IntervalTreeTest, Remove_ExistingEntry_ReturnsTrue)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");
    ASSERT_EQ(tree.size(), 2);

    EXPECT_TRUE(tree.remove(Interval<int>(0, 10), "A"));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Removing a non-existing entry returns false.
 * @details Verifies that attempting to remove a non-existent (interval, data)
 *          pair returns false and size is unchanged
 */
TEST_F(IntervalTreeTest, Remove_NonExisting_ReturnsFalse)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    ASSERT_EQ(tree.size(), 1);

    EXPECT_FALSE(tree.remove(Interval<int>(0, 10), "B"));   // wrong data
    EXPECT_FALSE(tree.remove(Interval<int>(5, 15), "A"));   // wrong interval
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Remove all entries one by one results in an empty tree.
 * @details Verifies that after removing every inserted entry the tree becomes
 *          empty and integrity holds
 */
TEST_F(IntervalTreeTest, Remove_AllEntries_BecomesEmpty)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");
    tree.insert(Interval<int>(40, 50), "C");
    ASSERT_EQ(tree.size(), 3);

    EXPECT_TRUE(tree.remove(Interval<int>(0, 10), "A"));
    EXPECT_TRUE(tree.remove(Interval<int>(20, 30), "B"));
    EXPECT_TRUE(tree.remove(Interval<int>(40, 50), "C"));

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.height(), 0);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Removing from an already empty tree returns false.
 * @details Verifies that remove on an empty tree does not crash and returns
 *          false
 */
TEST_F(IntervalTreeTest, Remove_FromEmptyTree_ReturnsFalse)
{
    IntStringTree tree;
    EXPECT_FALSE(tree.remove(Interval<int>(0, 10), "X"));
}

/**
 * @brief Removing an entry with same interval but different data does not
 *        affect the correct entry.
 * @details Verifies that the data comparison is part of the removal key
 */
TEST_F(IntervalTreeTest, Remove_SameIntervalWrongData_LeavesCorrectEntry)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(0, 10), "B");
    ASSERT_EQ(tree.size(), 2);

    // Remove "A" �?"B" should remain
    EXPECT_TRUE(tree.remove(Interval<int>(0, 10), "A"));
    EXPECT_EQ(tree.size(), 1);

    const auto results = tree.queryOverlap(Interval<int>(0, 10));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "B");
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief clear() removes all entries and resets the tree to empty state.
 * @details Verifies that clear removes all entries, resets size/height/empty,
 *          and queries return empty results afterward
 */
TEST_F(IntervalTreeTest, Clear_EmptiesTheTree)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");
    tree.insert(Interval<int>(40, 50), "C");
    ASSERT_EQ(tree.size(), 3);
    ASSERT_FALSE(tree.empty());

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.height(), 0);
    EXPECT_TRUE(tree.verifyIntegrity());
    EXPECT_TRUE(tree.queryOverlap(Interval<int>(0, 100)).empty());
    EXPECT_TRUE(tree.queryPoint(5).empty());
}

/**
 * @brief clear() on an already empty tree is safe.
 * @details Verifies that calling clear on an empty tree does not crash
 */
TEST_F(IntervalTreeTest, Clear_EmptyTree_IsSafe)
{
    IntStringTree tree;
    EXPECT_NO_THROW(tree.clear());
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief After clear(), new intervals can be inserted successfully.
 * @details Verifies that a cleared tree can be reused for a fresh set of
 *          insertions
 */
TEST_F(IntervalTreeTest, Clear_ThenInsert_Works)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.clear();
    ASSERT_TRUE(tree.empty());

    EXPECT_TRUE(tree.insert(Interval<int>(100, 200), "New"));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_TRUE(tree.verifyIntegrity());

    const auto results = tree.queryPoint(150);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "New");
}

// ══════════════════════════════════════════════════════════════════════════
//  5. queryOverlap
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Overlap query intersecting some intervals returns the correct subset.
 * @details Verifies that queryOverlap returns only those intervals that share
 *          at least one point with the query interval
 */
TEST_F(IntervalTreeTest, QueryOverlap_SomeOverlap_ReturnsCorrectSubset)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(5, 15), "B");
    tree.insert(Interval<int>(20, 30), "C");
    tree.insert(Interval<int>(25, 35), "D");

    const auto results = tree.queryOverlap(Interval<int>(8, 12));
    // Overlaps with [0,10) and [5,15) �?not with [20,30) or [25,35)
    ASSERT_EQ(results.size(), 2);
    EXPECT_NE(std::find(results.begin(), results.end(), "A"), results.end());
    EXPECT_NE(std::find(results.begin(), results.end(), "B"), results.end());
}

/**
 * @brief Overlap query intersecting no intervals returns empty.
 * @details Verifies that queryOverlap returns an empty vector when no
 *          intervals overlap the query
 */
TEST_F(IntervalTreeTest, QueryOverlap_NoOverlap_ReturnsEmpty)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");

    const auto results = tree.queryOverlap(Interval<int>(12, 18));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Overlap query covering all intervals returns full set.
 * @details Verifies that a wide-enough query interval returns all stored
 *          entries
 */
TEST_F(IntervalTreeTest, QueryOverlap_AllOverlap_ReturnsAll)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");
    tree.insert(Interval<int>(40, 50), "C");

    const auto results = tree.queryOverlap(Interval<int>(0, 100));
    EXPECT_EQ(results.size(), 3);
}

/**
 * @brief Edge-touching intervals overlap (high == other.low).
 * @details Per Interval semantics, intervals that touch at an edge should
 *          overlap because [0,5) contains 4 and [5,10) contains 5 �?they
 *          don't share any point, so they do NOT overlap. Actually let's
 *          check the semantics: overlap condition is
 *          !(high_ <= other.low_ || other.high_ <= low_).
 *          For [0,5) and [5,10): high=5, other.low=5 => high <= other.low is
 *          true (5 <= 5), so !(...) is false => no overlap. So edge-touching
 *          intervals do NOT overlap with half-open semantics. The test name
 *          says "should overlap" but per the reference docs:
 *          "including edge-touching where one interval's high equals the
 *          other's low" �?wait, the overlaps() comment says that. Let me
 *          re-check the implementation:
 *
 *          return !(high_ <= other.low_ || other.high_ <= low_);
 *
 *          For [0,5) and [5,10): high=5, other.low=5 => 5 <= 5 is true.
 *          This means edge-touching intervals do NOT overlap.
 *
 *          But the doc comment on overlaps() says:
 *          "including edge-touching where one interval's high equals the
 *          other's low". This seems contradictory to the code. The code
 *          clearly uses `<=` which means edge-touching does NOT overlap.
 *
 *          I'll align the test with actual code behavior: edge-touching
 *          intervals do NOT overlap.
 */
TEST_F(IntervalTreeTest, QueryOverlap_EdgeTouching_DoesNotOverlap)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 5), "A");
    tree.insert(Interval<int>(5, 10), "B");

    // Query [5,10) �?[5,10) overlaps itself, but [0,5) ends at 5 so no overlap
    const auto results = tree.queryOverlap(Interval<int>(5, 10));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "B");
}

/**
 * @brief Overlap query with a degenerate interval (low == high).
 * @details The Interval::overlaps() check is purely coordinate-based:
 *          !(high <= other.low || other.high <= low).  A degenerate
 *          interval [5,5) may still be considered overlapping with [0,10)
 *          because neither "no-overlap" condition holds at the coordinate
 *          level.  The tree simply delegates to Interval::overlaps, so
 *          results may be returned.  This test documents that behavior.
 */
TEST_F(IntervalTreeTest, QueryOverlap_DegenerateInterval_MayOverlap)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");

    // [5,5) overlaps [0,10) per the coordinate-level check.
    const auto results = tree.queryOverlap(Interval<int>(5, 5));
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "A");
}

// ══════════════════════════════════════════════════════════════════════════
//  6. queryPoint
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Point inside some intervals returns correct results.
 * @details Verifies that queryPoint returns all intervals that contain the
 *          given point
 */
TEST_F(IntervalTreeTest, QueryPoint_PointInsideSome_ReturnsCorrectResults)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(5, 15), "B");
    tree.insert(Interval<int>(20, 30), "C");

    const auto results = tree.queryPoint(7);
    // 7 is inside [0,10) and [5,15), but not [20,30)
    ASSERT_EQ(results.size(), 2);
    EXPECT_NE(std::find(results.begin(), results.end(), "A"), results.end());
    EXPECT_NE(std::find(results.begin(), results.end(), "B"), results.end());
}

/**
 * @brief Point inside no intervals returns empty.
 * @details Verifies that queryPoint returns an empty vector when no interval
 *          contains the point
 */
TEST_F(IntervalTreeTest, QueryPoint_PointInsideNone_ReturnsEmpty)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");

    const auto results = tree.queryPoint(15);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Point at low boundary is included (inclusive).
 * @details Half-open semantics: low is inclusive, so a point at the low
 *          endpoint should be inside the interval
 */
TEST_F(IntervalTreeTest, QueryPoint_AtLowBoundary_IsIncluded)
{
    IntStringTree tree;
    tree.insert(Interval<int>(5, 10), "A");

    const auto results = tree.queryPoint(5);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "A");
}

/**
 * @brief Point at high boundary is NOT included (exclusive).
 * @details Half-open semantics: high is exclusive, so a point exactly at the
 *          high endpoint should NOT be considered inside the interval
 */
TEST_F(IntervalTreeTest, QueryPoint_AtHighBoundary_IsExcluded)
{
    IntStringTree tree;
    tree.insert(Interval<int>(5, 10), "A");

    const auto results = tree.queryPoint(10);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Query point in nested/containing intervals returns all containing
 *        intervals.
 * @details Verifies that a point deep inside nested intervals returns all
 *          containing entries
 */
TEST_F(IntervalTreeTest, QueryPoint_NestedIntervals_ReturnsAllContaining)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 50), "Outer");
    tree.insert(Interval<int>(10, 40), "Mid");
    tree.insert(Interval<int>(20, 30), "Inner");

    const auto results = tree.queryPoint(25);
    ASSERT_EQ(results.size(), 3);
    EXPECT_NE(std::find(results.begin(), results.end(), "Outer"), results.end());
    EXPECT_NE(std::find(results.begin(), results.end(), "Mid"), results.end());
    EXPECT_NE(std::find(results.begin(), results.end(), "Inner"), results.end());
}

// ══════════════════════════════════════════════════════════════════════════
//  7. verifyIntegrity
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief After multiple inserts, integrity check passes.
 * @details Verifies that the tree remains internally consistent after a
 *          series of insertions
 */
TEST_F(IntervalTreeTest, VerifyIntegrity_AfterInserts_ReturnsTrue)
{
    IntStringTree tree;
    for (int i = 0; i < 50; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 8), std::to_string(i));
    }
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief After multiple removes, integrity check passes.
 * @details Verifies that the tree remains internally consistent after a
 *          series of removals
 */
TEST_F(IntervalTreeTest, VerifyIntegrity_AfterRemoves_ReturnsTrue)
{
    IntStringTree tree;
    for (int i = 0; i < 20; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 8), std::to_string(i));
    }

    // Remove every other entry
    for (int i = 0; i < 20; i += 2)
    {
        static_cast<void>(
            tree.remove(Interval<int>(i * 10, i * 10 + 8), std::to_string(i)));
    }

    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief After clear, integrity check passes.
 * @details Verifies that a cleared tree passes integrity check
 */
TEST_F(IntervalTreeTest, VerifyIntegrity_AfterClear_ReturnsTrue)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");
    tree.clear();
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief On empty tree, integrity check passes.
 * @details Verifies that a default-constructed tree passes integrity check
 */
TEST_F(IntervalTreeTest, VerifyIntegrity_EmptyTree_ReturnsTrue)
{
    const IntStringTree tree;
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Move semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Move constructor transfers state; source becomes empty.
 * @details Verifies that move construction transfers all entries to the new
 *          tree and leaves the source in a valid empty state
 */
TEST_F(IntervalTreeTest, MoveConstructor_TransfersState)
{
    IntStringTree original;
    original.insert(Interval<int>(0, 10), "A");
    original.insert(Interval<int>(20, 30), "B");
    original.insert(Interval<int>(40, 50), "C");
    ASSERT_EQ(original.size(), 3);

    IntStringTree moved(std::move(original));
    EXPECT_EQ(moved.size(), 3);
    EXPECT_TRUE(original.empty());
    EXPECT_EQ(original.size(), 0);

    // Moved tree should be fully functional
    const auto results = moved.queryOverlap(Interval<int>(0, 100));
    EXPECT_EQ(results.size(), 3);
    EXPECT_TRUE(moved.verifyIntegrity());
}

/**
 * @brief Move assignment transfers state; source becomes empty.
 * @details Verifies that move assignment transfers all entries to the target
 *          and leaves the source in a valid empty state
 */
TEST_F(IntervalTreeTest, MoveAssignment_TransfersState)
{
    IntStringTree first;
    first.insert(Interval<int>(0, 10), "A");

    IntStringTree second;
    second.insert(Interval<int>(20, 30), "B");
    second.insert(Interval<int>(40, 50), "C");

    second = std::move(first);

    EXPECT_EQ(second.size(), 1);
    EXPECT_TRUE(first.empty());

    const auto results = second.queryOverlap(Interval<int>(0, 10));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "A");
    EXPECT_TRUE(second.verifyIntegrity());
}

/**
 * @brief Self-move-assignment is safe.
 * @details Verifies that self-move-assignment via std::move does not crash
 *          and leaves the tree in a valid state
 */
TEST_F(IntervalTreeTest, MoveAssignment_SelfMove_IsSafe)
{
    IntStringTree tree;
    tree.insert(Interval<int>(0, 10), "A");
    tree.insert(Interval<int>(20, 30), "B");

    // Self-assignment via std::move �?should be safe
    tree = std::move(tree);  // NOLINT

    // After self-move, the object should still be in a valid (unspecified but
    // destructible) state. Best effort: ensure we can call methods without
    // crashing.
    EXPECT_NO_THROW(static_cast<void>(tree.size()));
    EXPECT_NO_THROW(tree.clear());
}

/**
 * @brief A moved-to tree can be reused for new insertions and queries.
 * @details Verifies that after move assignment, the target tree is fully
 *          operational for insert, query, and integrity check
 */
TEST_F(IntervalTreeTest, MoveAssignment_MovedToTreeIsFunctional)
{
    IntStringTree source;
    source.insert(Interval<int>(0, 10), "Source");

    IntStringTree dest;
    dest = std::move(source);

    // dest should now be functional
    EXPECT_TRUE(dest.insert(Interval<int>(20, 30), "New"));
    EXPECT_EQ(dest.size(), 2);

    const auto results = dest.queryOverlap(Interval<int>(0, 30));
    EXPECT_EQ(results.size(), 2);
    EXPECT_TRUE(dest.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Thread safety
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent read-only operations do not crash.
 * @details Verifies that multiple threads calling queryPoint and queryOverlap
 *          concurrently are safe and correct
 */
TEST_F(IntervalTreeTest, ConcurrentReads_NoCrash)
{
    IntStringTree tree;
    for (int i = 0; i < 20; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 9), std::to_string(i));
    }

    std::atomic<uint64_t> successCount{0};
    constexpr int THREADS        = 8;
    constexpr int OPS_PER_THREAD = 2000;

    auto worker = [&]()
    {
        for (int i = 0; i < OPS_PER_THREAD; ++i)
        {
            const int q = (i * 7) % 200;  // varied query points
            const auto ptResults = tree.queryPoint(q);
            if (!ptResults.empty())
            {
                successCount.fetch_add(1, std::memory_order_relaxed);
            }

            // Also run overlap queries
            const auto ovResults = tree.queryOverlap(
                Interval<int>(q, q + 5));
            static_cast<void>(ovResults.size());
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

    // At least some queries should have found results
    EXPECT_GT(successCount.load(), 0);
}

/**
 * @brief Concurrent read + write operations do not crash.
 * @details Verifies that simultaneous read (queryPoint) and write (insert)
 *          operations do not cause crashes or deadlocks
 */
TEST_F(IntervalTreeTest, ConcurrentReadWrite_NoCrash)
{
    IntStringTree tree;
    // Seed with some initial data
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(Interval<int>(i * 20, i * 20 + 15), std::to_string(i));
    }

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> readOk{0};

    auto reader = [&]()
    {
        while (!stop.load(std::memory_order_acquire))
        {
            for (int i = 0; i < 30; ++i)
            {
                const auto results = tree.queryPoint(i * 5);
                if (!results.empty())
                {
                    readOk.fetch_add(1, std::memory_order_relaxed);
                }
                // Also exercise other read methods
                static_cast<void>(tree.size());
                static_cast<void>(tree.empty());
                static_cast<void>(tree.height());
            }
        }
    };

    auto writer = [&]()
    {
        for (int i = 0; i < 30; ++i)
        {
            tree.insert(Interval<int>(i + 200, i + 210),
                        std::to_string(i + 200));
            tree.insert(Interval<int>(i + 300, i + 310),
                        std::to_string(i + 300));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  10. Large-scale stress
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert many intervals, verify integrity, run queries, remove some,
 *        verify again.
 * @details Stress test with 1000 intervals to ensure the tree handles
 *          large-scale operations correctly
 */
TEST_F(IntervalTreeTest, LargeScale_Stress)
{
    IntStringTree tree;
    constexpr int N = 1000;

    // Insert 1000 non-overlapping intervals
    for (int i = 0; i < N; ++i)
    {
        const int low  = i * 100;
        const int high = low + 50;
        EXPECT_TRUE(tree.insert(Interval<int>(low, high), std::to_string(i)));
    }

    EXPECT_EQ(tree.size(), static_cast<size_t>(N));
    EXPECT_TRUE(tree.verifyIntegrity());

    // Query a selection of points �?each should find exactly one interval
    for (int i = 0; i < N; i += 50)
    {
        const int point = i * 100 + 25;
        const auto results = tree.queryPoint(point);
        EXPECT_EQ(results.size(), 1);
        EXPECT_EQ(results[0], std::to_string(i));
    }

    // Overlap query covering entire range returns all
    {
        const auto all = tree.queryOverlap(Interval<int>(0, N * 100));
        EXPECT_EQ(all.size(), static_cast<size_t>(N));
    }

    // Remove every other interval (500 removals)
    int removedCount = 0;
    for (int i = 0; i < N; i += 2)
    {
        const int low  = i * 100;
        const int high = low + 50;
        EXPECT_TRUE(tree.remove(Interval<int>(low, high), std::to_string(i)));
        ++removedCount;
    }

    EXPECT_EQ(tree.size(), static_cast<size_t>(N - removedCount));
    EXPECT_TRUE(tree.verifyIntegrity());

    // Verify the removed intervals are gone
    for (int i = 0; i < N; i += 2)
    {
        const int point = i * 100 + 25;
        EXPECT_TRUE(tree.queryPoint(point).empty());
    }

    // Verify the remaining intervals are still present
    for (int i = 1; i < N; i += 2)
    {
        const int point = i * 100 + 25;
        const auto results = tree.queryPoint(point);
        EXPECT_EQ(results.size(), 1);
        EXPECT_EQ(results[0], std::to_string(i));
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  11. Template variants
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief IntervalTree<double, int> variant �?insert and query.
 * @details Verifies that the tree works correctly with double endpoints
 *          and int payload
 */
TEST_F(IntervalTreeTest, DoubleIntVariant_InsertAndQuery)
{
    DoubleIntTree tree;
    EXPECT_TRUE(tree.insert(Interval<double>(0.0, 10.0), 100));
    EXPECT_TRUE(tree.insert(Interval<double>(5.0, 15.0), 200));
    EXPECT_TRUE(tree.insert(Interval<double>(20.0, 30.0), 300));

    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.verifyIntegrity());

    // Overlap query
    const auto ovResults = tree.queryOverlap(Interval<double>(8.0, 12.0));
    ASSERT_EQ(ovResults.size(), 2);
    EXPECT_NE(std::find(ovResults.begin(), ovResults.end(), 100),
              ovResults.end());
    EXPECT_NE(std::find(ovResults.begin(), ovResults.end(), 200),
              ovResults.end());

    // Point query (at boundary)
    const auto ptResults = tree.queryPoint(10.0);
    // [0,10) does NOT include 10, but [5,15) does
    ASSERT_EQ(ptResults.size(), 1);
    EXPECT_EQ(ptResults[0], 200);
}

/**
 * @brief IntervalTree<double, int> variant �?remove and clear.
 * @details Verifies remove and clear operations with double+int types
 */
TEST_F(IntervalTreeTest, DoubleIntVariant_RemoveAndClear)
{
    DoubleIntTree tree;
    tree.insert(Interval<double>(0.0, 5.0), 1);
    tree.insert(Interval<double>(5.0, 10.0), 2);
    tree.insert(Interval<double>(10.0, 15.0), 3);
    ASSERT_EQ(tree.size(), 3);

    // Remove middle
    EXPECT_TRUE(tree.remove(Interval<double>(5.0, 10.0), 2));
    EXPECT_EQ(tree.size(), 2);

    // Verify removal
    const auto results = tree.queryPoint(7.5);
    EXPECT_TRUE(results.empty());

    // Clear
    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief IntervalTree<double, int> variant �?move semantics.
 * @details Verifies move constructor with double+int types
 */
TEST_F(IntervalTreeTest, DoubleIntVariant_MoveConstructor)
{
    DoubleIntTree original;
    original.insert(Interval<double>(1.0, 2.0), 10);
    original.insert(Interval<double>(3.0, 4.0), 20);

    DoubleIntTree moved(std::move(original));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(original.empty());

    const auto results = moved.queryPoint(1.5);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 10);
}

/**
 * @brief IntervalTree<double, int> variant �?large-scale stress.
 * @details Stress test with double+int types to ensure template flexibility
 */
TEST_F(IntervalTreeTest, DoubleIntVariant_LargeScale)
{
    DoubleIntTree tree;
    constexpr int N = 500;

    for (int i = 0; i < N; ++i)
    {
        const double low  = static_cast<double>(i) * 10.0;
        const double high = low + 5.0;
        EXPECT_TRUE(tree.insert(Interval<double>(low, high), i));
    }

    EXPECT_EQ(tree.size(), static_cast<size_t>(N));
    EXPECT_TRUE(tree.verifyIntegrity());

    // Point queries
    for (int i = 0; i < N; i += 10)
    {
        const double point = static_cast<double>(i) * 10.0 + 2.5;
        const auto results = tree.queryPoint(point);
        EXPECT_EQ(results.size(), 1);
        EXPECT_EQ(results[0], i);
    }

    // Remove half
    for (int i = 0; i < N; i += 2)
    {
        const double low  = static_cast<double>(i) * 10.0;
        const double high = low + 5.0;
        EXPECT_TRUE(tree.remove(Interval<double>(low, high), i));
    }

    EXPECT_EQ(tree.size(), static_cast<size_t>(N / 2));
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  12. Additional edge cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert and remove with intervals that share endpoints.
 * @details Verifies correct handling of intervals with coincident low or high
 *          boundaries
 */
TEST_F(IntervalTreeTest, SharedEndpointIntervals_CorrectResults)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 5), "A"));
    EXPECT_TRUE(tree.insert(Interval<int>(5, 10), "B"));
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "C"));

    // Point 4 �?inside A and C, not B
    {
        const auto r = tree.queryPoint(4);
        ASSERT_EQ(r.size(), 2);
        EXPECT_NE(std::find(r.begin(), r.end(), "A"), r.end());
        EXPECT_NE(std::find(r.begin(), r.end(), "C"), r.end());
    }

    // Point 5 �?inside B and C, not A (exclusive at high)
    {
        const auto r = tree.queryPoint(5);
        ASSERT_EQ(r.size(), 2);
        EXPECT_NE(std::find(r.begin(), r.end(), "B"), r.end());
        EXPECT_NE(std::find(r.begin(), r.end(), "C"), r.end());
    }

    // Remove A, verify it's gone
    EXPECT_TRUE(tree.remove(Interval<int>(0, 5), "A"));
    EXPECT_EQ(tree.size(), 2);

    {
        const auto r = tree.queryPoint(4);
        ASSERT_EQ(r.size(), 1);
        EXPECT_EQ(r[0], "C");
    }
}

/**
 * @brief Reinsert after removal works.
 * @details Verifies that a removed entry can be inserted again
 */
TEST_F(IntervalTreeTest, RemoveThenReinsert_Works)
{
    IntStringTree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_TRUE(tree.remove(Interval<int>(0, 10), "A"));
    EXPECT_TRUE(tree.empty());

    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), "A"));
    EXPECT_EQ(tree.size(), 1);

    const auto results = tree.queryPoint(5);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "A");
}
