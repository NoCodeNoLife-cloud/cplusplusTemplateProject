/**
 * @file FilterInputStreamTest.cc
 * @brief Unit tests for the FilterInputStream class
 * @details Tests cover delegation to wrapped input stream, including read
 *          operations, skip, available, mark/reset passthrough, and close propagation.
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <cppforge/io/reader/FilterInputStream.hpp>
#include <cppforge/io/reader/ByteArrayInputStream.hpp>

using namespace cppforge::io::reader;

/**
 * @brief Test fixture for FilterInputStreamTest tests
 */
class FilterInputStreamTest : public testing::Test
{
protected:
    std::vector<std::byte> data_;
    std::unique_ptr<FilterInputStream> stream_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}};
        auto inner = std::make_unique<ByteArrayInputStream>(data_);
        stream_ = std::make_unique<FilterInputStream>(std::move(inner));
    }
};

/**
 * @brief Test read() delegates to inner stream
 * @details Verifies single-byte read passthrough
 */
TEST_F(FilterInputStreamTest, ReadSingle)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read(buf) delegates to inner stream
 * @details Verifies buffer read passthrough
 */
TEST_F(FilterInputStreamTest, ReadIntoBuffer)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], std::byte{0x41});
    EXPECT_EQ(buf[1], std::byte{0x42});
    EXPECT_EQ(buf[2], std::byte{0x43});
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read(buf, off, len) delegates to inner stream
 * @details Verifies offset-based read passthrough
 */
TEST_F(FilterInputStreamTest, ReadIntoBufferWithOffset)
{
    std::vector<std::byte> buf(5);
    const auto n = stream_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], std::byte{0x41});
    EXPECT_EQ(buf[2], std::byte{0x42});
}

/**
 * @brief Test skip delegates to inner stream
 * @details Verifies skip passthrough
 */
TEST_F(FilterInputStreamTest, Skip)
{
    const auto skipped = stream_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

/**
 * @brief Test available delegates to inner stream
 * @details Verifies available passthrough
 */
TEST_F(FilterInputStreamTest, Available)
{
    EXPECT_EQ(stream_->available(), 3);
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 2);
}

/**
 * @brief Test markSupported delegates to inner stream
 * @details ByteArrayInputStream supports mark, so FilterInputStream should too
 */
TEST_F(FilterInputStreamTest, MarkSupported)
{
    EXPECT_TRUE(stream_->markSupported());
}

/**
 * @brief Test mark and reset passthrough
 * @details Verifies mark/reset are delegated correctly
 */
TEST_F(FilterInputStreamTest, MarkAndReset)
{
    EXPECT_TRUE(stream_->markSupported());
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

/**
 * @brief Test close propagates to inner stream
 * @details Closing the filter should close the underlying stream
 */
TEST_F(FilterInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/**
 * @brief Test read after close
 * @details After close, read should return EOF (-1)
 */
TEST_F(FilterInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test available after close returns 0
 * @details After close, available should be 0
 */
TEST_F(FilterInputStreamTest, AvailableAfterClose)
{
    stream_->close();
    EXPECT_EQ(stream_->available(), 0);
}

/**
 * @brief Test skip after close
 * @details After close, skip should return 0
 */
TEST_F(FilterInputStreamTest, SkipAfterClose)
{
    stream_->close();
    const auto skipped = stream_->skip(5);
    EXPECT_EQ(skipped, 0);
}

/**
 * @brief Test close is idempotent
 * @details Multiple close calls should not throw
 */
TEST_F(FilterInputStreamTest, CloseIsIdempotent)
{
    stream_->close();
    EXPECT_NO_THROW(stream_->close());
}

/**
 * @brief Test read(buf) propagates EOF from inner stream
 * @details When inner stream reaches end, outer should also report EOF
 */
TEST_F(FilterInputStreamTest, ReadBufPropagatesEof)
{
    std::vector<std::byte> buf(5);
    const auto n = stream_->read(buf);
    EXPECT_EQ(n, 3);
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read single byte on exhausted stream
 * @details Reading beyond the data should return EOF marker
 */
TEST_F(FilterInputStreamTest, ReadAfterExhaustion)
{
    (void)stream_->read();
    (void)stream_->read();
    (void)stream_->read();
    EXPECT_TRUE(stream_->isEof());

    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
}
