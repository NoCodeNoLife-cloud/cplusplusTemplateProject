/**
 * @file ICacheTest.cc
 * @brief Unit tests for the ICache interface contract
 * @details Tests verify the contract defined by ICache: get/put/remove/clear,
 *          size/empty/capacity/contains, and edge cases.
 *          Uses a minimal concrete map-based stub implementation.
 */

#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "interface/ICache.hpp"

using namespace cppforge::interface::cache;

/**
 * @brief Minimal concrete stub implementing ICache for contract testing.
 * @details Uses std::unordered_map with fixed capacity and LRU approximation.
 */
template <typename Key, typename Value>
class TestCache final : public ICache<Key, Value>
{
public:
    explicit TestCache(size_t capacity) : data_(), capacity_(capacity) {}

    std::optional<Value> get(const Key& key) override
    {
        auto it = data_.find(key);
        if (it == data_.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    bool put(const Key& key, const Value& value) override
    {
        if (data_.size() >= capacity_ && !data_.contains(key))
        {
            return false; // Simulate eviction failure
        }
        data_[key] = value;
        return true;
    }

    bool put(const Key& key, Value&& value) override
    {
        if (data_.size() >= capacity_ && !data_.contains(key))
        {
            return false;
        }
        data_[key] = std::move(value);
        return true;
    }

    bool remove(const Key& key) override
    {
        return data_.erase(key) > 0;
    }

    void clear() override
    {
        data_.clear();
    }

    [[nodiscard]] size_t size() const override
    {
        return data_.size();
    }

    [[nodiscard]] size_t capacity() const override
    {
        return capacity_;
    }

    [[nodiscard]] bool empty() const override
    {
        return data_.empty();
    }

    [[nodiscard]] bool contains(const Key& key) const override
    {
        return data_.contains(key);
    }

private:
    std::unordered_map<Key, Value> data_;
    size_t capacity_;
};

/**
 * @brief Test fixture for ICacheTest tests
 */
class ICacheTest : public testing::Test
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
 * @brief Test put and get basic
 * @details Verifies basic put/get contract
 */
TEST_F(ICacheTest, PutAndGet_Basic)
{
    TestCache<int, std::string> cache(3);
    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    const auto result1 = cache.get(1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "one");

    const auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");
}

/**
 * @brief Test get for non-existent key
 * @details Non-existent key should return nullopt
 */
TEST_F(ICacheTest, Get_NonExistentKey)
{
    TestCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    const auto result = cache.get(999);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put updates existing key
 * @details Putting with existing key should update value
 */
TEST_F(ICacheTest, Put_UpdateExistingKey)
{
    TestCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(1, "ONE"));

    const auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "ONE");
}

/**
 * @brief Test put with rvalue reference
 * @details Move semantics should work for put
 */
TEST_F(ICacheTest, Put_RValueReference)
{
    TestCache<int, std::string> cache(3);

    std::string value = "test";
    EXPECT_TRUE(cache.put(1, std::move(value)));

    const auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "test");
}

/**
 * @brief Test remove existing key
 * @details remove should return true and make key unavailable
 */
TEST_F(ICacheTest, Remove_ExistingKey)
{
    TestCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    const bool removed = cache.remove(1);
    EXPECT_TRUE(removed);

    const auto result = cache.get(1);
    EXPECT_FALSE(result.has_value());

    const auto result2 = cache.get(2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "two");
}

/**
 * @brief Test remove non-existent key
 * @details remove for non-existent key should return false
 */
TEST_F(ICacheTest, Remove_NonExistentKey)
{
    TestCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.put(1, "one"));
    const bool removed = cache.remove(999);
    EXPECT_FALSE(removed);
}

/**
 * @brief Test clear removes all entries
 * @details clear should empty the cache
 */
TEST_F(ICacheTest, Clear_AllEntries)
{
    TestCache<int, std::string> cache(3);

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
 * @details After clearing, cache should accept new entries
 */
TEST_F(ICacheTest, ClearThenReuse)
{
    TestCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));
    cache.clear();
    EXPECT_TRUE(cache.empty());

    EXPECT_TRUE(cache.put(3, "three"));
    EXPECT_TRUE(cache.put(4, "four"));
    EXPECT_EQ(cache.size(), 2);
}

/**
 * @brief Test size
 * @details size should reflect number of entries
 */
TEST_F(ICacheTest, Size)
{
    TestCache<int, std::string> cache(5);

    EXPECT_EQ(cache.size(), 0);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_EQ(cache.size(), 1);

    EXPECT_TRUE(cache.put(2, "two"));
    EXPECT_EQ(cache.size(), 2);

    EXPECT_TRUE(cache.put(1, "ONE"));
    EXPECT_EQ(cache.size(), 2);
}

/**
 * @brief Test capacity
 * @details capacity should return configured max
 */
TEST_F(ICacheTest, Capacity)
{
    const TestCache<int, std::string> cache(10);
    EXPECT_EQ(cache.capacity(), 10);
}

/**
 * @brief Test empty
 * @details empty should reflect cache state
 */
TEST_F(ICacheTest, Empty)
{
    TestCache<int, std::string> cache(3);

    EXPECT_TRUE(cache.empty());

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_FALSE(cache.empty());

    cache.clear();
    EXPECT_TRUE(cache.empty());
}

/**
 * @brief Test contains
 * @details contains should identify existing keys
 */
TEST_F(ICacheTest, Contains)
{
    TestCache<int, std::string> cache(3);

    EXPECT_FALSE(cache.contains(1));

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.contains(1));

    EXPECT_TRUE(cache.remove(1));
    EXPECT_FALSE(cache.contains(1));
}

/**
 * @brief Test capacity enforcement
 * @details When full, put of new key returns false
 */
TEST_F(ICacheTest, CapacityFullRejectsPut)
{
    TestCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // At capacity, new key should fail
    EXPECT_FALSE(cache.put(3, "three"));
}

/**
 * @brief Test update at capacity
 * @details Updating existing key at capacity should succeed
 */
TEST_F(ICacheTest, UpdateExistingAtCapacity)
{
    TestCache<int, std::string> cache(2);

    EXPECT_TRUE(cache.put(1, "one"));
    EXPECT_TRUE(cache.put(2, "two"));

    // Update should work even at capacity
    EXPECT_TRUE(cache.put(1, "ONE"));
    EXPECT_EQ(cache.size(), 2);
}

/**
 * @brief Test string keys
 * @details Verify cache works with string keys
 */
TEST_F(ICacheTest, StringKeys)
{
    TestCache<std::string, int> cache(3);

    EXPECT_TRUE(cache.put("alpha", 1));
    EXPECT_TRUE(cache.put("beta", 2));
    EXPECT_TRUE(cache.put("gamma", 3));

    EXPECT_EQ(cache.get("alpha").value_or(-1), 1);
    EXPECT_EQ(cache.get("beta").value_or(-1), 2);
    EXPECT_EQ(cache.get("gamma").value_or(-1), 3);
    EXPECT_FALSE(cache.get("delta").has_value());
}

/**
 * @brief Test remove non-existent key returns false
 * @edge Double-check remove contract
 */
TEST_F(ICacheTest, RemoveNonExistent)
{
    TestCache<int, std::string> cache(3);
    EXPECT_FALSE(cache.remove(42));
}

/**
 * @brief Test const correctness of size/capacity/empty/contains
 * @details Const methods should work on const cache reference
 */
TEST_F(ICacheTest, ConstMethods)
{
    TestCache<int, std::string> cache(3);
    cache.put(1, "one");

    const auto& constCache = cache;
    EXPECT_EQ(constCache.size(), 1);
    EXPECT_EQ(constCache.capacity(), 3);
    EXPECT_FALSE(constCache.empty());
    EXPECT_TRUE(constCache.contains(1));
}
