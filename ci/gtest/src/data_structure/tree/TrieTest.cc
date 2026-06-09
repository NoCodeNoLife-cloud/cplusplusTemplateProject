/**
 * @file TrieTest.cc
 * @brief Unit tests for the Trie class
 * @details Tests cover insertion, search, prefix query, deletion,
 *          iteration, edge cases, and type compatibility.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/tree/Trie.hpp"

using namespace common::data_structure;

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

TEST_F(TrieTest, DefaultConstructor_EmptyTrie)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.empty());
    EXPECT_EQ(trie.size(), 0);
}

TEST_F(TrieTest, Empty_InitiallyEmpty)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.empty());
}

TEST_F(TrieTest, Empty_AfterInsert)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_FALSE(trie.empty());
}

TEST_F(TrieTest, Empty_AfterClear)
{
    Trie<int> trie;
    trie.insert("key", 1);
    trie.clear();
    EXPECT_TRUE(trie.empty());
}

// ==================== Insertion Tests ====================

TEST_F(TrieTest, Insert_SingleKey_FindReturnsValue)
{
    Trie<int> trie;
    trie.insert("hello", 42);
    EXPECT_EQ(trie.size(), 1);
    const int* val = trie.find("hello");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 42);
}

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

TEST_F(TrieTest, Insert_EmptyStringKey)
{
    Trie<int> trie;
    trie.insert("", 42);
    EXPECT_EQ(trie.size(), 1);
    EXPECT_TRUE(trie.contains(""));
}

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

TEST_F(TrieTest, Find_NonExisting_ReturnsNullptr)
{
    const Trie<int> trie;
    EXPECT_EQ(trie.find("nonexistent"), nullptr);
}

TEST_F(TrieTest, Find_Existing_ReturnsPointer)
{
    Trie<int> trie;
    trie.insert("key", 42);
    const int* val = trie.find("key");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 42);
}

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

TEST_F(TrieTest, Contains_ExistingKey_ReturnsTrue)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_TRUE(trie.contains("key"));
}

TEST_F(TrieTest, Contains_NonExisting_ReturnsFalse)
{
    const Trie<int> trie;
    EXPECT_FALSE(trie.contains("anything"));
}

TEST_F(TrieTest, Contains_PartialPrefix_ReturnsFalse)
{
    Trie<int> trie;
    trie.insert("hello", 1);
    EXPECT_FALSE(trie.contains("hel"));
    EXPECT_FALSE(trie.contains("hell"));
    EXPECT_TRUE(trie.contains("hello"));
}

// ==================== Erase Tests ====================

TEST_F(TrieTest, Erase_ExistingKey_ReturnsTrue)
{
    Trie<int> trie;
    trie.insert("key", 1);
    EXPECT_TRUE(trie.erase("key"));
    EXPECT_FALSE(trie.contains("key"));
    EXPECT_TRUE(trie.empty());
}

TEST_F(TrieTest, Erase_NonExisting_ReturnsFalse)
{
    Trie<int> trie;
    EXPECT_FALSE(trie.erase("nonexistent"));
}

TEST_F(TrieTest, Erase_FromEmptyTrie_ReturnsFalse)
{
    Trie<int> trie;
    EXPECT_FALSE(trie.erase("anything"));
}

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

TEST_F(TrieTest, Erase_LeafKey_RemovesNode)
{
    Trie<int> trie;
    trie.insert("abc", 1);
    EXPECT_TRUE(trie.erase("abc"));
    EXPECT_TRUE(trie.empty());
}

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

TEST_F(TrieTest, StartsWith_NoMatch_ReturnsEmpty)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("banana", 2);
    const auto result = trie.startsWith("cherry");
    EXPECT_TRUE(result.empty());
}

TEST_F(TrieTest, StartsWith_EmptyPrefix_ReturnsAllKeys)
{
    Trie<int> trie;
    trie.insert("a", 1);
    trie.insert("b", 2);
    trie.insert("c", 3);
    const auto result = trie.startsWith("");
    EXPECT_EQ(result.size(), 3);
}

TEST_F(TrieTest, StartsWith_PartialPrefix)
{
    Trie<int> trie;
    trie.insert("apple", 1);
    trie.insert("appetite", 2);
    trie.insert("appliance", 3);
    trie.insert("banana", 4);

    const auto result = trie.startsWith("app");
    EXPECT_EQ(result.size(), 3);
    EXPECT_NE(std::find(result.begin(), result.end(), "apple"), result.end());
    EXPECT_NE(std::find(result.begin(), result.end(), "appetite"), result.end());
    EXPECT_NE(std::find(result.begin(), result.end(), "appliance"), result.end());
    EXPECT_EQ(std::find(result.begin(), result.end(), "banana"), result.end());
}

TEST_F(TrieTest, StartsWith_ExactKey)
{
    Trie<int> trie;
    trie.insert("test", 1);
    const auto result = trie.startsWith("test");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "test");
}

// ==================== Keys & Values Tests ====================

TEST_F(TrieTest, Keys_EmptyTrie_ReturnsEmpty)
{
    const Trie<int> trie;
    EXPECT_TRUE(trie.keys().empty());
}

TEST_F(TrieTest, Keys_ReturnsAllKeys)
{
    Trie<int> trie;
    trie.insert("banana", 1);
    trie.insert("apple", 2);
    trie.insert("cherry", 3);
    const auto keys = trie.keys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "apple"), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), "banana"), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), "cherry"), keys.end());
}

TEST_F(TrieTest, Values_ReturnsAllValues)
{
    Trie<int> trie;
    trie.insert("a", 10);
    trie.insert("b", 20);
    trie.insert("c", 30);
    const auto vals = trie.values();
    EXPECT_EQ(vals.size(), 3);
    EXPECT_NE(std::find(vals.begin(), vals.end(), 10), vals.end());
    EXPECT_NE(std::find(vals.begin(), vals.end(), 20), vals.end());
    EXPECT_NE(std::find(vals.begin(), vals.end(), 30), vals.end());
}

// ==================== Clear Tests ====================

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

TEST_F(TrieTest, Clear_EmptyTrie_NoOp)
{
    Trie<int> trie;
    trie.clear();
    EXPECT_TRUE(trie.empty());
}

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

TEST_F(TrieTest, MoveConstructor_TransfersOwnership)
{
    Trie<int> trie1;
    trie1.insert("key", 42);
    Trie<int> trie2(std::move(trie1));
    EXPECT_EQ(trie2.size(), 1);
    EXPECT_TRUE(trie2.contains("key"));
    EXPECT_TRUE(trie1.empty());
}

TEST_F(TrieTest, MoveAssignment_TransfersOwnership)
{
    Trie<int> trie1;
    trie1.insert("key", 42);
    Trie<int> trie2;
    trie2 = std::move(trie1);
    EXPECT_EQ(trie2.size(), 1);
    EXPECT_TRUE(trie2.contains("key"));
    EXPECT_TRUE(trie1.empty());
}

// ==================== String Type ====================

TEST_F(TrieTest, StringValue_StoresCorrectly)
{
    Trie<std::string> trie;
    trie.insert("greeting", "hello");
    const std::string* val = trie.find("greeting");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "hello");
}

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
