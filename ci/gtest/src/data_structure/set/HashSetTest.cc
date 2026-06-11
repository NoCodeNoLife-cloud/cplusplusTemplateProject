/**
 * @file HashSetTest.cc
 * @brief Unit tests for the HashSet class
 * @details Tests cover core set operations including insert, erase, contains, find,
 *          iterators, copy/move semantics, and edge cases.
 */

#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/set/HashSet.hpp"

using namespace common::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<HashSet<int>>);
static_assert(std::is_nothrow_move_assignable_v<HashSet<int>>);
static_assert(noexcept(std::declval<HashSet<int>&>().swap(std::declval<HashSet<int>&>())));
static_assert(noexcept(std::declval<HashSet<int>&>().clear()));
static_assert(noexcept(std::declval<const HashSet<int>&>().empty()));
static_assert(noexcept(std::declval<const HashSet<int>&>().size()));

/**
 * @brief Test fixture for HashSet tests
 */
class HashSetTest : public testing::Test
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
 * @brief Test default constructor creates empty set
 * @details Verifies that a newly constructed set is empty with zero size
 */
TEST_F(HashSetTest, DefaultConstructor_EmptySet)
{
    const HashSet<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

/**
 * @brief Test constructor with initial capacity
 * @details Verifies constructing with initial capacity works
 */
TEST_F(HashSetTest, Constructor_WithCapacity)
{
    const HashSet<int> set(32);
    EXPECT_TRUE(set.empty());
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST_F(HashSetTest, CopyConstructor_DeepCopy)
{
    HashSet<int> set1;
    set1.insert(10);
    set1.insert(20);
    set1.insert(30);

    const HashSet set2(set1);

    EXPECT_EQ(set2.size(), set1.size());
    EXPECT_TRUE(set2.contains(10));
    EXPECT_TRUE(set2.contains(20));
    EXPECT_TRUE(set2.contains(30));

    // Modify original should not affect copy
    set1.erase(10);
    EXPECT_TRUE(set2.contains(10));
}

/**
 * @brief Test copy constructor with empty set
 * @details Verifies copying an empty set works correctly
 */
TEST_F(HashSetTest, CopyConstructor_EmptySet)
{
    const HashSet<int> set1;
    const HashSet set2(set1);

    EXPECT_TRUE(set2.empty());
    EXPECT_EQ(set2.size(), 0);
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST_F(HashSetTest, MoveConstructor_TransfersOwnership)
{
    HashSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    const HashSet set2(std::move(set1));

    EXPECT_EQ(set2.size(), 2);
    EXPECT_TRUE(set2.contains(10));
    EXPECT_TRUE(set2.contains(20));

    // Moved-from set should be empty
    EXPECT_TRUE(set1.empty());
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment performs deep copy correctly
 */
TEST_F(HashSetTest, CopyAssignment_DeepCopy)
{
    HashSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    HashSet<int> set2;
    set2.insert(99);
    set2 = set1;

    EXPECT_EQ(set2.size(), 2);
    EXPECT_TRUE(set2.contains(10));
    EXPECT_TRUE(set2.contains(20));
    EXPECT_FALSE(set2.contains(99));
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership correctly
 */
TEST_F(HashSetTest, MoveAssignment_TransfersOwnership)
{
    HashSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    HashSet<int> set2;
    set2.insert(99);
    set2 = std::move(set1);

    EXPECT_EQ(set2.size(), 2);
    EXPECT_TRUE(set2.contains(10));
    EXPECT_TRUE(set2.contains(20));
    EXPECT_TRUE(set1.empty());
}

// ==================== Insert Tests ====================

/**
 * @brief Test insert adds new elements
 * @details Verifies that insert adds new elements and returns true
 */
TEST_F(HashSetTest, Insert_NewElement_ReturnsTrue)
{
    HashSet<int> set;
    auto [it, inserted] = set.insert(42);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(*it, 42);
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test insert duplicate element
 * @details Verifies that inserting existing element returns false
 */
TEST_F(HashSetTest, Insert_Duplicate_ReturnsFalse)
{
    HashSet<int> set;
    set.insert(42);
    auto [it, inserted] = set.insert(42);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(*it, 42);
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test insert multiple elements
 * @details Verifies inserting unique elements works correctly
 */
TEST_F(HashSetTest, Insert_MultipleElements)
{
    HashSet<int> set;
    for (int i = 0; i < 100; ++i)
    {
        auto [it, inserted] = set.insert(i);
        EXPECT_TRUE(inserted);
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(set.size(), 100);
}

// ==================== Erase Tests ====================

/**
 * @brief Test erase existing element
 * @details Verifies that erase removes existing element and returns true
 */
TEST_F(HashSetTest, Erase_ExistingElement_ReturnsTrue)
{
    HashSet<int> set;
    set.insert(10);
    set.insert(20);

    EXPECT_TRUE(set.erase(10));
    EXPECT_EQ(set.size(), 1);
    EXPECT_FALSE(set.contains(10));
    EXPECT_TRUE(set.contains(20));
}

/**
 * @brief Test erase non-existing element
 * @details Verifies that erasing non-existing element returns false
 */
TEST_F(HashSetTest, Erase_NonExisting_ReturnsFalse)
{
    HashSet<int> set;
    set.insert(10);
    EXPECT_FALSE(set.erase(99));
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test erase via iterator
 * @details Verifies that erasing via iterator works correctly
 */
TEST_F(HashSetTest, Erase_ByIterator)
{
    HashSet<int> set;
    set.insert(10);
    set.insert(20);
    set.insert(30);

    const auto it = set.find(20);
    ASSERT_NE(it, set.end());
    const auto next = set.erase(it);
    EXPECT_EQ(set.size(), 2);
    EXPECT_FALSE(set.contains(20));
}

// ==================== Contains & Find Tests ====================

/**
 * @brief Test contains returns true for existing elements
 * @details Verifies contains correctly identifies existing elements
 */
TEST_F(HashSetTest, Contains_ExistingElement_ReturnsTrue)
{
    HashSet<int> set;
    set.insert(42);
    EXPECT_TRUE(set.contains(42));
}

/**
 * @brief Test contains returns false for missing elements
 * @details Verifies contains correctly identifies missing elements
 */
TEST_F(HashSetTest, Contains_NonExisting_ReturnsFalse)
{
    const HashSet<int> set;
    EXPECT_FALSE(set.contains(42));
}

/**
 * @brief Test find returns iterator to element
 * @details Verifies find correctly locates elements
 */
TEST_F(HashSetTest, Find_ExistingElement_ReturnsIterator)
{
    HashSet<int> set;
    set.insert(42);
    const auto it = set.find(42);
    ASSERT_NE(it, set.end());
    EXPECT_EQ(*it, 42);
}

/**
 * @brief Test find returns end for missing elements
 * @details Verifies find returns end() for missing elements
 */
TEST_F(HashSetTest, Find_NonExisting_ReturnsEnd)
{
    HashSet<int> set;
    EXPECT_EQ(set.find(42), set.end());
}

// ==================== Iterator Tests ====================

/**
 * @brief Test begin and end iterators
 * @details Verifies iterating over all elements in the set
 */
TEST_F(HashSetTest, Iterators_AllElements)
{
    HashSet<int> set;
    set.insert(10);
    set.insert(20);
    set.insert(30);

    std::vector<int> found;
    for (auto it = set.begin(); it != set.end(); ++it)
    {
        found.push_back(*it);
    }

    std::ranges::sort(found);
    const std::vector expected = {10, 20, 30};
    EXPECT_EQ(found, expected);
}

/**
 * @brief Test const iterator
 * @details Verifies const iterators work correctly
 */
TEST_F(HashSetTest, Iterators_ConstIteration)
{
    HashSet<int> set;
    set.insert(1);
    set.insert(2);

    const auto& cset = set;
    int count = 0;
    for (auto it = cset.cbegin(); it != cset.cend(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 2);
}

/**
 * @brief Test range-based for loop
 * @details Verifies range-based for loop works with HashSet
 */
TEST_F(HashSetTest, Iterators_RangeBasedFor)
{
    HashSet<int> set;
    set.insert(1);
    set.insert(2);
    set.insert(3);

    int sum = 0;
    for (const auto& val : set)
    {
        sum += val;
    }
    EXPECT_EQ(sum, 6);
}

// ==================== Clear & Swap Tests ====================

/**
 * @brief Test clear empties the set
 * @details Verifies that clear removes all elements
 */
TEST_F(HashSetTest, Clear_EmptiesSet)
{
    HashSet<int> set;
    set.insert(1);
    set.insert(2);
    set.insert(3);

    set.clear();
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

/**
 * @brief Test clear on empty set is no-op
 * @details Verifies clearing an already empty set does nothing
 */
TEST_F(HashSetTest, Clear_EmptySet_NoOp)
{
    HashSet<int> set;
    set.clear();
    EXPECT_TRUE(set.empty());
}

/**
 * @brief Test clear then reuse
 * @details Verifies cleared set can be reused
 */
TEST_F(HashSetTest, Clear_ThenReuse)
{
    HashSet<int> set;
    set.insert(1);
    set.clear();

    set.insert(10);
    set.insert(20);
    EXPECT_EQ(set.size(), 2);
    EXPECT_TRUE(set.contains(10));
    EXPECT_TRUE(set.contains(20));
}

/**
 * @brief Test swap exchanges contents
 * @details Verifies swap correctly exchanges contents of two sets
 */
TEST_F(HashSetTest, Swap_ExchangesContents)
{
    HashSet<int> set1;
    set1.insert(1);
    set1.insert(2);

    HashSet<int> set2;
    set2.insert(10);
    set2.insert(20);
    set2.insert(30);

    set1.swap(set2);

    EXPECT_EQ(set1.size(), 3);
    EXPECT_TRUE(set1.contains(10));

    EXPECT_EQ(set2.size(), 2);
    EXPECT_TRUE(set2.contains(1));
}

// ==================== Reserve Tests ====================

/**
 * @brief Test reserve pre-allocates space
 * @details Verifies reserve allows inserting many elements without excessive rehashing
 */
TEST_F(HashSetTest, Reserve_AllowsManyInsertions)
{
    HashSet<int> set;
    set.reserve(200);

    for (int i = 0; i < 100; ++i)
    {
        set.insert(i);
    }
    EXPECT_EQ(set.size(), 100);
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(set.contains(i));
    }
}

// ==================== Edge Case Tests ====================

/**
 * @brief Test string type elements
 * @details Verifies HashSet works with std::string
 */
TEST_F(HashSetTest, StringType_Operations)
{
    HashSet<std::string> set;
    set.insert("hello");
    set.insert("world");

    EXPECT_EQ(set.size(), 2);
    EXPECT_TRUE(set.contains("hello"));
    EXPECT_TRUE(set.contains("world"));
    EXPECT_FALSE(set.contains("missing"));

    set.erase("hello");
    EXPECT_EQ(set.size(), 1);
    EXPECT_FALSE(set.contains("hello"));
}

/**
 * @brief Test with custom hash function
 * @details Verifies HashSet works with custom hash and equality
 */
TEST_F(HashSetTest, CustomHash_WorksCorrectly)
{
    struct StringLengthHash
    {
        std::size_t operator()(const std::string& s) const
        {
            return s.size();
        }
    };

    HashSet<std::string, StringLengthHash> set;
    set.insert("a");
    set.insert("bb");
    set.insert("ccc");

    EXPECT_EQ(set.size(), 3);
    EXPECT_TRUE(set.contains("a"));
    EXPECT_TRUE(set.contains("bb"));
    EXPECT_TRUE(set.contains("ccc"));
}

/**
 * @brief Test insert and erase cycle
 * @details Verifies alternating insert and erase operations work
 */
TEST_F(HashSetTest, InsertErase_Cycle)
{
    HashSet<int> set;
    for (int i = 0; i < 10; ++i)
    {
        set.insert(i);
    }
    EXPECT_EQ(set.size(), 10);

    for (int i = 0; i < 5; ++i)
    {
        set.erase(i);
    }
    EXPECT_EQ(set.size(), 5);

    for (int i = 5; i < 10; ++i)
    {
        EXPECT_TRUE(set.contains(i));
    }

    for (int i = 0; i < 5; ++i)
    {
        set.insert(i);
    }
    EXPECT_EQ(set.size(), 10);
}

// ==================== Large Data Set Tests ====================

/**
 * @brief Test large number of elements
 * @details Verifies set handles many elements correctly
 */
TEST_F(HashSetTest, LargeNumberOfElements_Correctness)
{
    HashSet<int> set;
    constexpr int n = 1000;

    for (int i = 0; i < n; ++i)
    {
        set.insert(i);
    }
    EXPECT_EQ(set.size(), n);

    for (int i = 0; i < n; ++i)
    {
        EXPECT_TRUE(set.contains(i));
    }

    for (int i = 0; i < n / 2; ++i)
    {
        set.erase(i);
    }
    EXPECT_EQ(set.size(), n / 2);

    for (int i = 0; i < n / 2; ++i)
    {
        EXPECT_FALSE(set.contains(i));
    }
    for (int i = n / 2; i < n; ++i)
    {
        EXPECT_TRUE(set.contains(i));
    }
}
