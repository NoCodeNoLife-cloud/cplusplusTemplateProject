/**
 * @file ByteArrayOutputStreamTest.cc
 * @brief Unit tests for ByteArrayOutputStream
 * @details Tests cover write operations, buffer expansion, conversion to byte array/string, reset, and edge cases.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <vector>

#include "io/writer/ByteArrayOutputStream.hpp"

using namespace common::io::writer;

/// @brief Test fixture for ByteArrayOutputStream tests.
class ByteArrayOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stream_ = std::make_unique<ByteArrayOutputStream>();
    }

    std::unique_ptr<ByteArrayOutputStream> stream_;
};

/** @brief A newly created stream is empty.
    @details Checks size=0, toByteArray is empty, and toString is "". */
TEST_F(ByteArrayOutputStreamTest, InitiallyEmpty)
{
    EXPECT_EQ(stream_->size(), 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
    EXPECT_EQ(stream_->toString(), "");
}

/** @brief Write a single byte.
    @details Writes 0x41 and verifies size and content. */
TEST_F(ByteArrayOutputStreamTest, WriteSingleByte)
{
    stream_->write(std::byte{0x41});
    EXPECT_EQ(stream_->size(), 1);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 1);
    EXPECT_EQ(arr[0], std::byte{0x41});
}

/** @brief Write multiple bytes sequentially.
    @details Writes three individual bytes and checks size and each position. */
TEST_F(ByteArrayOutputStreamTest, WriteMultipleBytes)
{
    stream_->write(std::byte{0x01});
    stream_->write(std::byte{0x02});
    stream_->write(std::byte{0x03});
    EXPECT_EQ(stream_->size(), 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[1], std::byte{0x02});
    EXPECT_EQ(arr[2], std::byte{0x03});
}

/** @brief Write a full byte vector.
    @details Writes {0x10,0x20,0x30} and verifies size and content. */
TEST_F(ByteArrayOutputStreamTest, WriteVector)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    stream_->write(data);
    EXPECT_EQ(stream_->size(), 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x10});
    EXPECT_EQ(arr[1], std::byte{0x20});
    EXPECT_EQ(arr[2], std::byte{0x30});
}

/** @brief Write a sub-range of a byte vector.
    @details Writes offset=1, length=2 from {0x00,0x11,0x22,0x33} and expects {0x11,0x22}. */
TEST_F(ByteArrayOutputStreamTest, WriteVectorPartial)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    stream_->write(data, 1, 2);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x11});
    EXPECT_EQ(arr[1], std::byte{0x22});
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-2 buffer triggers std::out_of_range. */
TEST_F(ByteArrayOutputStreamTest, WriteVectorOffsetOutOfRange)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(stream_->write(data, 5, 1), std::out_of_range);
}

/** @brief Vector length exceeding buffer size throws.
    @details Length 5 at offset 1 on a size-2 buffer triggers std::out_of_range. */
TEST_F(ByteArrayOutputStreamTest, WriteVectorLengthExceeds)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(stream_->write(data, 1, 5), std::out_of_range);
}

/** @brief Write a raw C-style byte buffer.
    @details Writes a constexpr array {0x01,0x02,0x03} with explicit length. */
TEST_F(ByteArrayOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    stream_->write(buf, 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[2], std::byte{0x03});
}

/** @brief Null raw buffer with non-zero length throws.
    @details Passing nullptr with length 3 triggers std::invalid_argument. */
TEST_F(ByteArrayOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(stream_->write(nullptr, 3), std::invalid_argument);
}

/** @brief Empty vector write is a no-op.
    @details Writing an empty vector leaves the buffer empty. */
TEST_F(ByteArrayOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    stream_->write(empty);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

/** @brief Zero-length raw buffer write is a no-op even with null pointer.
    @details Null pointer with length 0 gracefully returns without writing. */
TEST_F(ByteArrayOutputStreamTest, WriteZeroLengthRawBufferDoesNothing)
{
    const std::byte* buf = nullptr;
    stream_->write(buf, 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

/** @brief writeTo transfers content to another stream.
    @details Writes {0x41,0x42} and uses writeTo to copy to a target stream. */
TEST_F(ByteArrayOutputStreamTest, WriteToTransfersContent)
{
    stream_->write(std::byte{0x41});
    stream_->write(std::byte{0x42});

    ByteArrayOutputStream target;
    stream_->writeTo(target);
    const auto arr = target.toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x41});
    EXPECT_EQ(arr[1], std::byte{0x42});
}

/** @brief writeTo on an empty stream does nothing.
    @details Calling writeTo immediately after construction leaves target empty. */
TEST_F(ByteArrayOutputStreamTest, WriteToEmptyDoesNothing)
{
    ByteArrayOutputStream target;
    stream_->writeTo(target);
    EXPECT_TRUE(target.toByteArray().empty());
}

/** @brief Reset clears all buffered content.
    @details Writes a byte, resets, and verifies size=0 and empty buffer. */
TEST_F(ByteArrayOutputStreamTest, ResetClearsContent)
{
    stream_->write(std::byte{0x41});
    ASSERT_EQ(stream_->size(), 1);
    stream_->reset();
    EXPECT_EQ(stream_->size(), 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

/** @brief ToString converts written bytes to a string.
    @details Writes {'H','i'} and expects toString to return "Hi". */
TEST_F(ByteArrayOutputStreamTest, ToString)
{
    stream_->write(std::byte{'H'});
    stream_->write(std::byte{'i'});
    EXPECT_EQ(stream_->toString(), "Hi");
}

/** @brief Default capacity is at least 32 bytes.
    @details Checks that the initial allocation is sufficient for small writes. */
TEST_F(ByteArrayOutputStreamTest, Capacity)
{
    const size_t cap = stream_->capacity();
    EXPECT_GE(cap, 32);
}

/** @brief Constructor with explicit initial size.
    @details Creates a stream with capacity 64 and verifies size=0 and capacity >= 64. */
TEST_F(ByteArrayOutputStreamTest, ConstructorWithSize)
{
    auto s = ByteArrayOutputStream(64);
    EXPECT_EQ(s.size(), 0);
    EXPECT_GE(s.capacity(), 64);
}

/** @brief Constructor throws on zero initial size.
    @details Passing 0 triggers std::invalid_argument. */
TEST_F(ByteArrayOutputStreamTest, ConstructorThrowsOnZeroSize)
{
    EXPECT_THROW(ByteArrayOutputStream(0), std::invalid_argument);
}

/** @brief Buffer expands automatically for large writes.
    @details Writes 1000 bytes and verifies all are stored correctly (first and last). */
TEST_F(ByteArrayOutputStreamTest, BufferExpandsAutomatically)
{
    const std::vector<std::byte> large(1000, std::byte{0xFF});
    stream_->write(large);
    EXPECT_EQ(stream_->size(), 1000);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 1000);
    EXPECT_EQ(arr[0], std::byte{0xFF});
    EXPECT_EQ(arr[999], std::byte{0xFF});
}

/** @brief Flush is a no-op for this in-memory stream.
    @details After write+flush, size remains unchanged. */
TEST_F(ByteArrayOutputStreamTest, FlushIsNoOp)
{
    stream_->write(std::byte{0x41});
    stream_->flush();
    EXPECT_EQ(stream_->size(), 1);
}

/** @brief Close is a no-op; the stream remains usable.
    @details After close, isClosed returns false and writes still succeed. */
TEST_F(ByteArrayOutputStreamTest, CloseIsNoOp)
{
    stream_->write(std::byte{0x41});
    stream_->close();
    EXPECT_FALSE(stream_->isClosed());
    stream_->write(std::byte{0x42});
    EXPECT_EQ(stream_->size(), 2);
}

/** @brief isClosed returns false for a new stream.
    @details Verifies the default closed state is false. */
TEST_F(ByteArrayOutputStreamTest, IsClosed)
{
    EXPECT_FALSE(stream_->isClosed());
}

/** @brief Writing after writeTo appends to existing content.
    @details Writes 0x01, writeTo target, writes 0x02, verifies both bytes present. */
TEST_F(ByteArrayOutputStreamTest, WriteAfterWriteTo)
{
    stream_->write(std::byte{0x01});
    ByteArrayOutputStream target;
    stream_->writeTo(target);
    stream_->write(std::byte{0x02});
    EXPECT_EQ(stream_->size(), 2);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[1], std::byte{0x02});
}
