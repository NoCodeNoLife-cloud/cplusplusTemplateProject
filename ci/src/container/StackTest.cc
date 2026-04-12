/**
 * @file StackTest.cc
 * @brief Unit tests for the Stack class
 * @details Tests cover core stack operations including push, pop, top access,
 *          emplace, size management, and edge cases with different data types.
 */

#include <gtest/gtest.h>
#include "container/Stack.hpp"
#include <string>
#include <vector>

using namespace common::container;

/**
 * @brief Test default constructor creates empty stack
 * @details Verifies that a newly constructed stack is empty with zero size
 */
TEST(StackTest, DefaultConstructor_EmptyStack) {
    Stack<int> stack;
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test constructor from iterator range builds stack correctly
 * @details Verifies that stack is correctly initialized from iterator range
 */
TEST(StackTest, IteratorConstructor_InitializesCorrectly) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    Stack<int> stack(data.begin(), data.end());

    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.top(), 5); // Last element should be on top
}

/**
 * @brief Test constructor from empty iterator range
 * @details Verifies behavior when constructing from empty range
 */
TEST(StackTest, IteratorConstructor_EmptyRange) {
    std::vector<int> data;
    Stack<int> stack(data.begin(), data.end());

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test push with lvalue reference
 * @details Verifies that push correctly adds copy of value to stack
 */
TEST(StackTest, Push_LvalueReference) {
    Stack<int> stack;
    int value = 42;

    stack.push(value);

    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), 42);
}

/**
 * @brief Test push with rvalue reference
 * @details Verifies that push correctly moves value to stack
 */
TEST(StackTest, Push_RvalueReference) {
    Stack<std::string> stack;

    stack.push(std::string("hello"));
    stack.push("world");

    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), "world");
}

/**
 * @brief Test multiple push operations maintain LIFO order
 * @details Verifies that last-in-first-out ordering is maintained
 */
TEST(StackTest, Push_MaintainsLIFOOrder) {
    Stack<int> stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), 30);
}

/**
 * @brief Test emplace constructs element in-place
 * @details Verifies that emplace forwards arguments correctly to constructor
 */
TEST(StackTest, Emplace_ConstructsInPlace) {
    Stack<std::string> stack;

    stack.emplace("hello");
    stack.emplace("world");
    stack.emplace("test");

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), "test");
}

/**
 * @brief Test emplace with complex object construction
 * @details Verifies emplace works with multi-argument constructors
 */
TEST(StackTest, Emplace_ComplexConstruction) {
    Stack<std::string> stack;

    stack.emplace(5, 'a'); // Constructs string "aaaaa"

    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), "aaaaa");
}

/**
 * @brief Test pop removes top element
 * @details Verifies that pop correctly removes the top element and updates stack
 */
TEST(StackTest, Pop_RemovesTopElement) {
    Stack<int> stack;
    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.top(), 30);

    stack.pop();
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), 20);

    stack.pop();
    EXPECT_EQ(stack.top(), 10);
}

/**
 * @brief Test pop on empty stack throws exception
 * @details Verifies proper error handling when popping from empty stack
 */
TEST(StackTest, Pop_EmptyStack_ThrowsException) {
    Stack<int> stack;
    EXPECT_THROW(stack.pop(), std::out_of_range);
}

/**
 * @brief Test pop until stack becomes empty
 * @details Verifies that stack correctly transitions to empty state
 */
TEST(StackTest, Pop_UntilEmpty) {
    Stack<int> stack;
    stack.push(10);
    stack.push(20);

    stack.pop();
    stack.pop();

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test top returns reference to top element
 * @details Verifies const and non-const top accessors work correctly
 */
TEST(StackTest, Top_ReturnsTopElement) {
    Stack<int> stack;
    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.top(), 30);

    // Test non-const access allows modification
    stack.top() = 300;
    EXPECT_EQ(stack.top(), 300);
}

/**
 * @brief Test top on empty stack throws exception
 * @details Verifies proper error handling for empty stack
 */
TEST(StackTest, Top_EmptyStack_ThrowsException) {
    Stack<int> stack;
    EXPECT_THROW(stack.top(), std::out_of_range);
}

/**
 * @brief Test const top accessor
 * @details Verifies const-correctness of top method
 */
TEST(StackTest, Top_ConstAccessor) {
    Stack<int> stack;
    stack.push(10);
    stack.push(20);

    const Stack<int> &constStack = stack;
    EXPECT_EQ(constStack.top(), 20);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies accurate empty state detection throughout stack lifecycle
 */
TEST(StackTest, Empty_CorrectState) {
    Stack<int> stack;
    EXPECT_TRUE(stack.empty());

    stack.push(10);
    EXPECT_FALSE(stack.empty());

    stack.pop();
    EXPECT_TRUE(stack.empty());
}

/**
 * @brief Test size returns correct count
 * @details Verifies accurate element counting after various operations
 */
TEST(StackTest, Size_CorrectCount) {
    Stack<int> stack;
    EXPECT_EQ(stack.size(), 0);

    stack.push(10);
    EXPECT_EQ(stack.size(), 1);

    stack.push(20);
    stack.push(30);
    EXPECT_EQ(stack.size(), 3);

    stack.pop();
    EXPECT_EQ(stack.size(), 2);

    stack.pop();
    stack.pop();
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test stack with string elements
 * @details Verifies stack works correctly with non-numeric types
 */
TEST(StackTest, StringElements_CorrectBehavior) {
    Stack<std::string> stack;

    stack.emplace("hello");
    stack.emplace("world");
    stack.emplace("test");

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), "test");

    stack.pop();
    EXPECT_EQ(stack.top(), "world");
}

/**
 * @brief Test stack with duplicate values
 * @details Verifies correct handling of duplicate elements
 */
TEST(StackTest, DuplicateValues_CorrectHandling) {
    Stack<int> stack;

    stack.push(5);
    stack.push(5);
    stack.push(5);
    stack.push(10);
    stack.push(5);

    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.top(), 5);

    stack.pop();
    EXPECT_EQ(stack.top(), 10);
}

/**
 * @brief Test stack with negative numbers
 * @details Verifies correct handling of negative values
 */
TEST(StackTest, NegativeNumbers_CorrectHandling) {
    Stack<int> stack;

    stack.push(-10);
    stack.push(-5);
    stack.push(-20);
    stack.push(-1);

    EXPECT_EQ(stack.size(), 4);
    EXPECT_EQ(stack.top(), -1);

    stack.pop();
    EXPECT_EQ(stack.top(), -20);
}

/**
 * @brief Test stack with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign values
 */
TEST(StackTest, MixedSignNumbers_CorrectHandling) {
    Stack<int> stack;

    stack.push(-100);
    stack.push(50);
    stack.push(-50);
    stack.push(100);
    stack.push(0);

    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.top(), 0);
}

/**
 * @brief Test large number of elements
 * @details Verifies scalability and correctness with many elements
 */
TEST(StackTest, LargeNumberOfElements_Correctness) {
    Stack<int> stack;

    // Add 1000 elements
    for (int i = 0; i < 1000; ++i) {
        stack.push(i);
    }

    EXPECT_EQ(stack.size(), 1000);
    EXPECT_EQ(stack.top(), 999);

    // Remove half
    for (int i = 0; i < 500; ++i) {
        stack.pop();
    }

    EXPECT_EQ(stack.size(), 500);
    EXPECT_EQ(stack.top(), 499);
}

/**
 * @brief Test alternating push and pop operations
 * @details Verifies stack maintains correctness through mixed operations
 */
TEST(StackTest, AlternatingPushPop_Correctness) {
    Stack<int> stack;

    stack.push(1);
    stack.push(2);
    stack.pop();
    stack.push(3);
    stack.push(4);
    stack.pop();
    stack.pop();

    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), 1);
}

/**
 * @brief Test single element push and pop cycle
 * @details Verifies edge case with one element being added and removed repeatedly
 */
TEST(StackTest, SingleElementCycle_Correctness) {
    Stack<int> stack;

    for (int i = 0; i < 10; ++i) {
        stack.push(i);
        EXPECT_EQ(stack.size(), 1);
        EXPECT_EQ(stack.top(), i);
        stack.pop();
        EXPECT_TRUE(stack.empty());
    }
}

/**
 * @brief Test stack preserves LIFO property
 * @details Verifies last-in-first-out property is strictly maintained
 */
TEST(StackTest, PreservesLIFOProperty) {
    Stack<int> stack;

    std::vector<int> input = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    for (int val: input) {
        stack.push(val);
    }

    // Should come out in reverse order
    for (auto it = input.rbegin(); it != input.rend(); ++it) {
        EXPECT_EQ(stack.top(), *it);
        stack.pop();
    }

    EXPECT_TRUE(stack.empty());
}

/**
 * @brief Test stack with custom container type
 * @details Verifies stack works with different underlying container types
 */
TEST(StackTest, CustomContainer_CorrectBehavior) {
    Stack<int, std::vector<int> > stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), 30);
}