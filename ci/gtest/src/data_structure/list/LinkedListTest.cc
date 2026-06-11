/**
 * @file LinkedListTest.cc
 * @brief Unit tests for the LinkedList class
 * @details Tests cover core list operations including push, pop, front, back, insert,
 *          erase, remove, reverse, iterators, copy/move semantics, and edge cases.
 */

#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include "data_structure/list/LinkedList.hpp"

using namespace common::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<LinkedList<int>>);
static_assert(std::is_nothrow_move_assignable_v<LinkedList<int>>);
static_assert(noexcept(std::declval<LinkedList<int>&>().swap(std::declval<LinkedList<int>&>())));
static_assert(noexcept(std::declval<LinkedList<int>&>().clear()));
static_assert(noexcept(std::declval<const LinkedList<int>&>().empty()));
static_assert(noexcept(std::declval<const LinkedList<int>&>().size()));

/**
 * @brief Test fixture for LinkedList tests
 */
class LinkedListTest : public testing::Test
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
 * @brief Test default constructor creates empty list
 * @details Verifies that a newly constructed list is empty with zero size
 */
TEST_F(LinkedListTest, DefaultConstructor_EmptyList)
{
    const LinkedList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST_F(LinkedListTest, CopyConstructor_DeepCopy)
{
    LinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);
    list1.push_back(30);

    LinkedList list2(list1);

    EXPECT_EQ(list2.size(), list1.size());
    EXPECT_EQ(list2.front(), list1.front());
    EXPECT_EQ(list2.back(), list1.back());

    // Modify original should not affect copy
    list1.pop_front();
    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list2.front(), 10);
}

/**
 * @brief Test copy constructor with empty list
 * @details Verifies copying an empty list works correctly
 */
TEST_F(LinkedListTest, CopyConstructor_EmptyList)
{
    LinkedList<int> list1;
    const LinkedList list2(list1);

    EXPECT_TRUE(list2.empty());
    EXPECT_EQ(list2.size(), 0);

    // Modify original should not affect copy
    list1.push_back(10);
    EXPECT_TRUE(list2.empty());
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST_F(LinkedListTest, MoveConstructor_TransfersOwnership)
{
    LinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);
    list1.push_back(30);

    LinkedList list2(std::move(list1));

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list2.front(), 10);
    EXPECT_EQ(list2.back(), 30);

    // Moved-from list should be empty
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(list1.empty());
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_EQ(list1.size(), 0);
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment performs deep copy correctly
 */
TEST_F(LinkedListTest, CopyAssignment_DeepCopy)
{
    LinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);

    LinkedList<int> list2;
    list2.push_back(99);
    list2 = list1;

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list2.front(), 10);
    EXPECT_EQ(list2.back(), 20);

    // Modify original should not affect copy
    list1.pop_front();
    EXPECT_EQ(list2.front(), 10);
}

/**
 * @brief Test copy assignment self-assignment
 * @details Verifies self-assignment does not corrupt the list
 */
TEST_F(LinkedListTest, CopyAssignment_SelfAssignment)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    // NOLINTNEXTLINE(bugprone-self-assignment)
    list = list;

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 3);
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership correctly
 */
TEST_F(LinkedListTest, MoveAssignment_TransfersOwnership)
{
    LinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);

    LinkedList<int> list2;
    list2.push_back(99);
    list2 = std::move(list1);

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list2.front(), 10);
    EXPECT_EQ(list2.back(), 20);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(list1.empty());
}

// ==================== Push & Pop Tests ====================

/**
 * @brief Test push_back adds elements to the back
 * @details Verifies that push_back adds elements in correct order
 */
TEST_F(LinkedListTest, PushBack_MaintainsOrder)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 3);
}

/**
 * @brief Test push_front adds elements to the front
 * @details Verifies that push_front adds elements in reversed order
 */
TEST_F(LinkedListTest, PushFront_MaintainsOrder)
{
    LinkedList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front(), 3);
    EXPECT_EQ(list.back(), 1);
}

/**
 * @brief Test pop_front removes the first element
 * @details Verifies that pop_front correctly removes front and updates size
 */
TEST_F(LinkedListTest, PopFront_RemovesFirstElement)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_front();
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.back(), 3);

    list.pop_front();
    EXPECT_EQ(list.front(), 3);
    EXPECT_EQ(list.back(), 3);

    list.pop_front();
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test pop_back removes the last element
 * @details Verifies that pop_back correctly removes last and updates size
 */
TEST_F(LinkedListTest, PopBack_RemovesLastElement)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_back();
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 2);

    list.pop_back();
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 1);

    list.pop_back();
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test push and pop cycle
 * @details Verifies alternating push and pop operations work correctly
 */
TEST_F(LinkedListTest, PushPop_AlternatingCycle)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_front(2);
    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.back(), 1);

    list.pop_back();
    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.size(), 1);

    list.push_back(3);
    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.back(), 3);
}

// ==================== Element Access Tests ====================

/**
 * @brief Test front access
 * @details Verifies front returns reference to first element
 */
TEST_F(LinkedListTest, Front_ReturnsFirstElement)
{
    LinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    EXPECT_EQ(list.front(), 10);

    list.front() = 99;
    EXPECT_EQ(list.front(), 99);
}

/**
 * @brief Test back access
 * @details Verifies back returns reference to last element
 */
TEST_F(LinkedListTest, Back_ReturnsLastElement)
{
    LinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    EXPECT_EQ(list.back(), 20);

    list.back() = 99;
    EXPECT_EQ(list.back(), 99);
}

// ==================== Insert & Erase Tests ====================

/**
 * @brief Test insert at beginning
 * @details Verifies inserting before begin() works correctly
 */
TEST_F(LinkedListTest, Insert_Beginning)
{
    LinkedList<int> list;
    list.push_back(2);
    list.push_back(3);

    const auto it = list.insert(list.begin(), 1);
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 3);
}

/**
 * @brief Test insert at end
 * @details Verifies inserting before end() works correctly
 */
TEST_F(LinkedListTest, Insert_End)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    const auto it = list.insert(list.end(), 3);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.back(), 3);
}

/**
 * @brief Test insert in middle
 * @details Verifies inserting in the middle works correctly
 */
TEST_F(LinkedListTest, Insert_Middle)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(3);

    auto it = list.begin();
    ++it;
    it = list.insert(it, 2);
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(list.size(), 3);

    const std::vector expected = {1, 2, 3};
    EXPECT_EQ(list.toVector(), expected);
}

/**
 * @brief Test erase at beginning
 * @details Verifies erasing first element works correctly
 */
TEST_F(LinkedListTest, Erase_Beginning)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    const auto it = list.erase(list.begin());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 2);
}

/**
 * @brief Test erase at end
 * @details Verifies erasing last element works correctly
 */
TEST_F(LinkedListTest, Erase_End)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    auto it = list.begin();
    ++it;
    ++it;
    it = list.erase(it);
    EXPECT_EQ(it, list.end());
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.back(), 2);
}

/**
 * @brief Test erase in middle
 * @details Verifies erasing middle element works correctly
 */
TEST_F(LinkedListTest, Erase_Middle)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);

    auto it = list.begin();
    ++it;
    it = list.erase(it);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(list.size(), 3);

    const std::vector expected = {1, 3, 4};
    EXPECT_EQ(list.toVector(), expected);
}

// ==================== Remove Tests ====================

/**
 * @brief Test remove removes all matching elements
 * @details Verifies remove erases all occurrences of a value
 */
TEST_F(LinkedListTest, Remove_AllMatchingValues)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(1);
    list.push_back(3);
    list.push_back(1);

    const auto count = list.remove(1);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(list.size(), 2);

    const std::vector expected = {2, 3};
    EXPECT_EQ(list.toVector(), expected);
}

/**
 * @brief Test remove with no matches
 * @details Verifies remove returns 0 when no value matches
 */
TEST_F(LinkedListTest, Remove_NoMatch)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    const auto count = list.remove(99);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(list.size(), 2);
}

/**
 * @brief Test remove_if removes matching elements
 * @details Verifies remove_if erases elements matching predicate
 */
TEST_F(LinkedListTest, RemoveIf_RemovesMatching)
{
    LinkedList<int> list;
    for (int i = 1; i <= 6; ++i)
    {
        list.push_back(i);
    }

    const auto count = list.remove_if([](int x) { return x % 2 == 0; });
    EXPECT_EQ(count, 3);
    EXPECT_EQ(list.size(), 3);

    const std::vector expected = {1, 3, 5};
    EXPECT_EQ(list.toVector(), expected);
}

// ==================== Reverse Tests ====================

/**
 * @brief Test reverse on list with multiple elements
 * @details Verifies reverse correctly reverses element order
 */
TEST_F(LinkedListTest, Reverse_MultipleElements)
{
    LinkedList<int> list;
    for (int i = 1; i <= 5; ++i)
    {
        list.push_back(i);
    }

    list.reverse();

    const std::vector expected = {5, 4, 3, 2, 1};
    EXPECT_EQ(list.toVector(), expected);
}

/**
 * @brief Test reverse on single element list
 * @details Verifies reverse is a no-op on single element
 */
TEST_F(LinkedListTest, Reverse_SingleElement)
{
    LinkedList<int> list;
    list.push_back(42);

    list.reverse();

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 42);
    EXPECT_EQ(list.back(), 42);
}

/**
 * @brief Test reverse on empty list
 * @details Verifies reverse is a no-op on empty list
 */
TEST_F(LinkedListTest, Reverse_EmptyList)
{
    LinkedList<int> list;
    list.reverse();
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test reverse on two elements
 * @details Verifies reverse correctly swaps two elements
 */
TEST_F(LinkedListTest, Reverse_TwoElements)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    list.reverse();

    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.back(), 1);
    EXPECT_EQ(list.size(), 2);
}

// ==================== Iterator Tests ====================

/**
 * @brief Test forward iteration
 * @details Verifies begin/end iterate in correct order
 */
TEST_F(LinkedListTest, Iterators_ForwardTraversal)
{
    LinkedList<int> list;
    for (int i = 1; i <= 4; ++i)
    {
        list.push_back(i);
    }

    std::vector<int> result;
    for (const auto& val : list)
    {
        result.push_back(val);
    }

    const std::vector expected = {1, 2, 3, 4};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test backward iteration
 * @details Verifies bidirectional iterators traverse backwards correctly
 */
TEST_F(LinkedListTest, Iterators_BackwardTraversal)
{
    LinkedList<int> list;
    for (int i = 1; i <= 4; ++i)
    {
        list.push_back(i);
    }

    std::vector<int> result;
    // Start at the last element by advancing to end, then step back one
    auto it = list.begin();
    auto prev = list.begin();
    while (it != list.end())
    {
        prev = it;
        ++it;
    }
    // Now iterate backward from the last element
    while (true)
    {
        result.push_back(*prev);
        if (prev == list.begin()) break;
        --prev;
    }

    const std::vector expected = {4, 3, 2, 1};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test range-based for loop
 * @details Verifies range-based for loop works correctly
 */
TEST_F(LinkedListTest, Iterators_RangeBasedFor)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    int sum = 0;
    for (const auto& val : list)
    {
        sum += val;
    }
    EXPECT_EQ(sum, 6);
}

/**
 * @brief Test const iterators
 * @details Verifies const iterators work correctly
 */
TEST_F(LinkedListTest, Iterators_ConstIteration)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    const auto& clist = list;
    std::vector<int> result;
    for (const auto& val : clist)
    {
        result.push_back(val);
    }

    const std::vector expected = {1, 2};
    EXPECT_EQ(result, expected);
}

/**
 * @brief Test iterator pre and post increment
 * @details Verifies both increment operators work correctly
 */
TEST_F(LinkedListTest, Iterators_PrePostIncrement)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    auto it = list.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(it, list.end());
}

// ==================== Emplace Tests ====================

/**
 * @brief Test emplace_front constructs in-place
 * @details Verifies emplace_front constructs element at front
 */
TEST_F(LinkedListTest, EmplaceFront_ConstructsInPlace)
{
    LinkedList<std::pair<int, int>> list;
    list.emplace_front(1, 2);
    list.emplace_front(3, 4);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), std::make_pair(3, 4));
    EXPECT_EQ(list.back(), std::make_pair(1, 2));
}

/**
 * @brief Test emplace_back constructs in-place
 * @details Verifies emplace_back constructs element at back
 */
TEST_F(LinkedListTest, EmplaceBack_ConstructsInPlace)
{
    LinkedList<std::pair<int, int>> list;
    list.emplace_back(1, 2);
    list.emplace_back(3, 4);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), std::make_pair(1, 2));
    EXPECT_EQ(list.back(), std::make_pair(3, 4));
}

// ==================== Clear & Swap Tests ====================

/**
 * @brief Test clear empties the list
 * @details Verifies that clear removes all elements
 */
TEST_F(LinkedListTest, Clear_EmptiesList)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test clear on empty list is no-op
 * @details Verifies clearing an already empty list does nothing
 */
TEST_F(LinkedListTest, Clear_EmptyList_NoOp)
{
    LinkedList<int> list;
    list.clear();
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test clear then reuse
 * @details Verifies cleared list can be reused
 */
TEST_F(LinkedListTest, Clear_ThenReuse)
{
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.clear();

    list.push_back(10);
    list.push_back(20);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 10);
    EXPECT_EQ(list.back(), 20);
}

/**
 * @brief Test swap exchanges contents
 * @details Verifies swap correctly exchanges contents of two lists
 */
TEST_F(LinkedListTest, Swap_ExchangesContents)
{
    LinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);

    LinkedList<int> list2;
    list2.push_back(10);
    list2.push_back(20);
    list2.push_back(30);

    list1.swap(list2);

    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(list1.front(), 10);
    EXPECT_EQ(list1.back(), 30);

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list2.front(), 1);
    EXPECT_EQ(list2.back(), 2);
}

/**
 * @brief Test swap with empty list
 * @details Verifies swapping with empty list works correctly
 */
TEST_F(LinkedListTest, Swap_WithEmptyList)
{
    LinkedList<int> list1;
    list1.push_back(1);

    LinkedList<int> list2;

    list1.swap(list2);

    EXPECT_TRUE(list1.empty());
    EXPECT_EQ(list2.size(), 1);
    EXPECT_EQ(list2.front(), 1);
}

// ==================== ToVector Tests ====================

/**
 * @brief Test toVector returns elements in order
 * @details Verifies toVector preserves element order
 */
TEST_F(LinkedListTest, ToVector_ReturnsElementsInOrder)
{
    LinkedList<int> list;
    for (int i = 0; i < 5; ++i)
    {
        list.push_back(i);
    }

    const std::vector expected = {0, 1, 2, 3, 4};
    EXPECT_EQ(list.toVector(), expected);
}

/**
 * @brief Test toVector on empty list
 * @details Verifies toVector on empty list returns empty vector
 */
TEST_F(LinkedListTest, ToVector_EmptyList)
{
    const LinkedList<int> list;
    EXPECT_TRUE(list.toVector().empty());
}

// ==================== Edge Case Tests ====================

/**
 * @brief Test exceptions on empty list operations
 * @details Verifies that accessing elements on empty list throws
 */
TEST_F(LinkedListTest, Front_EmptyList_ThrowsException)
{
    const LinkedList<int> list;
    EXPECT_THROW(list.front(), std::out_of_range);
}

/**
 * @brief Test back on empty list throws
 * @details Verifies back throws on empty list
 */
TEST_F(LinkedListTest, Back_EmptyList_ThrowsException)
{
    const LinkedList<int> list;
    EXPECT_THROW(list.back(), std::out_of_range);
}

/**
 * @brief Test pop_front on empty list throws
 * @details Verifies pop_front throws on empty list
 */
TEST_F(LinkedListTest, PopFront_EmptyList_ThrowsException)
{
    LinkedList<int> list;
    EXPECT_THROW(list.pop_front(), std::out_of_range);
}

/**
 * @brief Test pop_back on empty list throws
 * @details Verifies pop_back throws on empty list
 */
TEST_F(LinkedListTest, PopBack_EmptyList_ThrowsException)
{
    LinkedList<int> list;
    EXPECT_THROW(list.pop_back(), std::out_of_range);
}

/**
 * @brief Test single element list
 * @details Verifies operations on a single-element list work correctly
 */
TEST_F(LinkedListTest, SingleElement_Operations)
{
    LinkedList<int> list;
    list.push_back(42);

    EXPECT_EQ(list.front(), 42);
    EXPECT_EQ(list.back(), 42);
    EXPECT_EQ(list.size(), 1);

    list.pop_front();
    EXPECT_TRUE(list.empty());

    list.push_front(99);
    EXPECT_EQ(list.front(), 99);
    EXPECT_EQ(list.back(), 99);
}

/**
 * @brief Test string type elements
 * @details Verifies LinkedList works with std::string
 */
TEST_F(LinkedListTest, StringType_Operations)
{
    LinkedList<std::string> list;
    list.push_back("hello");
    list.push_back("world");

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), "hello");
    EXPECT_EQ(list.back(), "world");

    list.pop_front();
    EXPECT_EQ(list.front(), "world");

    list.clear();
    EXPECT_TRUE(list.empty());
}

// ==================== Large Data Set Tests ====================

/**
 * @brief Test large number of elements
 * @details Verifies list handles many elements correctly
 */
TEST_F(LinkedListTest, LargeNumberOfElements_Correctness)
{
    LinkedList<int> list;
    constexpr int n = 1000;

    for (int i = 0; i < n; ++i)
    {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), n);
    EXPECT_EQ(list.front(), 0);
    EXPECT_EQ(list.back(), n - 1);

    for (int i = 0; i < n / 2; ++i)
    {
        list.pop_front();
    }
    EXPECT_EQ(list.size(), n / 2);
    EXPECT_EQ(list.front(), n / 2);

    // Reverse and verify
    list.reverse();
    EXPECT_EQ(list.front(), n - 1);
    EXPECT_EQ(list.back(), n / 2);
}

/**
 * @brief Test large dataset with remove
 * @details Verifies remove works correctly on large list
 */
TEST_F(LinkedListTest, LargeDataSet_RemoveEvenNumbers)
{
    LinkedList<int> list;
    constexpr int n = 1000;

    for (int i = 0; i < n; ++i)
    {
        list.push_back(i);
    }

    const auto count = list.remove_if([](int x) { return x % 2 == 0; });
    EXPECT_EQ(count, n / 2);
    EXPECT_EQ(list.size(), n / 2);

    // Verify all remaining are odd
    for (const auto& val : list)
    {
        EXPECT_NE(val % 2, 0);
    }
}
