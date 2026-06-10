#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

#include "buffer/ByteBuffer.hpp"
#include "buffer/CharBuffer.hpp"
#include "buffer/DoubleBuffer.hpp"
#include "buffer/FloatBuffer.hpp"
#include "buffer/IntBuffer.hpp"
#include "buffer/LongBuffer.hpp"
#include "buffer/ShortBuffer.hpp"

using namespace common::buffer;

// ============================================================
// IBuffer interface common behavior tests
// ============================================================

template <typename T>
class BufferCommonTest : public testing::Test
{
};

using BufferTypes = testing::Types<
    ByteBuffer, CharBuffer, DoubleBuffer, FloatBuffer,
    IntBuffer, LongBuffer, ShortBuffer>;

TYPED_TEST_SUITE(BufferCommonTest, BufferTypes);

TYPED_TEST(BufferCommonTest, InitialState)
{
    TypeParam buf(10);
    EXPECT_EQ(buf.capacity(), 10);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.remaining(), 10);
    EXPECT_TRUE(buf.hasRemaining());
}

TYPED_TEST(BufferCommonTest, PositionSetterValid)
{
    TypeParam buf(10);
    buf.position(5);
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.remaining(), 5);
}

TYPED_TEST(BufferCommonTest, PositionSetterThrowsOnExceedLimit)
{
    TypeParam buf(10);
    EXPECT_THROW(buf.position(11), std::out_of_range);
}

TYPED_TEST(BufferCommonTest, PositionSetterThrowsOnExceedLimitAfterLimitChange)
{
    TypeParam buf(10);
    buf.limit(5);
    EXPECT_THROW(buf.position(6), std::out_of_range);
    buf.position(5);
    EXPECT_EQ(buf.position(), 5);
}

TYPED_TEST(BufferCommonTest, LimitSetterValid)
{
    TypeParam buf(10);
    buf.limit(7);
    EXPECT_EQ(buf.limit(), 7);
    EXPECT_EQ(buf.remaining(), 7);
}

TYPED_TEST(BufferCommonTest, LimitSetterThrowsOnExceedCapacity)
{
    TypeParam buf(10);
    EXPECT_THROW(buf.limit(11), std::out_of_range);
}

TYPED_TEST(BufferCommonTest, LimitSetterClampsPosition)
{
    TypeParam buf(10);
    buf.position(8);
    buf.limit(5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_EQ(buf.position(), 5);
}

TYPED_TEST(BufferCommonTest, ClearResetsPositionAndLimit)
{
    TypeParam buf(10);
    buf.position(3);
    buf.limit(7);
    buf.clear();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.remaining(), 10);
}

TYPED_TEST(BufferCommonTest, FlipSetsLimitToPositionAndResetsPosition)
{
    TypeParam buf(10);
    buf.position(3);
    buf.flip();
    EXPECT_EQ(buf.limit(), 3);
    EXPECT_EQ(buf.position(), 0);
}

TYPED_TEST(BufferCommonTest, RewindResetsPositionOnly)
{
    TypeParam buf(10);
    buf.position(5);
    buf.limit(8);
    buf.rewind();
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 8);
}

TYPED_TEST(BufferCommonTest, Remaining)
{
    TypeParam buf(10);
    EXPECT_EQ(buf.remaining(), 10);
    buf.position(3);
    EXPECT_EQ(buf.remaining(), 7);
    buf.limit(6);
    EXPECT_EQ(buf.remaining(), 3);
}

TYPED_TEST(BufferCommonTest, HasRemaining)
{
    TypeParam buf(10);
    EXPECT_TRUE(buf.hasRemaining());
    buf.position(10);
    EXPECT_FALSE(buf.hasRemaining());
}

// ============================================================================
// Buffer Common: Boundary Condition Tests
// ============================================================================

TYPED_TEST(BufferCommonTest, ZeroCapacity)
{
    TypeParam buf(0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

TYPED_TEST(BufferCommonTest, SingleElementCapacity)
{
    TypeParam buf(1);
    EXPECT_EQ(buf.capacity(), 1);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 1);
    EXPECT_TRUE(buf.hasRemaining());
    buf.position(1);
    EXPECT_FALSE(buf.hasRemaining());
}

TYPED_TEST(BufferCommonTest, FlipOnEmptyBuffer)
{
    TypeParam buf(10);
    // position is 0, flip sets limit = position = 0
    buf.flip();
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

TYPED_TEST(BufferCommonTest, FlipTwice)
{
    TypeParam buf(10);
    buf.position(3);
    buf.flip(); // limit=3, pos=0
    buf.flip(); // limit=0, pos=0
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_EQ(buf.position(), 0);
}

TYPED_TEST(BufferCommonTest, ClearTwice)
{
    TypeParam buf(10);
    buf.position(3);
    buf.limit(7);
    buf.clear();
    buf.clear(); // second clear should be idempotent
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
}

TYPED_TEST(BufferCommonTest, RewindTwice)
{
    TypeParam buf(10);
    buf.position(5);
    buf.rewind();
    buf.rewind(); // second rewind should be idempotent
    EXPECT_EQ(buf.position(), 0);
}

TYPED_TEST(BufferCommonTest, PositionAtLimitHasNoRemaining)
{
    TypeParam buf(10);
    buf.position(10);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

TYPED_TEST(BufferCommonTest, FullCycleClearFlipRewind)
{
    TypeParam buf(10);
    buf.position(4);
    buf.limit(8);
    buf.flip();     // limit=4, pos=0
    buf.rewind();   // pos=0 (no change)
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 4);
    buf.clear();    // limit=10, pos=0
    EXPECT_EQ(buf.limit(), 10);
    EXPECT_EQ(buf.position(), 0);
}

TYPED_TEST(BufferCommonTest, ZeroRemainingOnFullCapacity)
{
    TypeParam buf(5);
    buf.position(5);
    EXPECT_EQ(buf.remaining(), 0);
    EXPECT_FALSE(buf.hasRemaining());
    buf.position(0);
    EXPECT_EQ(buf.remaining(), 5);
    EXPECT_TRUE(buf.hasRemaining());
}

// ============================================================
// ByteBuffer specific tests
// ============================================================

class ByteBufferTest : public testing::Test
{
};

TEST_F(ByteBufferTest, PutAndGetSingle)
{
    ByteBuffer buf(5);
    buf.put(std::byte{42});
    EXPECT_EQ(buf.position(), 1);
    buf.put(std::byte{100});
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{42});
    EXPECT_EQ(buf.get(), std::byte{100});
    EXPECT_EQ(buf.position(), 2);
}

TEST_F(ByteBufferTest, PutAndGetVector)
{
    ByteBuffer buf(5);
    const std::vector src = {std::byte{1}, std::byte{2}, std::byte{3}};
    buf.put(src);
    EXPECT_EQ(buf.position(), 3);
    buf.flip();
    const auto result = buf.get(3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{1});
    EXPECT_EQ(result[1], std::byte{2});
    EXPECT_EQ(result[2], std::byte{3});
}

TEST_F(ByteBufferTest, PutOverflowThrows)
{
    ByteBuffer buf(2);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    EXPECT_THROW(buf.put(std::byte{3}), std::overflow_error);
}

TEST_F(ByteBufferTest, GetUnderflowThrows)
{
    ByteBuffer buf(2);
    buf.put(std::byte{1});
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(ByteBufferTest, PutVectorOverflowThrows)
{
    ByteBuffer buf(2);
    const std::vector src = {std::byte{1}, std::byte{2}, std::byte{3}};
    EXPECT_THROW(buf.put(src), std::overflow_error);
}

TEST_F(ByteBufferTest, GetVectorUnderflowThrows)
{
    ByteBuffer buf(3);
    buf.put(std::byte{1});
    buf.flip();
    EXPECT_THROW({ (void)buf.get(5); }, std::underflow_error);
}

TEST_F(ByteBufferTest, GetZeroLength)
{
    ByteBuffer buf(5);
    const auto result = buf.get(0);
    EXPECT_TRUE(result.empty());
}

TEST_F(ByteBufferTest, PutEmptyVector)
{
    ByteBuffer buf(5);
    const std::vector<std::byte> empty;
    buf.put(empty);
    EXPECT_EQ(buf.position(), 0);
}

TEST_F(ByteBufferTest, Compact)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.put(std::byte{3});
    EXPECT_EQ(buf.position(), 3);
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{1});
    EXPECT_EQ(buf.position(), 1);
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{2});
    EXPECT_EQ(buf.get(), std::byte{3});
}

TEST_F(ByteBufferTest, CompactWithZeroPositionDoesNothing)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.flip();
    buf.compact();
    EXPECT_EQ(buf.position(), 1);
    EXPECT_EQ(buf.limit(), 5);
}

TEST_F(ByteBufferTest, GetRemaining)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.put(std::byte{3});
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 2);
    EXPECT_EQ(remaining[0], std::byte{2});
    EXPECT_EQ(remaining[1], std::byte{3});
}

TEST_F(ByteBufferTest, GetRemainingEmpty)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.flip();
    (void)buf.get();
    EXPECT_TRUE(buf.getRemaining().empty());
}

// ============================================================================
// ByteBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(ByteBufferTest, ZeroCapacity)
{
    ByteBuffer buf(0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_THROW(buf.put(std::byte{1}), std::overflow_error);
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(ByteBufferTest, FillToExactCapacity)
{
    ByteBuffer buf(3);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.put(std::byte{3});
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(std::byte{4}), std::overflow_error);
}

TEST_F(ByteBufferTest, PutThenGetThenPutAgain)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.flip();
    (void)buf.get();
    (void)buf.get();
    EXPECT_FALSE(buf.hasRemaining());
    // After consuming all data, re-fill via compact+put or clear+put
    buf.clear();
    buf.put(std::byte{3});
    buf.put(std::byte{4});
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{3});
    EXPECT_EQ(buf.get(), std::byte{4});
}

TEST_F(ByteBufferTest, CompactOnEmptyBuffer)
{
    ByteBuffer buf(5);
    buf.compact();
    // compact moves remaining (limit-pos = 5) bytes and sets pos=5
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(ByteBufferTest, GetExactRemaining)
{
    ByteBuffer buf(10);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.put(std::byte{3});
    buf.flip();
    const auto result = buf.get(3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(ByteBufferTest, PutVectorExactFit)
{
    ByteBuffer buf(3);
    const std::vector src = {std::byte{1}, std::byte{2}, std::byte{3}};
    EXPECT_NO_THROW(buf.put(src));
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(ByteBufferTest, PutSingleAfterVector)
{
    ByteBuffer buf(5);
    const std::vector src = {std::byte{1}, std::byte{2}};
    buf.put(src);
    buf.put(std::byte{3});
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{1});
    EXPECT_EQ(buf.get(), std::byte{2});
    EXPECT_EQ(buf.get(), std::byte{3});
}

TEST_F(ByteBufferTest, MultipleCompactCycles)
{
    ByteBuffer buf(3);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.put(std::byte{3});
    buf.flip();
    (void)buf.get(); // pos=1
    buf.compact();   // pos=2, limit=3
    buf.put(std::byte{4});
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{2});
    EXPECT_EQ(buf.get(), std::byte{3});
    EXPECT_EQ(buf.get(), std::byte{4});
}

// ============================================================
// CharBuffer specific tests
// ============================================================

class CharBufferTest : public testing::Test
{
};

TEST_F(CharBufferTest, PutAndGetSingle)
{
    CharBuffer buf(5);
    buf.put('a');
    buf.put('b');
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_EQ(buf.get(), 'a');
    EXPECT_EQ(buf.get(), 'b');
}

TEST_F(CharBufferTest, PutAndGetString)
{
    CharBuffer buf(10);
    buf.put("hello");
    EXPECT_EQ(buf.position(), 5);
    buf.flip();
    EXPECT_EQ(buf.getRemaining(), "hello");
}

TEST_F(CharBufferTest, PutStringOverflowThrows)
{
    CharBuffer buf(3);
    EXPECT_THROW(buf.put("toolong"), std::overflow_error);
}

TEST_F(CharBufferTest, PutOverflowThrows)
{
    CharBuffer buf(1);
    buf.put('a');
    EXPECT_THROW(buf.put('b'), std::overflow_error);
}

TEST_F(CharBufferTest, GetUnderflowThrows)
{
    CharBuffer buf(5);
    buf.put('x');
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(CharBufferTest, Compact)
{
    CharBuffer buf(5);
    buf.put('a');
    buf.put('b');
    buf.put('c');
    buf.flip();
    EXPECT_EQ(buf.get(), 'a');
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put('d');
    buf.put('e');
    buf.flip();
    EXPECT_EQ(buf.getRemaining(), "bcde");
}

TEST_F(CharBufferTest, GetRemaining)
{
    CharBuffer buf(11);
    buf.put("hello world");
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    EXPECT_EQ(remaining, "ello world");
}

TEST_F(CharBufferTest, GetRemainingEmptyAtLimit)
{
    CharBuffer buf(5);
    buf.put("abcde");
    buf.flip();
    (void)buf.get();
    (void)buf.get();
    (void)buf.get();
    (void)buf.get();
    (void)buf.get();
    EXPECT_TRUE(buf.getRemaining().empty());
}

// ============================================================================
// CharBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(CharBufferTest, ZeroCapacity)
{
    CharBuffer buf(0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_THROW(buf.put('x'), std::overflow_error);
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(CharBufferTest, FillToExactCapacity)
{
    CharBuffer buf(3);
    buf.put('a');
    buf.put('b');
    buf.put('c');
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put('d'), std::overflow_error);
}

TEST_F(CharBufferTest, PutEmptyString)
{
    CharBuffer buf(5);
    EXPECT_NO_THROW(buf.put(""));
    EXPECT_EQ(buf.position(), 0);
}

TEST_F(CharBufferTest, PutStringExactFit)
{
    CharBuffer buf(5);
    EXPECT_NO_THROW(buf.put("hello"));
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(CharBufferTest, CompactOnEmptyBuffer)
{
    CharBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(CharBufferTest, PutAfterFlipWithoutRead)
{
    CharBuffer buf(5);
    buf.put('a');
    buf.flip(); // limit=1, pos=0
    // Overwrite without reading
    buf.put('b');
    buf.flip();
    EXPECT_EQ(buf.get(), 'b');
}

TEST_F(CharBufferTest, GetRemainingAfterClear)
{
    CharBuffer buf(5);
    buf.put("hello");
    buf.clear();
    // clear() resets position and limit but does not erase internal buffer,
    // so getRemaining returns all content between position and limit
    EXPECT_EQ(buf.getRemaining(), "hello");
    EXPECT_FALSE(buf.getRemaining().empty());
}

// ============================================================
// DoubleBuffer specific tests
// ============================================================

class DoubleBufferTest : public testing::Test
{
};

TEST_F(DoubleBufferTest, PutAndGetSingle)
{
    DoubleBuffer buf(5);
    buf.put(3.14);
    buf.put(2.71);
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_DOUBLE_EQ(buf.get(), 3.14);
    EXPECT_DOUBLE_EQ(buf.get(), 2.71);
}

TEST_F(DoubleBufferTest, PutAndGetVector)
{
    DoubleBuffer buf(5);
    const std::vector src = {1.0, 2.0, 3.0};
    buf.put(src);
    EXPECT_EQ(buf.position(), 3);
    buf.flip();
    EXPECT_DOUBLE_EQ(buf.get(), 1.0);
    EXPECT_DOUBLE_EQ(buf.get(), 2.0);
    EXPECT_DOUBLE_EQ(buf.get(), 3.0);
}

TEST_F(DoubleBufferTest, PutReturnsReference)
{
    DoubleBuffer buf(5);
    DoubleBuffer& ref = buf.put(1.0).put(2.0);
    EXPECT_EQ(&ref, &buf);
    EXPECT_EQ(buf.position(), 2);
}

TEST_F(DoubleBufferTest, PutOverflowThrows)
{
    DoubleBuffer buf(2);
    buf.put(1.0);
    buf.put(2.0);
    EXPECT_THROW(buf.put(3.0), std::overflow_error);
}

TEST_F(DoubleBufferTest, GetUnderflowThrows)
{
    DoubleBuffer buf(2);
    buf.put(1.0);
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(DoubleBufferTest, PutVectorOverflowThrows)
{
    DoubleBuffer buf(2);
    const std::vector src = {1.0, 2.0, 3.0};
    EXPECT_THROW(buf.put(src), std::overflow_error);
}

TEST_F(DoubleBufferTest, Compact)
{
    DoubleBuffer buf(5);
    buf.put(1.0);
    buf.put(2.0);
    buf.put(3.0);
    buf.flip();
    (void)buf.get();
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put(4.0);
    buf.flip();
    EXPECT_DOUBLE_EQ(buf.get(), 2.0);
    EXPECT_DOUBLE_EQ(buf.get(), 3.0);
    EXPECT_DOUBLE_EQ(buf.get(), 4.0);
}

TEST_F(DoubleBufferTest, GetRemaining)
{
    DoubleBuffer buf(5);
    buf.put(1.0);
    buf.put(2.0);
    buf.put(3.0);
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 2);
    EXPECT_DOUBLE_EQ(remaining[0], 2.0);
    EXPECT_DOUBLE_EQ(remaining[1], 3.0);
}

// ============================================================================
// DoubleBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(DoubleBufferTest, ZeroCapacity)
{
    DoubleBuffer buf(0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_THROW(buf.put(1.0), std::overflow_error);
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(DoubleBufferTest, FillToExactCapacity)
{
    DoubleBuffer buf(3);
    buf.put(1.0);
    buf.put(2.0);
    buf.put(3.0);
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(4.0), std::overflow_error);
}

TEST_F(DoubleBufferTest, PutVectorExactFit)
{
    DoubleBuffer buf(3);
    const std::vector src = {1.0, 2.0, 3.0};
    EXPECT_NO_THROW(buf.put(src));
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(DoubleBufferTest, PutEmptyVector)
{
    DoubleBuffer buf(5);
    const std::vector<double> empty;
    EXPECT_NO_THROW(buf.put(empty));
    EXPECT_EQ(buf.position(), 0);
}

TEST_F(DoubleBufferTest, CompactOnEmptyBuffer)
{
    DoubleBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

// ============================================================
// FloatBuffer specific tests
// ============================================================

class FloatBufferTest : public testing::Test
{
};

TEST_F(FloatBufferTest, Allocate)
{
    const FloatBuffer buf = FloatBuffer::allocate(10);
    EXPECT_EQ(buf.capacity(), 10);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 10);
}

TEST_F(FloatBufferTest, PutAndGetSingle)
{
    FloatBuffer buf(5);
    buf.put(1.5f);
    buf.put(2.5f);
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_FLOAT_EQ(buf.get(), 1.5f);
    EXPECT_FLOAT_EQ(buf.get(), 2.5f);
}

TEST_F(FloatBufferTest, PutAndGetVector)
{
    FloatBuffer buf(5);
    const std::vector src = {1.0f, 2.0f, 3.0f};
    buf.put(src);
    EXPECT_EQ(buf.position(), 3);
    buf.flip();
    const auto result = buf.get(3);
    ASSERT_EQ(result.size(), 3);
    EXPECT_FLOAT_EQ(result[0], 1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_FLOAT_EQ(result[2], 3.0f);
}

TEST_F(FloatBufferTest, GetWithLength)
{
    FloatBuffer buf(10);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.put(3.0f);
    buf.flip();
    const auto result = buf.get(2);
    ASSERT_EQ(result.size(), 2);
    EXPECT_FLOAT_EQ(result[0], 1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_EQ(buf.position(), 2);
}

TEST_F(FloatBufferTest, GetZeroLength)
{
    FloatBuffer buf(5);
    const auto result = buf.get(0);
    EXPECT_TRUE(result.empty());
}

TEST_F(FloatBufferTest, PutOverflowThrows)
{
    FloatBuffer buf(2);
    buf.put(1.0f);
    buf.put(2.0f);
    EXPECT_THROW(buf.put(3.0f), std::overflow_error);
}

TEST_F(FloatBufferTest, GetUnderflowThrows)
{
    FloatBuffer buf(2);
    buf.put(1.0f);
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(FloatBufferTest, GetVectorUnderflowThrows)
{
    FloatBuffer buf(3);
    buf.put(1.0f);
    buf.flip();
    EXPECT_THROW({ (void)buf.get(5); }, std::underflow_error);
}

TEST_F(FloatBufferTest, PutVectorOverflowThrows)
{
    FloatBuffer buf(2);
    const std::vector src = {1.0f, 2.0f, 3.0f};
    EXPECT_THROW(buf.put(src), std::overflow_error);
}

TEST_F(FloatBufferTest, PutEmptyVector)
{
    FloatBuffer buf(5);
    const std::vector<float> empty;
    buf.put(empty);
    EXPECT_EQ(buf.position(), 0);
}

TEST_F(FloatBufferTest, Compact)
{
    FloatBuffer buf(5);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.put(3.0f);
    buf.flip();
    (void)buf.get();
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put(4.0f);
    buf.flip();
    EXPECT_FLOAT_EQ(buf.get(), 2.0f);
    EXPECT_FLOAT_EQ(buf.get(), 3.0f);
    EXPECT_FLOAT_EQ(buf.get(), 4.0f);
}

TEST_F(FloatBufferTest, GetRemaining)
{
    FloatBuffer buf(5);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 1);
    EXPECT_FLOAT_EQ(remaining[0], 2.0f);
}

// ============================================================================
// FloatBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(FloatBufferTest, CompactOnEmptyBuffer)
{
    FloatBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(FloatBufferTest, PutAfterFlip)
{
    FloatBuffer buf(5);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.flip();       // limit=2, pos=0
    (void)buf.get();  // pos=1
    buf.compact();    // pos=1, limit=5
    buf.put(3.0f);
    buf.flip();
    EXPECT_FLOAT_EQ(buf.get(), 2.0f);
    EXPECT_FLOAT_EQ(buf.get(), 3.0f);
}

TEST_F(FloatBufferTest, PutVectorExactFit)
{
    FloatBuffer buf(3);
    const std::vector src = {1.0f, 2.0f, 3.0f};
    EXPECT_NO_THROW(buf.put(src));
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(FloatBufferTest, FillToExactCapacity)
{
    FloatBuffer buf(3);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.put(3.0f);
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(4.0f), std::overflow_error);
}

// ============================================================
// IntBuffer specific tests
// ============================================================

class IntBufferTest : public testing::Test
{
};

TEST_F(IntBufferTest, PutAndGetSingle)
{
    IntBuffer buf(5);
    buf.put(42);
    buf.put(100);
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_EQ(buf.get(), 42);
    EXPECT_EQ(buf.get(), 100);
}

TEST_F(IntBufferTest, PutAndGetByIndex)
{
    IntBuffer buf(5);
    buf.put(0, 100);
    buf.put(1, 200);
    EXPECT_EQ(buf.get(0), 100);
    EXPECT_EQ(buf.get(1), 200);
    EXPECT_EQ(buf.position(), 0);
}

TEST_F(IntBufferTest, PutIndexOutOfRangeThrows)
{
    IntBuffer buf(5);
    EXPECT_THROW(buf.put(5, 100), std::out_of_range);
}

TEST_F(IntBufferTest, GetIndexOutOfRangeThrows)
{
    const IntBuffer buf(5);
    EXPECT_THROW({ (void)buf.get(5); }, std::out_of_range);
}

TEST_F(IntBufferTest, PutOverflowThrows)
{
    IntBuffer buf(2);
    buf.put(1);
    buf.put(2);
    EXPECT_THROW(buf.put(3), std::overflow_error);
}

TEST_F(IntBufferTest, GetUnderflowThrows)
{
    IntBuffer buf(2);
    buf.put(1);
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(IntBufferTest, Compact)
{
    IntBuffer buf(5);
    buf.put(1);
    buf.put(2);
    buf.put(3);
    buf.flip();
    (void)buf.get();
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put(4);
    buf.flip();
    EXPECT_EQ(buf.get(), 2);
    EXPECT_EQ(buf.get(), 3);
    EXPECT_EQ(buf.get(), 4);
}

TEST_F(IntBufferTest, GetRemaining)
{
    IntBuffer buf(5);
    buf.put(10);
    buf.put(20);
    buf.put(30);
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 2);
    EXPECT_EQ(remaining[0], 20);
    EXPECT_EQ(remaining[1], 30);
}

// ============================================================================
// IntBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(IntBufferTest, CompactOnEmptyBuffer)
{
    IntBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(IntBufferTest, FillToExactCapacity)
{
    IntBuffer buf(3);
    buf.put(1);
    buf.put(2);
    buf.put(3);
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(4), std::overflow_error);
}

TEST_F(IntBufferTest, PutNegativeValues)
{
    IntBuffer buf(5);
    buf.put(-1);
    buf.put(0);
    buf.put(INT_MIN);
    buf.put(INT_MAX);
    buf.flip();
    EXPECT_EQ(buf.get(), -1);
    EXPECT_EQ(buf.get(), 0);
    EXPECT_EQ(buf.get(), INT_MIN);
    EXPECT_EQ(buf.get(), INT_MAX);
}

TEST_F(IntBufferTest, PutAndGetByIndexOutOfBounds)
{
    IntBuffer buf(5);
    EXPECT_THROW({ (void)buf.get(5); }, std::out_of_range);
    EXPECT_THROW(buf.put(5, 100), std::out_of_range);
}

// ============================================================
// LongBuffer specific tests
// ============================================================

class LongBufferTest : public testing::Test
{
};

TEST_F(LongBufferTest, PutAndGet)
{
    LongBuffer buf(5);
    buf.put(10000000000LL);
    buf.put(20000000000LL);
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_EQ(buf.get(), 10000000000LL);
    EXPECT_EQ(buf.get(), 20000000000LL);
}

TEST_F(LongBufferTest, PutOverflowThrows)
{
    LongBuffer buf(2);
    buf.put(1);
    buf.put(2);
    EXPECT_THROW(buf.put(3), std::overflow_error);
}

TEST_F(LongBufferTest, GetUnderflowThrows)
{
    LongBuffer buf(2);
    buf.put(1);
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(LongBufferTest, Compact)
{
    LongBuffer buf(5);
    buf.put(10);
    buf.put(20);
    buf.put(30);
    buf.flip();
    (void)buf.get();
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put(40);
    buf.flip();
    EXPECT_EQ(buf.get(), 20);
    EXPECT_EQ(buf.get(), 30);
    EXPECT_EQ(buf.get(), 40);
}

TEST_F(LongBufferTest, GetRemaining)
{
    LongBuffer buf(5);
    buf.put(100);
    buf.put(200);
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 1);
    EXPECT_EQ(remaining[0], 200);
}

// ============================================================================
// LongBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(LongBufferTest, CompactOnEmptyBuffer)
{
    LongBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(LongBufferTest, FillToExactCapacity)
{
    LongBuffer buf(3);
    buf.put(1LL);
    buf.put(2LL);
    buf.put(3LL);
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(4LL), std::overflow_error);
}

TEST_F(LongBufferTest, PutNegativeValues)
{
    LongBuffer buf(5);
    buf.put(-1LL);
    buf.put(LLONG_MIN);
    buf.put(LLONG_MAX);
    buf.flip();
    EXPECT_EQ(buf.get(), -1);
    EXPECT_EQ(buf.get(), LLONG_MIN);
    EXPECT_EQ(buf.get(), LLONG_MAX);
}

// ============================================================
// ShortBuffer specific tests
// ============================================================

class ShortBufferTest : public testing::Test
{
};

TEST_F(ShortBufferTest, Wrap)
{
    constexpr int16_t arr[] = {10, 20, 30};
    const ShortBuffer buf = ShortBuffer::wrap(arr, 3);
    EXPECT_EQ(buf.capacity(), 3);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 3);
}

TEST_F(ShortBufferTest, WrapCopiesData)
{
    constexpr int16_t arr[] = {100, 200, 300};
    ShortBuffer buf = ShortBuffer::wrap(arr, 3);
    EXPECT_EQ(buf.get(), 100);
    EXPECT_EQ(buf.get(), 200);
    EXPECT_EQ(buf.get(), 300);
}

TEST_F(ShortBufferTest, WrapNullptr)
{
    const ShortBuffer buf = ShortBuffer::wrap(nullptr, 0);
    EXPECT_EQ(buf.capacity(), 0);
}

TEST_F(ShortBufferTest, PutAndGet)
{
    ShortBuffer buf(5);
    buf.put(42);
    buf.put(99);
    EXPECT_EQ(buf.position(), 2);
    buf.flip();
    EXPECT_EQ(buf.get(), 42);
    EXPECT_EQ(buf.get(), 99);
}

TEST_F(ShortBufferTest, PutAndGetByIndex)
{
    ShortBuffer buf(5);
    buf.put(0, 111);
    buf.put(1, 222);
    EXPECT_EQ(buf.get(0), 111);
    EXPECT_EQ(buf.get(1), 222);
}

TEST_F(ShortBufferTest, PutIndexOutOfRangeThrows)
{
    ShortBuffer buf(5);
    EXPECT_THROW(buf.put(5, 100), std::out_of_range);
}

TEST_F(ShortBufferTest, GetIndexOutOfRangeThrows)
{
    const ShortBuffer buf(5);
    EXPECT_THROW({ (void)buf.get(5); }, std::out_of_range);
}

TEST_F(ShortBufferTest, PutOverflowThrows)
{
    ShortBuffer buf(2);
    buf.put(1);
    buf.put(2);
    EXPECT_THROW(buf.put(3), std::overflow_error);
}

TEST_F(ShortBufferTest, GetUnderflowThrows)
{
    ShortBuffer buf(2);
    buf.put(1);
    buf.flip();
    (void)buf.get();
    EXPECT_THROW({ (void)buf.get(); }, std::underflow_error);
}

TEST_F(ShortBufferTest, Data)
{
    ShortBuffer buf(3);
    buf.put(10);
    buf.put(20);
    const int16_t* raw = buf.data();
    EXPECT_EQ(raw[0], 10);
    EXPECT_EQ(raw[1], 20);
}

TEST_F(ShortBufferTest, ConstData)
{
    ShortBuffer buf(3);
    buf.put(10);
    buf.put(20);
    const ShortBuffer& const_buf = buf;
    const int16_t* raw = const_buf.data();
    EXPECT_EQ(raw[0], 10);
    EXPECT_EQ(raw[1], 20);
}

TEST_F(ShortBufferTest, Compact)
{
    ShortBuffer buf(5);
    buf.put(1);
    buf.put(2);
    buf.put(3);
    buf.flip();
    (void)buf.get();
    buf.compact();
    EXPECT_EQ(buf.position(), 2);
    EXPECT_EQ(buf.limit(), 5);
    buf.put(4);
    buf.flip();
    EXPECT_EQ(buf.get(), 2);
    EXPECT_EQ(buf.get(), 3);
    EXPECT_EQ(buf.get(), 4);
}

TEST_F(ShortBufferTest, GetRemaining)
{
    ShortBuffer buf(5);
    buf.put(10);
    buf.put(20);
    buf.put(30);
    buf.flip();
    (void)buf.get();
    const auto remaining = buf.getRemaining();
    ASSERT_EQ(remaining.size(), 2);
    EXPECT_EQ(remaining[0], 20);
    EXPECT_EQ(remaining[1], 30);
}

// ============================================================================
// ShortBuffer: Boundary Condition Tests
// ============================================================================

TEST_F(ShortBufferTest, WrapZeroLength)
{
    constexpr int16_t arr[1] = {0};
    const ShortBuffer buf = ShortBuffer::wrap(arr, 0);
    EXPECT_EQ(buf.capacity(), 0);
    EXPECT_EQ(buf.position(), 0);
    EXPECT_EQ(buf.limit(), 0);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(ShortBufferTest, WrapLargeArray)
{
    const std::vector<int16_t> vec(1000, 42);
    ShortBuffer buf = ShortBuffer::wrap(vec.data(), vec.size());
    EXPECT_EQ(buf.capacity(), 1000);
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_EQ(buf.get(), 42);
    }
}

TEST_F(ShortBufferTest, CompactOnEmptyBuffer)
{
    ShortBuffer buf(5);
    buf.compact();
    EXPECT_EQ(buf.position(), 5);
    EXPECT_EQ(buf.limit(), 5);
    EXPECT_FALSE(buf.hasRemaining());
}

TEST_F(ShortBufferTest, FillToExactCapacity)
{
    ShortBuffer buf(3);
    buf.put(1);
    buf.put(2);
    buf.put(3);
    EXPECT_FALSE(buf.hasRemaining());
    EXPECT_THROW(buf.put(4), std::overflow_error);
}

TEST_F(ShortBufferTest, PutNegativeValues)
{
    ShortBuffer buf(5);
    buf.put(-1);
    buf.put(SHRT_MIN);
    buf.put(SHRT_MAX);
    buf.flip();
    EXPECT_EQ(buf.get(), -1);
    EXPECT_EQ(buf.get(), SHRT_MIN);
    EXPECT_EQ(buf.get(), SHRT_MAX);
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(ByteBufferTest, MultipleCompactCycles_Repeated)
{
    for (int cycle = 0; cycle < 5; ++cycle)
    {
        ByteBuffer buf(3);
        buf.put(std::byte{1});
        buf.put(std::byte{2});
        buf.put(std::byte{3});
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(std::byte{static_cast<std::byte>(4 + cycle)});
        buf.flip();
        EXPECT_EQ(buf.get(), std::byte{2});
        EXPECT_EQ(buf.get(), std::byte{3});
        EXPECT_EQ(buf.get(), std::byte{static_cast<std::byte>(4 + cycle)});
    }
}

TEST_F(ByteBufferTest, ReadWriteCycle)
{
    ByteBuffer buf(5);
    buf.put(std::byte{1});
    buf.put(std::byte{2});
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{1});
    EXPECT_EQ(buf.get(), std::byte{2});
    buf.clear();
    buf.put(std::byte{3});
    buf.put(std::byte{4});
    buf.flip();
    EXPECT_EQ(buf.get(), std::byte{3});
    EXPECT_EQ(buf.get(), std::byte{4});
}

TEST_F(CharBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        CharBuffer buf(3);
        buf.put('a');
        buf.put('b');
        buf.put('c');
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(static_cast<char>('d' + cycle));
        buf.flip();
        EXPECT_EQ(buf.getRemaining().size(), 3);
    }
}

TEST_F(CharBufferTest, ReadWriteCycle)
{
    CharBuffer buf(5);
    buf.put("ab");
    buf.flip();
    EXPECT_EQ(buf.getRemaining(), "ab");
    buf.clear();
    buf.put("cd");
    buf.flip();
    EXPECT_EQ(buf.getRemaining(), "cd");
}

TEST_F(DoubleBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        DoubleBuffer buf(3);
        buf.put(1.0);
        buf.put(2.0);
        buf.put(3.0);
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(static_cast<double>(4.0 + cycle));
        buf.flip();
        EXPECT_DOUBLE_EQ(buf.get(), 2.0);
        EXPECT_DOUBLE_EQ(buf.get(), 3.0);
        EXPECT_DOUBLE_EQ(buf.get(), static_cast<double>(4.0 + cycle));
    }
}

TEST_F(DoubleBufferTest, ReadWriteCycle)
{
    DoubleBuffer buf(5);
    buf.put(1.0);
    buf.put(2.0);
    buf.flip();
    (void)buf.get();
    (void)buf.get();
    buf.clear();
    buf.put(3.0);
    buf.put(4.0);
    buf.flip();
    EXPECT_DOUBLE_EQ(buf.get(), 3.0);
    EXPECT_DOUBLE_EQ(buf.get(), 4.0);
}

TEST_F(FloatBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        FloatBuffer buf(3);
        buf.put(1.0f);
        buf.put(2.0f);
        buf.put(3.0f);
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(static_cast<float>(4.0f + cycle));
        buf.flip();
        EXPECT_FLOAT_EQ(buf.get(), 2.0f);
        EXPECT_FLOAT_EQ(buf.get(), 3.0f);
        EXPECT_FLOAT_EQ(buf.get(), static_cast<float>(4.0f + cycle));
    }
}

TEST_F(FloatBufferTest, ReadWriteCycle)
{
    FloatBuffer buf(5);
    buf.put(1.0f);
    buf.put(2.0f);
    buf.flip();
    (void)buf.get();
    (void)buf.get();
    buf.clear();
    buf.put(3.0f);
    buf.put(4.0f);
    buf.flip();
    EXPECT_FLOAT_EQ(buf.get(), 3.0f);
    EXPECT_FLOAT_EQ(buf.get(), 4.0f);
}

TEST_F(IntBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        IntBuffer buf(3);
        buf.put(1);
        buf.put(2);
        buf.put(3);
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(4 + cycle);
        buf.flip();
        EXPECT_EQ(buf.get(), 2);
        EXPECT_EQ(buf.get(), 3);
        EXPECT_EQ(buf.get(), 4 + cycle);
    }
}

TEST_F(IntBufferTest, ReadWriteCycle)
{
    IntBuffer buf(5);
    buf.put(1);
    buf.put(2);
    buf.flip();
    EXPECT_EQ(buf.get(), 1);
    EXPECT_EQ(buf.get(), 2);
    buf.clear();
    buf.put(3);
    buf.put(4);
    buf.flip();
    EXPECT_EQ(buf.get(), 3);
    EXPECT_EQ(buf.get(), 4);
}

TEST_F(LongBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        LongBuffer buf(3);
        buf.put(10);
        buf.put(20);
        buf.put(30);
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(40 + cycle);
        buf.flip();
        EXPECT_EQ(buf.get(), 20);
        EXPECT_EQ(buf.get(), 30);
        EXPECT_EQ(buf.get(), 40 + cycle);
    }
}

TEST_F(LongBufferTest, ReadWriteCycle)
{
    LongBuffer buf(5);
    buf.put(100);
    buf.put(200);
    buf.flip();
    EXPECT_EQ(buf.get(), 100);
    EXPECT_EQ(buf.get(), 200);
    buf.clear();
    buf.put(300);
    buf.put(400);
    buf.flip();
    EXPECT_EQ(buf.get(), 300);
    EXPECT_EQ(buf.get(), 400);
}

TEST_F(ShortBufferTest, PutAfterFlip)
{
    ShortBuffer buf(5);
    buf.put(1);
    buf.put(2);
    buf.flip();
    (void)buf.get();
    buf.compact();
    buf.put(3);
    buf.flip();
    EXPECT_EQ(buf.get(), 2);
    EXPECT_EQ(buf.get(), 3);
}

TEST_F(ShortBufferTest, MultipleCompactCycles)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        ShortBuffer buf(3);
        buf.put(1);
        buf.put(2);
        buf.put(3);
        buf.flip();
        (void)buf.get();
        buf.compact();
        buf.put(static_cast<int16_t>(4 + cycle));
        buf.flip();
        EXPECT_EQ(buf.get(), 2);
        EXPECT_EQ(buf.get(), 3);
        EXPECT_EQ(buf.get(), static_cast<int16_t>(4 + cycle));
    }
}

TEST_F(ShortBufferTest, ReadWriteCycle)
{
    ShortBuffer buf(5);
    buf.put(10);
    buf.put(20);
    buf.flip();
    EXPECT_EQ(buf.get(), 10);
    EXPECT_EQ(buf.get(), 20);
    buf.clear();
    buf.put(30);
    buf.put(40);
    buf.flip();
    EXPECT_EQ(buf.get(), 30);
    EXPECT_EQ(buf.get(), 40);
}
