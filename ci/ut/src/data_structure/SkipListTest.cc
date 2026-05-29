/**
 * @file SkipListTest.cc
 * @brief Unit tests for the SkipList class
 * @details Tests cover core skip list functionality including insertion, deletion,
 *          searching, and edge cases with various data types.
 */

#include <gtest/gtest.h>
#include "data_structure/SkipList.hpp"
#include <string>
#include <vector>
#include <algorithm>

using namespace common::common::data_structure::tree;

/**
 * @brief Test default constructor
 * @details Verifies that SkipList can be created with default parameters
 */
TEST(SkipListTest, Constructor_Default_CreatesEmptyList)
{
    EXPECT_NO_THROW(SkipList<int> list);
}

/**
 * @brief Test constructor with custom maxLevel
 * @details Verifies that non-default maxLevel values are accepted
 */
TEST(SkipListTest, Constructor_CustomMaxLevel)
{
    EXPECT_NO_THROW(SkipList<int> list(8));
    EXPECT_NO_THROW(SkipList<int> list(32));
    EXPECT_NO_THROW(SkipList<int> list(1));
}

/**
 * @brief Test constructor with custom probability
 * @details Verifies that different probability values are accepted
 */
TEST(SkipListTest, Constructor_CustomProbability)
{
    EXPECT_NO_THROW(SkipList<int> list(16, 0.25f));
    EXPECT_NO_THROW(SkipList<int> list(16, 0.75f));
    EXPECT_NO_THROW(SkipList<int> list(16, 0.1f));
}

/**
 * @brief Test initial state is empty
 * @details Verifies that newly constructed SkipList is empty
 */
TEST(SkipListTest, InitialState_EmptyAndZeroSize)
{
    const SkipList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test insert single element
 * @details Verifies basic insertion functionality
 */
TEST(SkipListTest, Insert_SingleElement_Success)
{
    SkipList<int> list;

    list.insert(42);

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list.search(42));
}

/**
 * @brief Test insert multiple elements in ascending order
 * @details Verifies correct handling of sorted insertion
 */
TEST(SkipListTest, Insert_MultipleElements_AscendingOrder)
{
    SkipList<int> list;

    for (int i = 1; i <= 10; ++i)
    {
        list.insert(i);
    }

    EXPECT_EQ(list.size(), 10);

    // Verify all elements exist
    for (int i = 1; i <= 10; ++i)
    {
        EXPECT_TRUE(list.search(i));
    }
}

/**
 * @brief Test insert multiple elements in descending order
 * @details Verifies correct handling of reverse-sorted insertion
 */
TEST(SkipListTest, Insert_MultipleElements_DescendingOrder)
{
    SkipList<int> list;

    for (int i = 10; i >= 1; --i)
    {
        list.insert(i);
    }

    EXPECT_EQ(list.size(), 10);

    // Verify all elements exist
    for (int i = 1; i <= 10; ++i)
    {
        EXPECT_TRUE(list.search(i));
    }
}

/**
 * @brief Test insert multiple elements in random order
 * @details Verifies correct handling of unsorted insertion
 */
TEST(SkipListTest, Insert_MultipleElements_RandomOrder)
{
    SkipList<int> list;

    std::vector<int> values = {5, 3, 8, 1, 9, 2, 7, 4, 6, 10};

    for (const auto& val : values)
    {
        list.insert(val);
    }

    EXPECT_EQ(list.size(), 10);

    // Verify all elements exist
    for (const auto& val : values)
    {
        EXPECT_TRUE(list.search(val));
    }
}

/**
 * @brief Test insert duplicate elements (idempotent)
 * @details Verifies that duplicate insertions have no effect
 */
TEST(SkipListTest, Insert_DuplicateElements_Idempotent)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.insert(10); // Duplicate
    list.insert(30);
    list.insert(20); // Duplicate

    EXPECT_EQ(list.size(), 3);
    EXPECT_TRUE(list.search(10));
    EXPECT_TRUE(list.search(20));
    EXPECT_TRUE(list.search(30));
}

/**
 * @brief Test search for existing element
 * @details Verifies that search returns true for inserted elements
 */
TEST(SkipListTest, Search_ExistingElement_ReturnsTrue)
{
    SkipList<int> list;

    list.insert(100);
    list.insert(200);
    list.insert(300);

    EXPECT_TRUE(list.search(100));
    EXPECT_TRUE(list.search(200));
    EXPECT_TRUE(list.search(300));
}

/**
 * @brief Test search for non-existing element
 * @details Verifies that search returns false for absent elements
 */
TEST(SkipListTest, Search_NonExistingElement_ReturnsFalse)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.insert(30);

    EXPECT_FALSE(list.search(0));
    EXPECT_FALSE(list.search(15));
    EXPECT_FALSE(list.search(100));
}

/**
 * @brief Test search on empty list
 * @details Verifies that search returns false for empty list
 */
TEST(SkipListTest, Search_EmptyList_ReturnsFalse)
{
    const SkipList<int> list;

    EXPECT_FALSE(list.search(42));
}

/**
 * @brief Test erase existing element
 * @details Verifies successful deletion of existing elements
 */
TEST(SkipListTest, Erase_ExistingElement_ReturnsTrue)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.insert(30);

    EXPECT_TRUE(list.erase(20));
    EXPECT_EQ(list.size(), 2);
    EXPECT_FALSE(list.search(20));
    EXPECT_TRUE(list.search(10));
    EXPECT_TRUE(list.search(30));
}

/**
 * @brief Test erase non-existing element
 * @details Verifies that erasing absent element returns false
 */
TEST(SkipListTest, Erase_NonExistingElement_ReturnsFalse)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);

    EXPECT_FALSE(list.erase(999));
    EXPECT_EQ(list.size(), 2);
}

/**
 * @brief Test erase from empty list
 * @details Verifies that erasing from empty list returns false
 */
TEST(SkipListTest, Erase_EmptyList_ReturnsFalse)
{
    SkipList<int> list;

    EXPECT_FALSE(list.erase(42));
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test erase all elements
 * @details Verifies that all elements can be removed
 */
TEST(SkipListTest, Erase_AllElements_ListBecomesEmpty)
{
    SkipList<int> list;

    for (int i = 1; i <= 5; ++i)
    {
        list.insert(i);
    }

    for (int i = 1; i <= 5; ++i)
    {
        EXPECT_TRUE(list.erase(i));
    }

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test erase first element
 * @details Verifies correct deletion of minimum element
 */
TEST(SkipListTest, Erase_FirstElement_Success)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.insert(30);

    EXPECT_TRUE(list.erase(10));
    EXPECT_EQ(list.size(), 2);
    EXPECT_FALSE(list.search(10));
    EXPECT_TRUE(list.search(20));
    EXPECT_TRUE(list.search(30));
}

/**
 * @brief Test erase last element
 * @details Verifies correct deletion of maximum element
 */
TEST(SkipListTest, Erase_LastElement_Success)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.insert(30);

    EXPECT_TRUE(list.erase(30));
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list.search(10));
    EXPECT_TRUE(list.search(20));
    EXPECT_FALSE(list.search(30));
}

/**
 * @brief Test insert after erase
 * @details Verifies that re-insertion works after deletion
 */
TEST(SkipListTest, Insert_AfterErase_Success)
{
    SkipList<int> list;

    list.insert(10);
    list.insert(20);
    list.erase(10);

    EXPECT_EQ(list.size(), 1);
    EXPECT_FALSE(list.search(10));

    list.insert(10); // Re-insert
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list.search(10));
    EXPECT_TRUE(list.search(20));
}

/**
 * @brief Test with negative numbers
 * @details Verifies correct handling of negative integers
 */
TEST(SkipListTest, NegativeNumbers_InsertAndSearch_Success)
{
    SkipList<int> list;

    list.insert(-10);
    list.insert(-5);
    list.insert(-20);
    list.insert(-1);
    list.insert(-15);

    EXPECT_EQ(list.size(), 5);
    EXPECT_TRUE(list.search(-10));
    EXPECT_TRUE(list.search(-5));
    EXPECT_TRUE(list.search(-20));
    EXPECT_TRUE(list.search(-1));
    EXPECT_TRUE(list.search(-15));
    EXPECT_FALSE(list.search(0));
}

/**
 * @brief Test with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign integers
 */
TEST(SkipListTest, MixedSignNumbers_InsertAndSearch_Success)
{
    SkipList<int> list;

    list.insert(-100);
    list.insert(50);
    list.insert(-50);
    list.insert(100);
    list.insert(0);

    EXPECT_EQ(list.size(), 5);
    EXPECT_TRUE(list.search(-100));
    EXPECT_TRUE(list.search(50));
    EXPECT_TRUE(list.search(-50));
    EXPECT_TRUE(list.search(100));
    EXPECT_TRUE(list.search(0));
}

/**
 * @brief Test with string type
 * @details Verifies that SkipList works with string keys
 */
TEST(SkipListTest, StringType_InsertAndSearch_Success)
{
    SkipList<std::string> list;

    list.insert("apple");
    list.insert("banana");
    list.insert("cherry");
    list.insert("date");

    EXPECT_EQ(list.size(), 4);
    EXPECT_TRUE(list.search("apple"));
    EXPECT_TRUE(list.search("banana"));
    EXPECT_TRUE(list.search("cherry"));
    EXPECT_TRUE(list.search("date"));
    EXPECT_FALSE(list.search("elderberry"));
}

/**
 * @brief Test erase with string type
 * @details Verifies deletion works correctly with strings
 */
TEST(SkipListTest, StringType_Erase_Success)
{
    SkipList<std::string> list;

    list.insert("apple");
    list.insert("banana");
    list.insert("cherry");

    EXPECT_TRUE(list.erase("banana"));
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list.search("apple"));
    EXPECT_FALSE(list.search("banana"));
    EXPECT_TRUE(list.search("cherry"));
}

/**
 * @brief Test with double type
 * @details Verifies that SkipList works with floating point numbers
 */
TEST(SkipListTest, DoubleType_InsertAndSearch_Success)
{
    SkipList<double> list;

    list.insert(3.14);
    list.insert(2.71);
    list.insert(1.41);
    list.insert(1.73);

    EXPECT_EQ(list.size(), 4);
    EXPECT_TRUE(list.search(3.14));
    EXPECT_TRUE(list.search(2.71));
    EXPECT_TRUE(list.search(1.41));
    EXPECT_TRUE(list.search(1.73));
}

/**
 * @brief Test large dataset performance
 * @details Verifies correct behavior with many elements
 */
TEST(SkipListTest, LargeDataset_InsertAndSearch_Success)
{
    SkipList<int> list;

    const int numElements = 1000;

    // Insert elements
    for (int i = 0; i < numElements; ++i)
    {
        list.insert(i);
    }

    EXPECT_EQ(list.size(), numElements);

    // Verify all elements exist
    for (int i = 0; i < numElements; ++i)
    {
        EXPECT_TRUE(list.search(i));
    }

    // Verify non-existing elements don't exist
    EXPECT_FALSE(list.search(numElements));
    EXPECT_FALSE(list.search(numElements + 100));
}

/**
 * @brief Test large dataset with erasure
 * @details Verifies correct deletion with many elements
 */
TEST(SkipListTest, LargeDataset_Erase_Success)
{
    SkipList<int> list;

    const int numElements = 100;

    // Insert elements
    for (int i = 0; i < numElements; ++i)
    {
        list.insert(i);
    }

    EXPECT_EQ(list.size(), numElements);

    // Erase half of them
    for (int i = 0; i < numElements; i += 2)
    {
        EXPECT_TRUE(list.erase(i));
    }

    EXPECT_EQ(list.size(), numElements / 2);

    // Verify odd numbers still exist
    for (int i = 1; i < numElements; i += 2)
    {
        EXPECT_TRUE(list.search(i));
    }

    // Verify even numbers are gone
    for (int i = 0; i < numElements; i += 2)
    {
        EXPECT_FALSE(list.search(i));
    }
}

/**
 * @brief Test size consistency after multiple operations
 * @details Verifies that size() accurately reflects list state
 */
TEST(SkipListTest, Size_ConsistencyAfterOperations)
{
    SkipList<int> list;

    EXPECT_EQ(list.size(), 0);

    list.insert(10);
    EXPECT_EQ(list.size(), 1);

    list.insert(20);
    EXPECT_EQ(list.size(), 2);

    list.insert(10); // Duplicate
    EXPECT_EQ(list.size(), 2);

    list.erase(10);
    EXPECT_EQ(list.size(), 1);

    list.erase(20);
    EXPECT_EQ(list.size(), 0);

    list.erase(30); // Non-existing
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test empty consistency after operations
 * @details Verifies that empty() accurately reflects list state
 */
TEST(SkipListTest, Empty_ConsistencyAfterOperations)
{
    SkipList<int> list;

    EXPECT_TRUE(list.empty());

    list.insert(10);
    EXPECT_FALSE(list.empty());

    list.erase(10);
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test sequential insert and erase pattern
 * @details Verifies correctness under alternating operations
 */
TEST(SkipListTest, SequentialInsertErase_Pattern_Success)
{
    SkipList<int> list;

    // Insert 1-10
    for (int i = 1; i <= 10; ++i)
    {
        list.insert(i);
    }
    EXPECT_EQ(list.size(), 10);

    // Erase 1-5
    for (int i = 1; i <= 5; ++i)
    {
        EXPECT_TRUE(list.erase(i));
    }
    EXPECT_EQ(list.size(), 5);

    // Insert 11-15
    for (int i = 11; i <= 15; ++i)
    {
        list.insert(i);
    }
    EXPECT_EQ(list.size(), 10);

    // Verify remaining elements
    for (int i = 1; i <= 5; ++i)
    {
        EXPECT_FALSE(list.search(i));
    }
    for (int i = 6; i <= 10; ++i)
    {
        EXPECT_TRUE(list.search(i));
    }
    for (int i = 11; i <= 15; ++i)
    {
        EXPECT_TRUE(list.search(i));
    }
}

/**
 * @brief Test with single element repeated operations
 * @details Verifies behavior with minimal data
 */
TEST(SkipListTest, SingleElement_RepeatedOperations_Success)
{
    SkipList<int> list;

    list.insert(42);
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list.search(42));

    // Try inserting again
    list.insert(42);
    EXPECT_EQ(list.size(), 1);

    // Erase
    EXPECT_TRUE(list.erase(42));
    EXPECT_EQ(list.size(), 0);
    EXPECT_FALSE(list.search(42));

    // Try erasing again
    EXPECT_FALSE(list.erase(42));
    EXPECT_EQ(list.size(), 0);
}

/**
 * @brief Test toVector method returns sorted elements
 * @details Verifies that toVector returns elements in sorted order
 */
TEST(SkipListTest, ToVector_ReturnsSortedElements)
{
    SkipList<int> list;

    list.insert(30);
    list.insert(10);
    list.insert(20);
    list.insert(40);

    const auto elements = list.toVector();
    EXPECT_EQ(elements.size(), 4);
    EXPECT_EQ(elements[0], 10);
    EXPECT_EQ(elements[1], 20);
    EXPECT_EQ(elements[2], 30);
    EXPECT_EQ(elements[3], 40);
}

/**
 * @brief Test copy semantics are disabled
 * @details Verifies that copy construction and assignment are deleted
 */
TEST(SkipListTest, CopySemantics_Disabled)
{
    SkipList<int> list1;
    list1.insert(10);

    // This should not compile - copy constructor is deleted
    // SkipList<int> list2(list1);

    // Verify the type traits
    EXPECT_FALSE(std::is_copy_constructible_v<SkipList<int>>);
    EXPECT_FALSE(std::is_copy_assignable_v<SkipList<int>>);

    // Note: Move semantics are also not supported due to const member variables
    // (maxLevel_, probability_, header_) which prevent efficient moving
    EXPECT_FALSE(std::is_move_constructible_v<SkipList<int>>);
    EXPECT_FALSE(std::is_move_assignable_v<SkipList<int>>);
}
