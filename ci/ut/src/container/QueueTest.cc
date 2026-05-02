/**
 * @file QueueTest.cc
 * @brief Unit tests for the Queue class
 * @details Tests cover core queue operations including push, pop, front, back access,
 *          size management, copy/move semantics, and edge cases.
 */

#include <gtest/gtest.h>
#include "container/Queue.hpp"
#include <string>
#include <vector>

using namespace common::container;

/**
 * @brief Test default constructor creates empty queue
 * @details Verifies that a newly constructed queue is empty with zero size
 */
TEST(QueueTest, DefaultConstructor_EmptyQueue) {
    Queue<int> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST(QueueTest, CopyConstructor_DeepCopy) {
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue<int> queue2(queue1);

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
TEST(QueueTest, CopyConstructor_EmptyQueue) {
    Queue<int> queue1;
    Queue<int> queue2(queue1);

    EXPECT_TRUE(queue2.empty());
    EXPECT_EQ(queue2.size(), 0);
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST(QueueTest, MoveConstructor_TransfersOwnership) {
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
    EXPECT_EQ(queue1.size(), 0);
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment performs deep copy correctly
 */
TEST(QueueTest, CopyAssignment_DeepCopy) {
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
TEST(QueueTest, CopyAssignment_SelfAssignment) {
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    queue = queue;

    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front(), 10);
    EXPECT_EQ(queue.back(), 20);
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership efficiently
 */
TEST(QueueTest, MoveAssignment_TransfersOwnership) {
    Queue<int> queue1;
    queue1.push(10);
    queue1.push(20);
    queue1.push(30);

    Queue<int> queue2;
    queue2 = std::move(queue1);

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
TEST(QueueTest, Push_MaintainsFIFOOrder) {
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
TEST(QueueTest, Push_SingleElement) {
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
TEST(QueueTest, Push_MultipleElements) {
    Queue<int> queue;

    for (int i = 1; i <= 100; ++i) {
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
TEST(QueueTest, Pop_RemovesFrontElement) {
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
TEST(QueueTest, Pop_EmptyQueue_ThrowsException) {
    Queue<int> queue;
    EXPECT_THROW(queue.pop(), std::out_of_range);
}

/**
 * @brief Test pop until queue becomes empty
 * @details Verifies that queue correctly transitions to empty state
 */
TEST(QueueTest, Pop_UntilEmpty) {
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
TEST(QueueTest, Front_ReturnsFirstElement) {
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
TEST(QueueTest, Front_EmptyQueue_ThrowsException) {
    Queue<int> queue;
    EXPECT_THROW(queue.front(), std::out_of_range);
}

/**
 * @brief Test const front accessor
 * @details Verifies const-correctness of front method
 */
TEST(QueueTest, Front_ConstAccessor) {
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    const Queue<int> &constQueue = queue;
    EXPECT_EQ(constQueue.front(), 10);
}

/**
 * @brief Test back returns reference to last element
 * @details Verifies const and non-const back accessors work correctly
 */
TEST(QueueTest, Back_ReturnsLastElement) {
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
TEST(QueueTest, Back_EmptyQueue_ThrowsException) {
    Queue<int> queue;
    EXPECT_THROW(queue.back(), std::out_of_range);
}

/**
 * @brief Test const back accessor
 * @details Verifies const-correctness of back method
 */
TEST(QueueTest, Back_ConstAccessor) {
    Queue<int> queue;
    queue.push(10);
    queue.push(20);

    const Queue<int> &constQueue = queue;
    EXPECT_EQ(constQueue.back(), 20);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies accurate empty state detection throughout queue lifecycle
 */
TEST(QueueTest, Empty_CorrectState) {
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
TEST(QueueTest, Size_CorrectCount) {
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
TEST(QueueTest, Swap_ExchangesContents) {
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
TEST(QueueTest, Swap_WithEmptyQueue) {
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
TEST(QueueTest, Swap_Symmetric) {
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
TEST(QueueTest, StringElements_CorrectBehavior) {
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
TEST(QueueTest, DuplicateValues_CorrectHandling) {
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
TEST(QueueTest, NegativeNumbers_CorrectHandling) {
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
TEST(QueueTest, MixedSignNumbers_CorrectHandling) {
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
TEST(QueueTest, LargeNumberOfElements_Correctness) {
    Queue<int> queue;

    // Add 1000 elements
    for (int i = 0; i < 1000; ++i) {
        queue.push(i);
    }

    EXPECT_EQ(queue.size(), 1000);
    EXPECT_EQ(queue.front(), 0);
    EXPECT_EQ(queue.back(), 999);

    // Remove half
    for (int i = 0; i < 500; ++i) {
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
TEST(QueueTest, AlternatingPushPop_Correctness) {
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
TEST(QueueTest, SingleElementCycle_Correctness) {
    Queue<int> queue;

    for (int i = 0; i < 10; ++i) {
        queue.push(i);
        EXPECT_EQ(queue.size(), 1);
        EXPECT_EQ(queue.front(), i);
        EXPECT_EQ(queue.back(), i);
        queue.pop();
        EXPECT_TRUE(queue.empty());
    }
}

/**
 * @brief Test queue preserves insertion order
 * @details Verifies FIFO property is strictly maintained
 */
TEST(QueueTest, PreservesInsertionOrder) {
    Queue<int> queue;

    std::vector<int> input = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    for (int val: input) {
        queue.push(val);
    }

    for (int val: input) {
        EXPECT_EQ(queue.front(), val);
        queue.pop();
    }

    EXPECT_TRUE(queue.empty());
}