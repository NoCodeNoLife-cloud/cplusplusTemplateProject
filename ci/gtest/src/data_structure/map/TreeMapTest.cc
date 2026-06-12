/**
 * @file TreeMapTest.cc
 * @brief Unit tests for the TreeMap class
 * @details Tests cover core map operations including insert, erase, contains, at,
 *          operator[], findValue, min, max, toVector, copy/move semantics, and edge cases.
 */

#include <string>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/map/TreeMap.hpp"

using namespace common::data_structure::map;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<TreeMap<int, std::string>>);
static_assert(std::is_nothrow_move_assignable_v<TreeMap<int, std::string>>);
static_assert(noexcept(std::declval<TreeMap<int, std::string>&>().swap(std::declval<TreeMap<int, std::string>&>())));
static_assert(noexcept(std::declval<const TreeMap<int, std::string>&>().empty()));
static_assert(noexcept(std::declval<const TreeMap<int, std::string>&>().size()));

/**
 * @brief Test fixture for TreeMap tests
 */
class TreeMapTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Constructor & Empty Tests ====================

/**
 * @brief Test default constructor creates empty map
 */
TEST_F(TreeMapTest, DefaultConstructor_EmptyMap)
{
    const TreeMap<int, std::string> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 */
TEST_F(TreeMapTest, CopyConstructor_DeepCopy)
{
    TreeMap<int, std::string> map1;
    EXPECT_TRUE(map1.insert(1, "one"));
    EXPECT_TRUE(map1.insert(2, "two"));
    EXPECT_TRUE(map1.insert(3, "three"));

    const TreeMap map2(map1);

    EXPECT_EQ(map2.size(), map1.size());
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_TRUE(map2.contains(3));

    // Modify original should not affect copy
    map1.erase(1);
    EXPECT_TRUE(map2.contains(1));
}

/**
 * @brief Test copy constructor with empty map
 */
TEST_F(TreeMapTest, CopyConstructor_EmptyMap)
{
    const TreeMap<int, std::string> map1;
    const TreeMap map2(map1);

    EXPECT_TRUE(map2.empty());
    EXPECT_EQ(map2.size(), 0);
}

/**
 * @brief Test move constructor transfers ownership
 */
TEST_F(TreeMapTest, MoveConstructor_TransfersOwnership)
{
    TreeMap<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    TreeMap map2(std::move(map1));

    EXPECT_EQ(map2.size(), 2);
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));

    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_TRUE(map1.empty());
}

/**
 * @brief Test copy assignment operator
 */
TEST_F(TreeMapTest, CopyAssignment_DeepCopy)
{
    TreeMap<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    TreeMap<int, std::string> map2;
    map2.insert(99, "ninety-nine");
    map2 = map1;

    EXPECT_EQ(map2.size(), 2);
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    EXPECT_FALSE(map2.contains(99));
}

/**
 * @brief Test move assignment operator
 */
TEST_F(TreeMapTest, MoveAssignment_TransfersOwnership)
{
    TreeMap<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    TreeMap<int, std::string> map2;
    map2.insert(99, "ninety-nine");
    map2 = std::move(map1);

    EXPECT_EQ(map2.size(), 2);
    EXPECT_TRUE(map2.contains(1));
    EXPECT_TRUE(map2.contains(2));
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_TRUE(map1.empty());
}

// ==================== Insert Tests ====================

/**
 * @brief Test insert adds new element
 */
TEST_F(TreeMapTest, Insert_NewKey_ReturnsTrue)
{
    TreeMap<int, std::string> map;
    EXPECT_TRUE(map.insert(42, "answer"));
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(42));
}

/**
 * @brief Test insert duplicate key
 */
TEST_F(TreeMapTest, Insert_DuplicateKey_ReturnsFalse)
{
    TreeMap<int, std::string> map;
    EXPECT_TRUE(map.insert(42, "answer"));
    EXPECT_FALSE(map.insert(42, "updated"));
    EXPECT_EQ(map.size(), 1);
    // Value should remain unchanged
    EXPECT_EQ(map.at(42), "answer");
}

/**
 * @brief Test insert multiple keys in ascending order
 */
TEST_F(TreeMapTest, Insert_AscendingOrder)
{
    TreeMap<int, std::string> map;
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(map.insert(i, std::to_string(i)));
    }
    EXPECT_EQ(map.size(), 100);

    const auto vec = map.toVector();
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(vec[i].first, i);
        EXPECT_EQ(vec[i].second, std::to_string(i));
    }
}

/**
 * @brief Test insert keys in descending order
 */
TEST_F(TreeMapTest, Insert_DescendingOrder)
{
    TreeMap<int, std::string> map;
    for (int i = 99; i >= 0; --i)
    {
        EXPECT_TRUE(map.insert(i, std::to_string(i)));
    }
    EXPECT_EQ(map.size(), 100);

    const auto vec = map.toVector();
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(vec[i].first, i);
        EXPECT_EQ(vec[i].second, std::to_string(i));
    }
}

/**
 * @brief Test insert keys in random order
 */
TEST_F(TreeMapTest, Insert_RandomOrder)
{
    TreeMap<int, std::string> map;
    const std::vector<std::pair<int, std::string>> entries = {
        {5, "five"}, {3, "three"}, {8, "eight"}, {1, "one"},
        {9, "nine"}, {2, "two"}, {7, "seven"}, {4, "four"},
        {6, "six"}, {0, "zero"}
    };

    for (const auto& [k, v] : entries)
    {
        EXPECT_TRUE(map.insert(k, v));
    }
    EXPECT_EQ(map.size(), 10);

    const auto vec = map.toVector();
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(vec[i].first, i);
    }
}

// ==================== Insert or Assign Tests ====================

/**
 * @brief Test insert_or_assign with new key
 */
TEST_F(TreeMapTest, InsertOrAssign_NewKey_ReturnsTrue)
{
    TreeMap<int, std::string> map;
    EXPECT_TRUE(map.insert_or_assign(1, "one"));
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.at(1), "one");
}

/**
 * @brief Test insert_or_assign with existing key
 */
TEST_F(TreeMapTest, InsertOrAssign_ExistingKey_ReturnsFalse)
{
    TreeMap<int, std::string> map;
    EXPECT_TRUE(map.insert_or_assign(1, "one"));
    EXPECT_FALSE(map.insert_or_assign(1, "uno"));
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.at(1), "uno");
}

// ==================== Erase Tests ====================

/**
 * @brief Test erase existing key
 */
TEST_F(TreeMapTest, Erase_ExistingKey_ReturnsTrue)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    map.insert(2, "two");

    EXPECT_TRUE(map.erase(1));
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

/**
 * @brief Test erase non-existing key
 */
TEST_F(TreeMapTest, Erase_NonExisting_ReturnsFalse)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    EXPECT_FALSE(map.erase(99));
    EXPECT_EQ(map.size(), 1);
}

/**
 * @brief Test erase root key
 */
TEST_F(TreeMapTest, Erase_RootKey)
{
    TreeMap<int, std::string> map;
    map.insert(5, "five");
    map.insert(3, "three");
    map.insert(8, "eight");
    map.insert(1, "one");
    map.insert(4, "four");
    map.insert(7, "seven");
    map.insert(9, "nine");

    EXPECT_TRUE(map.erase(5));
    EXPECT_FALSE(map.contains(5));
    EXPECT_EQ(map.size(), 6);

    const std::vector<int> expected_keys = {1, 3, 4, 7, 8, 9};
    const auto vec = map.toVector();
    for (size_t i = 0; i < expected_keys.size(); ++i)
    {
        EXPECT_EQ(vec[i].first, expected_keys[i]);
    }
}

/**
 * @brief Test erase leaf key
 */
TEST_F(TreeMapTest, Erase_LeafKey)
{
    TreeMap<int, std::string> map;
    map.insert(5, "five");
    map.insert(3, "three");
    map.insert(8, "eight");

    EXPECT_TRUE(map.erase(3));
    EXPECT_FALSE(map.contains(3));
    EXPECT_EQ(map.size(), 2);

    const std::vector<int> expected_keys = {5, 8};
    const auto vec = map.toVector();
    for (size_t i = 0; i < expected_keys.size(); ++i)
    {
        EXPECT_EQ(vec[i].first, expected_keys[i]);
    }
}

/**
 * @brief Test erase all keys
 */
TEST_F(TreeMapTest, Erase_AllKeys)
{
    TreeMap<int, std::string> map;
    for (int i = 0; i < 10; ++i)
    {
        map.insert(i, std::to_string(i));
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_TRUE(map.empty());
}

// ==================== Contains & Find Tests ====================

/**
 * @brief Test contains returns true for existing keys
 */
TEST_F(TreeMapTest, Contains_ExistingKey_ReturnsTrue)
{
    TreeMap<int, std::string> map;
    map.insert(42, "answer");
    EXPECT_TRUE(map.contains(42));
}

/**
 * @brief Test contains returns false for missing keys
 */
TEST_F(TreeMapTest, Contains_NonExisting_ReturnsFalse)
{
    const TreeMap<int, std::string> map;
    EXPECT_FALSE(map.contains(42));
}

/**
 * @brief Test findValue returns value for existing key
 */
TEST_F(TreeMapTest, FindValue_ExistingKey_ReturnsValue)
{
    TreeMap<int, std::string> map;
    map.insert(42, "answer");
    const auto result = map.findValue(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "answer");
}

/**
 * @brief Test findValue returns nullopt for missing key
 */
TEST_F(TreeMapTest, FindValue_NonExisting_ReturnsNullopt)
{
    const TreeMap<int, std::string> map;
    EXPECT_FALSE(map.findValue(42).has_value());
}

// ==================== Element Access Tests ====================

/**
 * @brief Test at returns reference to existing key
 */
TEST_F(TreeMapTest, At_ExistingKey_ReturnsReference)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    EXPECT_EQ(map.at(1), "one");

    // Modify through reference
    map.at(1) = "uno";
    EXPECT_EQ(map.at(1), "uno");
}

/**
 * @brief Test at throws for missing key
 */
TEST_F(TreeMapTest, At_MissingKey_ThrowsOutOfRange)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    EXPECT_THROW(map.at(99), std::out_of_range);
}

/**
 * @brief Test const at throws for missing key
 */
TEST_F(TreeMapTest, At_Const_MissingKey_ThrowsOutOfRange)
{
    const TreeMap<int, std::string> map;
    EXPECT_THROW(map.at(42), std::out_of_range);
}

/**
 * @brief Test operator[] reads existing key
 */
TEST_F(TreeMapTest, OperatorBracket_ExistingKey_ReturnsReference)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    EXPECT_EQ(map[1], "one");

    // Modify through bracket
    map[1] = "uno";
    EXPECT_EQ(map[1], "uno");
}

/**
 * @brief Test operator[] inserts default for missing key
 */
TEST_F(TreeMapTest, OperatorBracket_MissingKey_InsertsDefault)
{
    TreeMap<int, std::string> map;
    EXPECT_EQ(map.size(), 0);

    const auto& ref = map[42];
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(ref.empty());
    EXPECT_TRUE(map.contains(42));

    // Assign after default insert
    map[42] = "answer";
    EXPECT_EQ(map[42], "answer");
}

// ==================== Min & Max Tests ====================

/**
 * @brief Test min returns smallest key-value pair
 */
TEST_F(TreeMapTest, Min_ReturnsSmallestKey)
{
    TreeMap<int, std::string> map;
    map.insert(10, "ten");
    map.insert(5, "five");
    map.insert(20, "twenty");
    map.insert(1, "one");

    const auto result = map.min();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, 1);
    EXPECT_EQ(result->second, "one");
}

/**
 * @brief Test max returns largest key-value pair
 */
TEST_F(TreeMapTest, Max_ReturnsLargestKey)
{
    TreeMap<int, std::string> map;
    map.insert(10, "ten");
    map.insert(5, "five");
    map.insert(20, "twenty");
    map.insert(1, "one");

    const auto result = map.max();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, 20);
    EXPECT_EQ(result->second, "twenty");
}

/**
 * @brief Test min on empty map returns nullopt
 */
TEST_F(TreeMapTest, Min_EmptyMap_ReturnsNullopt)
{
    const TreeMap<int, std::string> map;
    EXPECT_FALSE(map.min().has_value());
}

/**
 * @brief Test max on empty map returns nullopt
 */
TEST_F(TreeMapTest, Max_EmptyMap_ReturnsNullopt)
{
    const TreeMap<int, std::string> map;
    EXPECT_FALSE(map.max().has_value());
}

/**
 * @brief Test min and max on single element
 */
TEST_F(TreeMapTest, MinMax_SingleElement)
{
    TreeMap<int, std::string> map;
    map.insert(42, "answer");

    const auto min_result = map.min();
    ASSERT_TRUE(min_result.has_value());
    EXPECT_EQ(min_result->first, 42);
    EXPECT_EQ(min_result->second, "answer");

    const auto max_result = map.max();
    ASSERT_TRUE(max_result.has_value());
    EXPECT_EQ(max_result->first, 42);
    EXPECT_EQ(max_result->second, "answer");
}

/**
 * @brief Test min and max update correctly during insert and erase
 */
TEST_F(TreeMapTest, MinMax_AfterInsertAndErase)
{
    TreeMap<int, std::string> map;
    map.insert(50, "fifty");
    EXPECT_EQ(map.min()->first, 50);
    EXPECT_EQ(map.max()->first, 50);

    map.insert(10, "ten");
    EXPECT_EQ(map.min()->first, 10);
    EXPECT_EQ(map.max()->first, 50);

    map.insert(90, "ninety");
    EXPECT_EQ(map.min()->first, 10);
    EXPECT_EQ(map.max()->first, 90);

    map.erase(10);
    EXPECT_EQ(map.min()->first, 50);
    EXPECT_EQ(map.max()->first, 90);

    map.erase(90);
    EXPECT_EQ(map.min()->first, 50);
    EXPECT_EQ(map.max()->first, 50);
}

// ==================== ToVector Tests ====================

/**
 * @brief Test toVector returns sorted key-value pairs
 */
TEST_F(TreeMapTest, ToVector_ReturnsSortedOrder)
{
    TreeMap<int, std::string> map;
    map.insert(5, "five");
    map.insert(3, "three");
    map.insert(8, "eight");
    map.insert(1, "one");
    map.insert(4, "four");

    const std::vector<std::pair<int, std::string>> expected = {
        {1, "one"}, {3, "three"}, {4, "four"}, {5, "five"}, {8, "eight"}
    };
    EXPECT_EQ(map.toVector(), expected);
}

/**
 * @brief Test toVector on empty map
 */
TEST_F(TreeMapTest, ToVector_EmptyMap)
{
    const TreeMap<int, std::string> map;
    EXPECT_TRUE(map.toVector().empty());
}

// ==================== Clear & Swap Tests ====================

/**
 * @brief Test clear empties the map
 */
TEST_F(TreeMapTest, Clear_EmptiesMap)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

/**
 * @brief Test clear on empty map is no-op
 */
TEST_F(TreeMapTest, Clear_EmptyMap_NoOp)
{
    TreeMap<int, std::string> map;
    map.clear();
    EXPECT_TRUE(map.empty());
}

/**
 * @brief Test clear then reuse
 */
TEST_F(TreeMapTest, Clear_ThenReuse)
{
    TreeMap<int, std::string> map;
    map.insert(1, "one");
    map.clear();

    map.insert(10, "ten");
    map.insert(20, "twenty");
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(10));
    EXPECT_TRUE(map.contains(20));
}

/**
 * @brief Test swap exchanges contents
 */
TEST_F(TreeMapTest, Swap_ExchangesContents)
{
    TreeMap<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    TreeMap<int, std::string> map2;
    map2.insert(10, "ten");
    map2.insert(20, "twenty");
    map2.insert(30, "thirty");

    map1.swap(map2);

    EXPECT_EQ(map1.size(), 3);
    EXPECT_TRUE(map1.contains(10));
    EXPECT_EQ(map1.at(10), "ten");

    EXPECT_EQ(map2.size(), 2);
    EXPECT_TRUE(map2.contains(1));
    EXPECT_EQ(map2.at(1), "one");
}

// ==================== Edge Case Tests ====================

/**
 * @brief Test string keys
 */
TEST_F(TreeMapTest, StringKeys_Operations)
{
    TreeMap<std::string, int> map;
    EXPECT_TRUE(map.insert("banana", 2));
    EXPECT_TRUE(map.insert("apple", 5));
    EXPECT_TRUE(map.insert("cherry", 10));

    EXPECT_EQ(map.size(), 3);
    EXPECT_TRUE(map.contains("apple"));
    EXPECT_TRUE(map.contains("banana"));
    EXPECT_TRUE(map.contains("cherry"));
    EXPECT_EQ(map.at("apple"), 5);
    EXPECT_EQ(map.at("banana"), 2);
    EXPECT_EQ(map.at("cherry"), 10);

    // Verify sorted order by key
    const auto vec = map.toVector();
    const std::vector<std::pair<std::string, int>> expected = {
        {"apple", 5}, {"banana", 2}, {"cherry", 10}
    };
    EXPECT_EQ(vec, expected);

    // Verify min and max
    EXPECT_EQ(map.min()->first, "apple");
    EXPECT_EQ(map.max()->first, "cherry");
}

/**
 * @brief Test insert and erase cycle
 */
TEST_F(TreeMapTest, InsertErase_Cycle)
{
    TreeMap<int, std::string> map;

    for (int i = 0; i < 10; ++i)
    {
        map.insert(i, std::to_string(i));
    }
    EXPECT_EQ(map.size(), 10);

    for (int i = 5; i < 10; ++i)
    {
        map.erase(i);
    }
    EXPECT_EQ(map.size(), 5);

    const auto vec = map.toVector();
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec[i].first, i);
        EXPECT_EQ(vec[i].second, std::to_string(i));
    }
}

// ==================== Large Data Set Tests ====================

/**
 * @brief Test large number of elements
 */
TEST_F(TreeMapTest, LargeNumberOfElements_Correctness)
{
    TreeMap<int, int> map;
    constexpr int n = 1000;

    for (int i = 0; i < n; ++i)
    {
        EXPECT_TRUE(map.insert(i, i * 2));
    }
    EXPECT_EQ(map.size(), n);

    for (int i = 0; i < n; ++i)
    {
        EXPECT_TRUE(map.contains(i));
        EXPECT_EQ(map.at(i), i * 2);
    }

    // Verify sorted order
    const auto vec = map.toVector();
    EXPECT_EQ(vec.size(), n);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_EQ(vec[i].first, i);
        EXPECT_EQ(vec[i].second, i * 2);
    }

    // Erase every other key
    for (int i = 0; i < n; i += 2)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_EQ(map.size(), n / 2);

    // Verify remaining elements
    for (int i = 0; i < n; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(map.contains(i));
        }
        else
        {
            EXPECT_TRUE(map.contains(i));
            EXPECT_EQ(map.at(i), i * 2);
        }
    }
}
