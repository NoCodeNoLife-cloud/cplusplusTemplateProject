/**
 * @file IBufferTest.cc
 * @brief Unit tests for the IBuffer interface contract
 * @details Tests verify the contract defined by IBuffer: position/limit/capacity
 *          management, clear/flip/rewind semantics, and edge cases.
 *          Uses a minimal concrete stub implementation.
 */

#include <gtest/gtest.h>

#include "interface/io/IBuffer.hpp"

using namespace common::interface::io;

/**
 * @brief Minimal concrete stub implementing IBuffer for contract testing.
 * @details Provides a simple array-based buffer to verify IBuffer interface contracts.
 */
class TestBuffer final : public IBuffer
{
public:
    explicit TestBuffer(size_t capacity)
    {
        capacity_ = capacity;
        limit_ = capacity;
        position_ = 0;
    }

    void clear() override
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void flip() override
    {
        limit_ = position_;
        position_ = 0;
    }

    void rewind() override
    {
        position_ = 0;
    }

    [[nodiscard]] size_t capacity() const override
    {
        return capacity_;
    }

    [[nodiscard]] size_t position() const override
    {
        return position_;
    }

    void position(size_t newPosition) override
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("Position exceeds limit");
        }
        position_ = newPosition;
    }

    [[nodiscard]] size_t limit() const override
    {
        return limit_;
    }

    void limit(size_t newLimit) override
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("Limit exceeds capacity");
        }
        limit_ = newLimit;
        if (position_ > limit_)
        {
            position_ = limit_;
        }
    }

    [[nodiscard]] size_t remaining() const override
    {
        if (position_ > limit_) return 0;
        return limit_ - position_;
    }

    [[nodiscard]] bool hasRemaining() const override
    {
        return remaining() > 0;
    }

private:
    size_t capacity_{0};
    size_t position_{0};
    size_t limit_{0};
};

/**
 * @brief Test fixture for IBufferTest tests
 */
class IBufferTest : public testing::Test
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
 * @brief Test initial buffer state
 * @details Verifies capacity, position, limit are correctly initialized
 */
TEST_F(IBufferTest, InitialState)
{
    TestBuffer buf(10);
    EXPECT_EQ(buf.capacity(), 10);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.remaining(), 10);
    EXPECT_TRUE(buf.hasRemaining());
}

/**
 * @brief Test position setter within bounds
 * @details Setting position within limit should succeed
 */
TEST_F(IBufferTest, PositionSetterValid)
{
    TestBuffer buf(10);
    buf.position(5);
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.remaining(), 5);
}

/**
 * @brief Test position setter exceeding limit throws
 * @details Setting position beyond limit should throw out_of_range
 */
TEST_F(IBufferTest, PositionSetterExceedsLimit)
{
    TestBuffer buf(10);
    EXPECT_THROW(buf.position(11), std::out_of_range);
}

/**
 * @brief Test position setter after limit change
 * @details Reducing limit then setting position should enforce new limit
 */
TEST_F(IBufferTest, PositionSetterAfterLimitChange)
{
    TestBuffer buf(10);
    buf.limit(5);
    EXPECT_THROW(buf.position(6), std::out_of_range);
    buf.position(5);
    EXPECT_EQ(buf.position(), 5);
}

/**
 * @brief Test limit setter within bounds
 * @details Setting limit within capacity should succeed
 */
TEST_F(IBufferTest, LimitSetterValid)
{
    TestBuffer buf(10);
    buf.limit(7);
    EXPECT_EQ(buf.limit(), 7);
    EXPECT_EQ(buf.remaining(), 7);
}

/**
 * @brief Test limit setter exceeding capacity throws
 * @details Setting limit beyond capacity should throw out_of_range
 */
TEST_F(IBufferTest, LimitSetterExceedsCapacity)
{
    TestBuffer buf(10);
    EXPECT_THROW(buf.limit(11), std::out_of_range);
}

/**
 * @brief Test limit setter clamps position
 * @details When limit is reduced below position, position should be clamped
 */
TEST_F(IBufferTest, LimitSetterClampsPosition)
{
    TestBuffer buf(10);
    buf.position(8);
    buf.limit(5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_EQ(buf.position(), 5);
}

/**
 * @brief Test clear resets position and limit
 * @details clear() sets position=0, limit=capacity
 */
TEST_F(IBufferTest, ClearResetsPositionAndLimit)
{
    TestBuffer buf(10);
    buf.position(3);
    buf.limit(7);
    buf.clear();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.remaining(), 10);
    EXPECT_TRUE(buf.hasRemaining());
}

/**
 * @brief Test clear is idempotent
 * @details Multiple clear calls should produce same state
 */
TEST_F(IBufferTest, ClearTwice)
{
    TestBuffer buf(10);
    buf.position(3);
    buf.limit(7);
    buf.clear();
    buf.clear();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
}

/**
 * @brief Test flip sets limit to position and resets position
 * @details flip() prepares buffer for read after write
 */
TEST_F(IBufferTest, FlipSetsLimitToPosition)
{
    TestBuffer buf(10);
    buf.position(3);
    buf.flip();
    EXPECT_EQ(buf.limit(), 3);
    EXPECT_EQ(buf.position(), 0);
}

/**
 * @brief Test flip on empty buffer
 * @edge Flipping when position=0 results in limit=0
 */
TEST_F(IBufferTest, FlipOnEmptyBuffer)
{
    TestBuffer buf(10);
    buf.flip();
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

/**
 * @brief Test flip twice
 * @edge Second flip on a flipped buffer sets limit=0
 */
TEST_F(IBufferTest, FlipTwice)
{
    TestBuffer buf(10);
    buf.position(3);
    buf.flip();
    buf.flip();
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.position(), 0);
}

/**
 * @brief Test rewind resets position only
 * @details rewind() sets position=0, keeps limit unchanged
 */
TEST_F(IBufferTest, RewindResetsPositionOnly)
{
    TestBuffer buf(10);
    buf.position(5);
    buf.limit(8);
    buf.rewind();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 8);
}

/**
 * @brief Test rewind twice is idempotent
 * @edge Multiple rewind calls produce same state
 */
TEST_F(IBufferTest, RewindTwice)
{
    TestBuffer buf(10);
    buf.position(5);
    buf.rewind();
    buf.rewind();
    EXPECT_EQ(buf.position(), 0);
}

/**
 * @brief Test remaining calculation
 * @details remaining = limit - position
 */
TEST_F(IBufferTest, Remaining)
{
    TestBuffer buf(10);
    EXPECT_EQ(buf.remaining(), 10);
    buf.position(3);
    EXPECT_EQ(buf.remaining(), 7);
    buf.limit(6);
    EXPECT_EQ(buf.remaining(), 3);
}

/**
 * @brief Test hasRemaining
 * @details hasRemaining is true when remaining > 0
 */
TEST_F(IBufferTest, HasRemaining)
{
    TestBuffer buf(10);
    EXPECT_TRUE(buf.hasRemaining());
    buf.position(10);
    EXPECT_FALSE(buf.hasRemaining());
}

/**
 * @brief Test zero capacity buffer
 * @edge Buffer with capacity 0 should have no remaining
 */
TEST_F(IBufferTest, ZeroCapacity)
{
    TestBuffer buf(0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

/**
 * @brief Test single element capacity
 * @edge Smallest non-zero buffer
 */
TEST_F(IBufferTest, SingleElementCapacity)
{
    TestBuffer buf(1);
    EXPECT_EQ(buf.capacity(), 1);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 1);
    EXPECT_TRUE(buf.hasRemaining());
    buf.position(1);
    EXPECT_FALSE(buf.hasRemaining());
}

/**
 * @brief Test position at limit has no remaining
 * @edge When position equals limit, remaining is zero
 */
TEST_F(IBufferTest, PositionAtLimitHasNoRemaining)
{
    TestBuffer buf(10);
    buf.position(10);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

/**
 * @brief Test full cycle clear-flip-rewind
 * @details Verifies the common buffer lifecycle
 */
TEST_F(IBufferTest, FullCycleClearFlipRewind)
{
    TestBuffer buf(10);
    buf.position(4);
    buf.limit(8);
    buf.flip();
    buf.rewind();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 4);
    buf.clear();
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.position(), 0);
}

/**
 * @brief Test zero remaining on full capacity
 * @edge Positioning at capacity gives zero remaining
 */
TEST_F(IBufferTest, ZeroRemainingOnFullCapacity)
{
    TestBuffer buf(5);
    buf.position(5);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
    buf.position(0);
    EXPECT_EQ(buf.remaining(), 5);
    EXPECT_TRUE(buf.hasRemaining());
}

/**
 * @brief Test position setter at exact limit
 * @edge Setting position equal to limit should succeed
 */
TEST_F(IBufferTest, PositionAtExactLimit)
{
    TestBuffer buf(10);
    buf.limit(7);
    buf.position(7);
    EXPECT_EQ(buf.position(), 7);
    EXPECT_EQ(buf.remaining(), 0);
}

/**
 * @brief Test limit setter at exact capacity
 * @edge Setting limit equal to capacity should succeed
 */
TEST_F(IBufferTest, LimitAtExactCapacity)
{
    TestBuffer buf(10);
    buf.limit(10);
    EXPECT_EQ(buf.limit(), 10);
}
