/**
 * @file LRUCacheTest.cc
 * @brief Unit tests for the LRUCache class
 * @details Tests cover cache construction, get/put operations, eviction policy,
 *          and basic cache management functions including remove, clear, and status queries.
 */

#include <gtest/gtest.h>
#include "cache/LRUCache.hpp"
#include <string>

using namespace common::cache;

/**
 * @brief Test LRUCache constructor with valid capacity
 * @details Verifies cache is created successfully with positive capacity
 */
TEST(LRUCacheTest, Constructor_ValidCapacity) {
    EXPECT_NO_THROW((LRUCache<int, std::string>(10)));
}

/**
 * @brief Test LRUCache constructor with zero capacity
 * @details Verifies exception is thrown when capacity is zero
 */
TEST(LRUCacheTest, Constructor_ZeroCapacity) {
    EXPECT_THROW((LRUCache<int, std::string>(0)), std::invalid_argument);
}

/**
 * @brief Test LRUCache constructor with negative capacity
 * @details Verifies exception is thrown when capacity is negative
 */
TEST(LRUCacheTest, Constructor_NegativeCapacity) {
    EXPECT_THROW((LRUCache<int, std::string>(-5)), std::invalid_argument);
}

/**
 * @brief Test basic put and get operations
 * @details Verifies that values can be stored and retrieved correctly
 */
TEST(LRUCacheTest, PutAndGet_Basic) {
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
TEST(LRUCacheTest, Get_NonExistentKey) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    auto result = cache.get(999);
    
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put operation updates existing key
 * @details Verifies that putting an existing key updates its value and moves it to front
 */
TEST(LRUCacheTest, Put_UpdateExistingKey) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(1, "ONE"));
    
    auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "ONE");
}

/**
 * @brief Test LRU eviction policy - least recently used item is evicted
 * @details When cache is full, the item that was accessed least recently should be removed
 */
TEST(LRUCacheTest, Eviction_LRU_Policy) {
    LRUCache<int, std::string> cache(2);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    
    // Access key 1 to make it recently used
    auto val1 = cache.get(1);
    ASSERT_TRUE(val1.has_value());
    
    // Cache is full, adding new item should evict key 2 (least recently used)
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
 * @brief Test eviction order based on access pattern
 * @details Verifies that the least recently accessed item is evicted first
 */
TEST(LRUCacheTest, Eviction_AccessPattern) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three"));
    
    // Access key 1 and 2 to update their recent usage
    auto val1 = cache.get(1);
    ASSERT_TRUE(val1.has_value());
    auto val2 = cache.get(2);
    ASSERT_TRUE(val2.has_value());
    
    // Add new item, should evict key 3 (least recently used)
    EXPECT_TRUE(cache.put(4, "four"));
    
    EXPECT_FALSE(cache.contains(3));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test put with rvalue reference
 * @details Verifies move semantics work correctly for put operation
 */
TEST(LRUCacheTest, Put_RValueReference) {
    LRUCache<int, std::string> cache(3);
    
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
TEST(LRUCacheTest, Remove_ExistingKey) {
    LRUCache<int, std::string> cache(3);
    
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
TEST(LRUCacheTest, Remove_NonExistentKey) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    
    bool removed = cache.remove(999);
    EXPECT_FALSE(removed);
}

/**
 * @brief Test clear operation
 * @details Verifies that all entries are removed from cache
 */
TEST(LRUCacheTest, Clear_AllEntries) {
    LRUCache<int, std::string> cache(3);
    
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
TEST(LRUCacheTest, Size_CorrectCount) {
    LRUCache<int, std::string> cache(5);
    
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
TEST(LRUCacheTest, Capacity_CorrectValue) {
    LRUCache<int, std::string> cache(10);
    
    EXPECT_EQ(cache.capacity(), 10);
}

/**
 * @brief Test empty operation
 * @details Verifies that empty returns correct state
 */
TEST(LRUCacheTest, Empty_CorrectState) {
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
TEST(LRUCacheTest, Contains_ExistingKey) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_FALSE(cache.contains(1));
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.contains(1));
    
    EXPECT_TRUE(cache.remove(1));
    EXPECT_FALSE(cache.contains(1));
}

/**
 * @brief Test LRU order update on get operation
 * @details Verifies that accessing an item moves it to the front (most recently used)
 */
TEST(LRUCacheTest, LRUOrderUpdate_OnGet) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three"));
    
    // Access key 1 to make it most recently used
    auto val = cache.get(1);
    ASSERT_TRUE(val.has_value());
    
    // Add new item, should evict key 2 (now least recently used)
    EXPECT_TRUE(cache.put(4, "four"));
    
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test LRU order update on put operation for existing key
 * @details Verifies that updating an existing key moves it to the front
 */
TEST(LRUCacheTest, LRUOrderUpdate_OnPutUpdate) {
    LRUCache<int, std::string> cache(3);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_TRUE(cache.put(3, "three"));
    
    // Update key 1 (moves it to front)
    EXPECT_TRUE(cache.put(1, "ONE"));
    
    // Add new item, should evict key 2 (least recently used)
    EXPECT_TRUE(cache.put(4, "four"));
    
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
}

/**
 * @brief Test cache with capacity 1
 * @details Verifies edge case of single-element cache
 */
TEST(LRUCacheTest, EdgeCase_CapacityOne) {
    LRUCache<int, std::string> cache(1);
    
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
 * @details Verifies that const version of get works correctly and updates LRU order
 */
TEST(LRUCacheTest, Get_ConstVersion) {
    LRUCache<int, std::string> cache(3);
    EXPECT_TRUE(cache.put(1, "one"));
    
    const LRUCache<int, std::string> &constCache = cache;
    auto result = constCache.get(1);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}

/**
 * @brief Test multiple evictions in sequence
 * @details Verifies that consecutive evictions maintain correct LRU order
 */
TEST(LRUCacheTest, MultipleEvictions_Sequence) {
    LRUCache<int, std::string> cache(2);
    
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    
    // Evict key 1
    EXPECT_TRUE(cache.put(3, "three"));
    EXPECT_FALSE(cache.contains(1));
    
    // Evict key 2
    EXPECT_TRUE(cache.put(4, "four"));
    EXPECT_FALSE(cache.contains(2));
    
    // Only key 3 and 4 should remain
    EXPECT_TRUE(cache.contains(3));
    EXPECT_TRUE(cache.contains(4));
    EXPECT_EQ(cache.size(), 2);
}
