/**
 * @file QueueTest.cc
 * @brief Unit tests for the Queue class
 * @details Tests cover core queue operations including push, pop, front, back access,
 *          size management, copy/move semantics, and edge cases.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/queue/Queue.hpp"

using namespace cppforge::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<Queue<int>>);
static_assert(std::is_nothrow_move_assignable_v<Queue<int>>);
static_assert(noexcept(std::declval<Queue<int>&>().swap(std::declval<Queue<int>&>())));
static_assert(noexcept(std::declval<Queue<int>&>().clear()));
static_assert(noexcept(std::declval<const Queue<int>&>().empty()));
static_assert(noexcept(std::declval<const Queue<int>&>().size()));

/**
 * @brief Test fixture for QueueTest tests
 */
class QueueTest : public testing::Test
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
 * @brief Test default constructor creates empty queue
 * @details Verifies that a newly constructed queue is empty with zero size
 */
TEST_F(QueueTest, DefaultConstructor_EmptyQueue)
{
    const Queue<int> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST_F(QueueTest, CopyConstructor_DeepCopy)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue queue2(queue1);

    EXPECT_EQ(queue2.size(), queue1.size());
    EXPECT_EQ(queue2.front(), queue1.front());
    EXPECT_EQ(queue2.back(), queue1.back());

    // Modify original should not affect copy
    queue1.pop();
    EXPECT_EQ(queue2.size(), 3);
    EXPECT_EQ(queue2.front(), 10);
}

/**
 * @brief Test copy constructor with empty queue
 * @details Verifies copying an empty queue works correctly
 */
TEST_F(QueueTest, CopyConstructor_EmptyQueue)
{
    const Queue<int> queue1;
    const Queue queue2(queue1);

    EXPECT_TRUE(queue2.empty());
    EXPECT_EQ(queue2.size(), 0);
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST_F(QueueTest, MoveConstructor_TransfersOwnership)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue queue2(std::move(queue1));

    EXPECT_EQ(queue2.size(), 3);
    EXPECT_EQ(queue2.front(), 10);
    EXPECT_EQ(queue2.back(), 30);

    // Moved-from queue should be empty
    EXPECT_TRUE(queue1.empty());
    EXPECT_EQ(queue1.size(), 0);
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment performs deep copy correctly
 */
TEST_F(QueueTest, CopyAssignment_DeepCopy)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);

    Queue<int> queue2;
    queue2.push(30);

    queue2 = queue1;

    EXPECT_EQ(queue2.size(), 2);
    EXPECT_EQ(queue2.front(), 10);
    EXPECT_EQ(queue2.back(), 20);

    // Verify independence
    queue1.pop();
    EXPECT_EQ(queue2.size(), 2);
    EXPECT_EQ(queue2.front(), 10);
}

/**
 * @brief Test copy assignment to self
 * @details Verifies self-assignment is handled safely
 */
TEST_F(QueueTest, CopyAssignment_SelfAssignment)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    // NOLINTNEXTLINE(bugprone-self-assignment)
    queue = queue;

    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front(), 10);
    EXPECT_EQ(queue.back(), 20);
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership efficiently
 */
TEST_F(QueueTest, MoveAssignment_TransfersOwnership)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue<int> queue2(std::move(queue1));

    EXPECT_EQ(queue2.size(), 3);
    EXPECT_EQ(queue2.front(), 10);
    EXPECT_EQ(queue2.back(), 30);

    // Moved-from queue should be empty
    EXPECT_TRUE(queue1.empty());
}

/**
 * @brief Test push adds element to back of queue
 * @details Verifies FIFO ordering is maintained after push operations
 */
TEST_F(QueueTest, Push_MaintainsFIFOOrder)
{
    Queue<int> queue;

    queue.push(10);
    queue.push(20);
    queue.push(30);

    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.front(), 10);
    EXPECT_EQ(queue.back(), 30);
}

/**
 * @brief Test push with single element
 * @details Verifies behavior when adding first element to empty queue
 */
TEST_F(QueueTest, Push_SingleElement)
{
    Queue<int> queue;
    queue.push(42);

    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front(), 42);
    EXPECT_EQ(queue.back(), 42);
}

/**
 * @brief Test push with multiple elements
 * @details Verifies correct ordering with many elements
 */
TEST_F(QueueTest, Push_MultipleElements)
{
    Queue<int> queue;

    for (int i = 1; i <= 100; ++i)
    {
        queue.push(i);
    }

    EXPECT_EQ(queue.size(), 100);
    EXPECT_EQ(queue.front(), 1);
    EXPECT_EQ(queue.back(), 100);
}

/**
 * @brief Test pop removes front element
 * @details Verifies that pop correctly removes the front element and updates queue
 */
TEST_F(QueueTest, Pop_RemovesFrontElement)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);

    EXPECT_EQ(queue.front(), 10);

    queue.pop();
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front(), 20);
    EXPECT_EQ(queue.back(), 30);

    queue.pop();
    EXPECT_EQ(queue.front(), 30);
    EXPECT_EQ(queue.back(), 30);
}

/**
 * @brief Test pop on empty queue throws exception
 * @details Verifies proper error handling when popping from empty queue
 */
TEST_F(QueueTest, Pop_EmptyQueue_ThrowsException)
{
    Queue<int> queue;
    EXPECT_THROW(queue.pop(), std::out_of_range);
}

/**
 * @brief Test pop until queue becomes empty
 * @details Verifies that queue correctly transitions to empty state
 */
TEST_F(QueueTest, Pop_UntilEmpty)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    queue.pop();
    queue.pop();

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test front returns reference to first element
 * @details Verifies const and non-const front accessors work correctly
 */
TEST_F(QueueTest, Front_ReturnsFirstElement)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);

    EXPECT_EQ(queue.front(), 10);

    // Test non-const access allows modification
    queue.front() = 100;
    EXPECT_EQ(queue.front(), 100);
}

/**
 * @brief Test front on empty queue throws exception
 * @details Verifies proper error handling for empty queue
 */
TEST_F(QueueTest, Front_EmptyQueue_ThrowsException)
{
    Queue<int> queue;
    EXPECT_THROW(queue.front(), std::out_of_range);
}

/**
 * @brief Test const front accessor
 * @details Verifies const-correctness of front method
 */
TEST_F(QueueTest, Front_ConstAccessor)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    const Queue<int>& constQueue = queue;
    EXPECT_EQ(constQueue.front(), 10);
}

/**
 * @brief Test back returns reference to last element
 * @details Verifies const and non-const back accessors work correctly
 */
TEST_F(QueueTest, Back_ReturnsLastElement)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);

    EXPECT_EQ(queue.back(), 30);

    // Test non-const access allows modification
    queue.back() = 300;
    EXPECT_EQ(queue.back(), 300);
}

/**
 * @brief Test back on empty queue throws exception
 * @details Verifies proper error handling for empty queue
 */
TEST_F(QueueTest, Back_EmptyQueue_ThrowsException)
{
    Queue<int> queue;
    EXPECT_THROW(queue.back(), std::out_of_range);
}

/**
 * @brief Test const back accessor
 * @details Verifies const-correctness of back method
 */
TEST_F(QueueTest, Back_ConstAccessor)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    const Queue<int>& constQueue = queue;
    EXPECT_EQ(constQueue.back(), 20);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies accurate empty state detection throughout queue lifecycle
 */
TEST_F(QueueTest, Empty_CorrectState)
{
    Queue<int> queue;
    EXPECT_TRUE(queue.empty());

    queue.push(10);
    EXPECT_FALSE(queue.empty());

    queue.pop();
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test size returns correct count
 * @details Verifies accurate element counting after various operations
 */
TEST_F(QueueTest, Size_CorrectCount)
{
    Queue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(10);
    EXPECT_EQ(queue.size(), 1);

    queue.push(20);
    queue.push(30);
    EXPECT_EQ(queue.size(), 3);

    queue.pop();
    EXPECT_EQ(queue.size(), 2);

    queue.pop();
    queue.pop();
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test swap exchanges contents of two queues
 * @details Verifies that swap efficiently exchanges all elements
 */
TEST_F(QueueTest, Swap_ExchangesContents)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue<int> queue2;
    queue2.push(100);
    queue2.push(200);

    queue1.swap(queue2);

    EXPECT_EQ(queue1.size(), 2);
    EXPECT_EQ(queue1.front(), 100);
    EXPECT_EQ(queue1.back(), 200);

    EXPECT_EQ(queue2.size(), 3);
    EXPECT_EQ(queue2.front(), 10);
    EXPECT_EQ(queue2.back(), 30);
}

/**
 * @brief Test swap with empty queue
 * @details Verifies swapping with an empty queue works correctly
 */
TEST_F(QueueTest, Swap_WithEmptyQueue)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);

    Queue<int> queue2;

    queue1.swap(queue2);

    EXPECT_TRUE(queue1.empty());
    EXPECT_EQ(queue2.size(), 2);
    EXPECT_EQ(queue2.front(), 10);
}

/**
 * @brief Test swap is symmetric
 * @details Verifies that swapping twice restores original state
 */
TEST_F(QueueTest, Swap_Symmetric)
{
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);

    Queue<int> queue2;
    queue2.push(30);
    queue2.push(40);

    queue1.swap(queue2);
    queue1.swap(queue2);

    EXPECT_EQ(queue1.size(), 2);
    EXPECT_EQ(queue1.front(), 10);
    EXPECT_EQ(queue1.back(), 20);

    EXPECT_EQ(queue2.size(), 2);
    EXPECT_EQ(queue2.front(), 30);
    EXPECT_EQ(queue2.back(), 40);
}

/**
 * @brief Test queue with string elements
 * @details Verifies queue works correctly with non-numeric types
 */
TEST_F(QueueTest, StringElements_CorrectBehavior)
{
    Queue<std::string> queue;

    queue.push("hello");
    queue.push("world");
    queue.push("test");

    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.front(), "hello");
    EXPECT_EQ(queue.back(), "test");

    queue.pop();
    EXPECT_EQ(queue.front(), "world");
}

/**
 * @brief Test queue with duplicate values
 * @details Verifies correct handling of duplicate elements
 */
TEST_F(QueueTest, DuplicateValues_CorrectHandling)
{
    Queue<int> queue;

    queue.push(5);
    queue.push(5);
    queue.push(5);
    queue.push(10);
    queue.push(5);

    EXPECT_EQ(queue.size(), 5);
    EXPECT_EQ(queue.front(), 5);
    EXPECT_EQ(queue.back(), 5);

    queue.pop();
    EXPECT_EQ(queue.front(), 5);
}

/**
 * @brief Test queue with negative numbers
 * @details Verifies correct handling of negative values
 */
TEST_F(QueueTest, NegativeNumbers_CorrectHandling)
{
    Queue<int> queue;

    queue.push(-10);
    queue.push(-5);
    queue.push(-20);
    queue.push(-1);

    EXPECT_EQ(queue.size(), 4);
    EXPECT_EQ(queue.front(), -10);
    EXPECT_EQ(queue.back(), -1);

    queue.pop();
    EXPECT_EQ(queue.front(), -5);
}

/**
 * @brief Test queue with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign values
 */
TEST_F(QueueTest, MixedSignNumbers_CorrectHandling)
{
    Queue<int> queue;

    queue.push(-100);
    queue.push(50);
    queue.push(-50);
    queue.push(100);
    queue.push(0);

    EXPECT_EQ(queue.size(), 5);
    EXPECT_EQ(queue.front(), -100);
    EXPECT_EQ(queue.back(), 0);
}

/**
 * @brief Test large number of elements
 * @details Verifies scalability and correctness with many elements
 */
TEST_F(QueueTest, LargeNumberOfElements_Correctness)
{
    Queue<int> queue;

    // Add 1000 elements
    for (int i = 0; i < 1000; ++i)
    {
        queue.push(i);
    }

    EXPECT_EQ(queue.size(), 1000);
    EXPECT_EQ(queue.front(), 0);
    EXPECT_EQ(queue.back(), 999);

    // Remove half
    for (int i = 0; i < 500; ++i)
    {
        queue.pop();
    }

    EXPECT_EQ(queue.size(), 500);
    EXPECT_EQ(queue.front(), 500);
    EXPECT_EQ(queue.back(), 999);
}

/**
 * @brief Test alternating push and pop operations
 * @details Verifies queue maintains correctness through mixed operations
 */
TEST_F(QueueTest, AlternatingPushPop_Correctness)
{
    Queue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.pop();
    queue.push(3);
    queue.push(4);
    queue.pop();
    queue.pop();

    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front(), 4);
    EXPECT_EQ(queue.back(), 4);
}

/**
 * @brief Test single element push and pop cycle
 * @details Verifies edge case with one element being added and removed repeatedly
 */
TEST_F(QueueTest, SingleElementCycle_Correctness)
{
    Queue<int> queue;

    for (int i = 0; i < 10; ++i)
    {
        queue.push(i);
        EXPECT_EQ(queue.size(), 1);
        EXPECT_EQ(queue.front(), i);
        EXPECT_EQ(queue.back(), i);
        queue.pop();
    }
    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST_F(QueueTest, MoveConstructor)
{
    Queue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);
    EXPECT_EQ(queue.size(), 3);

    Queue<int> other(std::move(queue));
    EXPECT_EQ(other.size(), 3);
    EXPECT_EQ(other.front(), 1);
    EXPECT_EQ(other.back(), 3);
    EXPECT_TRUE(queue.empty());
}

TEST_F(QueueTest, MoveAssignment)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);
    EXPECT_EQ(queue.size(), 2);

    Queue<int> other;
    other.push(99);
    other = std::move(queue);
    EXPECT_EQ(other.size(), 2);
    EXPECT_EQ(other.front(), 10);
    EXPECT_EQ(other.back(), 20);
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test push with rvalue reference moves element
 * @details Verifies that push(T&&) correctly moves the element into the queue
 */
TEST_F(QueueTest, PushRValue_MovesElement)
{
    Queue<std::unique_ptr<int>> queue;
    auto ptr = std::make_unique<int>(42);
    queue.push(std::move(ptr));
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(*queue.front(), 42);
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @brief Test emplace constructs element in-place
 * @details Verifies that emplace forwards arguments to T's constructor
 */
TEST_F(QueueTest, Emplace_ConstructsInPlace)
{
    Queue<std::pair<int, int>> queue;
    queue.emplace(1, 2);
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front().first, 1);
    EXPECT_EQ(queue.front().second, 2);

    queue.emplace(3, 4);
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front().first, 1);
    EXPECT_EQ(queue.back().first, 3);
}

/**
 * @brief Test clear removes all elements
 * @details Verifies clear empties the queue and resets state correctly
 */
TEST_F(QueueTest, Clear_EmptiesQueue)
{
    Queue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);
    EXPECT_EQ(queue.size(), 3);

    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_THROW(queue.front(), std::out_of_range);
    EXPECT_THROW(queue.back(), std::out_of_range);
    EXPECT_THROW(queue.pop(), std::out_of_range);
}

/**
 * @brief Test clear on already empty queue
 * @details Verifies clear is safe to call on an empty queue
 */
TEST_F(QueueTest, Clear_EmptyQueue_NoOp)
{
    Queue<int> queue;
    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test queue preserves insertion order
 * @details Verifies FIFO property is strictly maintained
 */
TEST_F(QueueTest, PreservesInsertionOrder)
{
    Queue<int> queue;

    const std::vector input = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    for (int val : input)
    {
        queue.push(val);
    }

    for (int val : input)
    {
        EXPECT_EQ(queue.front(), val);
        queue.pop();
    }

    EXPECT_TRUE(queue.empty());
}
