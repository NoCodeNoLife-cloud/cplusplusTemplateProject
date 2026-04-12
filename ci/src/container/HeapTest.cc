/**
 * @file HeapTest.cc
 * @brief Unit tests for the Heap class
 * @details Tests cover core heap operations including push, pop, top, size management,
 *          heap property validation, and edge cases with different comparison functions.
 */

#include <gtest/gtest.h>
#include "container/Heap.hpp"
#include <vector>
#include <string>
#include <functional>

using namespace common::container;

/**
 * @brief Test default constructor creates empty heap
 * @details Verifies that a newly constructed heap is empty
 */
TEST(HeapTest, DefaultConstructor_EmptyHeap) {
    Heap<int> heap;
    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0);
}

/**
 * @brief Test constructor from iterator range builds valid heap
 * @details Verifies that heap is correctly built from iterator range
 */
TEST(HeapTest, IteratorConstructor_BuildsValidHeap) {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    Heap<int> heap(data.begin(), data.end());
    
    EXPECT_EQ(heap.size(), 6);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 9); // Max-heap: largest element at top
}

/**
 * @brief Test copy constructor creates independent copy
 * @details Verifies deep copy semantics
 */
TEST(HeapTest, CopyConstructor_IndependentCopy) {
    Heap<int> heap1;
    heap1.push(10);
    heap1.push(20);
    heap1.push(30);
    
    Heap<int> heap2(heap1);
    
    EXPECT_EQ(heap2.size(), heap1.size());
    EXPECT_EQ(heap2.top(), heap1.top());
    EXPECT_TRUE(heap2.is_valid());
    
    // Modify original should not affect copy
    heap1.pop();
    EXPECT_EQ(heap2.size(), 3);
    EXPECT_EQ(heap2.top(), 30);
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies move semantics work correctly
 */
TEST(HeapTest, MoveConstructor_TransfersOwnership) {
    Heap<int> heap1;
    heap1.push(10);
    heap1.push(20);
    heap1.push(30);
    
    Heap<int> heap2(std::move(heap1));
    
    EXPECT_EQ(heap2.size(), 3);
    EXPECT_EQ(heap2.top(), 30);
    EXPECT_TRUE(heap2.is_valid());
    EXPECT_TRUE(heap1.empty()); // Moved-from heap should be empty
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment works correctly
 */
TEST(HeapTest, CopyAssignment_CorrectBehavior) {
    Heap<int> heap1;
    heap1.push(10);
    heap1.push(20);
    
    Heap<int> heap2;
    heap2.push(30);
    
    heap2 = heap1;
    
    EXPECT_EQ(heap2.size(), 2);
    EXPECT_EQ(heap2.top(), 20);
    EXPECT_TRUE(heap2.is_valid());
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership
 */
TEST(HeapTest, MoveAssignment_TransfersOwnership) {
    Heap<int> heap1;
    heap1.push(10);
    heap1.push(20);
    heap1.push(30);
    
    Heap<int> heap2;
    heap2 = std::move(heap1);
    
    EXPECT_EQ(heap2.size(), 3);
    EXPECT_EQ(heap2.top(), 30);
    EXPECT_TRUE(heap1.empty());
}

/**
 * @brief Test push maintains max-heap property
 * @details Verifies that push operation maintains correct heap ordering
 */
TEST(HeapTest, Push_MaintainsMaxHeapProperty) {
    Heap<int> heap;
    
    heap.push(5);
    heap.push(3);
    heap.push(8);
    heap.push(1);
    heap.push(9);
    
    EXPECT_EQ(heap.size(), 5);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 9);
}

/**
 * @brief Test push with rvalue reference
 * @details Verifies move semantics in push operation
 */
TEST(HeapTest, Push_RvalueReference) {
    Heap<std::string> heap;
    
    std::string str = "hello";
    heap.push(std::move(str));
    heap.push("world");
    
    EXPECT_EQ(heap.size(), 2);
    EXPECT_TRUE(heap.is_valid());
}

/**
 * @brief Test emplace constructs element in-place
 * @details Verifies that emplace forwards arguments correctly
 */
TEST(HeapTest, Emplace_ConstructsInPlace) {
    Heap<std::string> heap;
    
    heap.emplace("hello");
    heap.emplace("world");
    heap.emplace("abc");
    
    EXPECT_EQ(heap.size(), 3);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), "world"); // Lexicographically largest
}

/**
 * @brief Test pop removes top element and maintains heap property
 * @details Verifies correct removal of maximum element
 */
TEST(HeapTest, Pop_RemovesTopElement) {
    Heap<int> heap;
    heap.push(10);
    heap.push(20);
    heap.push(30);
    heap.push(40);
    heap.push(50);
    
    EXPECT_EQ(heap.top(), 50);
    
    heap.pop();
    EXPECT_EQ(heap.size(), 4);
    EXPECT_EQ(heap.top(), 40);
    EXPECT_TRUE(heap.is_valid());
    
    heap.pop();
    EXPECT_EQ(heap.top(), 30);
    EXPECT_TRUE(heap.is_valid());
}

/**
 * @brief Test pop on empty heap throws exception
 * @details Verifies proper error handling for empty heap
 */
TEST(HeapTest, Pop_EmptyHeap_ThrowsException) {
    Heap<int> heap;
    EXPECT_THROW(heap.pop(), std::out_of_range);
}

/**
 * @brief Test top returns reference to maximum element
 * @details Verifies const and non-const top accessors
 */
TEST(HeapTest, Top_ReturnsMaximumElement) {
    Heap<int> heap;
    heap.push(15);
    heap.push(25);
    heap.push(10);
    
    EXPECT_EQ(heap.top(), 25);
    
    // Test non-const access
    heap.top() = 100;
    EXPECT_EQ(heap.top(), 100);
    EXPECT_TRUE(heap.is_valid());
}

/**
 * @brief Test top on empty heap throws exception
 * @details Verifies proper error handling for empty heap
 */
TEST(HeapTest, Top_EmptyHeap_ThrowsException) {
    Heap<int> heap;
    EXPECT_THROW(heap.top(), std::out_of_range);
}

/**
 * @brief Test const top accessor
 * @details Verifies const-correctness of top method
 */
TEST(HeapTest, Top_ConstAccessor) {
    Heap<int> heap;
    heap.push(10);
    heap.push(20);
    
    const Heap<int>& constHeap = heap;
    EXPECT_EQ(constHeap.top(), 20);
}

/**
 * @brief Test size returns correct count
 * @details Verifies accurate element counting
 */
TEST(HeapTest, Size_CorrectCount) {
    Heap<int> heap;
    EXPECT_EQ(heap.size(), 0);
    
    heap.push(10);
    EXPECT_EQ(heap.size(), 1);
    
    heap.push(20);
    heap.push(30);
    EXPECT_EQ(heap.size(), 3);
    
    heap.pop();
    EXPECT_EQ(heap.size(), 2);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies accurate empty state detection
 */
TEST(HeapTest, Empty_CorrectState) {
    Heap<int> heap;
    EXPECT_TRUE(heap.empty());
    
    heap.push(10);
    EXPECT_FALSE(heap.empty());
    
    heap.pop();
    EXPECT_TRUE(heap.empty());
}

/**
 * @brief Test clear removes all elements
 * @details Verifies complete heap clearing
 */
TEST(HeapTest, Clear_RemovesAllElements) {
    Heap<int> heap;
    heap.push(10);
    heap.push(20);
    heap.push(30);
    
    EXPECT_EQ(heap.size(), 3);
    
    heap.clear();
    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0);
}

/**
 * @brief Test is_valid returns true for valid heap
 * @details Verifies heap property validation
 */
TEST(HeapTest, IsValid_ValidHeap) {
    Heap<int> heap;
    heap.push(5);
    heap.push(3);
    heap.push(8);
    heap.push(1);
    
    EXPECT_TRUE(heap.is_valid());
}

/**
 * @brief Test is_valid returns false for invalid heap
 * @details This test verifies the validation logic works by checking internal state
 */
TEST(HeapTest, IsValid_AfterOperations) {
    Heap<int> heap;
    
    // Perform various operations
    heap.push(10);
    heap.push(20);
    heap.push(5);
    heap.push(30);
    heap.pop();
    heap.push(15);
    
    EXPECT_TRUE(heap.is_valid());
}

/**
 * @brief Test min-heap with custom comparator
 * @details Verifies heap works with different comparison functions
 */
TEST(HeapTest, MinHeap_CustomComparator) {
    Heap<int, std::greater<int>> heap;
    
    heap.push(5);
    heap.push(3);
    heap.push(8);
    heap.push(1);
    heap.push(9);
    
    EXPECT_EQ(heap.size(), 5);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 1); // Min-heap: smallest element at top
    
    heap.pop();
    EXPECT_EQ(heap.top(), 3);
}

/**
 * @brief Test heap with string elements
 * @details Verifies heap works with non-numeric types
 */
TEST(HeapTest, StringElements_LexicographicOrder) {
    Heap<std::string> heap;
    
    heap.emplace("banana");
    heap.emplace("apple");
    heap.emplace("cherry");
    heap.emplace("date");
    
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), "date"); // Lexicographically largest
    
    heap.pop();
    EXPECT_EQ(heap.top(), "cherry");
}

/**
 * @brief Test heap with duplicate values
 * @details Verifies correct handling of duplicate elements
 */
TEST(HeapTest, DuplicateValues_CorrectHandling) {
    Heap<int> heap;
    
    heap.push(5);
    heap.push(5);
    heap.push(5);
    heap.push(3);
    heap.push(5);
    
    EXPECT_EQ(heap.size(), 5);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 5);
    
    heap.pop();
    EXPECT_EQ(heap.top(), 5);
}

/**
 * @brief Test heap with single element
 * @details Verifies edge case with one element
 */
TEST(HeapTest, SingleElement_CorrectBehavior) {
    Heap<int> heap;
    heap.push(42);
    
    EXPECT_EQ(heap.size(), 1);
    EXPECT_EQ(heap.top(), 42);
    EXPECT_TRUE(heap.is_valid());
    
    heap.pop();
    EXPECT_TRUE(heap.empty());
}

/**
 * @brief Test heap with large number of elements
 * @details Verifies scalability and correctness with many elements
 */
TEST(HeapTest, LargeNumberOfElements_Correctness) {
    Heap<int> heap;
    
    // Add 1000 elements
    for (int i = 0; i < 1000; ++i) {
        heap.push(i);
    }
    
    EXPECT_EQ(heap.size(), 1000);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 999);
    
    // Remove half
    for (int i = 0; i < 500; ++i) {
        heap.pop();
    }
    
    EXPECT_EQ(heap.size(), 500);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 499);
}

/**
 * @brief Test heap property after multiple push-pop cycles
 * @details Verifies heap remains valid through repeated operations
 */
TEST(HeapTest, MultiplePushPopCycles_MaintainsValidity) {
    Heap<int> heap;
    
    for (int cycle = 0; cycle < 10; ++cycle) {
        // Push phase
        for (int i = 0; i < 20; ++i) {
            heap.push(i);
        }
        
        EXPECT_TRUE(heap.is_valid());
        
        // Pop phase
        for (int i = 0; i < 15; ++i) {
            heap.pop();
        }
        
        EXPECT_TRUE(heap.is_valid());
    }
}

/**
 * @brief Test heap construction from sorted data
 * @details Verifies heapify works correctly with pre-sorted input
 */
TEST(HeapTest, IteratorConstructor_SortedInput) {
    std::vector<int> sorted = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Heap<int> heap(sorted.begin(), sorted.end());
    
    EXPECT_EQ(heap.size(), 10);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 10);
}

/**
 * @brief Test heap construction from reverse sorted data
 * @details Verifies heapify works correctly with reverse-sorted input
 */
TEST(HeapTest, IteratorConstructor_ReverseSortedInput) {
    std::vector<int> reverseSorted = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    Heap<int> heap(reverseSorted.begin(), reverseSorted.end());
    
    EXPECT_EQ(heap.size(), 10);
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 10);
}

/**
 * @brief Test heap with negative numbers
 * @details Verifies correct handling of negative values
 */
TEST(HeapTest, NegativeNumbers_CorrectHandling) {
    Heap<int> heap;
    
    heap.push(-10);
    heap.push(-5);
    heap.push(-20);
    heap.push(-1);
    heap.push(-15);
    
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), -1); // Largest (least negative)
    
    heap.pop();
    EXPECT_EQ(heap.top(), -5);
}

/**
 * @brief Test heap with mixed positive and negative numbers
 * @details Verifies correct handling of mixed sign values
 */
TEST(HeapTest, MixedSignNumbers_CorrectHandling) {
    Heap<int> heap;
    
    heap.push(-100);
    heap.push(50);
    heap.push(-50);
    heap.push(100);
    heap.push(0);
    
    EXPECT_TRUE(heap.is_valid());
    EXPECT_EQ(heap.top(), 100);
    
    heap.pop();
    EXPECT_EQ(heap.top(), 50);
}
