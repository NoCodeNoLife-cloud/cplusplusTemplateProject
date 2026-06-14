/**
 * @file ConcurrentHashMapTest.cc
 * @brief Unit tests for ConcurrentHashMap — single-threaded functional tests
 * @details Comprehensive single-threaded tests covering construction, basic
 *          operations, boundary conditions, iteration, load-factor management,
 *          and noexcept guarantees for the segment-locked concurrent hash map.
 */

#include "data_structure/concurrent/ConcurrentHashMap.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace common::data_structure::concurrent;

// ══════════════════════════════════════════════════════════════════════════
//  Custom types for testing
// ══════════════════════════════════════════════════════════════════════════

/// @brief A 2-D point used as a custom key type.
struct Point
{
    int x;
    int y;

    [[nodiscard]] auto operator==(const Point& other) const -> bool
    {
        return x == other.x && y == other.y;
    }
};

/// @brief Hash functor for Point — simple combine of x and y.
struct PointHash
{
    [[nodiscard]] auto operator()(const Point& p) const -> std::size_t
    {
        return static_cast<std::size_t>(p.x)
             ^ (static_cast<std::size_t>(p.y) << 16);
    }
};

/// @brief A key whose hash is always zero — forces every key into the same
///        segment (segment 0) and the same bucket (bucket 0), so that chain
///        order is fully deterministic (head-insertion order).
struct ControlledKey
{
    int id;

    [[nodiscard]] auto operator==(const ControlledKey& other) const -> bool
    {
        return id == other.id;
    }
};

/// @brief Hash that returns zero unconditionally.
struct ZeroHash
{
    [[nodiscard]] auto operator()(const ControlledKey& /*key*/) const -> std::size_t
    {
        return 0;
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class ConcurrentHashMapTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Construction / destruction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed map is empty with size 0 and load factor 0.
 * @details Verifies the post-conditions of the no-argument constructor.
 */
TEST_F(ConcurrentHashMapTest, DefaultConstructor_Empty)
{
    const ConcurrentHashMap<int, int> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
}

/**
 * @brief Custom max_load_factor constructor is accepted.
 * @details A map constructed with an explicit max_load_factor should still
 *          be empty; the factor is stored internally and used during rehash
 *          decisions.
 */
TEST_F(ConcurrentHashMapTest, CustomMaxLoadFactor_Empty)
{
    const ConcurrentHashMap<int, int> map(0.5);
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

/**
 * @brief Move constructor transfers data and leaves source empty.
 * @details After moving, the target owns all entries and the source is in a
 *          valid empty state (can be destroyed safely).
 */
TEST_F(ConcurrentHashMapTest, MoveConstructor_TransfersData)
{
    ConcurrentHashMap<int, int> src;
    ASSERT_TRUE(src.insert(1, 10));
    ASSERT_TRUE(src.insert(2, 20));
    ASSERT_TRUE(src.insert(3, 30));

    const ConcurrentHashMap<int, int> dst(std::move(src));

    EXPECT_EQ(dst.size(), 3);
    EXPECT_TRUE(dst.contains(1));
    EXPECT_TRUE(dst.contains(2));
    EXPECT_TRUE(dst.contains(3));
    EXPECT_EQ(*dst.get(1), 10);
    EXPECT_EQ(*dst.get(2), 20);
    EXPECT_EQ(*dst.get(3), 30);

    // Source should be empty but valid.
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);
}

/**
 * @brief Move assignment transfers data and releases old resources.
 * @details Assigning from another map transfers ownership; the target's old
 *          entries are freed and the source becomes empty.
 */
TEST_F(ConcurrentHashMapTest, MoveAssignment_TransfersData)
{
    ConcurrentHashMap<int, int> dst;
    ASSERT_TRUE(dst.insert(99, 999));  // will be discarded

    ConcurrentHashMap<int, int> src;
    ASSERT_TRUE(src.insert(1, 10));
    ASSERT_TRUE(src.insert(2, 20));

    dst = std::move(src);

    EXPECT_EQ(dst.size(), 2);
    EXPECT_TRUE(dst.contains(1));
    EXPECT_TRUE(dst.contains(2));
    EXPECT_FALSE(dst.contains(99));  // old data gone

    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);
}

/**
 * @brief Self-move-assignment is a no-op (sanity).
 * @details Move-assigning to self should not corrupt the map.
 */
TEST_F(ConcurrentHashMapTest, MoveAssignment_SelfAssign_NoOp)
{
    ConcurrentHashMap<int, int> map;
    ASSERT_TRUE(map.insert(1, 10));
    ASSERT_TRUE(map.insert(2, 20));

    // Self-assignment through a reference to avoid compiler warning.
    auto& ref = map;
    map = std::move(ref);

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Basic operations — insert / get / contains
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting a new key returns true and the value is retrievable.
 * @details Verifies the happy path for insert + get.
 */
TEST_F(ConcurrentHashMapTest, InsertNewKey_ReturnsTrue_GetReturnsValue)
{
    ConcurrentHashMap<std::string, int> map;
    EXPECT_TRUE(map.insert("alpha", 1));
    EXPECT_TRUE(map.insert("bravo", 2));

    auto val = map.get("alpha");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 1);

    val = map.get("bravo");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 2);
}

/**
 * @brief Inserting a duplicate key returns false and does not overwrite.
 * @details The first value persists; the second insert is a no-op.
 */
TEST_F(ConcurrentHashMapTest, InsertDuplicateKey_ReturnsFalse_NoOverwrite)
{
    ConcurrentHashMap<std::string, int> map;
    EXPECT_TRUE(map.insert("key", 100));
    EXPECT_FALSE(map.insert("key", 200));

    auto val = map.get("key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 100);  // original value preserved
}

/**
 * @brief insert_or_assign on a new key returns true.
 * @details When the key is absent, insert_or_assign behaves like insert.
 */
TEST_F(ConcurrentHashMapTest, InsertOrAssign_NewKey_ReturnsTrue)
{
    ConcurrentHashMap<int, int> map;
    EXPECT_TRUE(map.insert_or_assign(1, 10));

    auto val = map.get(1);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 10);
}

/**
 * @brief insert_or_assign on an existing key returns false and overwrites.
 * @details When the key is present, the value is updated and false is returned.
 */
TEST_F(ConcurrentHashMapTest, InsertOrAssign_ExistingKey_ReturnsFalse_Overwrites)
{
    ConcurrentHashMap<int, int> map;
    ASSERT_TRUE(map.insert_or_assign(1, 10));
    EXPECT_FALSE(map.insert_or_assign(1, 20));

    auto val = map.get(1);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 20);  // value updated
}

/**
 * @brief contains returns true for existing keys and false for missing keys.
 * @details Covers both positive and negative lookup.
 */
TEST_F(ConcurrentHashMapTest, Contains_ExistingAndMissing)
{
    ConcurrentHashMap<int, int> map;
    map.insert(1, 11);
    map.insert(2, 22);

    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
    EXPECT_FALSE(map.contains(3));
    EXPECT_FALSE(map.contains(0));
}

/**
 * @brief get on a missing key returns std::nullopt.
 * @details Edge case for lookups that fail.
 */
TEST_F(ConcurrentHashMapTest, Get_MissingKey_ReturnsNullopt)
{
    const ConcurrentHashMap<int, int> map;
    auto val = map.get(42);
    EXPECT_FALSE(val.has_value());
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Erase
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Erasing an existing key returns true and decrements size.
 * @details Happy path for erase.
 */
TEST_F(ConcurrentHashMapTest, Erase_ExistingKey_ReturnsTrue_SizeDecrements)
{
    ConcurrentHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);
    ASSERT_EQ(map.size(), 3);

    EXPECT_TRUE(map.erase(2));
    EXPECT_EQ(map.size(), 2);
    EXPECT_FALSE(map.contains(2));
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(3));
}

/**
 * @brief Erasing a non-existent key returns false and size is unchanged.
 * @details Safely handles missing keys.
 */
TEST_F(ConcurrentHashMapTest, Erase_MissingKey_ReturnsFalse)
{
    ConcurrentHashMap<int, int> map;
    map.insert(1, 10);
    EXPECT_FALSE(map.erase(999));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Erase the head (first) node in a chain.
 * @details Uses a ZeroHash to force all keys into the same bucket so chain
 *          position is deterministic.  With head-insertion, keys 1,2,3
 *          produce chain 3→2→1.  Erasing key 3 (head) must unlink correctly.
 */
TEST_F(ConcurrentHashMapTest, Erase_HeadNode)
{
    ConcurrentHashMap<ControlledKey, int, ZeroHash> map;

    // Insert 1,2,3 → chain: 3→2→1 (head insertion).
    ASSERT_TRUE(map.insert({1}, 10));
    ASSERT_TRUE(map.insert({2}, 20));
    ASSERT_TRUE(map.insert({3}, 30));

    // Erase head (key 3).
    EXPECT_TRUE(map.erase({3}));
    EXPECT_EQ(map.size(), 2);
    EXPECT_FALSE(map.contains({3}));
    EXPECT_TRUE(map.contains({1}));
    EXPECT_TRUE(map.contains({2}));
}

/**
 * @brief Erase a middle node in a chain.
 * @details After erasing 3 (head) and inserting 4,5, chain is 5→4→2→1.
 *          Erasing 2 (middle) must correctly link 4→1.
 */
TEST_F(ConcurrentHashMapTest, Erase_MiddleNode)
{
    ConcurrentHashMap<ControlledKey, int, ZeroHash> map;

    ASSERT_TRUE(map.insert({1}, 10));
    ASSERT_TRUE(map.insert({2}, 20));
    ASSERT_TRUE(map.insert({3}, 30));
    ASSERT_TRUE(map.erase({3}));  // remove head → chain: 2→1

    // Insert 4,5 → chain: 5→4→2→1
    ASSERT_TRUE(map.insert({4}, 40));
    ASSERT_TRUE(map.insert({5}, 50));

    // Erase middle node (key 2, which is third in chain 5→4→2→1).
    EXPECT_TRUE(map.erase({2}));
    EXPECT_EQ(map.size(), 3);
    EXPECT_FALSE(map.contains({2}));
    EXPECT_TRUE(map.contains({1}));
    EXPECT_TRUE(map.contains({4}));
    EXPECT_TRUE(map.contains({5}));

    // Verify remaining values are intact.
    EXPECT_EQ(*map.get({1}), 10);
    EXPECT_EQ(*map.get({4}), 40);
    EXPECT_EQ(*map.get({5}), 50);
}

/**
 * @brief Erase a tail (last) node in a chain.
 * @details Chain is 5→4→1 after middle erase.  Erasing 1 (tail) must set
 *          prev->next to nullptr.
 */
TEST_F(ConcurrentHashMapTest, Erase_TailNode)
{
    ConcurrentHashMap<ControlledKey, int, ZeroHash> map;

    ASSERT_TRUE(map.insert({1}, 10));
    ASSERT_TRUE(map.insert({2}, 20));
    ASSERT_TRUE(map.insert({3}, 30));
    ASSERT_TRUE(map.erase({3}));   // head
    ASSERT_TRUE(map.insert({4}, 40));
    ASSERT_TRUE(map.insert({5}, 50));
    ASSERT_TRUE(map.erase({2}));   // middle

    // Chain is now 5→4→1.  Erase tail (key 1).
    EXPECT_TRUE(map.erase({1}));
    EXPECT_EQ(map.size(), 2);
    EXPECT_FALSE(map.contains({1}));
    EXPECT_TRUE(map.contains({4}));
    EXPECT_TRUE(map.contains({5}));
}

/**
 * @brief Erase and re-insert — verify the key can be re-added.
 * @details After erasing a key, inserting it again must succeed.
 */
TEST_F(ConcurrentHashMapTest, EraseThenInsert_SameKey_Succeeds)
{
    ConcurrentHashMap<int, int> map;
    ASSERT_TRUE(map.insert(1, 10));
    ASSERT_TRUE(map.erase(1));
    EXPECT_TRUE(map.insert(1, 20));
    EXPECT_EQ(*map.get(1), 20);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief clear removes all entries; map becomes empty.
 * @details Verifies that after clear, size is 0 and contains returns false.
 */
TEST_F(ConcurrentHashMapTest, Clear_EmptiesMap)
{
    ConcurrentHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);
    ASSERT_EQ(map.size(), 3);

    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_FALSE(map.contains(3));
}

/**
 * @brief Clear on an already-empty map is a no-op.
 * @details Calling clear() on an empty map must not crash or change state.
 */
TEST_F(ConcurrentHashMapTest, Clear_EmptyMap_NoOp)
{
    ConcurrentHashMap<int, int> map;
    EXPECT_NO_THROW(map.clear());
    EXPECT_TRUE(map.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Bulk insert & rehash
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert 2000 entries, verify all are retrievable — exercises rehash.
 * @details With default SegmentBits=6 (64 segments) and 16 initial buckets
 *          per segment, inserting 2000 entries forces many segments to rehash
 *          (threshold ~12 entries per segment).  All data must survive.
 */
TEST_F(ConcurrentHashMapTest, BulkInsert_2000Entries_AllRetrievable)
{
    constexpr int N = 2000;
    ConcurrentHashMap<int, int> map;

    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(map.insert(i, i * 2));
    }

    EXPECT_EQ(map.size(), static_cast<std::size_t>(N));

    for (int i = 0; i < N; ++i)
    {
        ASSERT_TRUE(map.contains(i));
        auto val = map.get(i);
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(*val, i * 2);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Different key types
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Map with std::string keys works correctly.
 * @details Standard string keys: insert, get, contains, erase.
 */
TEST_F(ConcurrentHashMapTest, StringKeys)
{
    ConcurrentHashMap<std::string, double> map;
    EXPECT_TRUE(map.insert("pi", 3.14159));
    EXPECT_TRUE(map.insert("e", 2.71828));

    EXPECT_TRUE(map.contains("pi"));
    EXPECT_DOUBLE_EQ(*map.get("pi"), 3.14159);
    EXPECT_TRUE(map.contains("e"));
    EXPECT_DOUBLE_EQ(*map.get("e"), 2.71828);

    EXPECT_TRUE(map.erase("pi"));
    EXPECT_FALSE(map.contains("pi"));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Map with custom struct keys (Point + PointHash) works.
 * @details Custom hash and equality must be passed explicitly.
 */
TEST_F(ConcurrentHashMapTest, CustomKeyType_Point)
{
    ConcurrentHashMap<Point, std::string, PointHash> map;

    EXPECT_TRUE(map.insert(Point{1, 2}, "first"));
    EXPECT_TRUE(map.insert(Point{3, 4}, "second"));

    EXPECT_TRUE(map.contains(Point{1, 2}));
    EXPECT_EQ(*map.get(Point{1, 2}), "first");

    // Identical point (same x,y) should be treated as duplicate.
    EXPECT_FALSE(map.insert(Point{1, 2}, "duplicate"));

    EXPECT_TRUE(map.erase(Point{3, 4}));
    EXPECT_FALSE(map.contains(Point{3, 4}));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Empty string "" can be used as a key.
 * @details Edge case: empty string must be treated like any other key.
 */
TEST_F(ConcurrentHashMapTest, EmptyStringKey)
{
    ConcurrentHashMap<std::string, int> map;
    EXPECT_TRUE(map.insert("", 42));
    EXPECT_TRUE(map.contains(""));
    EXPECT_EQ(*map.get(""), 42);

    EXPECT_FALSE(map.insert("", 99));  // duplicate
    EXPECT_EQ(*map.get(""), 42);       // not overwritten by plain insert

    EXPECT_TRUE(map.erase(""));
    EXPECT_FALSE(map.contains(""));
    EXPECT_TRUE(map.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  7. for_each traversal
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief for_each on an empty map does nothing (no crash).
 * @details Trivial case: the callable should never be invoked.
 */
TEST_F(ConcurrentHashMapTest, ForEach_EmptyMap_NoOp)
{
    const ConcurrentHashMap<int, int> map;
    int count = 0;
    map.for_each([&count](int, int) { ++count; });
    EXPECT_EQ(count, 0);
}

/**
 * @brief for_each visits every entry exactly once.
 * @details Insert N entries and verify the visitor sees each exactly once.
 */
TEST_F(ConcurrentHashMapTest, ForEach_PopulatedMap_VisitsAll)
{
    constexpr int N = 500;
    ConcurrentHashMap<int, int> map;
    for (int i = 0; i < N; ++i)
    {
        map.insert(i, i * 10);
    }

    std::vector<bool> seen(N, false);
    int               count = 0;

    map.for_each([&](int k, int v) {
        ASSERT_GE(k, 0);
        ASSERT_LT(k, N);
        EXPECT_FALSE(seen[static_cast<std::size_t>(k)]) << "key " << k << " visited twice";
        EXPECT_EQ(v, k * 10);
        seen[static_cast<std::size_t>(k)] = true;
        ++count;
    });

    EXPECT_EQ(count, N);
    // Every key was seen (no false remaining).
    EXPECT_TRUE(std::all_of(seen.begin(), seen.end(), [](bool b) { return b; }));
}

/**
 * @brief for_each can compute aggregate values (sum of values).
 * @details Validates that the callable can accumulate state without side
 *          effects on the map.
 */
TEST_F(ConcurrentHashMapTest, ForEach_SumValues)
{
    ConcurrentHashMap<int, int> map;
    for (int i = 1; i <= 100; ++i)
    {
        map.insert(i, i);
    }

    int sum = 0;
    map.for_each([&sum](int, int v) { sum += v; });

    constexpr int expected = 100 * 101 / 2;  // 5050
    EXPECT_EQ(sum, expected);
}

// ══════════════════════════════════════════════════════════════════════════
//  8. load_factor / bucket_count
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initial bucket_count = SEGMENT_COUNT × INITIAL_SEGMENT_CAPACITY.
 * @details For default SegmentBits=6, SEGMENT_COUNT=64, INITIAL_SEGMENT_CAPACITY=16,
 *          so bucket_count() must return 1024.
 */
TEST_F(ConcurrentHashMapTest, InitialBucketCount)
{
    const ConcurrentHashMap<int, int> map;
    EXPECT_EQ(map.bucket_count(), 64ULL * 16);  // 1024
}

/**
 * @brief After bulk insert, bucket_count may increase due to rehash, and
 *        load_factor stays within reasonable bounds.
 * @details Insert enough entries to trigger rehashes, then verify that
 *          load_factor is above zero and bucket_count has grown.
 */
TEST_F(ConcurrentHashMapTest, LoadFactorAfterBulkInsert)
{
    ConcurrentHashMap<int, int> map;
    const auto initial_buckets = map.bucket_count();

    constexpr int N = 2000;
    for (int i = 0; i < N; ++i)
    {
        map.insert(i, i);
    }

    // Bucket count should have grown due to rehashes.
    EXPECT_GT(map.bucket_count(), initial_buckets);
    // Load factor must be positive and non-zero.
    EXPECT_GT(map.load_factor(), 0.0);
    // Load factor should be below the default max (0.75) — after inserts,
    // it will be <= max_load_factor_ (some segments may be just under).
    EXPECT_LE(map.load_factor(), 0.75);
}

/**
 * @brief load_factor returns 0.0 for an empty map.
 * @details Edge case: no buckets → load factor is zero.
 */
TEST_F(ConcurrentHashMapTest, LoadFactor_EmptyMap_Zero)
{
    const ConcurrentHashMap<int, int> map;
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Noexcept guarantees
// ══════════════════════════════════════════════════════════════════════════

/// @brief The destructor is noexcept.
static_assert(std::is_nothrow_destructible_v<ConcurrentHashMap<int, std::string>>,
              "ConcurrentHashMap destructor must be noexcept");

/// @brief Move constructor is noexcept.
static_assert(std::is_nothrow_move_constructible_v<ConcurrentHashMap<int, std::string>>,
              "ConcurrentHashMap move constructor must be noexcept");

/// @brief Move assignment is noexcept.
static_assert(std::is_nothrow_move_assignable_v<ConcurrentHashMap<int, std::string>>,
              "ConcurrentHashMap move assignment must be noexcept");

// ══════════════════════════════════════════════════════════════════════════
//  10. Interface completeness — all IConcurrentMap methods are callable
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verifies that the full IConcurrentMap interface is accessible and
 *        works in a typical usage scenario.
 * @details Smoke-test covering insert, insert_or_assign, get, contains, erase,
 *          size, empty, clear, load_factor, bucket_count, for_each.
 */
TEST_F(ConcurrentHashMapTest, FullInterfaceSmoke)
{
    ConcurrentHashMap<std::string, int> map;

    // Initially empty.
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);

    // Insert.
    EXPECT_TRUE(map.insert("a", 1));
    EXPECT_TRUE(map.insert("b", 2));
    EXPECT_EQ(map.size(), 2);

    // Duplicate insert.
    EXPECT_FALSE(map.insert("a", 10));
    EXPECT_EQ(*map.get("a"), 1);

    // Insert or assign (existing).
    EXPECT_FALSE(map.insert_or_assign("a", 10));
    EXPECT_EQ(*map.get("a"), 10);

    // Insert or assign (new).
    EXPECT_TRUE(map.insert_or_assign("c", 3));

    // Contains.
    EXPECT_TRUE(map.contains("a"));
    EXPECT_TRUE(map.contains("b"));
    EXPECT_TRUE(map.contains("c"));
    EXPECT_FALSE(map.contains("z"));

    // Get.
    EXPECT_EQ(*map.get("b"), 2);
    EXPECT_FALSE(map.get("z").has_value());

    // Erase.
    EXPECT_TRUE(map.erase("b"));
    EXPECT_FALSE(map.contains("b"));
    EXPECT_EQ(map.size(), 2);

    // Erase missing.
    EXPECT_FALSE(map.erase("b"));

    // for_each counts remaining entries.
    int count = 0;
    map.for_each([&count](const std::string&, int) { ++count; });
    EXPECT_EQ(count, 2);

    // Clear.
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);

    // load_factor / bucket_count after clear.
    EXPECT_GT(map.bucket_count(), 0);
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
}

/**
 * @brief Large number of entries with string keys — stress test.
 * @details Inserts 5000 entries using string keys to exercise the hasher
 *          and rehash logic with non-trivial key types.
 */
TEST_F(ConcurrentHashMapTest, LargeScaleStringKeys)
{
    constexpr int N = 5000;
    ConcurrentHashMap<std::string, int> map;

    for (int i = 0; i < N; ++i)
    {
        const auto key = "key-" + std::to_string(i);
        EXPECT_TRUE(map.insert(key, i));
    }

    EXPECT_EQ(map.size(), static_cast<std::size_t>(N));

    for (int i = 0; i < N; ++i)
    {
        const auto key = "key-" + std::to_string(i);
        ASSERT_TRUE(map.contains(key));
        EXPECT_EQ(*map.get(key), i);
    }
}
