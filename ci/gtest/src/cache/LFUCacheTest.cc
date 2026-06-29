/**
 * @file LFUCacheTest.cc
 * @brief Unit tests for the LFUCache class
 * @details Tests cover cache construction, get/put operations, eviction policy,
 *          and basic cache management functions including remove, clear, and status queries.
 */

#include <string>
#include <gtest/gtest.h>

#include <cppforge/cache/LFUCache.hpp>

using namespace cppforge::cache;

/**
 * @brief Test fixture for LFUCacheTest tests
 */
class LFUCacheTest : public testing::Test
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
 * @brief Test LFUCache constructor with valid capacity
 * @details Verifies cache is created successfully with positive capacity
 */
TEST_F(LFUCacheTest, Constructor_ValidCapacity)
{
    EXPECT_NO_THROW((LFUCache<int, std::string>(10)));
}

/**
 * @brief Test LFUCache constructor with zero capacity
 * @details Verifies exception is thrown when capacity is zero
 */
TEST_F(LFUCacheTest, Constructor_ZeroCapacity)
{
    EXPECT_THROW((LFUCache<int, std::string>(0)), std::invalid_argument);
}

/**
 * @brief Test LFUCache constructor with negative capacity
 * @details Verifies exception is thrown when capacity is negative
 */
TEST_F(LFUCacheTest, Constructor_NegativeCapacity)
{
    EXPECT_THROW((LFUCache<int, std::string>(-5)), std::invalid_argument);
}

/**
 * @brief Test basic put and get operations
 * @details Verifies that values can be stored and retrieved correctly
 */
TEST_F(LFUCacheTest, PutAndGet_Basic)
{
    LFUCache<int, std::string> cache(3);

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
TEST_F(LFUCacheTest, Get_NonExistentKey)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    const auto result = cache.get(999);

    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put operation updates existing key
 * @details Verifies that putting an existing key updates its value
 */
TEST_F(LFUCacheTest, Put_UpdateExistingKey)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(1, "ONE"));

    const auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "ONE");
}

/**
 * @brief Test LFU eviction policy - least frequently used item is evicted
 * @details When cache is full, the item with lowest access frequency should be removed
 */
TEST_F(LFUCacheTest, Eviction_LFU_Policy)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Access key 1 twice to increase its frequency
    auto val1 = cache.get(1);
    ASSERT_TRUE(val1.has_value());
    val1 = cache.get(1);
    ASSERT_TRUE(val1.has_value());

    // Cache is full, adding new item should evict key 2 (lower frequency)
    EXPECT_TRUE(cache.put(3, "three"));

    // Key 2 should be evicted
    auto result2 = cache.get(2);
    EXPECT_FALSE(result2.has_value());

    // Key 1 and 3 should still exist
    auto result1 = cache.get(1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "one");

    auto result3 = cache.get(3);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "three");
}

/**
 * @brief Test eviction with same frequency - LRU within same frequency
 * @details When multiple items have the same frequency, the least recently used should be evicted
 */
TEST_F(LFUCacheTest, Eviction_SameFrequency_LRUTiebreaker)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Both have frequency 1, key 1 was added first (LRU)
    EXPECT_TRUE(cache.put(3, "three"));

    // Key 1 should be evicted (least recently used among frequency 1)
    auto result1 = cache.get(1);
    EXPECT_FALSE(result1.has_value());

    auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");

    auto result3 = cache.get(3);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "three");
}

/**
 * @brief Test put with rvalue reference
 * @details Verifies move semantics work correctly for put operation
 */
TEST_F(LFUCacheTest, Put_RValueReference)
{
    LFUCache<int, std::string> cache(3);

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
TEST_F(LFUCacheTest, Remove_ExistingKey)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    bool removed = cache.remove(1);
    EXPECT_TRUE(removed);

    auto result = cache.get(1);
    EXPECT_FALSE(result.has_value());

    // Key 2 should still exist
    auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");
}

/**
 * @brief Test remove operation for non-existent key
 * @details Verifies that removing a non-existent key returns false
 */
TEST_F(LFUCacheTest, Remove_NonExistentKey)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));

    const bool removed = cache.remove(999);
    EXPECT_FALSE(removed);
}

/**
 * @brief Test clear operation
 * @details Verifies that all entries are removed from cache
 */
TEST_F(LFUCacheTest, Clear_AllEntries)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three"));

    cache.clear();

    EXPECT_EQ(cache.size(), 0);
    EXPECT_TRUE(cache.empty());

    // All keys should be gone
    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_FALSE(cache.get(3).has_value());
}

/**
 * @brief Test size operation
 * @details Verifies that size returns correct number of entries
 */
TEST_F(LFUCacheTest, Size_CorrectCount)
{
    LFUCache<int, std::string> cache(5);

    EXPECT_EQ(cache.size(), 0);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_EQ(cache.size(), 1);

    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_EQ(cache.size(), 2);

    EXPECT_TRUE(cache.put(1, "ONE")); // Update existing
    EXPECT_EQ(cache.size(), 2); // Size should not change on update
}

/**
 * @brief Test capacity operation
 * @details Verifies that capacity returns the configured maximum capacity
 */
TEST_F(LFUCacheTest, Capacity_CorrectValue)
{
    const LFUCache<int, std::string> cache(10);

    EXPECT_EQ(cache.capacity(), 10);
}

/**
 * @brief Test empty operation
 * @details Verifies that empty returns correct state
 */
TEST_F(LFUCacheTest, Empty_CorrectState)
{
    LFUCache<int, std::string> cache(3);

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
TEST_F(LFUCacheTest, Contains_ExistingKey)
{
    LFUCache<int, std::string> cache(3);

    EXPECT_FALSE(cache.contains(1));

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.contains(1));

    EXPECT_TRUE(cache.remove(1));
    EXPECT_FALSE(cache.contains(1));
}

/**
 * @brief Test frequency update on get operation
 * @details Verifies that accessing an item increases its frequency
 */
TEST_F(LFUCacheTest, FrequencyUpdate_OnGet)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Access key 1 multiple times
    auto val = cache.get(1);
    ASSERT_TRUE(val.has_value());
    val = cache.get(1);
    ASSERT_TRUE(val.has_value());
    val = cache.get(1);
    ASSERT_TRUE(val.has_value());

    // Add new item, should evict key 2 (lower frequency)
    EXPECT_TRUE(cache.put(3, "three"));

    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
}

/**
 * @brief Test frequency update on put operation for existing key
 * @details Verifies that updating an existing key increases its frequency
 */
TEST_F(LFUCacheTest, FrequencyUpdate_OnPutUpdate)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Update key 1 (increases frequency)
    EXPECT_TRUE(cache.put(1, "ONE"));

    // Add new item, should evict key 2 (lower frequency)
    EXPECT_TRUE(cache.put(3, "three"));

    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
}

/**
 * @brief Test cache with capacity 1
 * @details Verifies edge case of single-element cache
 */
TEST_F(LFUCacheTest, EdgeCase_CapacityOne)
{
    LFUCache<int, std::string> cache(1);

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
 * @brief Test const get operation
 * @details Verifies that const version of get works correctly
 */
TEST_F(LFUCacheTest, Get_ConstVersion)
{
    LFUCache<int, std::string> cache(3);
    EXPECT_TRUE(cache.put(1, "one"));

    const LFUCache<int, std::string>& constCache = cache;
    const auto result = constCache.get(1);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}

/**
 * @brief Test eviction chain with multiple insertions at capacity
 * @details Each new insertion beyond capacity triggers exactly one eviction
 */
TEST_F(LFUCacheTest, Eviction_MultipleEvictions)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three")); // evicts 1 (LRU among freq=1)
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));

    EXPECT_TRUE(cache.put(4, "four")); // evicts 2 (LRU among freq=1)
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test frequency distribution with mixed access patterns
 * @details Items with different frequencies are evicted correctly:
 *          lowest frequency first, then LRU within same frequency
 */
TEST_F(LFUCacheTest, Eviction_ComplexFrequencyDistribution)
{
    LFUCache<int, std::string> cache(4);

    EXPECT_TRUE(cache.put(1, "one"));   // freq=1
    EXPECT_TRUE(cache.put(2, "two"));   // freq=1
    EXPECT_TRUE(cache.put(3, "three")); // freq=1
    EXPECT_TRUE(cache.put(4, "four"));  // freq=1

    // Increase frequencies: 1â†?, 2â†?, 3â†?, 4â†?
    (void)cache.get(1);
    (void)cache.get(1);
    (void)cache.get(2);

    // Freq: 1â†?, 2â†?, 3â†? (LRU: 3â†?), 4â†? (LRU: 4â†?)
    // Add 5: should evict 3 (lowest freq=1, LRU among 3,4 is 3)
    EXPECT_TRUE(cache.put(5, "five"));
    EXPECT_FALSE(cache.contains(3));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(4));
    EXPECT_TRUE(cache.contains(5));
}

/**
 * @brief Test string keys and values with LFU cache
 * @details Verifies cache works with non-integer key and value types
 */
TEST_F(LFUCacheTest, PutAndGet_StringKeys)
{
    LFUCache<std::string, int> cache(3);

    EXPECT_TRUE(cache.put("alpha", 1));
    EXPECT_TRUE(cache.put("beta", 2));
    EXPECT_TRUE(cache.put("gamma", 3));

    EXPECT_EQ(cache.get("alpha").value_or(-1), 1);
    EXPECT_EQ(cache.get("beta").value_or(-1), 2);
    EXPECT_EQ(cache.get("gamma").value_or(-1), 3);
    EXPECT_FALSE(cache.get("delta").has_value());

    // Increase freq of alpha
    (void)cache.get("alpha");
    (void)cache.get("alpha");

    // Add delta beyond capacity: should evict beta (freq=1, LRU)
    EXPECT_TRUE(cache.put("delta", 4));
    EXPECT_FALSE(cache.contains("beta"));
    EXPECT_TRUE(cache.contains("alpha"));
    EXPECT_TRUE(cache.contains("gamma"));
    EXPECT_TRUE(cache.contains("delta"));
}

/**
 * @brief Test remove operation that creates empty frequency list
 * @details Removing the only item at a frequency level should clean up
 *          the empty frequency list and update min_freq_
 */
TEST_F(LFUCacheTest, Remove_EmptiesFrequencyList)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    // Increase freq of key 1
    (void)cache.get(1);
    (void)cache.get(1);

    // Remove key 1 (freq=3). The freq=3 list becomes empty.
    EXPECT_TRUE(cache.remove(1));
    EXPECT_FALSE(cache.contains(1));

    // Add items: only key2 remains, capacity=2
    EXPECT_TRUE(cache.put(3, "three")); // key3:freq1 (size=1)
    EXPECT_TRUE(cache.put(4, "four"));  // size=2, reaches capacity, both freq=1
    // Both key2 and key3 are at freq=1; key2 was added first (LRU in freq=1)
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test clear followed by reuse
 * @details After clearing, the cache should accept new entries
 */
TEST_F(LFUCacheTest, ClearThenReuse)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    cache.clear();
    EXPECT_TRUE(cache.empty());

    // Reuse after clear
    EXPECT_TRUE(cache.put(3, "three"));
    EXPECT_TRUE(cache.put(4, "four"));
    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(3).value_or(""), "three");
    EXPECT_EQ(cache.get(4).value_or(""), "four");

    // Verify old keys are gone
    EXPECT_FALSE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
}

/**
 * @brief Test get after eviction and reinsertion of same key
 * @details After a key is evicted, it should be insertable again as new
 */
TEST_F(LFUCacheTest, Eviction_ReinsertEvictedKey)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three")); // evicts 1
    EXPECT_FALSE(cache.contains(1));

    // Reinsert key 1 - should work as a new entry
    EXPECT_TRUE(cache.put(1, "ONE"));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_EQ(cache.get(1).value_or(""), "ONE");
}

/**
 * @brief Test put with large number of items
 * @stress Verifies cache handles many items without errors
 */
TEST_F(LFUCacheTest, LargeScalePutAndGet)
{
    LFUCache<int, int> cache(100);

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(cache.put(i, i * 10));
    }
    EXPECT_EQ(cache.size(), 100);

    // Access all items
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
    // Size should still be at capacity
    EXPECT_EQ(cache.size(), 100);
}

/**
 * @brief Test get on non-const cache updates frequency, const does not
 * @details Verifies the const/non-const get distinction for frequency tracking
 */
TEST_F(LFUCacheTest, Get_ConstDoesNotUpdateFrequency)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Const get should NOT increase frequency
    const auto& constCache = cache;
    (void)constCache.get(1);
    (void)constCache.get(1);

    // Add third item: if freq of key1 is still 1, it gets evicted (LRU)
    EXPECT_TRUE(cache.put(3, "three"));

    // key1 should be evicted because const get didn't update frequency
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
}

/**
 * @brief Test capacity 2 edge case with various frequencies
 * @details Smallest multi-element cache boundary
 */
TEST_F(LFUCacheTest, EdgeCase_CapacityTwo)
{
    LFUCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    (void)cache.get(1); // key1â†’freq2, key2â†’freq1
    EXPECT_TRUE(cache.put(3, "three")); // evicts key2
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));

    // Now key1â†’freq2, key3â†’freq1
    (void)cache.get(3); // key3â†’freq2, key1â†’freq2
    EXPECT_TRUE(cache.put(4, "four")); // both freq2, LRU among them is key1
    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}
