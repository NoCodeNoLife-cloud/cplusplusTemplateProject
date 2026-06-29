/**
 * @file TreeSetTest.cc
 * @brief Unit tests for the TreeSet class
 * @details Tests cover core set operations including insert, erase, contains, min, max,
 *          toVector, copy/move semantics, and edge cases.
 */

#include <string>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include <cppforge/data_structure/set/TreeSet.hpp>

using namespace cppforge::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<TreeSet<int>>);
static_assert(std::is_nothrow_move_assignable_v<TreeSet<int>>);
static_assert(noexcept(std::declval<TreeSet<int>&>().swap(std::declval<TreeSet<int>&>())));
static_assert(noexcept(std::declval<const TreeSet<int>&>().empty()));
static_assert(noexcept(std::declval<const TreeSet<int>&>().size()));

/**
 * @brief Test fixture for TreeSet tests
 */
class TreeSetTest : public testing::Test
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
TEST_F(TreeSetTest, DefaultConstructor_EmptySet)
{
    const TreeSet<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST_F(TreeSetTest, CopyConstructor_DeepCopy)
{
    TreeSet<int> set1;
    EXPECT_TRUE(set1.insert(10));
    EXPECT_TRUE(set1.insert(20));
    EXPECT_TRUE(set1.insert(30));

    const TreeSet set2(set1);

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
TEST_F(TreeSetTest, CopyConstructor_EmptySet)
{
    const TreeSet<int> set1;
    const TreeSet set2(set1);

    EXPECT_TRUE(set2.empty());
    EXPECT_EQ(set2.size(), 0);
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST_F(TreeSetTest, MoveConstructor_TransfersOwnership)
{
    TreeSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    const TreeSet set2(std::move(set1));

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
TEST_F(TreeSetTest, CopyAssignment_DeepCopy)
{
    TreeSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    TreeSet<int> set2;
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
TEST_F(TreeSetTest, MoveAssignment_TransfersOwnership)
{
    TreeSet<int> set1;
    set1.insert(10);
    set1.insert(20);

    TreeSet<int> set2;
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
TEST_F(TreeSetTest, Insert_NewElement_ReturnsTrue)
{
    TreeSet<int> set;
    EXPECT_TRUE(set.insert(42));
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test insert duplicate element
 * @details Verifies that inserting existing element returns false
 */
TEST_F(TreeSetTest, Insert_Duplicate_ReturnsFalse)
{
    TreeSet<int> set;
    EXPECT_TRUE(set.insert(42));
    EXPECT_FALSE(set.insert(42));
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test insert multiple elements in order
 * @details Verifies inserting elements in ascending order works correctly
 */
TEST_F(TreeSetTest, Insert_AscendingOrder)
{
    TreeSet<int> set;
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(set.insert(i));
    }
    EXPECT_EQ(set.size(), 100);

    const auto vec = set.toVector();
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

/**
 * @brief Test insert elements in descending order
 * @details Verifies inserting elements in descending order works correctly
 */
TEST_F(TreeSetTest, Insert_DescendingOrder)
{
    TreeSet<int> set;
    for (int i = 99; i >= 0; --i)
    {
        EXPECT_TRUE(set.insert(i));
    }
    EXPECT_EQ(set.size(), 100);

    const auto vec = set.toVector();
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

/**
 * @brief Test insert random order elements
 * @details Verifies inserting elements in arbitrary order maintains sorted order
 */
TEST_F(TreeSetTest, Insert_RandomOrder)
{
    TreeSet<int> set;
    const std::vector<int> values = {5, 3, 8, 1, 9, 2, 7, 4, 6, 0};

    for (int v : values)
    {
        EXPECT_TRUE(set.insert(v));
    }
    EXPECT_EQ(set.size(), 10);

    const std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(set.toVector(), expected);
}

// ==================== Erase Tests ====================

/**
 * @brief Test erase existing element
 * @details Verifies that erase removes existing element and returns true
 */
TEST_F(TreeSetTest, Erase_ExistingElement_ReturnsTrue)
{
    TreeSet<int> set;
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
TEST_F(TreeSetTest, Erase_NonExisting_ReturnsFalse)
{
    TreeSet<int> set;
    set.insert(10);
    EXPECT_FALSE(set.erase(99));
    EXPECT_EQ(set.size(), 1);
}

/**
 * @brief Test erase root element
 * @details Verifies erasing the root of the tree works correctly
 */
TEST_F(TreeSetTest, Erase_RootElement)
{
    TreeSet<int> set;
    set.insert(5);
    set.insert(3);
    set.insert(8);
    set.insert(1);
    set.insert(4);
    set.insert(7);
    set.insert(9);

    EXPECT_TRUE(set.erase(5));
    EXPECT_FALSE(set.contains(5));
    EXPECT_EQ(set.size(), 6);

    const std::vector<int> expected = {1, 3, 4, 7, 8, 9};
    EXPECT_EQ(set.toVector(), expected);
}

/**
 * @brief Test erase leaf element
 * @details Verifies erasing a leaf node works correctly
 */
TEST_F(TreeSetTest, Erase_LeafElement)
{
    TreeSet<int> set;
    set.insert(5);
    set.insert(3);
    set.insert(8);

    EXPECT_TRUE(set.erase(3));
    EXPECT_FALSE(set.contains(3));
    EXPECT_EQ(set.size(), 2);

    const std::vector<int> expected = {5, 8};
    EXPECT_EQ(set.toVector(), expected);
}

/**
 * @brief Test erase all elements
 * @details Verifies erasing all elements empties the set
 */
TEST_F(TreeSetTest, Erase_AllElements)
{
    TreeSet<int> set;
    for (int i = 0; i < 10; ++i)
    {
        set.insert(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(set.erase(i));
    }
    EXPECT_TRUE(set.empty());
}

// ==================== Contains & Find Tests ====================

/**
 * @brief Test contains returns true for existing elements
 * @details Verifies contains correctly identifies existing elements
 */
TEST_F(TreeSetTest, Contains_ExistingElement_ReturnsTrue)
{
    TreeSet<int> set;
    set.insert(42);
    EXPECT_TRUE(set.contains(42));
}

/**
 * @brief Test contains returns false for missing elements
 * @details Verifies contains correctly identifies missing elements
 */
TEST_F(TreeSetTest, Contains_NonExisting_ReturnsFalse)
{
    const TreeSet<int> set;
    EXPECT_FALSE(set.contains(42));
}

/**
 * @brief Test findValue returns value for existing elements
 * @details Verifies findValue returns the value for existing elements
 */
TEST_F(TreeSetTest, FindValue_ExistingElement_ReturnsValue)
{
    TreeSet<int> set;
    set.insert(42);
    const auto result = set.findValue(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

/**
 * @brief Test findValue returns nullopt for missing elements
 * @details Verifies findValue returns nullopt for missing elements
 */
TEST_F(TreeSetTest, FindValue_NonExisting_ReturnsNullopt)
{
    const TreeSet<int> set;
    EXPECT_FALSE(set.findValue(42).has_value());
}

// ==================== Min & Max Tests ====================

/**
 * @brief Test min returns smallest element
 * @details Verifies min returns the minimum element in the set
 */
TEST_F(TreeSetTest, Min_ReturnsSmallestElement)
{
    TreeSet<int> set;
    set.insert(10);
    set.insert(5);
    set.insert(20);
    set.insert(1);

    const auto result = set.min();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);
}

/**
 * @brief Test max returns largest element
 * @details Verifies max returns the maximum element in the set
 */
TEST_F(TreeSetTest, Max_ReturnsLargestElement)
{
    TreeSet<int> set;
    set.insert(10);
    set.insert(5);
    set.insert(20);
    set.insert(1);

    const auto result = set.max();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 20);
}

/**
 * @brief Test min on empty set returns nullopt
 * @details Verifies min returns nullopt when set is empty
 */
TEST_F(TreeSetTest, Min_EmptySet_ReturnsNullopt)
{
    const TreeSet<int> set;
    EXPECT_FALSE(set.min().has_value());
}

/**
 * @brief Test max on empty set returns nullopt
 * @details Verifies max returns nullopt when set is empty
 */
TEST_F(TreeSetTest, Max_EmptySet_ReturnsNullopt)
{
    const TreeSet<int> set;
    EXPECT_FALSE(set.max().has_value());
}

/**
 * @brief Test min and max on single element
 * @details Verifies min and max both return the same element for single-element set
 */
TEST_F(TreeSetTest, MinMax_SingleElement)
{
    TreeSet<int> set;
    set.insert(42);

    EXPECT_EQ(set.min(), 42);
    EXPECT_EQ(set.max(), 42);
}

// ==================== ToVector Tests ====================

/**
 * @brief Test toVector returns sorted elements
 * @details Verifies toVector returns all elements in sorted order
 */
TEST_F(TreeSetTest, ToVector_ReturnsSortedOrder)
{
    TreeSet<int> set;
    set.insert(5);
    set.insert(3);
    set.insert(8);
    set.insert(1);
    set.insert(4);

    const std::vector<int> expected = {1, 3, 4, 5, 8};
    EXPECT_EQ(set.toVector(), expected);
}

/**
 * @brief Test toVector on empty set
 * @details Verifies toVector on empty set returns empty vector
 */
TEST_F(TreeSetTest, ToVector_EmptySet)
{
    const TreeSet<int> set;
    EXPECT_TRUE(set.toVector().empty());
}

// ==================== Clear & Swap Tests ====================

/**
 * @brief Test clear empties the set
 * @details Verifies that clear removes all elements
 */
TEST_F(TreeSetTest, Clear_EmptiesSet)
{
    TreeSet<int> set;
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
TEST_F(TreeSetTest, Clear_EmptySet_NoOp)
{
    TreeSet<int> set;
    set.clear();
    EXPECT_TRUE(set.empty());
}

/**
 * @brief Test clear then reuse
 * @details Verifies cleared set can be reused
 */
TEST_F(TreeSetTest, Clear_ThenReuse)
{
    TreeSet<int> set;
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
TEST_F(TreeSetTest, Swap_ExchangesContents)
{
    TreeSet<int> set1;
    set1.insert(1);
    set1.insert(2);

    TreeSet<int> set2;
    set2.insert(10);
    set2.insert(20);
    set2.insert(30);

    set1.swap(set2);

    EXPECT_EQ(set1.size(), 3);
    EXPECT_TRUE(set1.contains(10));

    EXPECT_EQ(set2.size(), 2);
    EXPECT_TRUE(set2.contains(1));
}

// ==================== Edge Case Tests ====================

/**
 * @brief Test string type elements
 * @details Verifies TreeSet works with std::string
 */
TEST_F(TreeSetTest, StringType_Operations)
{
    TreeSet<std::string> set;
    EXPECT_TRUE(set.insert("banana"));
    EXPECT_TRUE(set.insert("apple"));
    EXPECT_TRUE(set.insert("cherry"));

    EXPECT_EQ(set.size(), 3);
    EXPECT_TRUE(set.contains("apple"));
    EXPECT_TRUE(set.contains("banana"));
    EXPECT_TRUE(set.contains("cherry"));

    // Verify sorted order
    const auto vec = set.toVector();
    const std::vector<std::string> expected = {"apple", "banana", "cherry"};
    EXPECT_EQ(vec, expected);

    // Verify min and max
    EXPECT_EQ(set.min(), "apple");
    EXPECT_EQ(set.max(), "cherry");
}

/**
 * @brief Test insert and erase cycle
 * @details Verifies alternating insert and erase operations maintain tree balance
 */
TEST_F(TreeSetTest, InsertErase_Cycle)
{
    TreeSet<int> set;

    for (int i = 0; i < 10; ++i)
    {
        set.insert(i);
    }
    EXPECT_EQ(set.size(), 10);

    for (int i = 5; i < 10; ++i)
    {
        set.erase(i);
    }
    EXPECT_EQ(set.size(), 5);

    const auto vec = set.toVector();
    const std::vector<int> expected = {0, 1, 2, 3, 4};
    EXPECT_EQ(vec, expected);
}

// ==================== Large Data Set Tests ====================

/**
 * @brief Test large number of elements
 * @details Verifies set handles many elements correctly with log n performance
 */
TEST_F(TreeSetTest, LargeNumberOfElements_Correctness)
{
    TreeSet<int> set;
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

    // Verify sorted order
    const auto vec = set.toVector();
    EXPECT_EQ(vec.size(), n);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }

    // Erase every other element
    for (int i = 0; i < n; i += 2)
    {
        set.erase(i);
    }
    EXPECT_EQ(set.size(), n / 2);

    // Verify remaining elements
    for (int i = 0; i < n; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(set.contains(i));
        }
        else
        {
            EXPECT_TRUE(set.contains(i));
        }
    }
}

/**
 * @brief Test min and max after insertions
 * @details Verifies min and max are updated correctly during insertions
 */
TEST_F(TreeSetTest, MinMax_AfterInsertions)
{
    TreeSet<int> set;
    set.insert(50);
    EXPECT_EQ(set.min(), 50);
    EXPECT_EQ(set.max(), 50);

    set.insert(10);
    EXPECT_EQ(set.min(), 10);
    EXPECT_EQ(set.max(), 50);

    set.insert(90);
    EXPECT_EQ(set.min(), 10);
    EXPECT_EQ(set.max(), 90);

    set.erase(10);
    EXPECT_EQ(set.min(), 50);
    EXPECT_EQ(set.max(), 90);

    set.erase(90);
    EXPECT_EQ(set.min(), 50);
    EXPECT_EQ(set.max(), 50);
}
