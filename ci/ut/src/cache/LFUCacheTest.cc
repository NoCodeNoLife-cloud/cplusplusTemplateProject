/**
 * @file LFUCacheTest.cc
 * @brief Unit tests for the LFUCache class
 * @details Tests cover cache construction, get/put operations, eviction policy,
 *          and basic cache management functions including remove, clear, and status queries.
 */

#include <gtest/gtest.h>
#include "cache/LFUCache.hpp"
#include <string>

using namespace common::cache;

/**
 * @brief Test LFUCache constructor with valid capacity
 * @details Verifies cache is created successfully with positive capacity
 */
TEST(LFUCacheTest, Constructor_ValidCapacity) {
    EXPECT_NO_THROW((LFUCache<int, std::string>(10)));
}

/**
 * @brief Test LFUCache constructor with zero capacity
 * @details Verifies exception is thrown when capacity is zero
 */
TEST(LFUCacheTest, Constructor_ZeroCapacity) {
    EXPECT_THROW((LFUCache<int, std::string>(0)), std::invalid_argument);
}

/**
 * @brief Test LFUCache constructor with negative capacity
 * @details Verifies exception is thrown when capacity is negative
 */
TEST(LFUCacheTest, Constructor_NegativeCapacity) {
    EXPECT_THROW((LFUCache<int, std::string>(-5)), std::invalid_argument);
}

/**
 * @brief Test basic put and get operations
 * @details Verifies that values can be stored and retrieved correctly
 */
TEST(LFUCacheTest, PutAndGet_Basic) {
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
TEST(LFUCacheTest, Get_NonExistentKey) {
    LFUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    auto result = cache.get(999);
    
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put operation updates existing key
 * @details Verifies that putting an existing key updates its value
 */
TEST(LFUCacheTest, Put_UpdateExistingKey) {
    LFUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(1, "ONE"));
    
    auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "ONE");
}

/**
 * @brief Test LFU eviction policy - least frequently used item is evicted
 * @details When cache is full, the item with lowest access frequency should be removed
 */
TEST(LFUCacheTest, Eviction_LFU_Policy) {
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
TEST(LFUCacheTest, Eviction_SameFrequency_LRUTiebreaker) {
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
TEST(LFUCacheTest, Put_RValueReference) {
    LFUCache<int, std::string> cache(3);
    
    std::string value = "test";
    EXPECT_TRUE(cache.put(1, std::move(value)));
    
    auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "test");
}

/**
 * @brief Test remove operation
 * @details Verifies that entries can be removed from cache
 */
TEST(LFUCacheTest, Remove_ExistingKey) {
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
TEST(LFUCacheTest, Remove_NonExistentKey) {
    LFUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    
    bool removed = cache.remove(999);
    EXPECT_FALSE(removed);
}

/**
 * @brief Test clear operation
 * @details Verifies that all entries are removed from cache
 */
TEST(LFUCacheTest, Clear_AllEntries) {
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
TEST(LFUCacheTest, Size_CorrectCount) {
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
TEST(LFUCacheTest, Capacity_CorrectValue) {
    LFUCache<int, std::string> cache(10);

    EXPECT_EQ(cache.capacity(), 10);
}

/**
 * @brief Test empty operation
 * @details Verifies that empty returns correct state
 */
TEST(LFUCacheTest, Empty_CorrectState) {
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
TEST(LFUCacheTest, Contains_ExistingKey) {
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
TEST(LFUCacheTest, FrequencyUpdate_OnGet) {
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
TEST(LFUCacheTest, FrequencyUpdate_OnPutUpdate) {
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
TEST(LFUCacheTest, EdgeCase_CapacityOne) {
    LFUCache<int, std::string> cache(1);
    
    EXPECT_TRUE(cache.put(1, "one"));
    auto result1 = cache.get(1);
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
TEST(LFUCacheTest, Get_ConstVersion) {
    LFUCache<int, std::string> cache(3);
    EXPECT_TRUE(cache.put(1, "one"));
    
    const LFUCache<int, std::string> &constCache = cache;
    auto result = constCache.get(1);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}
