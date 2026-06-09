/**
 * @file LRUCacheTest.cc
 * @brief Unit tests for the LRUCache class
 * @details Tests cover get, put, eviction, erase, move semantics, and edge cases.
 */

#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "data_structure/hash/LRUCache.hpp"

using namespace common::data_structure::hash;

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

TEST_F(LRUCacheTest, Constructor_ValidCapacity)
{
    const LRUCache<int, std::string> cache(10);
    EXPECT_EQ(cache.capacity(), 10);
    EXPECT_EQ(cache.size(), 0);
    EXPECT_TRUE(cache.empty());
}

TEST_F(LRUCacheTest, Constructor_ThrowsOnZeroCapacity)
{
    EXPECT_THROW((LRUCache<int, int>(0)), std::invalid_argument);
}

TEST_F(LRUCacheTest, Put_InsertsNewEntry)
{
    LRUCache<int, std::string> cache(5);
    cache.put(1, "one");
    EXPECT_EQ(cache.size(), 1);
    EXPECT_TRUE(cache.contains(1));
}

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

TEST_F(LRUCacheTest, Get_ReturnsNulloptWhenMissing)
{
    LRUCache<int, std::string> cache(5);
    EXPECT_FALSE(cache.get(99).has_value());
}

TEST_F(LRUCacheTest, Get_ReturnsValueWhenPresent)
{
    LRUCache<int, int> cache(5);
    cache.put(42, 100);
    const auto val = cache.get(42);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 100);
}

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

TEST_F(LRUCacheTest, Erase_MissingKeyReturnsFalse)
{
    LRUCache<int, int> cache(5);
    EXPECT_FALSE(cache.erase(99));
}

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

TEST_F(LRUCacheTest, GetDoesNotChangeCacheSize)
{
    LRUCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    static_cast<void>(cache.get(1));
    EXPECT_EQ(cache.size(), 2);
}
