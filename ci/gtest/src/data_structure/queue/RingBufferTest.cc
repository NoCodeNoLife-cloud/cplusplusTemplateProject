/**
 * @file RingBufferTest.cc
 * @brief Unit tests for the RingBuffer class
 * @details Tests cover core ring buffer operations including push, pop, front, back,
 *          overwrite behavior, resize, copy/move semantics, and edge cases.
 */

#include <string>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

#include <cppforge/data_structure/queue/RingBuffer.hpp>

using namespace cppforge::data_structure;

// Compile-time noexcept guarantees
static_assert(std::is_nothrow_move_constructible_v<RingBuffer<int>>);
static_assert(std::is_nothrow_move_assignable_v<RingBuffer<int>>);
static_assert(noexcept(std::declval<RingBuffer<int>&>().swap(std::declval<RingBuffer<int>&>())));
static_assert(noexcept(std::declval<RingBuffer<int>&>().clear()));
static_assert(noexcept(std::declval<const RingBuffer<int>&>().empty()));
static_assert(noexcept(std::declval<const RingBuffer<int>&>().size()));
static_assert(noexcept(std::declval<const RingBuffer<int>&>().capacity()));
static_assert(noexcept(std::declval<const RingBuffer<int>&>().full()));

/**
 * @brief Test fixture for RingBuffer tests
 */
class RingBufferTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Constructor Tests ====================

/**
 * @brief Test constructor with valid capacity
 * @details Verifies that a ring buffer is constructed with correct capacity
 */
TEST_F(RingBufferTest, Constructor_ValidCapacity)
{
    const RingBuffer<int> buf(8);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0);
    EXPECT_EQ(buf.capacity(), 8);
    EXPECT_FALSE(buf.full());
}

/**
 * @brief Test constructor with capacity 1
 * @details Verifies minimum capacity of 1 works correctly
 */
TEST_F(RingBufferTest, Constructor_CapacityOne)
{
    const RingBuffer<int> buf(1);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.capacity(), 1);
}

/**
 * @brief Test constructor with capacity 0 throws
 * @details Verifies constructing with zero capacity throws invalid_argument
 */
TEST_F(RingBufferTest, Constructor_ZeroCapacity_ThrowsException)
{
    EXPECT_THROW(RingBuffer<int>(0), std::invalid_argument);
}

/**
 * @brief Test copy constructor creates deep copy
 * @details Verifies that copy constructor performs deep copy of all elements
 */
TEST_F(RingBufferTest, CopyConstructor_DeepCopy)
{
    RingBuffer<int> buf1(8);
    buf1.push_back(10);
    buf1.push_back(20);
    buf1.push_back(30);

    RingBuffer buf2(buf1);

    EXPECT_EQ(buf2.size(), buf1.size());
    EXPECT_EQ(buf2.capacity(), buf1.capacity());
    EXPECT_EQ(buf2.front(), buf1.front());
    EXPECT_EQ(buf2.back(), buf1.back());

    // Modify original should not affect copy
    buf1.pop_front();
    EXPECT_EQ(buf2.size(), 3);
    EXPECT_EQ(buf2.front(), 10);
}

/**
 * @brief Test copy constructor with empty buffer
 * @details Verifies copying an empty buffer works correctly
 */
TEST_F(RingBufferTest, CopyConstructor_EmptyBuffer)
{
    RingBuffer<int> buf1(4);
    const RingBuffer buf2(buf1);

    EXPECT_TRUE(buf2.empty());
    EXPECT_EQ(buf2.capacity(), 4);

    // Modify original should not affect copy
    buf1.push_back(99);
    EXPECT_TRUE(buf2.empty());
}

/**
 * @brief Test move constructor transfers ownership
 * @details Verifies that move constructor transfers all elements efficiently
 */
TEST_F(RingBufferTest, MoveConstructor_TransfersOwnership)
{
    RingBuffer<int> buf1(8);
    buf1.push_back(10);
    buf1.push_back(20);

    RingBuffer buf2(std::move(buf1));

    EXPECT_EQ(buf2.size(), 2);
    EXPECT_EQ(buf2.capacity(), 8);
    EXPECT_EQ(buf2.front(), 10);
    EXPECT_EQ(buf2.back(), 20);

    // Moved-from buffer should be empty
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(buf1.empty());
}

/**
 * @brief Test copy assignment operator
 * @details Verifies copy assignment performs deep copy correctly
 */
TEST_F(RingBufferTest, CopyAssignment_DeepCopy)
{
    RingBuffer<int> buf1(8);
    buf1.push_back(10);
    buf1.push_back(20);

    RingBuffer<int> buf2(4);
    buf2.push_back(99);
    buf2 = buf1;

    EXPECT_EQ(buf2.size(), 2);
    EXPECT_EQ(buf2.capacity(), 8);
    EXPECT_EQ(buf2.front(), 10);
    EXPECT_EQ(buf2.back(), 20);
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment transfers ownership correctly
 */
TEST_F(RingBufferTest, MoveAssignment_TransfersOwnership)
{
    RingBuffer<int> buf1(8);
    buf1.push_back(10);
    buf1.push_back(20);

    RingBuffer<int> buf2(4);
    buf2.push_back(99);
    buf2 = std::move(buf1);

    EXPECT_EQ(buf2.size(), 2);
    EXPECT_EQ(buf2.capacity(), 8);
    EXPECT_EQ(buf2.front(), 10);
    EXPECT_EQ(buf2.back(), 20);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(buf1.empty());
}

// ==================== Element Access Tests ====================

/**
 * @brief Test front access
 * @details Verifies front returns reference to first element
 */
TEST_F(RingBufferTest, Front_ReturnsFirstElement)
{
    RingBuffer<int> buf(8);
    buf.push_back(10);
    buf.push_back(20);
    EXPECT_EQ(buf.front(), 10);

    buf.front() = 99;
    EXPECT_EQ(buf.front(), 99);
}

/**
 * @brief Test back access
 * @details Verifies back returns reference to last element
 */
TEST_F(RingBufferTest, Back_ReturnsLastElement)
{
    RingBuffer<int> buf(8);
    buf.push_back(10);
    buf.push_back(20);
    EXPECT_EQ(buf.back(), 20);

    buf.back() = 99;
    EXPECT_EQ(buf.back(), 99);
}

/**
 * @brief Test at access by index
 * @details Verifies at returns correct element by index
 */
TEST_F(RingBufferTest, At_ReturnsElementByIndex)
{
    RingBuffer<int> buf(8);
    buf.push_back(10);
    buf.push_back(20);
    buf.push_back(30);

    EXPECT_EQ(buf.at(0), 10);
    EXPECT_EQ(buf.at(1), 20);
    EXPECT_EQ(buf.at(2), 30);

    buf.at(1) = 99;
    EXPECT_EQ(buf.at(1), 99);
}

/**
 * @brief Test at throws for out-of-bounds index
 * @details Verifies at throws out_of_range for invalid index
 */
TEST_F(RingBufferTest, At_OutOfBounds_ThrowsException)
{
    RingBuffer<int> buf(8);
    buf.push_back(10);
    EXPECT_THROW(buf.at(5u), std::out_of_range);
}

/**
 * @brief Test operator[] access
 * @details Verifies operator[] returns element without bounds checking
 */
TEST_F(RingBufferTest, Subscript_AccessByIndex)
{
    RingBuffer<int> buf(8);
    buf.push_back(10);
    buf.push_back(20);

    EXPECT_EQ(buf[0], 10);
    EXPECT_EQ(buf[1], 20);

    buf[0] = 99;
    EXPECT_EQ(buf[0], 99);
}

// ==================== Push & Pop Tests ====================

/**
 * @brief Test push_back adds elements
 * @details Verifies push_back adds elements correctly
 */
TEST_F(RingBufferTest, PushBack_AddsElements)
{
    RingBuffer<int> buf(8);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 3);
}

/**
 * @brief Test push_front adds elements
 * @details Verifies push_front adds elements correctly
 */
TEST_F(RingBufferTest, PushFront_AddsElements)
{
    RingBuffer<int> buf(8);
    buf.push_front(1);
    buf.push_front(2);
    buf.push_front(3);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 3);
    EXPECT_EQ(buf.back(), 1);
}

/**
 * @brief Test pop_front removes first element
 * @details Verifies pop_front correctly removes the first element
 */
TEST_F(RingBufferTest, PopFront_RemovesFirstElement)
{
    RingBuffer<int> buf(8);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.pop_front();
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), 2);
    EXPECT_EQ(buf.back(), 3);

    buf.pop_front();
    buf.pop_front();
    EXPECT_TRUE(buf.empty());
}

/**
 * @brief Test pop_back removes last element
 * @details Verifies pop_back correctly removes the last element
 */
TEST_F(RingBufferTest, PopBack_RemovesLastElement)
{
    RingBuffer<int> buf(8);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.pop_back();
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 2);

    buf.pop_back();
    buf.pop_back();
    EXPECT_TRUE(buf.empty());
}

// ==================== Overwrite Behavior Tests ====================

/**
 * @brief Test push_back overwrites when full
 * @details Verifies push_back overwrites the oldest element when buffer is full
 */
TEST_F(RingBufferTest, PushBack_OverwritesWhenFull)
{
    RingBuffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 3);

    // This should overwrite the oldest (1)
    buf.push_back(4);
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 2);
    EXPECT_EQ(buf.back(), 4);

    buf.push_back(5);
    EXPECT_EQ(buf.front(), 3);
    EXPECT_EQ(buf.back(), 5);
}

/**
 * @brief Test push_front overwrites when full
 * @details Verifies push_front overwrites the oldest element when buffer is full
 */
TEST_F(RingBufferTest, PushFront_OverwritesWhenFull)
{
    RingBuffer<int> buf(3);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_TRUE(buf.full());

    // This should overwrite the oldest (3 at back)
    buf.push_front(4);
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 4);
    EXPECT_EQ(buf.back(), 2);

    buf.push_front(5);
    EXPECT_EQ(buf.front(), 5);
    EXPECT_EQ(buf.back(), 1);
}

/**
 * @brief Test push_front and push_back interleaved overwrite
 * @details Verifies interleaved push operations maintain correct order during overwrite
 */
TEST_F(RingBufferTest, PushBoth_InterleavedOverwrite)
{
    RingBuffer<int> buf(4);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.push_back(4);
    EXPECT_TRUE(buf.full());

    // Add from both ends
    buf.push_back(5);   // overwrites 1
    EXPECT_EQ(buf.front(), 2);
    EXPECT_EQ(buf.back(), 5);

    buf.push_front(6);  // overwrites 5
    EXPECT_EQ(buf.front(), 6);
    EXPECT_EQ(buf.back(), 4);
}

// ==================== Clear & Swap Tests ====================

/**
 * @brief Test clear empties the buffer
 * @details Verifies that clear removes all elements
 */
TEST_F(RingBufferTest, Clear_EmptiesBuffer)
{
    RingBuffer<int> buf(8);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0);
    EXPECT_EQ(buf.capacity(), 8);
}

/**
 * @brief Test clear on empty buffer is no-op
 * @details Verifies clearing an already empty buffer does nothing
 */
TEST_F(RingBufferTest, Clear_EmptyBuffer_NoOp)
{
    RingBuffer<int> buf(4);
    buf.clear();
    EXPECT_TRUE(buf.empty());
}

/**
 * @brief Test clear then reuse
 * @details Verifies cleared buffer can be reused
 */
TEST_F(RingBufferTest, Clear_ThenReuse)
{
    RingBuffer<int> buf(4);
    buf.push_back(1);
    buf.push_back(2);
    buf.clear();

    buf.push_back(10);
    buf.push_back(20);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), 10);
    EXPECT_EQ(buf.back(), 20);
}

/**
 * @brief Test swap exchanges contents
 * @details Verifies swap correctly exchanges contents of two ring buffers
 */
TEST_F(RingBufferTest, Swap_ExchangesContents)
{
    RingBuffer<int> buf1(8);
    buf1.push_back(1);
    buf1.push_back(2);

    RingBuffer<int> buf2(4);
    buf2.push_back(10);

    buf1.swap(buf2);

    EXPECT_EQ(buf1.size(), 1);
    EXPECT_EQ(buf1.capacity(), 4);
    EXPECT_EQ(buf1.front(), 10);

    EXPECT_EQ(buf2.size(), 2);
    EXPECT_EQ(buf2.capacity(), 8);
    EXPECT_EQ(buf2.front(), 1);
}

// ==================== Resize Tests ====================

/**
 * @brief Test resize to larger capacity
 * @details Verifies resizing to larger capacity preserves elements
 */
TEST_F(RingBufferTest, Resize_LargerCapacity)
{
    RingBuffer<int> buf(4);
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.resize(8);
    EXPECT_EQ(buf.capacity(), 8);
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 3);
}

/**
 * @brief Test resize to smaller capacity
 * @details Verifies resizing to smaller capacity drops oldest elements
 */
TEST_F(RingBufferTest, Resize_SmallerCapacity)
{
    RingBuffer<int> buf(8);
    for (int i = 1; i <= 6; ++i)
    {
        buf.push_back(i);
    }

    buf.resize(4);
    EXPECT_EQ(buf.capacity(), 4);
    EXPECT_EQ(buf.size(), 4);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 4);
}

/**
 * @brief Test resize to smaller than current size
 * @details Verifies resizing smaller than element count drops oldest
 */
TEST_F(RingBufferTest, Resize_SmallerThanSize)
{
    RingBuffer<int> buf(8);
    for (int i = 1; i <= 6; ++i)
    {
        buf.push_back(i);
    }

    buf.resize(3);
    EXPECT_EQ(buf.capacity(), 3);
    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.front(), 1);
    EXPECT_EQ(buf.back(), 3);
}

/**
 * @brief Test resize with empty buffer
 * @details Verifies resizing an empty buffer works correctly
 */
TEST_F(RingBufferTest, Resize_EmptyBuffer)
{
    RingBuffer<int> buf(4);
    buf.resize(10);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.capacity(), 10);
}

/**
 * @brief Test resize to zero throws
 * @details Verifies resizing to zero capacity throws invalid_argument
 */
TEST_F(RingBufferTest, Resize_ZeroCapacity_ThrowsException)
{
    RingBuffer<int> buf(4);
    EXPECT_THROW(buf.resize(0), std::invalid_argument);
}

// ==================== Edge Case Tests ====================

/**
 * @brief Test exceptions on empty buffer operations
 * @details Verifies that accessing elements on empty buffer throws
 */
TEST_F(RingBufferTest, Front_EmptyBuffer_ThrowsException)
{
    const RingBuffer<int> buf(4);
    EXPECT_THROW(buf.front(), std::out_of_range);
}

/**
 * @brief Test back on empty buffer throws
 * @details Verifies back throws on empty buffer
 */
TEST_F(RingBufferTest, Back_EmptyBuffer_ThrowsException)
{
    const RingBuffer<int> buf(4);
    EXPECT_THROW(buf.back(), std::out_of_range);
}

/**
 * @brief Test pop_front on empty buffer throws
 * @details Verifies pop_front throws on empty buffer
 */
TEST_F(RingBufferTest, PopFront_EmptyBuffer_ThrowsException)
{
    RingBuffer<int> buf(4);
    EXPECT_THROW(buf.pop_front(), std::out_of_range);
}

/**
 * @brief Test pop_back on empty buffer throws
 * @details Verifies pop_back throws on empty buffer
 */
TEST_F(RingBufferTest, PopBack_EmptyBuffer_ThrowsException)
{
    RingBuffer<int> buf(4);
    EXPECT_THROW(buf.pop_back(), std::out_of_range);
}

/**
 * @brief Test single element buffer
 * @details Verifies operations on single-element buffer work correctly
 */
TEST_F(RingBufferTest, SingleElement_Operations)
{
    RingBuffer<int> buf(1);
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());

    buf.push_back(42);
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.front(), 42);
    EXPECT_EQ(buf.back(), 42);

    // Push should overwrite
    buf.push_back(99);
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.front(), 99);
    EXPECT_EQ(buf.back(), 99);

    buf.pop_front();
    EXPECT_TRUE(buf.empty());
}

/**
 * @brief Test string type elements
 * @details Verifies RingBuffer works with std::string
 */
TEST_F(RingBufferTest, StringType_Operations)
{
    RingBuffer<std::string> buf(4);
    buf.push_back("hello");
    buf.push_back("world");

    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), "hello");
    EXPECT_EQ(buf.back(), "world");

    buf.pop_front();
    EXPECT_EQ(buf.front(), "world");

    buf.clear();
    EXPECT_TRUE(buf.empty());
}

// ==================== Large Data Set Tests ====================

/**
 * @brief Test large number of elements
 * @details Verifies ring buffer handles many elements correctly
 */
TEST_F(RingBufferTest, LargeNumberOfElements_Correctness)
{
    RingBuffer<int> buf(100);
    constexpr int n = 1000;

    // Write more elements than capacity, verifies overwrite behavior
    for (int i = 0; i < n; ++i)
    {
        buf.push_back(i);
    }

    // Should have only the last 100 elements
    EXPECT_EQ(buf.size(), 100);
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.front(), n - 100);
    EXPECT_EQ(buf.back(), n - 1);

    // Verify all elements
    for (std::size_t i = 0; i < buf.size(); ++i)
    {
        EXPECT_EQ(buf[i], n - 100 + static_cast<int>(i));
    }
}

/**
 * @brief Test full to empty cycle
 * @details Verifies filling then emptying the buffer works correctly
 */
TEST_F(RingBufferTest, FullToEmpty_Cycle)
{
    RingBuffer<int> buf(8);

    // Fill
    for (int i = 0; i < 8; ++i)
    {
        buf.push_back(i);
    }
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.size(), 8);

    // Remove all
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(buf.front(), i);
        buf.pop_front();
    }
    EXPECT_TRUE(buf.empty());

    // Refill
    for (int i = 10; i < 18; ++i)
    {
        buf.push_back(i);
    }
    EXPECT_TRUE(buf.full());
    EXPECT_EQ(buf.front(), 10);
    EXPECT_EQ(buf.back(), 17);
}

/**
 * @brief Test wrap-around behavior
 * @details Verifies internal wrap-around with push_front and pop_back works
 */
TEST_F(RingBufferTest, WrapAround_Correctness)
{
    RingBuffer<int> buf(5);

    // Fill
    for (int i = 1; i <= 5; ++i)
    {
        buf.push_back(i);
    }
    EXPECT_TRUE(buf.full());

    // Overwrite front elements via push_back
    buf.push_back(6);  // removes 1
    buf.push_back(7);  // removes 2
    EXPECT_EQ(buf.front(), 3);
    EXPECT_EQ(buf.back(), 7);

    // Now pop_front the rest
    std::vector<int> result;
    while (!buf.empty())
    {
        result.push_back(buf.front());
        buf.pop_front();
    }
    const std::vector expected = {3, 4, 5, 6, 7};
    EXPECT_EQ(result, expected);
}
