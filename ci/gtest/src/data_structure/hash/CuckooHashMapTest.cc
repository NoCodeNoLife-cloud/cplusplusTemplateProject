/**
 * @file CuckooHashMapTest.cc
 * @brief Unit tests for the CuckooHashMap class
 * @details Tests cover construction, insert, insert_or_assign, lookup, erase,
 *          iteration, clear, load-factor management, rehash, reserve,
 *          copy/move semantics, swap, cuckoo-specific stash/kick behaviour,
 *          string keys, and large-scale stress scenarios.
 */

#include <string>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/hash/CuckooHashMap.hpp"

using namespace common::data_structure::hash;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class CuckooHashMapTest : public testing::Test
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
//  1. Construction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test default constructor creates empty map
 * @details Verifies that a newly constructed CuckooHashMap is empty, has zero size, and has a positive bucket count
 */
TEST_F(CuckooHashMapTest, DefaultConstructor_EmptyMap)
{
    const CuckooHashMap<int, std::string> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_GT(map.bucket_count(), 0);
}

/**
 * @brief Test capacity constructor creates map with minimum bucket count
 * @details Verifies that constructing with a capacity hint creates a map with at least that many total buckets and is empty
 */
TEST_F(CuckooHashMapTest, CapacityConstructor)
{
    const CuckooHashMap<int, int> map(100);
    EXPECT_GE(map.bucket_count(), 100);
    EXPECT_TRUE(map.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Insert
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test inserting a single key-value pair
 * @details Verifies that inserting a new key returns true, stores the correct value, and increments size
 */
TEST_F(CuckooHashMapTest, Insert_Single)
{
    CuckooHashMap<int, std::string> map;
    auto [it, inserted] = map.insert(42, "hello");
    EXPECT_TRUE(inserted);
    EXPECT_EQ((*it).second, "hello");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test inserting a duplicate key is ignored
 * @details Verifies that inserting a key that already exists returns false and does not overwrite the original value
 */
TEST_F(CuckooHashMapTest, Insert_DuplicateKeyIgnored)
{
    CuckooHashMap<int, std::string> map;
    (void)map.insert(1, "first");
    auto [it, inserted] = map.insert(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "first");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test insert_or_assign overwrites existing value
 * @details Verifies that insert_or_assign updates the value for an existing key and returns false (insertion did not occur)
 */
TEST_F(CuckooHashMapTest, InsertOrAssign_OverwritesValue)
{
    CuckooHashMap<int, std::string> map;
    (void)map.insert(1, "first");
    auto [it, inserted] = map.insert_or_assign(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "second");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test insert_or_assign inserts when key is missing
 * @details Verifies that insert_or_assign inserts a new key-value pair when the key does not exist, returning true
 */
TEST_F(CuckooHashMapTest, InsertOrAssign_InsertsWhenMissing)
{
    CuckooHashMap<int, std::string> map;
    auto [it, inserted] = map.insert_or_assign(42, "value");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test inserting many key-value pairs
 * @details Verifies that inserting 1000 entries increases the size correctly and all values are retrievable via at()
 */
TEST_F(CuckooHashMapTest, Insert_Many)
{
    CuckooHashMap<int, int> map;
    constexpr int n = 1000;
    for (int i = 0; i < n; ++i)
    {
        (void)map.insert(i, i * 10);
    }
    EXPECT_EQ(map.size(), n);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_EQ(map.at(i), i * 10);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Lookup
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test at returns value for an existing key
 * @details Verifies that at() returns the correct value for keys that exist in the map
 */
TEST_F(CuckooHashMapTest, At_ExistingKey)
{
    CuckooHashMap<std::string, int> map;
    (void)map.insert("one", 1);
    (void)map.insert("two", 2);
    EXPECT_EQ(map.at("one"), 1);
    EXPECT_EQ(map.at("two"), 2);
}

/**
 * @brief Test at throws on missing key
 * @details Verifies that at() throws std::out_of_range when the key is not present in the map
 */
TEST_F(CuckooHashMapTest, At_ThrowsOnMissingKey)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 100);
    EXPECT_THROW(static_cast<void>(map.at(99)), std::out_of_range);
}

/**
 * @brief Test contains returns correct presence status
 * @details Verifies that contains() returns true for inserted keys and false for missing keys
 */
TEST_F(CuckooHashMapTest, Contains)
{
    CuckooHashMap<int, std::string> map;
    (void)map.insert(1, "a");
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_FALSE(map.contains(0));
}

/**
 * @brief Test find returns end iterator for missing key
 * @details Verifies that find() returns end() for non-existent keys and a valid iterator for existing keys
 */
TEST_F(CuckooHashMapTest, Find_ReturnsEndWhenMissing)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    EXPECT_EQ(map.find(99), map.end());
    EXPECT_NE(map.find(1), map.end());
}

// ══════════════════════════════════════════════════════════════════════════
//  4. operator[]
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test operator[] inserts default value and allows modification
 * @details Verifies that operator[] inserts a default-constructed value for a missing key and returns a mutable reference
 */
TEST_F(CuckooHashMapTest, OperatorBracket_InsertDefault)
{
    CuckooHashMap<int, int> map;
    map[5] = 50;
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[5], 50);

    int& ref = map[10];
    EXPECT_EQ(ref, 0);  // default-inserted
    ref = 100;
    EXPECT_EQ(map[10], 100);
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Erase
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test erase removes an existing key
 * @details Verifies that erasing an existing key returns true, decreases size, and the key is no longer contained
 */
TEST_F(CuckooHashMapTest, Erase_ExistingKey)
{
    CuckooHashMap<int, std::string> map;
    (void)map.insert(1, "a");
    (void)map.insert(2, "b");
    EXPECT_TRUE(map.erase(1));
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

/**
 * @brief Test erase returns false for a missing key
 * @details Verifies that erasing a key not in the map returns false and size remains unchanged
 */
TEST_F(CuckooHashMapTest, Erase_MissingKey)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    EXPECT_FALSE(map.erase(99));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test reinserting after erase works correctly
 * @details Verifies that after erasing a key, a new value can be inserted with the same key
 */
TEST_F(CuckooHashMapTest, Erase_ReinsertAfterErase)
{
    CuckooHashMap<int, std::string> map;
    (void)map.insert(1, "a");
    EXPECT_TRUE(map.erase(1));
    EXPECT_TRUE(map.empty());
    auto [it, inserted] = map.insert(1, "b");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(map.at(1), "b");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test erase by const_iterator removes the entry
 * @details Verifies that erasing via a const iterator removes the correct entry and updates size
 */
TEST_F(CuckooHashMapTest, Erase_ByConstIterator)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    (void)map.insert(2, 20);
    const auto it = map.find(1);
    ASSERT_NE(it, map.end());
    (void)map.erase(it);
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
}

/**
 * @brief Test erase by iterator removes the entry
 * @details Verifies that erasing via a non-const iterator removes the correct entry and updates size
 */
TEST_F(CuckooHashMapTest, Erase_ByIterator)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    (void)map.insert(2, 20);
    auto it = map.find(1);
    ASSERT_NE(it, map.end());
    (void)map.erase(it);
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Iterator
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test range-based for iteration over the map
 * @details Verifies that the map supports range-based for loops and iterates over all inserted key-value pairs
 */
TEST_F(CuckooHashMapTest, ForEach_Iteration)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    (void)map.insert(2, 20);
    (void)map.insert(3, 30);

    int sum = 0;
    for (const auto& [key, value] : map)
    {
        sum += value;
    }
    EXPECT_EQ(sum, 60);
}

/**
 * @brief Test const iteration over the map
 * @details Verifies that iterating over a const-qualified map works and yields the correct number of elements
 */
TEST_F(CuckooHashMapTest, ConstIteration)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    (void)map.insert(2, 20);

    const auto& cmap = map;
    int count = 0;
    for ([[maybe_unused]] auto [key, value] : cmap)
    {
        ++count;
    }
    EXPECT_EQ(count, 2);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test clear removes all entries from the map
 * @details Verifies that clear() empties the map, resets size to zero, and previously contained keys are no longer found
 */
TEST_F(CuckooHashMapTest, Clear)
{
    CuckooHashMap<int, int> map;
    (void)map.insert(1, 10);
    (void)map.insert(2, 20);
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains(1));
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Load Factor, Rehash & Reserve
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test load factor increases with insertions
 * @details Verifies that load_factor() starts at zero, increases after inserts, and never exceeds max_load_factor()
 */
TEST_F(CuckooHashMapTest, LoadFactor_GrowsWithInsert)
{
    CuckooHashMap<int, int> map;
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
    (void)map.insert(1, 10);
    EXPECT_GT(map.load_factor(), 0.0);
    EXPECT_LE(map.load_factor(), map.max_load_factor());
}

/**
 * @brief Test rehash increases bucket count and preserves entries
 * @details Verifies that rehash() increases the total bucket count to at least the requested size while preserving all existing key-value pairs
 */
TEST_F(CuckooHashMapTest, Rehash)
{
    CuckooHashMap<int, int> map;
    for (int i = 0; i < 50; ++i)
    {
        (void)map.insert(i, i);
    }
    const auto old_buckets = map.bucket_count();
    map.rehash(old_buckets * 2);
    EXPECT_GT(map.bucket_count(), old_buckets);
    EXPECT_EQ(map.size(), 50);
    for (int i = 0; i < 50; ++i)
    {
        EXPECT_EQ(map.at(i), i);
    }
}

/**
 * @brief Test reserve pre-allocates buckets without triggering rehash
 * @details Verifies that reserve() pre-allocates sufficient buckets so that subsequent insertions do not cause a rehash
 */
TEST_F(CuckooHashMapTest, Reserve)
{
    CuckooHashMap<int, int> map;
    map.reserve(1000);
    const auto cap = map.bucket_count();
    for (int i = 0; i < 500; ++i)
    {
        (void)map.insert(i, i);
    }
    EXPECT_EQ(map.bucket_count(), cap);  // no rehash triggered
    EXPECT_EQ(map.size(), 500);
}

/**
 * @brief Test max_load_factor setter changes the threshold
 * @details Verifies that setting max_load_factor changes the value returned by max_load_factor() getter
 */
TEST_F(CuckooHashMapTest, MaxLoadFactor_Setter)
{
    CuckooHashMap<int, int> map;
    constexpr double new_mlf = 0.5;
    map.max_load_factor(new_mlf);
    EXPECT_DOUBLE_EQ(map.max_load_factor(), new_mlf);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Copy / Move / Swap
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test copy constructor creates an independent copy
 * @details Verifies that the copy constructor duplicates all entries and modifications to the original do not affect the copy
 */
TEST_F(CuckooHashMapTest, CopyConstructor)
{
    CuckooHashMap<int, std::string> map1;
    (void)map1.insert(1, "a");
    (void)map1.insert(2, "b");

    CuckooHashMap<int, std::string> map2(map1);
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), "a");
    EXPECT_EQ(map2.at(2), "b");

    (void)map1.insert(3, "c");
    EXPECT_EQ(map2.size(), 2);
    EXPECT_FALSE(map2.contains(3));
}

/**
 * @brief Test copy assignment duplicates map contents
 * @details Verifies that copy assignment copies all entries from source to target map
 */
TEST_F(CuckooHashMapTest, CopyAssignment)
{
    CuckooHashMap<int, int> map1;
    (void)map1.insert(1, 10);
    (void)map1.insert(2, 20);

    CuckooHashMap<int, int> map2;
    map2 = map1;
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
    EXPECT_EQ(map2.at(2), 20);
}

/**
 * @brief Test move constructor transfers map state
 * @details Verifies that the move constructor transfers all entries and the source map is left in a valid but unspecified state
 */
TEST_F(CuckooHashMapTest, MoveConstructor)
{
    CuckooHashMap<int, int> map1;
    (void)map1.insert(1, 10);
    (void)map1.insert(2, 20);

    CuckooHashMap<int, int> map2(std::move(map1));
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
    EXPECT_EQ(map2.at(2), 20);
}

/**
 * @brief Test move assignment transfers map state
 * @details Verifies that move assignment transfers entries from source to target, replacing previous content
 */
TEST_F(CuckooHashMapTest, MoveAssignment)
{
    CuckooHashMap<int, int> map1;
    (void)map1.insert(1, 10);

    CuckooHashMap<int, int> map2;
    (void)map2.insert(99, 999);
    map2 = std::move(map1);

    EXPECT_EQ(map2.size(), 1);
    EXPECT_EQ(map2.at(1), 10);
}

/**
 * @brief Test swap exchanges contents of two maps
 * @details Verifies that swap() exchanges the sizes and entries between two maps correctly
 */
TEST_F(CuckooHashMapTest, Swap)
{
    CuckooHashMap<int, int> map1;
    (void)map1.insert(1, 10);
    CuckooHashMap<int, int> map2;
    (void)map2.insert(2, 20);
    (void)map2.insert(3, 30);

    map1.swap(map2);
    EXPECT_EQ(map1.size(), 2);
    EXPECT_EQ(map2.size(), 1);
    EXPECT_TRUE(map1.contains(2));
    EXPECT_TRUE(map2.contains(1));
}

// ══════════════════════════════════════════════════════════════════════════
//  10. Cuckoo-Specific Behaviours
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test bucket_count returns sum of both tables
 * @details Verifies that bucket_count() returns the combined capacity of table1 and table2 (default: 4 + 4 = 8), not including the stash
 */
TEST_F(CuckooHashMapTest, BucketCount_ReturnsSumOfBothTables)
{
    const CuckooHashMap<int, int> map;
    // Default capacity: INITIAL_CAPACITY = 4 per table, so bucket_count = 4 + 4 = 8
    EXPECT_EQ(map.bucket_count(), 8);
}

/**
 * @brief Test iteration covers all three storage regions
 * @details Uses a custom zero-hasher to make all keys collide into the same two candidate
 *          slots, forcing the stash to fill. Verifies that iterating over the map visits all
 *          entries across table1, table2, and the stash.
 */
TEST_F(CuckooHashMapTest, Iteration_CoversAllThreeRegions)
{
    // Custom hasher that returns constant 0, making every key share the same
    // h1 and h2 positions — this forces the cuckoo-kick chain into the stash
    struct ZeroHasher
    {
        std::size_t operator()(int) const { return 0; }
    };

    CuckooHashMap<int, int, ZeroHasher> map;
    // Raise max_load_factor well above 1.0 to prevent early rehash and let
    // the stash fill with colliding entries
    map.max_load_factor(5.0);

    // 10 entries: 2 fit in the tables (one at h1, one at h2), 8 fill the stash
    constexpr int N = 10;
    for (int i = 0; i < N; ++i)
    {
        (void)map.insert(i, i * 10);
    }
    EXPECT_EQ(map.size(), N);

    // Iteration must find every entry regardless of which region it lives in
    int count = 0;
    int sum = 0;
    for (const auto& [key, value] : map)
    {
        EXPECT_EQ(value, key * 10);
        ++count;
        sum += value;
    }
    EXPECT_EQ(count, N);
    // Sum of 0*10 + 1*10 + ... + 9*10 = 10 * (0+1+...+9) = 10 * 45 = 450
    EXPECT_EQ(sum, 450);
}

/**
 * @brief Test high-load-factor stress with stash overflow
 * @details Inserts many entries with a standard hash function under high load factor,
 *          exercising rehash and re-insertion paths. Verifies all entries remain findable.
 */
TEST_F(CuckooHashMapTest, StashStressTest)
{
    CuckooHashMap<int, int> map;
    // Use a high load factor to force more collisions and stash usage
    map.max_load_factor(0.9);

    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
    {
        (void)map.insert(i, i * 10);
    }
    EXPECT_EQ(map.size(), N);
    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ(map.at(i), i * 10);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  11. Stress & Edge Cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test map works with string keys
 * @details Verifies that the CuckooHashMap handles std::string keys correctly for insert, at, contains, and negative lookup
 */
TEST_F(CuckooHashMapTest, StringKeys)
{
    CuckooHashMap<std::string, int> map;
    (void)map.insert("alpha", 1);
    (void)map.insert("beta", 2);
    (void)map.insert("gamma", 3);
    EXPECT_EQ(map.at("beta"), 2);
    EXPECT_TRUE(map.contains("gamma"));
    EXPECT_FALSE(map.contains("delta"));
}

/**
 * @brief Test large-scale insert and erase stress
 * @details Verifies that the map handles 10000 insertions followed by erasing every even key, maintaining correct size and data integrity
 */
TEST_F(CuckooHashMapTest, LargeStressTest)
{
    CuckooHashMap<int, int> map;
    constexpr int n = 10000;
    for (int i = 0; i < n; ++i)
    {
        (void)map.insert(i, i * 2);
    }
    EXPECT_EQ(map.size(), n);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_EQ(map.at(i), i * 2);
    }
    for (int i = 0; i < n; i += 2)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_EQ(map.size(), n / 2);
    for (int i = 0; i < n; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(map.contains(i));
        }
        else
        {
            EXPECT_EQ(map.at(i), i * 2);
        }
    }
}

/**
 * @brief Test insert after many erases
 * @details Verifies that inserting and erasing many entries, then re-inserting, works correctly without corruption
 */
TEST_F(CuckooHashMapTest, InsertAfterManyErases)
{
    CuckooHashMap<int, int> map;
    for (int i = 0; i < 100; ++i)
    {
        (void)map.insert(i, i);
    }
    EXPECT_EQ(map.size(), 100);
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_TRUE(map.empty());

    // Re-insert and verify
    for (int i = 0; i < 100; ++i)
    {
        (void)map.insert(i, i * 2);
    }
    EXPECT_EQ(map.size(), 100);
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(map.at(i), i * 2);
    }
}

/**
 * @brief Test interleaved insert and erase patterns
 * @details Verifies that alternating insert and erase operations maintain data integrity across cycles
 */
TEST_F(CuckooHashMapTest, InterleavedInsertErase)
{
    CuckooHashMap<int, int> map;

    // Phase 1: insert 0..49
    for (int i = 0; i < 50; ++i)
    {
        (void)map.insert(i, i);
    }
    EXPECT_EQ(map.size(), 50);

    // Phase 2: erase every even key from 0..48 (25 keys total)
    for (int i = 0; i < 50; i += 2)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_EQ(map.size(), 25);

    // Phase 3: insert keys 50..99 (50 new keys)
    for (int i = 50; i < 100; ++i)
    {
        (void)map.insert(i, i * 10);
    }

    // Expect: 25 odds (1,3,5,...,49) + 50 new keys (50..99) = 75 entries
    EXPECT_EQ(map.size(), 75);

    // Verify odds from first batch survived
    for (int i = 1; i < 50; i += 2)
    {
        EXPECT_EQ(map.at(i), i);
    }
    // Verify new keys are present
    for (int i = 50; i < 100; ++i)
    {
        EXPECT_EQ(map.at(i), i * 10);
    }
    // Verify erased evens are gone
    for (int i = 0; i < 50; i += 2)
    {
        EXPECT_FALSE(map.contains(i));
    }
}
