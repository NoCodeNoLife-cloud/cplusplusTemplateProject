/**
 * @file IntervalTreeTest.cc
 * @brief Unit tests for IntervalTree<ValueT, DataT> — AVL-based interval tree
 * @details Tests cover insertion, removal, overlap queries, point queries,
 *          AVL balance, max_high_ augmentation consistency, thread safety,
 *          move semantics, and state queries (clear, empty, size).
 *          All random tests use a fixed seed for determinism.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <mutex>
#include <random>
#include <thread>
#include <utility>
#include <vector>

#include "data_structure/spatial/IntervalTree.hpp"

using namespace common::data_structure::spatial;

// ══════════════════════════════════════════════════════════════════════════
//  Test Fixture
// ══════════════════════════════════════════════════════════════════════════

class IntervalTreeTest : public testing::Test
{
protected:
    // Type aliases for brevity.
    using Tree = IntervalTree<int, int>;
    using Entry = std::pair<Interval<int>, int>;

    void SetUp() override {}
    void TearDown() override {}

    // ── Brute-force helpers for query verification ─────────────────────

    /// @brief Brute-force overlap query against a vector of entries.
    static auto bruteForceOverlap(
        const std::vector<Entry>& entries,
        const Interval<int>& query) -> std::vector<int>
    {
        std::vector<int> results;
        for (const auto& [iv, data] : entries)
        {
            if (iv.overlaps(query))
            {
                results.push_back(data);
            }
        }
        return results;
    }

    /// @brief Brute-force point query against a vector of entries.
    static auto bruteForcePoint(
        const std::vector<Entry>& entries,
        int point) -> std::vector<int>
    {
        std::vector<int> results;
        for (const auto& [iv, data] : entries)
        {
            if (iv.contains(point))
            {
                results.push_back(data);
            }
        }
        return results;
    }

    // ── Random interval generators ────────────────────────────────────

    /// @brief Generates a deterministic sequence of random intervals.
    struct IntervalGenerator
    {
        std::mt19937_64 rng;  // NOLINT: fixed seed for determinism

        explicit IntervalGenerator(uint64_t seed = 42)
            : rng(seed) // NOLINT: fixed seed
        {
        }

        /// @brief Returns a random interval with low in [min_low, max_low]
        ///        and width in [1, max_width].
        auto next(int min_low, int max_low, int max_width) -> Interval<int>
        {
            std::uniform_int_distribution<int> dist_low(min_low, max_low);
            std::uniform_int_distribution<int> dist_width(1, max_width);
            const int low = dist_low(rng);
            const int width = dist_width(rng);
            return Interval<int>(low, low + width);
        }
    };
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Insert Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert a single interval and verify size and retrievability.
 */
TEST_F(IntervalTreeTest, Insert_SingleInterval)
{
    Tree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), 1));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_FALSE(tree.empty());

    // Verify it can be found via overlap query.
    const auto results = tree.queryOverlap(Interval<int>(0, 10));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 1);
}

/**
 * @brief Insert multiple non-overlapping intervals.
 */
TEST_F(IntervalTreeTest, Insert_MultipleIntervals)
{
    Tree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), 1));
    EXPECT_TRUE(tree.insert(Interval<int>(20, 30), 2));
    EXPECT_TRUE(tree.insert(Interval<int>(40, 50), 3));
    EXPECT_EQ(tree.size(), 3);

    // Query each individually.
    {
        const auto r = tree.queryOverlap(Interval<int>(0, 10));
        ASSERT_EQ(r.size(), 1);
        EXPECT_EQ(r[0], 1);
    }
    {
        const auto r = tree.queryOverlap(Interval<int>(20, 30));
        ASSERT_EQ(r.size(), 1);
        EXPECT_EQ(r[0], 2);
    }
    {
        const auto r = tree.queryOverlap(Interval<int>(40, 50));
        ASSERT_EQ(r.size(), 1);
        EXPECT_EQ(r[0], 3);
    }
}

/**
 * @brief Inserting an exact duplicate (interval + data) returns false.
 */
TEST_F(IntervalTreeTest, Insert_DuplicateRejected)
{
    Tree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), 1));
    EXPECT_FALSE(tree.insert(Interval<int>(0, 10), 1));
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Inserting the same interval with different data is allowed.
 */
TEST_F(IntervalTreeTest, Insert_SameIntervalDifferentData)
{
    Tree tree;
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), 1));
    EXPECT_TRUE(tree.insert(Interval<int>(0, 10), 2));
    EXPECT_EQ(tree.size(), 2);

    // Both should be queryable.
    auto results = tree.queryOverlap(Interval<int>(0, 10));
    ASSERT_EQ(results.size(), 2);
    std::sort(results.begin(), results.end());
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
}

/**
 * @brief Insert 1000 random intervals and verify size.
 */
TEST_F(IntervalTreeTest, Insert_1000Random)
{
    Tree tree;
    IntervalGenerator gen(42);

    for (int i = 0; i < 1000; ++i)
    {
        const auto iv = gen.next(0, 10000, 100);
        tree.insert(iv, i);
    }

    EXPECT_EQ(tree.size(), 1000);
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Remove Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Remove an exact match that exists.
 */
TEST_F(IntervalTreeTest, Remove_ExactMatch)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);
    ASSERT_EQ(tree.size(), 1);

    EXPECT_TRUE(tree.remove(Interval<int>(0, 10), 1));
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

/**
 * @brief Remove a non-existent entry returns false.
 */
TEST_F(IntervalTreeTest, Remove_NonExistent)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);

    // Wrong data.
    EXPECT_FALSE(tree.remove(Interval<int>(0, 10), 99));
    // Wrong interval.
    EXPECT_FALSE(tree.remove(Interval<int>(99, 100), 1));
    // Both wrong.
    EXPECT_FALSE(tree.remove(Interval<int>(99, 100), 99));
    // Empty tree case.
    Tree empty_tree;
    EXPECT_FALSE(empty_tree.remove(Interval<int>(0, 10), 1));

    // Original entry still present.
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Insert N entries then remove them all.
 */
TEST_F(IntervalTreeTest, Remove_All)
{
    Tree tree;
    for (int i = 0; i < 50; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 8), i);
    }
    ASSERT_EQ(tree.size(), 50);

    for (int i = 0; i < 50; ++i)
    {
        EXPECT_TRUE(tree.remove(Interval<int>(i * 10, i * 10 + 8), i));
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Remove a leaf node (no children).
 */
TEST_F(IntervalTreeTest, Remove_LeafNode)
{
    Tree tree;
    // Build a small tree where we know which nodes are leaves.
    tree.insert(Interval<int>(50, 60), 0);   // root
    tree.insert(Interval<int>(20, 30), 1);   // left child
    tree.insert(Interval<int>(80, 90), 2);   // right child -> leaf

    ASSERT_EQ(tree.size(), 3);

    // Remove the right leaf.
    EXPECT_TRUE(tree.remove(Interval<int>(80, 90), 2));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_TRUE(tree.verifyIntegrity());

    // The other two should still be there.
    {
        const auto remaining = tree.queryOverlap(Interval<int>(0, 100));
        EXPECT_EQ(remaining.size(), 2);
    }
}

/**
 * @brief Remove a node with one child.
 */
TEST_F(IntervalTreeTest, Remove_NodeWithOneChild)
{
    Tree tree;
    // Build: root(50), left(30), left-right(40) gives a chain.
    tree.insert(Interval<int>(50, 60), 0);   // root
    tree.insert(Interval<int>(30, 40), 1);   // left
    tree.insert(Interval<int>(40, 45), 2);   // left's right (one-child scenario for 30)
    tree.insert(Interval<int>(70, 80), 3);   // right leaf

    ASSERT_EQ(tree.size(), 4);

    // Remove interval (30, 40) which has one child (40, 45).
    EXPECT_TRUE(tree.remove(Interval<int>(30, 40), 1));
    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.verifyIntegrity());

    // (40, 45) should still be there.
    const auto r = tree.queryOverlap(Interval<int>(40, 45));
    EXPECT_FALSE(r.empty());
}

/**
 * @brief Remove a node with two children.
 */
TEST_F(IntervalTreeTest, Remove_NodeWithTwoChildren)
{
    Tree tree;
    // Build: root(50) with left(30)->left(20), right(40) and right(70).
    tree.insert(Interval<int>(50, 60), 0);   // root (two children)
    tree.insert(Interval<int>(30, 40), 1);   // left child
    tree.insert(Interval<int>(20, 25), 2);   // left-left
    tree.insert(Interval<int>(40, 45), 3);   // left-right
    tree.insert(Interval<int>(70, 80), 4);   // right child

    ASSERT_EQ(tree.size(), 5);

    // Remove the root (50,60) which has two children.
    EXPECT_TRUE(tree.remove(Interval<int>(50, 60), 0));
    EXPECT_EQ(tree.size(), 4);
    EXPECT_TRUE(tree.verifyIntegrity());

    // Verify total via overlap of the full range.
    const auto all = tree.queryOverlap(Interval<int>(0, 100));
    EXPECT_EQ(all.size(), 4);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Overlap Query Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Query with an interval that does not overlap any stored interval.
 */
TEST_F(IntervalTreeTest, QueryOverlap_NoOverlap)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);
    tree.insert(Interval<int>(20, 30), 2);

    const auto results = tree.queryOverlap(Interval<int>(15, 18));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Query that partially overlaps some intervals.
 */
TEST_F(IntervalTreeTest, QueryOverlap_PartialOverlap)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);    // [0,10)
    tree.insert(Interval<int>(5, 15), 2);    // [5,15) overlaps [8,12)
    tree.insert(Interval<int>(20, 30), 3);   // [20,30) does not overlap [8,12)

    const auto results = tree.queryOverlap(Interval<int>(8, 12));
    ASSERT_EQ(results.size(), 2);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 1);
    EXPECT_EQ(sorted[1], 2);
}

/**
 * @brief Query interval fully contains some stored intervals.
 */
TEST_F(IntervalTreeTest, QueryOverlap_CompleteOverlap)
{
    Tree tree;
    tree.insert(Interval<int>(10, 20), 1);
    tree.insert(Interval<int>(15, 25), 2);
    tree.insert(Interval<int>(30, 40), 3);

    // Query [5, 35) overlaps [10,20), [15,25), AND [30,40) because 30 < 35.
    const auto results = tree.queryOverlap(Interval<int>(5, 35));
    ASSERT_EQ(results.size(), 3);

    std::vector<int> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted[0], 1);
    EXPECT_EQ(sorted[1], 2);
    EXPECT_EQ(sorted[2], 3);
}

/**
 * @brief Query that overlaps every stored interval.
 */
TEST_F(IntervalTreeTest, QueryOverlap_AllOverlap)
{
    Tree tree;
    for (int i = 0; i < 50; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 8), i);
    }
    ASSERT_EQ(tree.size(), 50);

    // Full-range query covers all.
    const auto results = tree.queryOverlap(Interval<int>(0, 500));
    EXPECT_EQ(results.size(), 50);
}

/**
 * @brief Overlap query on an empty tree returns an empty result.
 */
TEST_F(IntervalTreeTest, QueryOverlap_EmptyTree)
{
    const Tree tree;
    const auto results = tree.queryOverlap(Interval<int>(0, 10));
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Random overlap queries vs brute-force on 1000 random intervals.
 */
TEST_F(IntervalTreeTest, QueryOverlap_1000Random)
{
    Tree tree;
    std::vector<Entry> entries;

    IntervalGenerator gen(42);

    // Insert 1000 random intervals.
    for (int i = 0; i < 1000; ++i)
    {
        const auto iv = gen.next(0, 10000, 100);
        tree.insert(iv, i);
        entries.emplace_back(iv, i);
    }
    ASSERT_EQ(tree.size(), 1000);
    ASSERT_EQ(entries.size(), 1000);

    // Run 100 random queries and compare with brute-force.
    std::mt19937_64 query_rng(123);  // NOLINT: fixed seed
    std::uniform_int_distribution<int> dist_low(0, 10000);
    std::uniform_int_distribution<int> dist_width(1, 500);

    for (int q = 0; q < 100; ++q)
    {
        const int low = dist_low(query_rng);
        const int width = dist_width(query_rng);
        const Interval<int> query(low, low + width);

        const auto expected = bruteForceOverlap(entries, query);
        auto actual = tree.queryOverlap(query);

        std::sort(actual.begin(), actual.end());
        // NOTE: expected is already sorted since bruteForceOverlap preserves
        // insertion order; sort it too for comparison.
        auto sorted_expected = expected;
        std::sort(sorted_expected.begin(), sorted_expected.end());

        EXPECT_EQ(actual, sorted_expected)
            << "Mismatch for query interval [" << low << ", " << low + width << ")";
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Point Query Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Point inside an interval is found.
 */
TEST_F(IntervalTreeTest, QueryPoint_Inside)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);

    const auto results = tree.queryPoint(5);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 1);
}

/**
 * @brief Point at the low edge is included (half-open: low is inclusive).
 */
TEST_F(IntervalTreeTest, QueryPoint_LowEdge)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);

    const auto results = tree.queryPoint(0);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 1);
}

/**
 * @brief Point at the high edge is excluded (half-open: high is exclusive).
 */
TEST_F(IntervalTreeTest, QueryPoint_HighEdge)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);

    const auto results = tree.queryPoint(10);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief A point can be contained by multiple overlapping intervals.
 */
TEST_F(IntervalTreeTest, QueryPoint_MultipleResults)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);
    tree.insert(Interval<int>(5, 15), 2);
    tree.insert(Interval<int>(20, 30), 3);

    // Point 7 is contained by [0,10) and [5,15).
    auto results = tree.queryPoint(7);
    ASSERT_EQ(results.size(), 2);

    std::sort(results.begin(), results.end());
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
}

/**
 * @brief Point query on an empty tree returns empty.
 */
TEST_F(IntervalTreeTest, QueryPoint_EmptyTree)
{
    const Tree tree;
    const auto results = tree.queryPoint(5);
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Random point queries vs brute-force.
 */
TEST_F(IntervalTreeTest, QueryPoint_100Random)
{
    Tree tree;
    std::vector<Entry> entries;

    IntervalGenerator gen(42);

    // Insert 1000 random intervals.
    for (int i = 0; i < 1000; ++i)
    {
        const auto iv = gen.next(0, 10000, 100);
        tree.insert(iv, i);
        entries.emplace_back(iv, i);
    }
    ASSERT_EQ(tree.size(), 1000);

    // Run 100 random point queries.
    std::mt19937_64 point_rng(456);  // NOLINT: fixed seed
    std::uniform_int_distribution<int> dist_point(0, 10100);

    for (int q = 0; q < 100; ++q)
    {
        const int pt = dist_point(point_rng);

        const auto expected = bruteForcePoint(entries, pt);
        auto actual = tree.queryPoint(pt);

        std::sort(actual.begin(), actual.end());
        auto sorted_expected = expected;
        std::sort(sorted_expected.begin(), sorted_expected.end());

        EXPECT_EQ(actual, sorted_expected)
            << "Mismatch for point " << pt;
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Balance (AVL) Verification Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert ascending intervals and verify height bound.
 *
 * AVL tree height is bounded by 1.44 * log2(n + 2) - 0.328.
 * We use the conservative bound 2 * log2(n + 1) for the test.
 */
TEST_F(IntervalTreeTest, Balance_InsertAscending)
{
    Tree tree;
    constexpr int kCount = 100;

    for (int i = 0; i < kCount; ++i)
    {
        tree.insert(Interval<int>(i, i + 1), i);
    }

    ASSERT_EQ(tree.size(), static_cast<size_t>(kCount));
    // Conservative AVL height bound: 2 * log2(n + 1)
    // For n = 100: 2 * log2(101) ≈ 2 * 6.66 = 13.32 → bound ~13
    const size_t kMaxHeight = 2 * static_cast<size_t>(
        std::log2(static_cast<double>(kCount + 1)) + 1); // +1 for integer rounding
    EXPECT_LE(tree.height(), kMaxHeight);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Insert descending intervals and verify height bound.
 */
TEST_F(IntervalTreeTest, Balance_InsertDescending)
{
    Tree tree;
    constexpr int kCount = 100;

    for (int i = kCount - 1; i >= 0; --i)
    {
        tree.insert(Interval<int>(i, i + 1), i);
    }

    ASSERT_EQ(tree.size(), static_cast<size_t>(kCount));

    const size_t kMaxHeight = 2 * static_cast<size_t>(
        std::log2(static_cast<double>(kCount + 1)) + 1);
    EXPECT_LE(tree.height(), kMaxHeight);
    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief Insert random intervals and verify height bound.
 */
TEST_F(IntervalTreeTest, Balance_RandomSequence)
{
    Tree tree;
    constexpr int kCount = 1000;

    IntervalGenerator gen(42);
    for (int i = 0; i < kCount; ++i)
    {
        const auto iv = gen.next(0, 100000, 500);
        tree.insert(iv, i);
    }

    ASSERT_EQ(tree.size(), static_cast<size_t>(kCount));

    // For n = 1000: 2 * log2(1001) ≈ 2 * 9.97 = 19.94 → bound ~20
    const size_t kMaxHeight = 2 * static_cast<size_t>(
        std::log2(static_cast<double>(kCount + 1)) + 1);
    EXPECT_LE(tree.height(), kMaxHeight);
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  6. max_high_ Augmentation Consistency Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief After inserting intervals, verifyIntegrity passes.
 */
TEST_F(IntervalTreeTest, MaxHigh_AfterInsert)
{
    Tree tree;
    IntervalGenerator gen(42);

    for (int i = 0; i < 500; ++i)
    {
        const auto iv = gen.next(0, 10000, 200);
        tree.insert(iv, i);
    }

    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief After insertions and removals, verifyIntegrity still passes.
 */
TEST_F(IntervalTreeTest, MaxHigh_AfterRemove)
{
    Tree tree;
    std::vector<Entry> entries;
    IntervalGenerator gen(42);

    // Insert 200 intervals.
    for (int i = 0; i < 200; ++i)
    {
        const auto iv = gen.next(0, 10000, 200);
        tree.insert(iv, i);
        entries.emplace_back(iv, i);
    }

    // Remove every other entry (100 removals).
    for (int i = 0; i < 200; i += 2)
    {
        EXPECT_TRUE(tree.remove(entries[static_cast<size_t>(i)].first,
                                entries[static_cast<size_t>(i)].second));
    }

    EXPECT_TRUE(tree.verifyIntegrity());
    EXPECT_EQ(tree.size(), 100);
}

/**
 * @brief Random interleaved insert/remove followed by verifyIntegrity.
 */
TEST_F(IntervalTreeTest, MaxHigh_AfterRandomOps)
{
    Tree tree;
    std::vector<Entry> entries;
    IntervalGenerator gen(42);
    IntervalGenerator op_gen(99);

    std::mt19937_64 op_rng(77);  // NOLINT: fixed seed
    std::uniform_int_distribution<int> op_dist(0, 1); // 0 = insert, 1 = remove

    int next_id = 0;

    for (int step = 0; step < 100; ++step)
    {
        const bool do_insert = (entries.empty() || op_dist(op_rng) == 0);

        if (do_insert)
        {
            const auto iv = gen.next(0, 1000, 100);
            tree.insert(iv, next_id);
            entries.emplace_back(iv, next_id);
            ++next_id;
        }
        else
        {
            // Remove a random entry.
            std::uniform_int_distribution<size_t> pick(0, entries.size() - 1);
            const size_t idx = pick(op_rng);
            EXPECT_TRUE(tree.remove(entries[idx].first, entries[idx].second));
            entries.erase(entries.begin() + static_cast<ptrdiff_t>(idx));
        }

        // Periodically verify integrity.
        if (step % 10 == 0)
        {
            EXPECT_TRUE(tree.verifyIntegrity());
        }
    }

    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Thread Safety Test
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent insertions and queries from multiple threads.
 *
 * Uses 4 writer threads (each inserting 25 intervals) and 4 reader threads
 * (each running 25 overlap queries).  After joining, the tree must contain
 * exactly 100 entries and all queries from readers must be safe.
 */
TEST_F(IntervalTreeTest, ThreadSafety_ConcurrentInsertAndQuery)
{
    Tree tree;
    constexpr int kItemsPerWriter = 25;
    constexpr int kWriters = 4;
    constexpr int kReaders = 4;
    constexpr int kQueriesPerReader = 25;
    constexpr int kTotal = kItemsPerWriter * kWriters;

    // ── Writer function ──
    auto writer = [&tree](int thread_id, int seed)
    {
        IntervalGenerator gen(static_cast<uint64_t>(seed)); // NOLINT
        for (int i = 0; i < kItemsPerWriter; ++i)
        {
            const auto iv = gen.next(0, 10000, 200);
            tree.insert(iv, thread_id * kItemsPerWriter + i);
        }
    };

    // ── Reader function ──
    auto reader = [&tree](int seed)
    {
        IntervalGenerator gen(static_cast<uint64_t>(seed)); // NOLINT
        for (int i = 0; i < kQueriesPerReader; ++i)
        {
            const auto iv = gen.next(0, 10000, 500);
            // Just query — results may vary due to concurrent writes.
            // The assertion is that the query does not crash.
            [[maybe_unused]] const auto results = tree.queryOverlap(iv);
        }
    };

    std::vector<std::thread> threads;

    // Start writers.
    for (int t = 0; t < kWriters; ++t)
    {
        threads.emplace_back(writer, t, 100 + t);
    }

    // Start readers.
    for (int t = 0; t < kReaders; ++t)
    {
        threads.emplace_back(reader, 200 + t);
    }

    // Join all threads.
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(tree.size(), static_cast<size_t>(kTotal));
    EXPECT_TRUE(tree.verifyIntegrity());
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Move Semantics Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Move construction transfers ownership; source becomes empty.
 */
TEST_F(IntervalTreeTest, MoveConstructor_TransfersOwnership)
{
    Tree src;
    src.insert(Interval<int>(0, 10), 1);
    src.insert(Interval<int>(20, 30), 2);
    ASSERT_EQ(src.size(), 2);

    Tree dst(std::move(src));

    // Destination has the data.
    EXPECT_EQ(dst.size(), 2);
    EXPECT_FALSE(dst.empty());
    EXPECT_TRUE(dst.verifyIntegrity());

    // Source is empty.
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);

    // Destination queries work correctly.
    auto results = dst.queryOverlap(Interval<int>(0, 30));
    EXPECT_EQ(results.size(), 2);
}

/**
 * @brief Move assignment transfers ownership; source becomes empty.
 */
TEST_F(IntervalTreeTest, MoveAssignment_TransfersOwnership)
{
    Tree src;
    src.insert(Interval<int>(0, 10), 1);
    src.insert(Interval<int>(20, 30), 2);

    Tree dst;
    dst = std::move(src);

    // Destination has the data.
    EXPECT_EQ(dst.size(), 2);
    EXPECT_FALSE(dst.empty());
    EXPECT_TRUE(dst.verifyIntegrity());

    // Source is empty.
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);

    // Destination queries work correctly.
    auto results = dst.queryOverlap(Interval<int>(0, 30));
    EXPECT_EQ(results.size(), 2);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Clear and State Tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief clear() empties the tree.
 */
TEST_F(IntervalTreeTest, Clear_EmptiesTree)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);
    tree.insert(Interval<int>(20, 30), 2);
    ASSERT_EQ(tree.size(), 2);

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.height(), 0);
}

/**
 * @brief After clear(), the tree can be reused for further operations.
 */
TEST_F(IntervalTreeTest, Clear_Reusable)
{
    Tree tree;
    tree.insert(Interval<int>(0, 10), 1);
    tree.clear();

    // Reuse.
    EXPECT_TRUE(tree.insert(Interval<int>(100, 200), 42));
    EXPECT_EQ(tree.size(), 1);

    const auto results = tree.queryOverlap(Interval<int>(50, 150));
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 42);

    EXPECT_TRUE(tree.verifyIntegrity());
}

/**
 * @brief A default-constructed tree is empty.
 */
TEST_F(IntervalTreeTest, Empty_Initially)
{
    const Tree tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.height(), 0);
}

/**
 * @brief size() correctly reflects the number of entries.
 */
TEST_F(IntervalTreeTest, Size_Correct)
{
    Tree tree;
    EXPECT_EQ(tree.size(), 0);

    for (int i = 1; i <= 100; ++i)
    {
        tree.insert(Interval<int>(i * 10, i * 10 + 5), i);
        EXPECT_EQ(tree.size(), static_cast<size_t>(i));
    }

    // Remove half.
    for (int i = 1; i <= 100; i += 2)
    {
        EXPECT_TRUE(tree.remove(Interval<int>(i * 10, i * 10 + 5), i));
    }
    EXPECT_EQ(tree.size(), 50);
}

// ═══════════════════════════════════════════════════════════════════════════
//  10. DataT = std::string Tests
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert, query, and remove intervals with std::string payload.
 *
 * Covers non-trivial DataT to validate move semantics and noexcept
 * correctness in the Node constructor.
 */
TEST_F(IntervalTreeTest, StringData_InsertRemoveQuery)
{
    IntervalTree<int, std::string> tree;

    // Insert intervals with string data
    EXPECT_TRUE(tree.insert(Interval<int>(1, 5), "alpha"));
    EXPECT_TRUE(tree.insert(Interval<int>(3, 7), "beta"));
    EXPECT_TRUE(tree.insert(Interval<int>(6, 9), "gamma"));

    EXPECT_EQ(tree.size(), 3);

    // Query overlap — order not guaranteed
    auto results = tree.queryOverlap(Interval<int>(2, 6));
    EXPECT_THAT(results, testing::UnorderedElementsAre("alpha", "beta"));

    // Query point
    auto point_results = tree.queryPoint(4);
    EXPECT_THAT(point_results, testing::UnorderedElementsAre("alpha", "beta"));

    // Remove
    EXPECT_TRUE(tree.remove(Interval<int>(1, 5), "alpha"));
    EXPECT_EQ(tree.size(), 2);

    // Duplicate rejection
    EXPECT_FALSE(tree.insert(Interval<int>(3, 7), "beta"));

    // Verify integrity after all operations
    EXPECT_TRUE(tree.verifyIntegrity());
}
