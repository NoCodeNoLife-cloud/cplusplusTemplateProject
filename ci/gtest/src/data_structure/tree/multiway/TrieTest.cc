/**
 * @file TrieTest.cc
 * @brief Unit tests for the Trie class
 * @details Tests cover insertion, search, prefix query, deletion,
 *          iteration, edge cases, and type compatibility.
 */

#include <algorithm>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/tree/multiway/Trie.hpp"

using namespace common::data_structure;
using namespace common::data_structure::tree::multiway;

class TrieTest : public testing::Test
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
 * @brief Test default constructor creates an empty trie
 * @details Verifies that a newly constructed trie is empty with size 0
 */
TEST_F(TrieTest, DefaultConstructor_EmptyTrie)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.empty());
    EXPECT_EQ(trie.size(), 0);
}

/**
 * @brief Test that a new trie is initially empty
 * @details Verifies empty() returns true for a default-constructed trie
 */
TEST_F(TrieTest, Empty_InitiallyEmpty)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.empty());
}

/**
 * @brief Test that trie is not empty after insertion
 * @details Verifies that empty() returns false after inserting a key
 */
TEST_F(TrieTest, Empty_AfterInsert)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_FALSE(trie.empty());
}

/**
 * @brief Test that trie is empty after clear
 * @details Verifies that clear restores the trie to empty state
 */
TEST_F(TrieTest, Empty_AfterClear)
{
    Trie<int> trie;
    trie.insert("key", 1);
    trie.clear();
    EXPECT_TRUE(trie.empty());
}

// ==================== Insertion Tests ====================

/**
 * @brief Test inserting a single key
 * @details Verifies that find returns the correct value after insertion
 */
TEST_F(TrieTest, Insert_SingleKey_FindReturnsValue)
{
    Trie<int> trie;
    trie.insert("hello", 42);
    EXPECT_EQ(trie.size(), 1);
    const int* val = trie.find("hello");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 42);
}

/**
 * @brief Test inserting multiple distinct keys
 * @details Verifies that all inserted keys can be found and non-existing keys are not
 */
TEST_F(TrieTest, Insert_MultipleKeys_AllFound)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("banana", 2);
    trie.insert("cherry", 3);
    EXPECT_EQ(trie.size(), 3);
    EXPECT_TRUE(trie.contains("apple"));
    EXPECT_TRUE(trie.contains("banana"));
    EXPECT_TRUE(trie.contains("cherry"));
    EXPECT_FALSE(trie.contains("grape"));
}

/**
 * @brief Test inserting a duplicate key updates the value
 * @details Verifies that re-inserting an existing key overwrites its value
 */
TEST_F(TrieTest, Insert_DuplicateKey_UpdatesValue)
{
    Trie<int> trie;
    trie.insert("key", 1);
    trie.insert("key", 100);
    EXPECT_EQ(trie.size(), 1);
    const int* val = trie.find("key");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 100);
}

/**
 * @brief Test inserting an empty string as a key
 * @details Verifies that empty string can be used as a valid key
 */
TEST_F(TrieTest, Insert_EmptyStringKey)
{
    Trie<int> trie;
    trie.insert("", 42);
    EXPECT_EQ(trie.size(), 1);
    EXPECT_TRUE(trie.contains(""));
}

/**
 * @brief Test inserting keys with overlapping prefixes
 * @details Verifies that keys sharing a common prefix are all stored correctly
 */
TEST_F(TrieTest, Insert_OverlappingPrefixes)
{
    Trie<int> trie;
    trie.insert("a", 1);
    trie.insert("ab", 2);
    trie.insert("abc", 3);
    EXPECT_EQ(trie.size(), 3);
    EXPECT_TRUE(trie.contains("a"));
    EXPECT_TRUE(trie.contains("ab"));
    EXPECT_TRUE(trie.contains("abc"));
}

// ==================== Find & Contains Tests ====================

/**
 * @brief Test find on a non-existing key
 * @details Verifies that find returns nullptr for missing keys
 */
TEST_F(TrieTest, Find_NonExisting_ReturnsNullptr)
{
    const Trie<int> trie;
    EXPECT_EQ(trie.find("nonexistent"), nullptr);
}

/**
 * @brief Test find on an existing key
 * @details Verifies that find returns a pointer to the stored value
 */
TEST_F(TrieTest, Find_Existing_ReturnsPointer)
{
    Trie<int> trie;
    trie.insert("key", 42);
    const int* val = trie.find("key");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 42);
}

/**
 * @brief Test that mutable find allows value modification
 * @details Verifies that the pointer returned by find can be used to modify the value
 */
TEST_F(TrieTest, Find_Mutable_AllowsModification)
{
    Trie<int> trie;
    trie.insert("key", 1);
    int* val = trie.find("key");
    ASSERT_NE(val, nullptr);
    *val = 999;
    const int* val2 = trie.find("key");
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(*val2, 999);
}

/**
 * @brief Test contains on an existing key
 * @details Verifies that contains returns true for an inserted key
 */
TEST_F(TrieTest, Contains_ExistingKey_ReturnsTrue)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_TRUE(trie.contains("key"));
}

/**
 * @brief Test contains on a non-existing key
 * @details Verifies that contains returns false for missing keys
 */
TEST_F(TrieTest, Contains_NonExisting_ReturnsFalse)
{
    const Trie<int> trie;
    EXPECT_FALSE(trie.contains("anything"));
}

/**
 * @brief Test that contains rejects partial prefixes
 * @details Verifies that only full keys match, not prefixes
 */
TEST_F(TrieTest, Contains_PartialPrefix_ReturnsFalse)
{
    Trie<int> trie;
    trie.insert("hello", 1);
    EXPECT_FALSE(trie.contains("hel"));
    EXPECT_FALSE(trie.contains("hell"));
    EXPECT_TRUE(trie.contains("hello"));
}

// ==================== Erase Tests ====================

/**
 * @brief Test erasing an existing key
 * @details Verifies that erase returns true and removes the key
 */
TEST_F(TrieTest, Erase_ExistingKey_ReturnsTrue)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_TRUE(trie.erase("key"));
    EXPECT_FALSE(trie.contains("key"));
    EXPECT_TRUE(trie.empty());
}

/**
 * @brief Test erasing a non-existing key
 * @details Verifies that erase returns false for missing keys
 */
TEST_F(TrieTest, Erase_NonExisting_ReturnsFalse)
{
    Trie<int> trie;
    EXPECT_FALSE(trie.erase("nonexistent"));
}

/**
 * @brief Test erasing from an empty trie
 * @details Verifies that erase returns false on empty trie
 */
TEST_F(TrieTest, Erase_FromEmptyTrie_ReturnsFalse)
{
    Trie<int> trie;
    EXPECT_FALSE(trie.erase("anything"));
}

/**
 * @brief Test erasing a key that shares a prefix with others
 * @details Verifies that erasing one key does not affect other keys with the same prefix
 */
TEST_F(TrieTest, Erase_SharedPrefix_DoesNotAffectOthers)
{
    Trie<int> trie;
    trie.insert("a", 1);
    trie.insert("ab", 2);
    trie.insert("abc", 3);

    EXPECT_TRUE(trie.erase("ab"));
    EXPECT_FALSE(trie.contains("ab"));
    EXPECT_TRUE(trie.contains("a"));
    EXPECT_TRUE(trie.contains("abc"));
    EXPECT_EQ(trie.size(), 2);
}

/**
 * @brief Test erasing a leaf key removes the node
 * @details Verifies that erasing the only key empties the trie
 */
TEST_F(TrieTest, Erase_LeafKey_RemovesNode)
{
    Trie<int> trie;
    trie.insert("abc", 1);
    EXPECT_TRUE(trie.erase("abc"));
    EXPECT_TRUE(trie.empty());
}

/**
 * @brief Test erasing all keys results in empty trie
 * @details Verifies that removing all keys restores the empty state
 */
TEST_F(TrieTest, Erase_AllKeys_Empty)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("banana", 2);
    trie.insert("cherry", 3);

    trie.erase("apple");
    trie.erase("banana");
    trie.erase("cherry");

    EXPECT_TRUE(trie.empty());
    EXPECT_EQ(trie.size(), 0);
}

// ==================== StartsWith Tests ====================

/**
 * @brief Test startsWith with no matching prefix
 * @details Verifies that an empty result is returned when no keys match the prefix
 */
TEST_F(TrieTest, StartsWith_NoMatch_ReturnsEmpty)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("banana", 2);
    const auto result = trie.startsWith("cherry");
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test startsWith with empty prefix returns all keys
 * @details Verifies that empty string prefix matches every key
 */
TEST_F(TrieTest, StartsWith_EmptyPrefix_ReturnsAllKeys)
{
    Trie<int> trie;
    trie.insert("a", 1);
    trie.insert("b", 2);
    trie.insert("c", 3);
    const auto result = trie.startsWith("");
    EXPECT_EQ(result.size(), 3);
}

/**
 * @brief Test startsWith with a partial prefix
 * @details Verifies that all keys starting with the given prefix are returned
 */
TEST_F(TrieTest, StartsWith_PartialPrefix)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("appetite", 2);
    trie.insert("appliance", 3);
    trie.insert("banana", 4);

    const auto result = trie.startsWith("app");
    EXPECT_EQ(result.size(), 3);
    EXPECT_NE(std::ranges::find(result, "apple"), result.end());
    EXPECT_NE(std::ranges::find(result, "appetite"), result.end());
    EXPECT_NE(std::ranges::find(result, "appliance"), result.end());
    EXPECT_EQ(std::ranges::find(result, "banana"), result.end());
}

/**
 * @brief Test startsWith with exact key as prefix
 * @details Verifies that the key itself is returned when prefix matches exactly
 */
TEST_F(TrieTest, StartsWith_ExactKey)
{
    Trie<int> trie;
    trie.insert("test", 1);
    const auto result = trie.startsWith("test");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "test");
}

// ==================== Keys & Values Tests ====================

/**
 * @brief Test keys on an empty trie
 * @details Verifies that keys() returns an empty vector for empty trie
 */
TEST_F(TrieTest, Keys_EmptyTrie_ReturnsEmpty)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.keys().empty());
}

/**
 * @brief Test that keys returns all inserted keys
 * @details Verifies that keys() contains every key that was inserted
 */
TEST_F(TrieTest, Keys_ReturnsAllKeys)
{
    Trie<int> trie;
    trie.insert("banana", 1);
    trie.insert("apple", 2);
    trie.insert("cherry", 3);
    const auto keys = trie.keys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_NE(std::ranges::find(keys, "apple"), keys.end());
    EXPECT_NE(std::ranges::find(keys, "banana"), keys.end());
    EXPECT_NE(std::ranges::find(keys, "cherry"), keys.end());
}

/**
 * @brief Test that values returns all stored values
 * @details Verifies that values() contains the value for every inserted key
 */
TEST_F(TrieTest, Values_ReturnsAllValues)
{
    Trie<int> trie;
    trie.insert("a", 10);
    trie.insert("b", 20);
    trie.insert("c", 30);
    const auto vals = trie.values();
    EXPECT_EQ(vals.size(), 3);
    EXPECT_NE(std::ranges::find(vals, 10), vals.end());
    EXPECT_NE(std::ranges::find(vals, 20), vals.end());
    EXPECT_NE(std::ranges::find(vals, 30), vals.end());
}

// ==================== Clear Tests ====================

/**
 * @brief Test clear removes all keys
 * @details Verifies that clear empties the trie and resets size
 */
TEST_F(TrieTest, Clear_RemovesAllKeys)
{
    Trie<int> trie;
    trie.insert("key1", 1);
    trie.insert("key2", 2);
    trie.insert("key3", 3);
    EXPECT_EQ(trie.size(), 3);

    trie.clear();
    EXPECT_TRUE(trie.empty());
    EXPECT_EQ(trie.size(), 0);
    EXPECT_FALSE(trie.contains("key1"));
}

/**
 * @brief Test clear on an empty trie is a no-op
 * @details Verifies that clearing an empty trie does not cause errors
 */
TEST_F(TrieTest, Clear_EmptyTrie_NoOp)
{
    Trie<int> trie;
    trie.clear();
    EXPECT_TRUE(trie.empty());
}

/**
 * @brief Test that insert works after clear
 * @details Verifies that the trie is reusable after a clear operation
 */
TEST_F(TrieTest, Clear_ThenInsert_Works)
{
    Trie<int> trie;
    trie.insert("key", 1);
    trie.clear();
    trie.insert("newkey", 42);
    EXPECT_TRUE(trie.contains("newkey"));
    EXPECT_EQ(trie.size(), 1);
}

// ==================== Move Semantics Tests ====================

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that moving a trie transfers all key-value pairs
 */
TEST_F(TrieTest, MoveConstructor_TransfersOwnership)
{
    Trie<int> trie1;
    trie1.insert("key", 42);
    const Trie<int> trie2(std::move(trie1));
    EXPECT_EQ(trie2.size(), 1);
    EXPECT_TRUE(trie2.contains("key"));
}

/**
 * @brief Test move assignment transfers ownership
 * @details Verifies that move assignment correctly transfers key-value pairs
 */
TEST_F(TrieTest, MoveAssignment_TransfersOwnership)
{
    Trie<int> trie1;
    trie1.insert("key", 42);
    Trie<int> trie2 = std::move(trie1);
    EXPECT_EQ(trie2.size(), 1);
    EXPECT_TRUE(trie2.contains("key"));
}

// ==================== String Type ====================

/**
 * @brief Test storing string values in the trie
 * @details Verifies that string values can be stored and retrieved
 */
TEST_F(TrieTest, StringValue_StoresCorrectly)
{
    Trie<std::string> trie;
    trie.insert("greeting", "hello");
    const std::string* val = trie.find("greeting");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "hello");
}

/**
 * @brief Test storing moved string values
 * @details Verifies that move semantics work correctly for string values
 */
TEST_F(TrieTest, MoveValue_StoresCorrectly)
{
    Trie<std::string> trie;
    std::string s = "moved";
    trie.insert("key", std::move(s));
    const std::string* val = trie.find("key");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "moved");
}

// ==================== Large Dataset ====================

/**
 * @brief Test large dataset correctness
 * @details Verifies that inserting, erasing, and contains work correctly at scale
 */
TEST_F(TrieTest, LargeDataset_Correctness)
{
    Trie<int> trie;
    constexpr int kCount = 500;

    for (int i = 0; i < kCount; ++i)
    {
        trie.insert("key_" + std::to_string(i), i);
    }

    EXPECT_EQ(trie.size(), kCount);

    for (int i = 0; i < kCount; ++i)
    {
        EXPECT_TRUE(trie.contains("key_" + std::to_string(i)));
    }

    for (int i = 0; i < kCount; i += 2)
    {
        trie.erase("key_" + std::to_string(i));
    }

    for (int i = 0; i < kCount; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(trie.contains("key_" + std::to_string(i)));
        }
        else
        {
            EXPECT_TRUE(trie.contains("key_" + std::to_string(i)));
        }
    }
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

/**
 * @brief Test move constructor with multiple keys
 * @details Verifies that moving a trie with multiple keys transfers all of them
 */
TEST_F(TrieTest, MoveConstructor)
{
    Trie<int> trie;
    trie.insert("hello", 1);
    trie.insert("world", 2);
    trie.insert("hi", 3);
    EXPECT_TRUE(trie.contains("hello"));
    EXPECT_EQ(trie.size(), 3);

    const Trie<int> other(std::move(trie));
    EXPECT_TRUE(other.contains("hello"));
    EXPECT_TRUE(other.contains("world"));
    EXPECT_TRUE(other.contains("hi"));
    EXPECT_EQ(other.size(), 3);
}

/**
 * @brief Test move assignment with multiple keys
 * @details Verifies that move assignment correctly transfers all key-value pairs
 */
TEST_F(TrieTest, MoveAssignment)
{
    Trie<int> trie;
    trie.insert("move", 10);
    trie.insert("assignment", 20);
    EXPECT_EQ(trie.size(), 2);

    Trie<int> other;
    other.insert("dummy", 99);
    other = std::move(trie);
    EXPECT_TRUE(other.contains("move"));
    EXPECT_TRUE(other.contains("assignment"));
    EXPECT_EQ(other.size(), 2);
}
