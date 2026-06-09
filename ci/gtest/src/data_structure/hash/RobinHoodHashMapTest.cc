/**
 * @file RobinHoodHashMapTest.cc
 * @brief Unit tests for the RobinHoodHashMap class
 * @details Tests cover insert, find, erase, iterator, copy/move semantics,
 *          load factor management, and edge cases.
 */

#include <string>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/hash/RobinHoodHashMap.hpp"

using namespace common::data_structure::hash;

/**
 * @brief Test fixture for RobinHoodHashMap tests
 */
class RobinHoodHashMapTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(RobinHoodHashMapTest, DefaultConstructor_EmptyMap)
{
    const RobinHoodHashMap<int, std::string> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_GT(map.bucket_count(), 0);
}

TEST_F(RobinHoodHashMapTest, CapacityConstructor)
{
    const RobinHoodHashMap<int, int> map(100);
    EXPECT_GE(map.bucket_count(), 100);
    EXPECT_TRUE(map.empty());
}

TEST_F(RobinHoodHashMapTest, Insert_Single)
{
    RobinHoodHashMap<int, std::string> map;
    auto [it, inserted] = map.insert(42, "hello");
    EXPECT_TRUE(inserted);
    EXPECT_EQ((*it).second, "hello");
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, Insert_DuplicateKeyIgnored)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "first");
    auto [it, inserted] = map.insert(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "first");
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, InsertOrAssign_OverwritesValue)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "first");
    auto [it, inserted] = map.insert_or_assign(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "second");
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, InsertOrAssign_InsertsWhenMissing)
{
    RobinHoodHashMap<int, std::string> map;
    auto [it, inserted] = map.insert_or_assign(42, "value");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, Insert_Many)
{
    RobinHoodHashMap<int, int> map;
    constexpr int n = 1000;
    for (int i = 0; i < n; ++i)
    {
        map.insert(i, i * 10);
    }
    EXPECT_EQ(map.size(), n);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_EQ(map.at(i), i * 10);
    }
}

TEST_F(RobinHoodHashMapTest, At_ExistingKey)
{
    RobinHoodHashMap<std::string, int> map;
    map.insert("one", 1);
    map.insert("two", 2);
    EXPECT_EQ(map.at("one"), 1);
    EXPECT_EQ(map.at("two"), 2);
}

TEST_F(RobinHoodHashMapTest, At_ThrowsOnMissingKey)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 100);
    EXPECT_THROW(static_cast<void>(map.at(99)), std::out_of_range);
}

TEST_F(RobinHoodHashMapTest, OperatorBracket_InsertDefault)
{
    RobinHoodHashMap<int, int> map;
    map[5] = 50;
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[5], 50);

    int& ref = map[10];
    EXPECT_EQ(ref, 0); // default-inserted
    ref = 100;
    EXPECT_EQ(map[10], 100);
}

TEST_F(RobinHoodHashMapTest, Contains)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "a");
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_FALSE(map.contains(0));
}

TEST_F(RobinHoodHashMapTest, Find_ReturnsEndWhenMissing)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    EXPECT_EQ(map.find(99), map.end());
    EXPECT_NE(map.find(1), map.end());
}

TEST_F(RobinHoodHashMapTest, Erase_ExistingKey)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "a");
    map.insert(2, "b");
    EXPECT_TRUE(map.erase(1));
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

TEST_F(RobinHoodHashMapTest, Erase_MissingKey)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    EXPECT_FALSE(map.erase(99));
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, Erase_ReinsertAfterErase)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "a");
    EXPECT_TRUE(map.erase(1));
    EXPECT_TRUE(map.empty());
    auto [it, inserted] = map.insert(1, "b");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(map.at(1), "b");
    EXPECT_EQ(map.size(), 1);
}

TEST_F(RobinHoodHashMapTest, Erase_ByConstIterator)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    auto it = map.find(1);
    ASSERT_NE(it, map.end());
    map.erase(it);
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
}

TEST_F(RobinHoodHashMapTest, Clear)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains(1));
}

TEST_F(RobinHoodHashMapTest, LoadFactor_GrowsWithInsert)
{
    RobinHoodHashMap<int, int> map;
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
    map.insert(1, 10);
    EXPECT_GT(map.load_factor(), 0.0);
    EXPECT_LE(map.load_factor(), map.max_load_factor());
}

TEST_F(RobinHoodHashMapTest, Rehash)
{
    RobinHoodHashMap<int, int> map;
    for (int i = 0; i < 50; ++i)
    {
        map.insert(i, i);
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

TEST_F(RobinHoodHashMapTest, Reserve)
{
    RobinHoodHashMap<int, int> map;
    map.reserve(1000);
    const auto cap = map.bucket_count();
    for (int i = 0; i < 500; ++i)
    {
        map.insert(i, i);
    }
    EXPECT_EQ(map.bucket_count(), cap); // no rehash triggered
    EXPECT_EQ(map.size(), 500);
}

TEST_F(RobinHoodHashMapTest, CopyConstructor)
{
    RobinHoodHashMap<int, std::string> map1;
    map1.insert(1, "a");
    map1.insert(2, "b");

    RobinHoodHashMap<int, std::string> map2(map1);
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), "a");
    EXPECT_EQ(map2.at(2), "b");

    map1.insert(3, "c");
    EXPECT_EQ(map2.size(), 2);
    EXPECT_FALSE(map2.contains(3));
}

TEST_F(RobinHoodHashMapTest, CopyAssignment)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);
    map1.insert(2, 20);

    RobinHoodHashMap<int, int> map2;
    map2 = map1;
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
}

TEST_F(RobinHoodHashMapTest, MoveConstructor)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);
    map1.insert(2, 20);

    RobinHoodHashMap<int, int> map2(std::move(map1));
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
}

TEST_F(RobinHoodHashMapTest, MoveAssignment)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);

    RobinHoodHashMap<int, int> map2;
    map2.insert(99, 999);
    map2 = std::move(map1);

    EXPECT_EQ(map2.size(), 1);
    EXPECT_EQ(map2.at(1), 10);
}

TEST_F(RobinHoodHashMapTest, Swap)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);
    RobinHoodHashMap<int, int> map2;
    map2.insert(2, 20);
    map2.insert(3, 30);

    map1.swap(map2);
    EXPECT_EQ(map1.size(), 2);
    EXPECT_EQ(map2.size(), 1);
    EXPECT_TRUE(map1.contains(2));
    EXPECT_TRUE(map2.contains(1));
}

TEST_F(RobinHoodHashMapTest, ForEach_Iteration)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);

    int sum = 0;
    for (auto [key, value] : map)
    {
        sum += value;
    }
    EXPECT_EQ(sum, 60);
}

TEST_F(RobinHoodHashMapTest, ConstIteration)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);

    const auto& cmap = map;
    int count = 0;
    for ([[maybe_unused]] auto [key, value] : cmap)
    {
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(RobinHoodHashMapTest, StringKeys)
{
    RobinHoodHashMap<std::string, int> map;
    map.insert("alpha", 1);
    map.insert("beta", 2);
    map.insert("gamma", 3);
    EXPECT_EQ(map.at("beta"), 2);
    EXPECT_TRUE(map.contains("gamma"));
    EXPECT_FALSE(map.contains("delta"));
}

TEST_F(RobinHoodHashMapTest, LargeStressTest)
{
    RobinHoodHashMap<int, int> map;
    constexpr int n = 10000;
    for (int i = 0; i < n; ++i)
    {
        map.insert(i, i * 2);
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
