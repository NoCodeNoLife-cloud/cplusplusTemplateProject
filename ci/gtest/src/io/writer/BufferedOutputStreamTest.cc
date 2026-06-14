/**
 * @file BufferedOutputStreamTest.cc
 * @brief Unit tests for BufferedOutputStream
 * @details Tests cover write operations, buffer flushing, close, and edge cases using ByteArrayOutputStream.
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/writer/BufferedOutputStream.hpp"
#include "io/writer/ByteArrayOutputStream.hpp"

using namespace common::io::writer;

/// @brief Test fixture for BufferedOutputStream tests using ByteArrayOutputStream.
class BufferedOutputStreamTest : public testing::Test
{
protected:
    ByteArrayOutputStream* inner_ = nullptr;
    std::unique_ptr<BufferedOutputStream> stream_;

    void SetUp() override
    {
        auto inner = std::make_unique<ByteArrayOutputStream>();
        inner_ = inner.get();
        stream_ = std::make_unique<BufferedOutputStream>(std::move(inner));
    }
};

/** @brief Write a single byte through the buffer.
    @details Writes 0x41, flushes, and verifies it reaches the inner ByteArrayOutputStream. */
TEST_F(BufferedOutputStreamTest, WriteSingleByte)
{
    stream_->write(std::byte{0x41});
    stream_->flush();
    auto data = inner_->toByteArray();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], std::byte{0x41});
}

/** @brief Write multiple bytes through the buffer.
    @details Writes {0x10,0x20,0x30}, flushes, and checks each position in the output. */
TEST_F(BufferedOutputStreamTest, WriteMultipleBytes)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    stream_->write(data, 0, data.size());
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{0x10});
    EXPECT_EQ(result[1], std::byte{0x20});
    EXPECT_EQ(result[2], std::byte{0x30});
}

/** @brief Write a raw byte buffer through the buffer.
    @details Writes a constexpr array {0x01,0x02,0x03} with explicit length and flushes. */
TEST_F(BufferedOutputStreamTest, WriteRawBuffer)
{
    const std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    stream_->write(buf, 3);
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{0x01});
}

/** @brief Flush empties the internal buffer.
    @details After write+flush, getBufferedDataSize returns 0. */
TEST_F(BufferedOutputStreamTest, FlushEmptiesBuffer)
{
    stream_->write(std::byte{0xFF});
    stream_->flush();
    EXPECT_EQ(stream_->getBufferedDataSize(), 0);
}

/** @brief Close marks the stream as closed.
    @details Verifies isClosed returns true after close. */
TEST_F(BufferedOutputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/** @brief Close is idempotent.
    @details Calling close twice does not throw and leaves the stream closed. */
TEST_F(BufferedOutputStreamTest, CloseIsIdempotent)
{
    stream_->close();
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/** @brief Default buffer size is 8192 bytes.
    @details Checks the default construction buffer capacity before any write. */
TEST_F(BufferedOutputStreamTest, GetBufferSize)
{
    EXPECT_EQ(stream_->getBufferSize(), 8192);
}

/** @brief Constructor accepts a custom buffer size.
    @details Creates a BufferedOutputStream with buffer size 64 and verifies the setting. */
TEST_F(BufferedOutputStreamTest, CustomBufferSize)
{
    auto inner = std::make_unique<ByteArrayOutputStream>();
    auto custom = std::make_unique<BufferedOutputStream>(std::move(inner), 64);
    EXPECT_EQ(custom->getBufferSize(), 64);
}

/** @brief Large write triggers automatic flush when buffer is exceeded.
    @details Writing 16384 bytes forces flush when the 8192-byte buffer is filled. */
TEST_F(BufferedOutputStreamTest, LargeWriteTriggersAutoFlush)
{
    std::vector<std::byte> large(16384, std::byte{0xAA});
    stream_->write(large, 0, large.size());
    auto result = inner_->toByteArray();
    ASSERT_GE(result.size(), 8192);
}

/** @brief Write after close leaves the stream closed.
    @details After close, isClosed returns true. */
TEST_F(BufferedOutputStreamTest, WriteAfterClose)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/** @brief Write vector with offset and length.
    @details Writes buffer[1..2] from {0x00,0x11,0x22,0x33} and verifies correct sub-range. */
TEST_F(BufferedOutputStreamTest, WriteByteArrayWithOffset)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    stream_->write(data, 1, 2);
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], std::byte{0x11});
    EXPECT_EQ(result[1], std::byte{0x22});
}

/** @brief Constructor throws on null inner stream.
    @details Passing nullptr triggers std::invalid_argument. */
TEST_F(BufferedOutputStreamTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(BufferedOutputStream(nullptr), std::invalid_argument);
}

/** @brief Constructor throws on zero buffer size.
    @details Passing 0 as the buffer size triggers std::invalid_argument. */
TEST_F(BufferedOutputStreamTest, ConstructorThrowsOnZeroSize)
{
    auto inner = std::make_unique<ByteArrayOutputStream>();
    EXPECT_THROW(BufferedOutputStream(std::move(inner), 0), std::invalid_argument);
}