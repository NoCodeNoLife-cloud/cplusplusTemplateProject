/**
 * @file LRUCacheTest.cc
 * @brief Unit tests for the LRUCache class
 * @details Tests cover cache construction, get/put operations, LRU eviction policy,
 *          and basic cache management functions including remove, clear, and status queries.
 */

#include <string>
#include <gtest/gtest.h>

#include "cache/LRUCache.hpp"

using namespace common::cache;

/**
 * @brief Test fixture for LRUCacheTest tests
 */
class CacheLRUCacheTest : public testing::Test
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
 * @brief Test LRUCache constructor with valid capacity
 * @details Verifies cache is created successfully with positive capacity
 */
TEST_F(CacheLRUCacheTest, Constructor_ValidCapacity)
{
    EXPECT_NO_THROW((LRUCache<int, std::string>(10)));
}

/**
 * @brief Test LRUCache constructor with zero capacity
 * @details Verifies exception is thrown when capacity is zero
 */
TEST_F(CacheLRUCacheTest, Constructor_ZeroCapacity)
{
    EXPECT_THROW((LRUCache<int, std::string>(0)), std::invalid_argument);
}

/**
 * @brief Test basic put and get operations
 * @details Verifies that values can be stored and retrieved correctly
 */
TEST_F(CacheLRUCacheTest, PutAndGet_Basic)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    auto result1 = cache.get(1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "one");

    auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");
}

/**
 * @brief Test get operation for non-existent key
 * @details Verifies that get returns std::nullopt for missing keys
 */
TEST_F(CacheLRUCacheTest, Get_NonExistentKey)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    const auto result = cache.get(999);

    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put operation updates existing key
 * @details Verifies that putting an existing key updates its value
 */
TEST_F(CacheLRUCacheTest, Put_UpdateExistingKey)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(1, "ONE"));

    const auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "ONE");
}

/**
 * @brief Test LRU eviction policy - least recently used item is evicted
 * @details When cache is full, the item accessed least recently should be evicted
 */
TEST_F(CacheLRUCacheTest, Eviction_LRU_Policy)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Cache is full, adding new item should evict key 1 (oldest)
    EXPECT_TRUE(cache.put(3, "three"));

    // Key 1 should be evicted
    auto result1 = cache.get(1);
    EXPECT_FALSE(result1.has_value());

    // Key 2 and 3 should still exist
    auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");

    auto result3 = cache.get(3);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "three");
}

/**
 * @brief Test get operation moves item to front (prevents eviction)
 * @details Accessing an item makes it most recently used, so it won't be evicted next
 */
TEST_F(CacheLRUCacheTest, Get_MovesToFront)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Access key 1 to make it most recently used
    (void)cache.get(1);

    // Adding new item should evict key 2 (least recently used)
    EXPECT_TRUE(cache.put(3, "three"));

    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
}

/**
 * @brief Test put with rvalue reference
 * @details Verifies move semantics work correctly for put operation
 */
TEST_F(CacheLRUCacheTest, Put_RValueReference)
{
    LRUCache<int, std::string> cache(3);

    std::string value = "test";
    EXPECT_TRUE(cache.put(1, std::move(value)));

    const auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "test");
}

/**
 * @brief Test remove operation
 * @details Verifies that entries can be removed from cache
 */
TEST_F(CacheLRUCacheTest, Remove_ExistingKey)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    const bool removed = cache.remove(1);
    EXPECT_TRUE(removed);

    const auto result = cache.get(1);
    EXPECT_FALSE(result.has_value());

    // Key 2 should still exist
    const auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");
}

/**
 * @brief Test remove operation for non-existent key
 * @details Verifies that removing a non-existent key returns false
 */
TEST_F(CacheLRUCacheTest, Remove_NonExistentKey)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));

    const bool removed = cache.remove(999);
    EXPECT_FALSE(removed);
}

/**
 * @brief Test clear operation
 * @details Verifies that all entries are removed from cache
 */
TEST_F(CacheLRUCacheTest, Clear_AllEntries)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three"));

    cache.clear();

    EXPECT_EQ(cache.size(), 0);
    EXPECT_TRUE(cache.empty());

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_FALSE(cache.get(3).has_value());
}

/**
 * @brief Test clear then reuse
 * @details After clearing, the cache should accept new entries
 */
TEST_F(CacheLRUCacheTest, ClearThenReuse)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    cache.clear();
    EXPECT_TRUE(cache.empty());

    EXPECT_TRUE(cache.put(3, "three"));
    EXPECT_TRUE(cache.put(4, "four"));
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(3).value_or(""), "three");
    EXPECT_EQ(cache.get(4).value_or(""), "four");

    EXPECT_FALSE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
}

/**
 * @brief Test size operation
 * @details Verifies that size returns correct number of entries
 */
TEST_F(CacheLRUCacheTest, Size_CorrectCount)
{
    LRUCache<int, std::string> cache(5);

    EXPECT_EQ(cache.size(), 0);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_EQ(cache.size(), 1);

    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_EQ(cache.size(), 2);

    // Update existing - size should not change
    EXPECT_TRUE(cache.put(1, "ONE"));
    EXPECT_EQ(cache.size(), 2);
}

/**
 * @brief Test capacity operation
 * @details Verifies that capacity returns the configured maximum capacity
 */
TEST_F(CacheLRUCacheTest, Capacity_CorrectValue)
{
    const LRUCache<int, std::string> cache(10);
    EXPECT_EQ(cache.capacity(), 10);
}

/**
 * @brief Test empty operation
 * @details Verifies that empty returns correct state
 */
TEST_F(CacheLRUCacheTest, Empty_CorrectState)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.empty());

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_FALSE(cache.empty());

    cache.clear();
    EXPECT_TRUE(cache.empty());
}

/**
 * @brief Test contains operation
 * @details Verifies that contains correctly identifies existing keys
 */
TEST_F(CacheLRUCacheTest, Contains_ExistingKey)
{
    LRUCache<int, std::string> cache(3);

    EXPECT_FALSE(cache.contains(1));

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.contains(1));

    EXPECT_TRUE(cache.remove(1));
    EXPECT_FALSE(cache.contains(1));
}

/**
 * @brief Test const get operation
 * @details Verifies that const version of get works correctly
 */
TEST_F(CacheLRUCacheTest, Get_ConstVersion)
{
    LRUCache<int, std::string> cache(3);
    EXPECT_TRUE(cache.put(1, "one"));

    const LRUCache<int, std::string>& constCache = cache;
    const auto result = constCache.get(1);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}

/**
 * @brief Test cache with capacity 1
 * @details Verifies edge case of single-element cache
 */
TEST_F(CacheLRUCacheTest, EdgeCase_CapacityOne)
{
    LRUCache<int, std::string> cache(1);

    EXPECT_TRUE(cache.put(1, "one"));
    const auto result1 = cache.get(1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "one");

    // Adding second item should evict first
    EXPECT_TRUE(cache.put(2, "two"));

    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
}

/**
 * @brief Test eviction chain with multiple insertions at capacity
 * @details Each new insertion beyond capacity triggers exactly one eviction
 */
TEST_F(CacheLRUCacheTest, Eviction_MultipleEvictions)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three")); // evicts 1
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));

    EXPECT_TRUE(cache.put(4, "four")); // evicts 2
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test string keys and values with LRU cache
 * @details Verifies cache works with non-integer key and value types
 */
TEST_F(CacheLRUCacheTest, PutAndGet_StringKeys)
{
    LRUCache<std::string, int> cache(3);

    EXPECT_TRUE(cache.put("alpha", 1));
    EXPECT_TRUE(cache.put("beta", 2));
    EXPECT_TRUE(cache.put("gamma", 3));

    EXPECT_EQ(cache.get("alpha").value_or(-1), 1);
    EXPECT_EQ(cache.get("beta").value_or(-1), 2);
    EXPECT_EQ(cache.get("gamma").value_or(-1), 3);
    EXPECT_FALSE(cache.get("delta").has_value());

    // Adding delta evicts alpha (oldest)
    EXPECT_TRUE(cache.put("delta", 4));
    EXPECT_FALSE(cache.contains("alpha"));
    EXPECT_TRUE(cache.contains("beta"));
    EXPECT_TRUE(cache.contains("gamma"));
    EXPECT_TRUE(cache.contains("delta"));
}

/**
 * @brief Test large number of items
 * @details Verifies cache handles many items without errors
 */
TEST_F(CacheLRUCacheTest, LargeScalePutAndGet)
{
    LRUCache<int, int> cache(100);

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(cache.put(i, i * 10));
    }
    EXPECT_EQ(cache.size(), 100);

    for (int i = 0; i < 100; ++i)
    {
        auto val = cache.get(i);
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i * 10);
    }

    // Add more items to trigger evictions
    for (int i = 100; i < 150; ++i)
    {
        EXPECT_TRUE(cache.put(i, i * 10));
    }
    EXPECT_EQ(cache.size(), 100);
}

/**
 * @brief Test eviction after updating existing key
 * @details Updating an existing key should move it to front (most recently used)
 */
TEST_F(CacheLRUCacheTest, PutUpdate_MovesToFront)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Update key 1 - should move to front
    EXPECT_TRUE(cache.put(1, "ONE"));

    // Adding third item: key 2 is now LRU
    EXPECT_TRUE(cache.put(3, "three"));

    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_EQ(cache.get(1).value_or(""), "ONE");
    EXPECT_TRUE(cache.contains(3));
}

/**
 * @brief Test eviction and re-insertion of same key
 * @details After a key is evicted, it should be insertable again
 */
TEST_F(CacheLRUCacheTest, Eviction_ReinsertEvictedKey)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three")); // evicts 1
    EXPECT_FALSE(cache.contains(1));

    // Reinsert key 1 as a new entry
    EXPECT_TRUE(cache.put(1, "ONE"));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_EQ(cache.get(1).value_or(""), "ONE");
}

/**
 * @brief Test remove from full cache then add
 * @details Removing from a full cache should make room for new items
 */
TEST_F(CacheLRUCacheTest, RemoveThenAdd)
{
    LRUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    EXPECT_TRUE(cache.remove(1));

    // Should be able to add two more (cache has only 1 item)
    EXPECT_TRUE(cache.put(3, "three"));
    EXPECT_TRUE(cache.put(4, "four")); // evicts 2, adds 4

    EXPECT_FALSE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
    EXPECT_EQ(cache.size(), 2);
}
