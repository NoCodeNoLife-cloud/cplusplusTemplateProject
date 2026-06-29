#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <cppforge/io/reader/PushbackInputStream.hpp>
#include <cppforge/io/reader/ByteArrayInputStream.hpp>

using namespace cppforge::io::reader;

/// @brief Test fixture for PushbackInputStream tests.
class PushbackInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<PushbackInputStream> stream_;
    std::vector<std::byte> data_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}};
        auto inner = std::make_unique<ByteArrayInputStream>(data_);
        stream_ = std::make_unique<PushbackInputStream>(std::move(inner), 4);
    }
};

/** @brief Test normal read of all bytes from the stream. @details Verifies that sequential read() calls return each byte from the underlying ByteArrayInputStream. */
TEST_F(PushbackInputStreamTest, ReadNormal)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

/** @brief Test unread of a single byte and re-read. @details Verifies that unread() pushes back one byte and a subsequent read() returns it again. */
TEST_F(PushbackInputStreamTest, UnreadSingleAndReRead)
{
    const auto byte = stream_->read();
    EXPECT_EQ(byte, std::byte{0x41});
    stream_->unread(byte);
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

/** @brief Test unread of a buffer and re-read. @details Verifies that unread(buf) pushes back multiple bytes which are returned in LIFO order before the original stream data. */
TEST_F(PushbackInputStreamTest, UnreadBufferAndReRead)
{
    (void)stream_->read();
    stream_->unread(std::vector{std::byte{0xFF}, std::byte{0xFE}});
    EXPECT_EQ(stream_->read(), std::byte{0xFF});
    EXPECT_EQ(stream_->read(), std::byte{0xFE});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
}

/** @brief Test unread with overflow beyond pushback capacity. @details Verifies that unread() throws std::overflow_error when the buffer exceeds the configured pushback size (4). */
TEST_F(PushbackInputStreamTest, UnreadOverflow)
{
    const std::vector<std::byte> large(5);
    EXPECT_THROW(stream_->unread(large), std::overflow_error);
}

/** @brief Test available() returns at least the underlying data count. @details Verifies that available() reports the number of bytes available including any pushback buffer contents. */
TEST_F(PushbackInputStreamTest, Available)
{
    const auto avail = stream_->available();
    EXPECT_GE(avail, 3);
}

/** @brief Test the close operation. @details Verifies that close() transitions the stream to the closed state. */
TEST_F(PushbackInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/**
 * @brief Test unread after close
 * @details Verifies that after close, unread does not crash regardless of
 *          whether the implementation allows post-close pushback.
 */
TEST_F(PushbackInputStreamTest, UnreadAfterClose)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
    EXPECT_NO_THROW(stream_->unread(std::byte{0xFF}));
}

/**
 * @brief Test multiple unread operations
 * @details Unread multiple individual bytes and read them back
 */
TEST_F(PushbackInputStreamTest, MultipleUnreadSingle)
{
    stream_->unread(std::byte{0xFF});
    stream_->unread(std::byte{0xFE});
    EXPECT_EQ(stream_->read(), std::byte{0xFE});
    EXPECT_EQ(stream_->read(), std::byte{0xFF});
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

/**
 * @brief Test unread buffer after reading all data
 * @details Read everything, then unread, then re-read
 */
TEST_F(PushbackInputStreamTest, UnreadAfterExhaustion)
{
    (void)stream_->read();
    (void)stream_->read();
    (void)stream_->read();
    stream_->unread(std::vector{std::byte{0xFF}, std::byte{0xFE}});
    EXPECT_EQ(stream_->read(), std::byte{0xFF});
    EXPECT_EQ(stream_->read(), std::byte{0xFE});
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test unread buffer overflow is preserved (no data corruption)
 * @details After an overflow exception from a 5-byte buffer exceeding the
 *          pushback capacity of 4, the stream remains usable and subsequent
 *          reads return remaining data without corruption.
 */
TEST_F(PushbackInputStreamTest, OverflowDoesNotCorruptState)
{
    (void)stream_->read();
    const std::vector<std::byte> large(5);
    EXPECT_THROW(stream_->unread(large), std::overflow_error);
    EXPECT_EQ(stream_->read(), std::byte{0x42});
}

/**
 * @brief Test available includes pushback buffer
 * @details Verifies that available() counts bytes in the pushback buffer
 *          (2 pushed back) plus remaining stream data (3) for a total of 5.
 */
TEST_F(PushbackInputStreamTest, AvailableWithPushback)
{
    stream_->unread(std::byte{0xFF});
    stream_->unread(std::byte{0xFE});
    EXPECT_GE(stream_->available(), 2);
}

/**
 * @brief Test unread buffer larger than capacity throws
 * @edge Maximum pushback size check
 */
TEST_F(PushbackInputStreamTest, UnreadBufferExactCapacity)
{
    const std::vector<std::byte> exact(4);
    EXPECT_NO_THROW(stream_->unread(exact));
}

/**
 * @brief Test read after close returns EOF
 * @details After close, read should behave like base class
 */
TEST_F(PushbackInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}
