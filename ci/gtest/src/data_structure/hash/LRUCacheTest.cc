/**
 * @file LRUCacheTest.cc
 * @brief Unit tests for the LRUCache class
 * @details Tests cover get, put, eviction, erase, move semantics, and edge cases.
 */

#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "data_structure/hash/LRUCache.hpp"

using namespace cppforge::data_structure::hash;

/**
 * @brief Test fixture for LRUCache tests
 */
class LRUCacheTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test constructor creates cache with valid capacity
 * @details Verifies that a newly constructed LRUCache has the specified capacity, zero size, and is empty
 */
TEST_F(LRUCacheTest, Constructor_ValidCapacity)
{
    const LRUCache<int, std::string> cache(10);
    EXPECT_EQ(cache.capacity(), 10);
    EXPECT_EQ(cache.size(), 0);
    EXPECT_TRUE(cache.empty());
}

/**
 * @brief Test constructor throws on zero capacity
 * @details Verifies that constructing an LRUCache with zero capacity throws std::invalid_argument
 */
TEST_F(LRUCacheTest, Constructor_ThrowsOnZeroCapacity)
{
    EXPECT_THROW((LRUCache<int, int>(0)), std::invalid_argument);
}

/**
 * @brief Test put inserts a new entry into the cache
 * @details Verifies that putting a new key-value pair increases the cache size and the key becomes contained
 */
TEST_F(LRUCacheTest, Put_InsertsNewEntry)
{
    LRUCache<int, std::string> cache(5);
    cache.put(1, "one");
    EXPECT_EQ(cache.size(), 1);
    EXPECT_TRUE(cache.contains(1));
}

/**
 * @brief Test put updates an existing entry in the cache
 * @details Verifies that putting a key that already exists updates its value without increasing the cache size
 */
TEST_F(LRUCacheTest, Put_UpdatesExistingEntry)
{
    LRUCache<int, std::string> cache(5);
    cache.put(1, "one");
    cache.put(1, "updated");
    const auto val = cache.get(1);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "updated");
    EXPECT_EQ(cache.size(), 1);
}

/**
 * @brief Test get returns nullopt for missing key
 * @details Verifies that get() for a key not present in the cache returns std::nullopt
 */
TEST_F(LRUCacheTest, Get_ReturnsNulloptWhenMissing)
{
    LRUCache<int, std::string> cache(5);
    EXPECT_FALSE(cache.get(99).has_value());
}

/**
 * @brief Test get returns the value when key is present
 * @details Verifies that get() returns the correct value for a key that exists in the cache
 */
TEST_F(LRUCacheTest, Get_ReturnsValueWhenPresent)
{
    LRUCache<int, int> cache(5);
    cache.put(42, 100);
    const auto val = cache.get(42);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 100);
}

/**
 * @brief Test eviction removes the least recently used entry
 * @details Verifies that when the cache exceeds capacity, the least recently used entry is evicted while other entries remain accessible
 */
TEST_F(LRUCacheTest, EvictsLeastRecentlyUsed)
{
    LRUCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40); // should evict key 1 (least recently used)

    EXPECT_EQ(cache.size(), 3);
    EXPECT_FALSE(cache.contains(1));
    EXPECT_EQ(cache.get(2).value(), 20);
    EXPECT_EQ(cache.get(3).value(), 30);
    EXPECT_EQ(cache.get(4).value(), 40);
}

/**
 * @brief Test get promotes the accessed entry to most recently used
 * @details Verifies that calling get() on an entry promotes it to MRU status, preventing its eviction when a new entry is added
 */
TEST_F(LRUCacheTest, GetPromotesToMRU)
{
    LRUCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    static_cast<void>(cache.get(1)); // promote 1 to MRU (discard return value intentionally)
    cache.put(3, 30); // should evict 2 (now LRU)

    EXPECT_TRUE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_EQ(cache.size(), 2);
}

/**
 * @brief Test put promotes the updated entry to most recently used
 * @details Verifies that updating an existing entry via put() promotes it to MRU status, changing eviction behaviour
 */
TEST_F(LRUCacheTest, PutPromotesToMRU)
{
    LRUCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(1, 100); // update 1, promotes to MRU
    cache.put(3, 30); // should evict 2

    EXPECT_TRUE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
    EXPECT_EQ(cache.get(1).value(), 100);
}

/**
 * @brief Test erase removes an existing entry
 * @details Verifies that erasing a key present in the cache returns true, decreases size, and the key is no longer contained
 */
TEST_F(LRUCacheTest, Erase_RemovesEntry)
{
    LRUCache<int, int> cache(5);
    cache.put(1, 10);
    cache.put(2, 20);
    EXPECT_TRUE(cache.erase(1));
    EXPECT_EQ(cache.size(), 1);
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
}

/**
 * @brief Test erase returns false for a missing key
 * @details Verifies that erasing a key not present in the cache returns false
 */
TEST_F(LRUCacheTest, Erase_MissingKeyReturnsFalse)
{
    LRUCache<int, int> cache(5);
    EXPECT_FALSE(cache.erase(99));
}

/**
 * @brief Test erase followed by reinsert works correctly
 * @details Verifies that after erasing a key, reinserting it adds a fresh entry without conflicts
 */
TEST_F(LRUCacheTest, Erase_ThenReinsert)
{
    LRUCache<int, std::string> cache(3);
    cache.put(1, "a");
    cache.put(2, "b");
    cache.erase(1);
    cache.put(1, "new_a");
    cache.put(3, "c");
    EXPECT_EQ(cache.size(), 3);
    EXPECT_EQ(cache.get(1).value(), "new_a");
}

/**
 * @brief Test clear removes all entries from the cache
 * @details Verifies that clear() empties the cache, resets size to zero, and contained keys become absent
 */
TEST_F(LRUCacheTest, Clear)
{
    LRUCache<int, int> cache(5);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.clear();
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains(1));
}

/**
 * @brief Test move constructor transfers cache state
 * @details Verifies that moving an LRUCache transfers capacity, size, and entries to the new cache
 */
TEST_F(LRUCacheTest, MoveConstructor)
{
    LRUCache<int, int> cache1(3);
    cache1.put(1, 10);
    cache1.put(2, 20);

    LRUCache cache2(std::move(cache1));
    EXPECT_EQ(cache2.capacity(), 3);
    EXPECT_EQ(cache2.size(), 2);
    EXPECT_EQ(cache2.get(1).value(), 10);
}

/**
 * @brief Test move assignment transfers cache state
 * @details Verifies that move assignment transfers entries from source to target, replacing previous content
 */
TEST_F(LRUCacheTest, MoveAssignment)
{
    LRUCache<int, int> cache1(3);
    cache1.put(1, 10);
    LRUCache<int, int> cache2(10);
    cache2.put(99, 999);

    cache2 = std::move(cache1);
    EXPECT_EQ(cache2.size(), 1);
    EXPECT_EQ(cache2.get(1).value(), 10);
}

/**
 * @brief Test cache works with string keys
 * @details Verifies that the LRUCache handles std::string keys correctly, including insertion, eviction, and containment checks
 */
TEST_F(LRUCacheTest, StringKeys)
{
    LRUCache<std::string, int> cache(3);
    cache.put("alpha", 1);
    cache.put("beta", 2);
    cache.put("gamma", 3);
    cache.put("delta", 4); // evicts "alpha"

    EXPECT_FALSE(cache.contains("alpha"));
    EXPECT_EQ(cache.get("beta").value(), 2);
    EXPECT_EQ(cache.get("gamma").value(), 3);
    EXPECT_EQ(cache.get("delta").value(), 4);
    EXPECT_EQ(cache.size(), 3);
}

/**
 * @brief Test cache with capacity of one evicts correctly
 * @details Verifies that a cache of capacity 1 evicts the existing entry when a new one is inserted
 */
TEST_F(LRUCacheTest, CapacityOne)
{
    LRUCache<int, int> cache(1);
    cache.put(1, 10);
    EXPECT_EQ(cache.size(), 1);
    cache.put(2, 20); // evicts 1
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_EQ(cache.size(), 1);
}

/**
 * @brief Test large cache under stress with sequential insertion
 * @details Verifies that a cache of capacity 100 correctly evicts the first 100 entries after 200 sequential insertions, keeping the last 100 entries intact
 */
TEST_F(LRUCacheTest, LargeCacheStress)
{
    LRUCache<int, int> cache(100);
    for (int i = 0; i < 200; ++i)
    {
        cache.put(i, i * 10);
    }
    EXPECT_EQ(cache.size(), 100);
    // first 100 entries should be evicted
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_FALSE(cache.contains(i));
    }
    // last 100 entries should be present
    for (int i = 100; i < 200; ++i)
    {
        EXPECT_EQ(cache.get(i).value(), i * 10);
    }
}

/**
 * @brief Test get does not change the cache size
 * @details Verifies that calling get() on an existing key does not alter the cache size
 */
TEST_F(LRUCacheTest, GetDoesNotChangeCacheSize)
{
    LRUCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    static_cast<void>(cache.get(1));
    EXPECT_EQ(cache.size(), 2);
}
