/**
 * @file RobinHoodHashMapTest.cc
 * @brief Unit tests for the RobinHoodHashMap class
 * @details Tests cover insert, find, erase, iterator, copy/move semantics,
 *          load factor management, and edge cases.
 */

#include <string>
#include <utility>
#include <gtest/gtest.h>

#include <cppforge/data_structure/hash/RobinHoodHashMap.hpp>

using namespace cppforge::data_structure::hash;

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

/**
 * @brief Test default constructor creates empty map
 * @details Verifies that a newly constructed RobinHoodHashMap is empty, has zero size, and has a positive bucket count
 */
TEST_F(RobinHoodHashMapTest, DefaultConstructor_EmptyMap)
{
    const RobinHoodHashMap<int, std::string> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_GT(map.bucket_count(), 0);
}

/**
 * @brief Test capacity constructor creates map with minimum bucket count
 * @details Verifies that constructing with a capacity hint creates a map with at least that many buckets and is empty
 */
TEST_F(RobinHoodHashMapTest, CapacityConstructor)
{
    const RobinHoodHashMap<int, int> map(100);
    EXPECT_GE(map.bucket_count(), 100);
    EXPECT_TRUE(map.empty());
}

/**
 * @brief Test inserting a single key-value pair
 * @details Verifies that inserting a new key returns true, stores the correct value, and increments size
 */
TEST_F(RobinHoodHashMapTest, Insert_Single)
{
    RobinHoodHashMap<int, std::string> map;
    auto [it, inserted] = map.insert(42, "hello");
    EXPECT_TRUE(inserted);
    EXPECT_EQ((*it).second, "hello");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test inserting a duplicate key is ignored
 * @details Verifies that inserting a key that already exists returns false and does not overwrite the original value
 */
TEST_F(RobinHoodHashMapTest, Insert_DuplicateKeyIgnored)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "first");
    auto [it, inserted] = map.insert(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "first");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test insert_or_assign overwrites existing value
 * @details Verifies that insert_or_assign updates the value for an existing key and returns false (insertion did not occur)
 */
TEST_F(RobinHoodHashMapTest, InsertOrAssign_OverwritesValue)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "first");
    auto [it, inserted] = map.insert_or_assign(1, "second");
    EXPECT_FALSE(inserted);
    EXPECT_EQ((*it).second, "second");
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test insert_or_assign inserts when key is missing
 * @details Verifies that insert_or_assign inserts a new key-value pair when the key does not exist, returning true
 */
TEST_F(RobinHoodHashMapTest, InsertOrAssign_InsertsWhenMissing)
{
    RobinHoodHashMap<int, std::string> map;
    auto [it, inserted] = map.insert_or_assign(42, "value");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test inserting many key-value pairs
 * @details Verifies that inserting 1000 entries increases the size correctly and all values are retrievable via at()
 */
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

/**
 * @brief Test at returns value for an existing key
 * @details Verifies that at() returns the correct value for keys that exist in the map
 */
TEST_F(RobinHoodHashMapTest, At_ExistingKey)
{
    RobinHoodHashMap<std::string, int> map;
    map.insert("one", 1);
    map.insert("two", 2);
    EXPECT_EQ(map.at("one"), 1);
    EXPECT_EQ(map.at("two"), 2);
}

/**
 * @brief Test at throws on missing key
 * @details Verifies that at() throws std::out_of_range when the key is not present in the map
 */
TEST_F(RobinHoodHashMapTest, At_ThrowsOnMissingKey)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 100);
    EXPECT_THROW(static_cast<void>(map.at(99)), std::out_of_range);
}

/**
 * @brief Test operator[] inserts default value and allows modification
 * @details Verifies that operator[] inserts a default-constructed value for a missing key and returns a mutable reference
 */
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

/**
 * @brief Test contains returns correct presence status
 * @details Verifies that contains() returns true for inserted keys and false for missing keys
 */
TEST_F(RobinHoodHashMapTest, Contains)
{
    RobinHoodHashMap<int, std::string> map;
    map.insert(1, "a");
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_FALSE(map.contains(0));
}

/**
 * @brief Test find returns end iterator for missing key
 * @details Verifies that find() returns end() for non-existent keys and a valid iterator for existing keys
 */
TEST_F(RobinHoodHashMapTest, Find_ReturnsEndWhenMissing)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    EXPECT_EQ(map.find(99), map.end());
    EXPECT_NE(map.find(1), map.end());
}

/**
 * @brief Test erase removes an existing key
 * @details Verifies that erasing an existing key returns true, decreases size, and the key is no longer contained
 */
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

/**
 * @brief Test erase returns false for a missing key
 * @details Verifies that erasing a key not in the map returns false and size remains unchanged
 */
TEST_F(RobinHoodHashMapTest, Erase_MissingKey)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    EXPECT_FALSE(map.erase(99));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test reinserting after erase works correctly
 * @details Verifies that after erasing a key, a new value can be inserted with the same key
 */
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

/**
 * @brief Test erase by const iterator removes the entry
 * @details Verifies that erasing via a const iterator removes the correct entry and updates size
 */
TEST_F(RobinHoodHashMapTest, Erase_ByConstIterator)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    const auto it = map.find(1);
    ASSERT_NE(it, map.end());
    map.erase(it);
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
}

/**
 * @brief Test clear removes all entries from the map
 * @details Verifies that clear() empties the map, resets size to zero, and previously contained keys are no longer found
 */
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

/**
 * @brief Test load factor increases with insertions
 * @details Verifies that load_factor() starts at zero, increases after inserts, and never exceeds max_load_factor()
 */
TEST_F(RobinHoodHashMapTest, LoadFactor_GrowsWithInsert)
{
    RobinHoodHashMap<int, int> map;
    EXPECT_DOUBLE_EQ(map.load_factor(), 0.0);
    map.insert(1, 10);
    EXPECT_GT(map.load_factor(), 0.0);
    EXPECT_LE(map.load_factor(), map.max_load_factor());
}

/**
 * @brief Test rehash increases bucket count and preserves entries
 * @details Verifies that rehash() increases the bucket count to at least the requested size while preserving all existing key-value pairs
 */
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

/**
 * @brief Test reserve pre-allocates buckets without triggering rehash
 * @details Verifies that reserve() pre-allocates sufficient buckets so that subsequent insertions do not cause a rehash
 */
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

/**
 * @brief Test copy constructor creates an independent copy
 * @details Verifies that the copy constructor duplicates all entries and modifications to the original do not affect the copy
 */
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

/**
 * @brief Test copy assignment duplicates map contents
 * @details Verifies that copy assignment copies all entries from source to target map
 */
TEST_F(RobinHoodHashMapTest, CopyAssignment)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);
    map1.insert(2, 20);

    RobinHoodHashMap<int, int> map2(map1);
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
}

/**
 * @brief Test move constructor transfers map state
 * @details Verifies that the move constructor transfers all entries and the source map is left in a valid but unspecified state
 */
TEST_F(RobinHoodHashMapTest, MoveConstructor)
{
    RobinHoodHashMap<int, int> map1;
    map1.insert(1, 10);
    map1.insert(2, 20);

    RobinHoodHashMap<int, int> map2(std::move(map1));
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map2.at(1), 10);
}

/**
 * @brief Test move assignment transfers map state
 * @details Verifies that move assignment transfers entries from source to target, replacing previous content
 */
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

/**
 * @brief Test swap exchanges contents of two maps
 * @details Verifies that swap() exchanges the sizes and entries between two maps correctly
 */
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

/**
 * @brief Test range-based for iteration over the map
 * @details Verifies that the map supports range-based for loops and iterates over all inserted key-value pairs
 */
TEST_F(RobinHoodHashMapTest, ForEach_Iteration)
{
    RobinHoodHashMap<int, int> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);

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

/**
 * @brief Test map works with string keys
 * @details Verifies that the RobinHoodHashMap handles std::string keys correctly for insert, at, contains, and negative lookup
 */
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

/**
 * @brief Test large-scale insert and erase stress
 * @details Verifies that the map handles 10000 insertions followed by erasing every even key, maintaining correct size and data integrity
 */
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
