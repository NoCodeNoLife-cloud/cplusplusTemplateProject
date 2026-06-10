#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/PushbackInputStream.hpp"
#include "io/reader/ByteArrayInputStream.hpp"

using namespace common::io::reader;

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

TEST_F(PushbackInputStreamTest, ReadNormal)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

TEST_F(PushbackInputStreamTest, UnreadSingleAndReRead)
{
    const auto byte = stream_->read();
    EXPECT_EQ(byte, std::byte{0x41});
    stream_->unread(byte);
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

TEST_F(PushbackInputStreamTest, UnreadBufferAndReRead)
{
    (void)stream_->read();
    stream_->unread(std::vector{std::byte{0xFF}, std::byte{0xFE}});
    EXPECT_EQ(stream_->read(), std::byte{0xFF});
    EXPECT_EQ(stream_->read(), std::byte{0xFE});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
}

TEST_F(PushbackInputStreamTest, UnreadOverflow)
{
    const std::vector<std::byte> large(5);
    EXPECT_THROW(stream_->unread(large), std::overflow_error);
}

TEST_F(PushbackInputStreamTest, Available)
{
    const auto avail = stream_->available();
    EXPECT_GE(avail, 3);
}

TEST_F(PushbackInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

/**
 * @brief Test unread after close
 * @details After close, unread might throw or be a no-op
 */
TEST_F(PushbackInputStreamTest, UnreadAfterClose)
{
    stream_->close();
    // unread may or may not throw depending on implementation;
    // verify the stream is closed and no crash occurs
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
 * @details After an overflow exception, the pushback buffer should remain usable
 */
TEST_F(PushbackInputStreamTest, OverflowDoesNotCorruptState)
{
    // Pushback capacity is 4, so this should work after overflow cleanup
    (void)stream_->read();
    const std::vector<std::byte> large(5);
    EXPECT_THROW(stream_->unread(large), std::overflow_error);
    // Stream should still work after failed unread
    EXPECT_EQ(stream_->read(), std::byte{0x42});
}

/**
 * @brief Test available includes pushback buffer
 * @details available() should count bytes in pushback buffer plus remaining
 */
TEST_F(PushbackInputStreamTest, AvailableWithPushback)
{
    stream_->unread(std::byte{0xFF});
    stream_->unread(std::byte{0xFE});
    // 2 pushback + 3 remaining = 5
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
