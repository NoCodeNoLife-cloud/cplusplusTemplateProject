#include <gtest/gtest.h>
#include <vector>

#include "io/reader/ByteArrayInputStream.hpp"

using namespace common::io::reader;

/// @brief Test fixture for ByteArrayInputStream tests.
class ByteArrayInputStreamTest : public testing::Test
{
protected:
    std::vector<std::byte> data_;
    std::unique_ptr<ByteArrayInputStream> stream_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}};
        stream_ = std::make_unique<ByteArrayInputStream>(data_);
    }
};

/** @brief Test reading single bytes from the stream. @details Verifies that sequential read() calls return the expected bytes followed by EOF. */
TEST_F(ByteArrayInputStreamTest, ReadSingleByte)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
    EXPECT_TRUE(stream_->isEof());
}

/** @brief Test reading into a buffer. @details Verifies that read(buf) fills the buffer with all available bytes and returns the count. */
TEST_F(ByteArrayInputStreamTest, ReadIntoBuffer)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], std::byte{0x41});
    EXPECT_EQ(buf[1], std::byte{0x42});
    EXPECT_EQ(buf[2], std::byte{0x43});
}

/** @brief Test reading into a buffer with an offset. @details Verifies that read(buf, off, len) inserts data starting at the specified buffer offset. */
TEST_F(ByteArrayInputStreamTest, ReadIntoBufferWithOffset)
{
    std::vector<std::byte> buf(5);
    const auto n = stream_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], std::byte{0x41});
    EXPECT_EQ(buf[2], std::byte{0x42});
}

/** @brief Test available() returns the remaining byte count. @details Verifies that available() reports the correct number of bytes and decreases after a read. */
TEST_F(ByteArrayInputStreamTest, Available)
{
    EXPECT_EQ(stream_->available(), 3);
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 2);
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position by the specified number of bytes. */
TEST_F(ByteArrayInputStreamTest, Skip)
{
    const auto skipped = stream_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

/** @brief Test mark and reset functionality. @details Verifies that mark() records a position and reset() restores the stream to that position. */
TEST_F(ByteArrayInputStreamTest, MarkAndReset)
{
    EXPECT_TRUE(stream_->markSupported());
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

/** @brief Test the close operation. @details Verifies that close() transitions the stream to closed state and marks EOF. */
TEST_F(ByteArrayInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
    EXPECT_TRUE(stream_->isEof());
}

/** @brief Test read() after the stream is closed. @details Verifies that read() returns EOF (-1) and isEof() returns true on a closed stream. */
TEST_F(ByteArrayInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/** @brief Test EOF behavior on an empty buffer. @details Verifies that an empty ByteArrayInputStream immediately reports EOF and read() returns -1. */
TEST_F(ByteArrayInputStreamTest, EofOnEmptyBuffer)
{
    ByteArrayInputStream empty(std::vector<std::byte>{});
    EXPECT_TRUE(empty.isEof());
    EXPECT_EQ(empty.read(), static_cast<std::byte>(-1));
}

/** @brief Test read with zero length. @details Verifies that read(buf, off, 0) returns 0 without reading any bytes. */
TEST_F(ByteArrayInputStreamTest, ReadZeroLenReturnsZero)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
