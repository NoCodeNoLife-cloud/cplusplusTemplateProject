/**
 * @file StackTest.cc
 * @brief Unit tests for the Stack class
 * @details Tests cover core stack operations including push, pop, top access,
 *          emplace, size management, and edge cases with different data types.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/stack/Stack.hpp"

using namespace common::data_structure;

/**
 * @brief Test fixture for StackTest tests
 */
class StackTest : public testing::Test
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
 * @brief Test default constructor creates empty stack
 * @details Verifies that a newly constructed stack is empty with zero size
 */
TEST_F(StackTest, DefaultConstructor_EmptyStack)
{
    const Stack<int> stack;
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test constructor from iterator range builds stack correctly
 * @details Verifies that stack is correctly initialized from iterator range
 */
TEST_F(StackTest, IteratorConstructor_InitializesCorrectly)
{
    std::vector data = {1, 2, 3, 4, 5};
    Stack<int> stack(data.begin(), data.end());

    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.top(), 5); // Last element should be on top
}

/**
 * @brief Test constructor from empty iterator range
 * @details Verifies behavior when constructing from empty range
 */
TEST_F(StackTest, IteratorConstructor_EmptyRange)
{
    std::vector<int> data;
    const Stack<int> stack(data.begin(), data.end());

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief Test push with lvalue reference
 * @details Verifies that push correctly adds copy of value to stack
 */
TEST_F(StackTest, Push_LvalueReference)
{
    Stack<int> stack;
    const int value = 42;

    stack.push(value);

    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), 42);
}

/**
 * @brief Test push with rvalue reference
 * @details Verifies that push correctly moves value to stack
 */
TEST_F(StackTest, Push_RvalueReference)
{
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
TEST_F(StackTest, Push_MaintainsLIFOOrder)
{
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
TEST_F(StackTest, Emplace_ConstructsInPlace)
{
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
TEST_F(StackTest, Emplace_ComplexConstruction)
{
    Stack<std::string> stack;

    stack.emplace(5, 'a'); // Constructs string "aaaaa"

    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), "aaaaa");
}

/**
 * @brief Test pop removes top element
 * @details Verifies that pop correctly removes the top element and updates stack
 */
TEST_F(StackTest, Pop_RemovesTopElement)
{
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
TEST_F(StackTest, Pop_EmptyStack_ThrowsException)
{
    Stack<int> stack;
    EXPECT_THROW(stack.pop(), std::out_of_range);
}

/**
 * @brief Test pop until stack becomes empty
 * @details Verifies that stack correctly transitions to empty state
 */
TEST_F(StackTest, Pop_UntilEmpty)
{
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
TEST_F(StackTest, Top_ReturnsTopElement)
{
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
TEST_F(StackTest, Top_EmptyStack_ThrowsException)
{
    Stack<int> stack;
    EXPECT_THROW(stack.top(), std::out_of_range);
}

/**
 * @brief Test const top accessor
 * @details Verifies const-correctness of top method
 */
TEST_F(StackTest, Top_ConstAccessor)
{
    Stack<int> stack;
    stack.push(10);
    stack.push(20);

    const Stack<int>& constStack = stack;
    EXPECT_EQ(constStack.top(), 20);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies accurate empty state detection throughout stack lifecycle
 */
TEST_F(StackTest, Empty_CorrectState)
{
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
TEST_F(StackTest, Size_CorrectCount)
{
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
TEST_F(StackTest, StringElements_CorrectBehavior)
{
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
TEST_F(StackTest, DuplicateValues_CorrectHandling)
{
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
TEST_F(StackTest, NegativeNumbers_CorrectHandling)
{
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
TEST_F(StackTest, MixedSignNumbers_CorrectHandling)
{
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
TEST_F(StackTest, LargeNumberOfElements_Correctness)
{
    Stack<int> stack;

    // Add 1000 elements
    for (int i = 0; i < 1000; ++i)
    {
        stack.push(i);
    }

    EXPECT_EQ(stack.size(), 1000);
    EXPECT_EQ(stack.top(), 999);

    // Remove half
    for (int i = 0; i < 500; ++i)
    {
        stack.pop();
    }

    EXPECT_EQ(stack.size(), 500);
    EXPECT_EQ(stack.top(), 499);
}

/**
 * @brief Test alternating push and pop operations
 * @details Verifies stack maintains correctness through mixed operations
 */
TEST_F(StackTest, AlternatingPushPop_Correctness)
{
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
TEST_F(StackTest, SingleElementCycle_Correctness)
{
    Stack<int> stack;

    for (int i = 0; i < 10; ++i)
    {
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
TEST_F(StackTest, PreservesLIFOProperty)
{
    Stack<int> stack;

    std::vector input = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    for (int val : input)
    {
        stack.push(val);
    }

    // Should come out in reverse order
    for (auto it = input.rbegin(); it != input.rend(); ++it)
    {
        EXPECT_EQ(stack.top(), *it);
        stack.pop();
    }

    EXPECT_TRUE(stack.empty());
}

/**
 * @brief Test stack with custom container type
 * @details Verifies stack works with different underlying container types
 */
TEST_F(StackTest, CustomContainer_CorrectBehavior)
{
    Stack<int, std::vector<int>> stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), 30);
}

/**
 * @brief Test swap exchanges contents of two stacks
 * @details Verifies that swap efficiently exchanges all elements
 */
TEST_F(StackTest, Swap_ExchangesContents)
{
    Stack<int> stack1;
    stack1.push(10);
    stack1.push(20);
    stack1.push(30);

    Stack<int> stack2;
    stack2.push(100);
    stack2.push(200);

    stack1.swap(stack2);

    EXPECT_EQ(stack1.size(), 2);
    EXPECT_EQ(stack1.top(), 200);

    EXPECT_EQ(stack2.size(), 3);
    EXPECT_EQ(stack2.top(), 30);
}

/**
 * @brief Test swap with empty stack
 * @details Verifies swapping with an empty stack works correctly
 */
TEST_F(StackTest, Swap_WithEmptyStack)
{
    Stack<int> stack1;
    stack1.push(10);
    stack1.push(20);

    Stack<int> stack2;

    stack1.swap(stack2);

    EXPECT_TRUE(stack1.empty());
    EXPECT_EQ(stack2.size(), 2);
    EXPECT_EQ(stack2.top(), 20);
}

/**
 * @brief Test stack memory efficiency with move semantics
 * @details Verifies that move operations are efficient
 */
TEST_F(StackTest, MoveSemantics_Efficiency)
{
    Stack<std::string> stack1;
    for (int i = 0; i < 100; ++i)
    {
        stack1.push("test_string_" + std::to_string(i));
    }

    // Move should be efficient
    Stack stack2(std::move(stack1));

    EXPECT_EQ(stack2.size(), 100);
    EXPECT_TRUE(stack1.empty());
}

/**
 * @brief Test stack with zero values
 * @details Verifies correct handling of zero elements
 */
TEST_F(StackTest, ZeroValues_CorrectHandling)
{
    Stack<int> stack;

    stack.push(0);
    stack.push(0);
    stack.push(0);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), 0);

    stack.pop();
    EXPECT_EQ(stack.top(), 0);
}
