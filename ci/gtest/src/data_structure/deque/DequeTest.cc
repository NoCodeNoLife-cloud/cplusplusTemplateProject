/**
 * @file DequeTest.cc
 * @brief Unit tests for the Deque class
 * @details Tests cover core deque operations including push_front/push_back, pop_front/pop_back,
 *          random access, copy/move semantics, and edge cases.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/deque/Deque.hpp"

using namespace common::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<Deque<int>>);
static_assert(std::is_nothrow_move_assignable_v<Deque<int>>);
static_assert(noexcept(std::declval<Deque<int>&>().swap(std::declval<Deque<int>&>())));
static_assert(noexcept(std::declval<Deque<int>&>().clear()));
static_assert(noexcept(std::declval<const Deque<int>&>().empty()));
static_assert(noexcept(std::declval<const Deque<int>&>().size()));

/**
 * @brief Test fixture for DequeTest tests
 */
class DequeTest : public testing::Test
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
 * @brief Test default constructor creates empty deque
 */
TEST_F(DequeTest, DefaultConstructor_EmptyDeque)
{
    const Deque<int> deque;
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 */
TEST_F(DequeTest, CopyConstructor_DeepCopy)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);
    deque1.push_back(30);

    Deque deque2(deque1);

    EXPECT_EQ(deque2.size(), deque1.size());
    EXPECT_EQ(deque2.front(), deque1.front());
    EXPECT_EQ(deque2.back(), deque1.back());

    // Modify original should not affect copy
    deque1.pop_front();
    EXPECT_EQ(deque2.size(), 3);
    EXPECT_EQ(deque2.front(), 10);
}

/**
 * @brief Test copy constructor with empty deque
 */
TEST_F(DequeTest, CopyConstructor_EmptyDeque)
{
    Deque<int> deque1;
    const Deque deque2(deque1);

    EXPECT_TRUE(deque2.empty());
    EXPECT_EQ(deque2.size(), 0);

    // Modify original should not affect copy
    deque1.push_back(10);
    EXPECT_TRUE(deque2.empty());
}

/**
 * @brief Test move constructor transfers ownership
 */
TEST_F(DequeTest, MoveConstructor_TransfersOwnership)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);
    deque1.push_back(30);

    Deque deque2(std::move(deque1));

    EXPECT_EQ(deque2.size(), 3);
    EXPECT_EQ(deque2.front(), 10);
    EXPECT_EQ(deque2.back(), 30);

    // Moved-from deque should be empty
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(deque1.empty());
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_EQ(deque1.size(), 0);
}

/**
 * @brief Test copy assignment operator
 */
TEST_F(DequeTest, CopyAssignment_DeepCopy)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);

    Deque<int> deque2;
    deque2.push_back(30);

    deque2 = deque1;

    EXPECT_EQ(deque2.size(), 2);
    EXPECT_EQ(deque2.front(), 10);
    EXPECT_EQ(deque2.back(), 20);

    // Verify independence
    deque1.pop_front();
    EXPECT_EQ(deque2.size(), 2);
    EXPECT_EQ(deque2.front(), 10);
}

/**
 * @brief Test move assignment operator
 */
TEST_F(DequeTest, MoveAssignment_TransfersOwnership)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);
    deque1.push_back(30);

    Deque<int> deque2(std::move(deque1));

    EXPECT_EQ(deque2.size(), 3);
    EXPECT_EQ(deque2.front(), 10);
    EXPECT_EQ(deque2.back(), 30);

    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(deque1.empty());
}

/**
 * @brief Test push_back adds element to back
 */
TEST_F(DequeTest, PushBack_AddsToBack)
{
    Deque<int> deque;

    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    EXPECT_EQ(deque.size(), 3);
    EXPECT_EQ(deque.front(), 10);
    EXPECT_EQ(deque.back(), 30);
}

/**
 * @brief Test push_front adds element to front
 */
TEST_F(DequeTest, PushFront_AddsToFront)
{
    Deque<int> deque;

    deque.push_front(10);
    deque.push_front(20);
    deque.push_front(30);

    EXPECT_EQ(deque.size(), 3);
    EXPECT_EQ(deque.front(), 30);
    EXPECT_EQ(deque.back(), 10);
}

/**
 * @brief Test mixed push_front and push_back
 */
TEST_F(DequeTest, MixedPush_CorrectOrder)
{
    Deque<int> deque;

    deque.push_back(20);
    deque.push_front(10);
    deque.push_back(30);
    deque.push_front(5);

    EXPECT_EQ(deque.size(), 4);
    EXPECT_EQ(deque.front(), 5);
    EXPECT_EQ(deque.back(), 30);
}

/**
 * @brief Test pop_front removes front element
 */
TEST_F(DequeTest, PopFront_RemovesFront)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    EXPECT_EQ(deque.front(), 10);

    deque.pop_front();
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front(), 20);
    EXPECT_EQ(deque.back(), 30);

    deque.pop_front();
    EXPECT_EQ(deque.front(), 30);
    EXPECT_EQ(deque.back(), 30);
}

/**
 * @brief Test pop_back removes back element
 */
TEST_F(DequeTest, PopBack_RemovesBack)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    EXPECT_EQ(deque.back(), 30);

    deque.pop_back();
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front(), 10);
    EXPECT_EQ(deque.back(), 20);

    deque.pop_back();
    EXPECT_EQ(deque.front(), 10);
    EXPECT_EQ(deque.back(), 10);
}

/**
 * @brief Test alternate pop_front and pop_back
 */
TEST_F(DequeTest, AlternatePop_CorrectState)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    deque.pop_front();
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front(), 20);

    deque.pop_back();
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front(), 20);
    EXPECT_EQ(deque.back(), 20);
}

/**
 * @brief Test pop on empty deque throws exception
 */
TEST_F(DequeTest, PopFront_EmptyDeque_ThrowsException)
{
    Deque<int> deque;
    EXPECT_THROW(deque.pop_front(), std::out_of_range);
}

/**
 * @brief Test pop_back on empty deque throws exception
 */
TEST_F(DequeTest, PopBack_EmptyDeque_ThrowsException)
{
    Deque<int> deque;
    EXPECT_THROW(deque.pop_back(), std::out_of_range);
}

/**
 * @brief Test pop until deque becomes empty
 */
TEST_F(DequeTest, Pop_UntilEmpty)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);

    deque.pop_front();
    deque.pop_front();

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

/**
 * @brief Test front returns reference to first element
 */
TEST_F(DequeTest, Front_ReturnsFirstElement)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);

    EXPECT_EQ(deque.front(), 10);

    // Test non-const access allows modification
    deque.front() = 100;
    EXPECT_EQ(deque.front(), 100);
}

/**
 * @brief Test front on empty deque throws exception
 */
TEST_F(DequeTest, Front_EmptyDeque_ThrowsException)
{
    Deque<int> deque;
    EXPECT_THROW(deque.front(), std::out_of_range);
}

/**
 * @brief Test back returns reference to last element
 */
TEST_F(DequeTest, Back_ReturnsLastElement)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    EXPECT_EQ(deque.back(), 30);

    deque.back() = 300;
    EXPECT_EQ(deque.back(), 300);
}

/**
 * @brief Test back on empty deque throws exception
 */
TEST_F(DequeTest, Back_EmptyDeque_ThrowsException)
{
    Deque<int> deque;
    EXPECT_THROW(deque.back(), std::out_of_range);
}

/**
 * @brief Test operator[] access
 */
TEST_F(DequeTest, SubscriptOperator_Access)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);

    EXPECT_EQ(deque[0], 10);
    EXPECT_EQ(deque[1], 20);
    EXPECT_EQ(deque[2], 30);

    // Non-const allows modification
    deque[1] = 200;
    EXPECT_EQ(deque[1], 200);
}

/**
 * @brief Test operator[] with front inserts (wrap-around)
 */
TEST_F(DequeTest, SubscriptOperator_WithFrontInserts)
{
    Deque<int> deque;
    deque.push_front(30);
    deque.push_front(20);
    deque.push_front(10);

    EXPECT_EQ(deque[0], 10);
    EXPECT_EQ(deque[1], 20);
    EXPECT_EQ(deque[2], 30);
}

/**
 * @brief Test at() with bounds checking
 */
TEST_F(DequeTest, At_BoundsChecking)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);

    EXPECT_EQ(deque.at(0), 10);
    EXPECT_EQ(deque.at(1), 20);

    EXPECT_THROW(deque.at(2), std::out_of_range);
    EXPECT_THROW(deque.at(100), std::out_of_range);
}

/**
 * @brief Test empty returns correct state
 */
TEST_F(DequeTest, Empty_CorrectState)
{
    Deque<int> deque;
    EXPECT_TRUE(deque.empty());

    deque.push_back(10);
    EXPECT_FALSE(deque.empty());

    deque.pop_front();
    EXPECT_TRUE(deque.empty());
}

/**
 * @brief Test size returns correct count
 */
TEST_F(DequeTest, Size_CorrectCount)
{
    Deque<int> deque;
    EXPECT_EQ(deque.size(), 0);

    deque.push_back(10);
    EXPECT_EQ(deque.size(), 1);

    deque.push_back(20);
    deque.push_back(30);
    EXPECT_EQ(deque.size(), 3);

    deque.pop_front();
    EXPECT_EQ(deque.size(), 2);

    deque.pop_front();
    deque.pop_front();
    EXPECT_EQ(deque.size(), 0);
}

/**
 * @brief Test swap exchanges contents
 */
TEST_F(DequeTest, Swap_ExchangesContents)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);
    deque1.push_back(30);

    Deque<int> deque2;
    deque2.push_back(100);
    deque2.push_back(200);

    deque1.swap(deque2);

    EXPECT_EQ(deque1.size(), 2);
    EXPECT_EQ(deque1.front(), 100);
    EXPECT_EQ(deque1.back(), 200);

    EXPECT_EQ(deque2.size(), 3);
    EXPECT_EQ(deque2.front(), 10);
    EXPECT_EQ(deque2.back(), 30);
}

/**
 * @brief Test swap with empty deque
 */
TEST_F(DequeTest, Swap_WithEmptyDeque)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);

    Deque<int> deque2;

    deque1.swap(deque2);

    EXPECT_TRUE(deque1.empty());
    EXPECT_EQ(deque2.size(), 2);
    EXPECT_EQ(deque2.front(), 10);
}

/**
 * @brief Test swap is symmetric
 */
TEST_F(DequeTest, Swap_Symmetric)
{
    Deque<int> deque1;
    deque1.push_back(10);
    deque1.push_back(20);

    Deque<int> deque2;
    deque2.push_back(30);
    deque2.push_back(40);

    deque1.swap(deque2);
    deque1.swap(deque2);

    EXPECT_EQ(deque1.size(), 2);
    EXPECT_EQ(deque1.front(), 10);
    EXPECT_EQ(deque1.back(), 20);

    EXPECT_EQ(deque2.size(), 2);
    EXPECT_EQ(deque2.front(), 30);
    EXPECT_EQ(deque2.back(), 40);
}

/**
 * @brief Test deque with string elements
 */
TEST_F(DequeTest, StringElements_CorrectBehavior)
{
    Deque<std::string> deque;

    deque.push_back("hello");
    deque.push_front("start");
    deque.push_back("world");

    EXPECT_EQ(deque.size(), 3);
    EXPECT_EQ(deque.front(), "start");
    EXPECT_EQ(deque.back(), "world");

    deque.pop_front();
    EXPECT_EQ(deque.front(), "hello");
}

/**
 * @brief Test deque with duplicate values
 */
TEST_F(DequeTest, DuplicateValues_CorrectHandling)
{
    Deque<int> deque;

    deque.push_back(5);
    deque.push_front(5);
    deque.push_back(5);
    deque.push_back(10);
    deque.push_front(5);

    EXPECT_EQ(deque.size(), 5);
    EXPECT_EQ(deque.front(), 5);
    EXPECT_EQ(deque.back(), 10);

    deque.pop_front();
    EXPECT_EQ(deque.front(), 5);
}

/**
 * @brief Test large number of elements
 */
TEST_F(DequeTest, LargeNumberOfElements_Correctness)
{
    Deque<int> deque;

    for (int i = 0; i < 1000; ++i)
    {
        deque.push_back(i);
    }

    EXPECT_EQ(deque.size(), 1000);
    EXPECT_EQ(deque.front(), 0);
    EXPECT_EQ(deque.back(), 999);

    for (int i = 0; i < 500; ++i)
    {
        deque.pop_front();
    }

    EXPECT_EQ(deque.size(), 500);
    EXPECT_EQ(deque.front(), 500);
    EXPECT_EQ(deque.back(), 999);
}

/**
 * @brief Test alternating push and pop
 */
TEST_F(DequeTest, AlternatingPushPop_Correctness)
{
    Deque<int> deque;

    deque.push_back(1);
    deque.push_back(2);
    deque.pop_front();
    deque.push_back(3);
    deque.push_back(4);
    deque.pop_front();
    deque.pop_front();

    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front(), 4);
    EXPECT_EQ(deque.back(), 4);
}

/**
 * @brief Test alternating front and back operations
 */
TEST_F(DequeTest, AlternatingFrontBackPush_Correctness)
{
    Deque<int> deque;

    deque.push_back(2);
    deque.push_front(1);
    deque.push_back(3);
    deque.push_front(0);

    EXPECT_EQ(deque.size(), 4);
    EXPECT_EQ(deque[0], 0);
    EXPECT_EQ(deque[1], 1);
    EXPECT_EQ(deque[2], 2);
    EXPECT_EQ(deque[3], 3);

    deque.pop_front();
    deque.pop_back();

    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque[0], 1);
    EXPECT_EQ(deque[1], 2);
}

/**
 * @brief Test single element push and pop cycle
 */
TEST_F(DequeTest, SingleElementCycle_Correctness)
{
    Deque<int> deque;

    for (int i = 0; i < 10; ++i)
    {
        deque.push_back(i);
        EXPECT_EQ(deque.size(), 1);
        EXPECT_EQ(deque.front(), i);
        EXPECT_EQ(deque.back(), i);
        deque.pop_front();
        EXPECT_TRUE(deque.empty());
    }
}

/**
 * @brief Test push_front with rvalue reference
 */
TEST_F(DequeTest, PushFrontRValue_MovesElement)
{
    Deque<std::unique_ptr<int>> deque;
    auto ptr = std::make_unique<int>(42);
    deque.push_front(std::move(ptr));
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(*deque.front(), 42);
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @brief Test push_back with rvalue reference
 */
TEST_F(DequeTest, PushBackRValue_MovesElement)
{
    Deque<std::unique_ptr<int>> deque;
    auto ptr = std::make_unique<int>(42);
    deque.push_back(std::move(ptr));
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(*deque.front(), 42);
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @brief Test emplace constructs element in-place at back
 */
TEST_F(DequeTest, EmplaceBack_ConstructsInPlace)
{
    Deque<std::pair<int, int>> deque;
    deque.emplace_back(1, 2);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front().first, 1);
    EXPECT_EQ(deque.front().second, 2);

    deque.emplace_back(3, 4);
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front().first, 1);
    EXPECT_EQ(deque.back().first, 3);
}

/**
 * @brief Test emplace constructs element in-place at front
 */
TEST_F(DequeTest, EmplaceFront_ConstructsInPlace)
{
    Deque<std::pair<int, int>> deque;
    deque.emplace_front(1, 2);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front().first, 1);
    EXPECT_EQ(deque.front().second, 2);

    deque.emplace_front(3, 4);
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front().first, 3);
    EXPECT_EQ(deque.back().first, 1);
    EXPECT_EQ(deque.back().second, 2);
}

/**
 * @brief Test clear removes all elements
 */
TEST_F(DequeTest, Clear_EmptiesDeque)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    deque.push_back(30);
    EXPECT_EQ(deque.size(), 3);

    deque.clear();
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
    EXPECT_THROW(deque.front(), std::out_of_range);
    EXPECT_THROW(deque.back(), std::out_of_range);
    EXPECT_THROW(deque.pop_front(), std::out_of_range);
    EXPECT_THROW(deque.pop_back(), std::out_of_range);
}

/**
 * @brief Test clear on already empty deque
 */
TEST_F(DequeTest, Clear_EmptyDeque_NoOp)
{
    Deque<int> deque;
    deque.clear();
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

/**
 * @brief Test copy assignment to self
 */
TEST_F(DequeTest, CopyAssignment_SelfAssignment)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);

    // NOLINTNEXTLINE(bugprone-self-assignment)
    deque = deque;

    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front(), 10);
    EXPECT_EQ(deque.back(), 20);
}

/**
 * @brief Test deque with negative numbers
 */
TEST_F(DequeTest, NegativeNumbers_CorrectHandling)
{
    Deque<int> deque;

    deque.push_back(-10);
    deque.push_front(-5);
    deque.push_back(-20);
    deque.push_front(-1);

    EXPECT_EQ(deque.size(), 4);
    EXPECT_EQ(deque.front(), -1);
    EXPECT_EQ(deque.back(), -20);

    deque.pop_back();
    EXPECT_EQ(deque.back(), -10);
}

/**
 * @brief Test deque preserves insertion order via random access
 */
TEST_F(DequeTest, RandomAccess_CorrectOrder)
{
    Deque<int> deque;

    const std::vector input = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    for (int val : input)
    {
        deque.push_back(val);
    }

    EXPECT_EQ(deque.size(), input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        EXPECT_EQ(deque[i], input[i]);
    }
}

/**
 * @brief Test extraction from both ends via pop
 */
TEST_F(DequeTest, PopFrontBack_AllElements)
{
    Deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);
    deque.push_back(4);

    EXPECT_EQ(deque.front(), 1);
    EXPECT_EQ(deque.back(), 4);

    deque.pop_front();
    EXPECT_EQ(deque.front(), 2);
    EXPECT_EQ(deque.back(), 4);

    deque.pop_back();
    EXPECT_EQ(deque.front(), 2);
    EXPECT_EQ(deque.back(), 3);

    deque.pop_front();
    EXPECT_EQ(deque.front(), 3);
    EXPECT_EQ(deque.back(), 3);

    deque.pop_back();
    EXPECT_TRUE(deque.empty());
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST_F(DequeTest, MoveConstructor)
{
    Deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);
    EXPECT_EQ(deque.size(), 3);

    Deque<int> other(std::move(deque));
    EXPECT_EQ(other.size(), 3);
    EXPECT_EQ(other.front(), 1);
    EXPECT_EQ(other.back(), 3);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(deque.empty());
}

TEST_F(DequeTest, MoveAssignment)
{
    Deque<int> deque;
    deque.push_back(10);
    deque.push_back(20);
    EXPECT_EQ(deque.size(), 2);

    Deque<int> other;
    other.push_back(99);
    other = std::move(deque);
    EXPECT_EQ(other.size(), 2);
    EXPECT_EQ(other.front(), 10);
    EXPECT_EQ(other.back(), 20);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(deque.empty());
}
