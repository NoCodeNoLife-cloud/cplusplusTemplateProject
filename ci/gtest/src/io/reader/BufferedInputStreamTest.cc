#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/BufferedInputStream.hpp"
#include "io/reader/ByteArrayInputStream.hpp"

using namespace common::io::reader;

/// @brief Test fixture for BufferedInputStream tests.
class BufferedInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<BufferedInputStream> stream_;

    void SetUp() override
    {
        auto inner = std::make_unique<ByteArrayInputStream>(
            std::vector{std::byte{0x10}, std::byte{0x20}, std::byte{0x30}, std::byte{0x40}, std::byte{0x50}});
        stream_ = std::make_unique<BufferedInputStream>(std::move(inner));
    }
};

/** @brief Test reading a single byte from the stream. @details Verifies that sequential single-byte read() calls return the expected byte values from the underlying ByteArrayInputStream. */
TEST_F(BufferedInputStreamTest, ReadSingleByte)
{
    EXPECT_EQ(stream_->read(), std::byte{0x10});
    EXPECT_EQ(stream_->read(), std::byte{0x20});
}

/** @brief Test reading multiple bytes into a buffer. @details Verifies that read(buf, off, len) fills the buffer with the correct bytes and returns the count of bytes read. */
TEST_F(BufferedInputStreamTest, ReadMultipleBytes)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], std::byte{0x10});
    EXPECT_EQ(buf[1], std::byte{0x20});
    EXPECT_EQ(buf[2], std::byte{0x30});
}

/** @brief Test reading all data with a small intermediate buffer. @details Verifies that chunked read() calls using a buffer smaller than the total stream correctly accumulate the full 5-byte dataset. */
TEST_F(BufferedInputStreamTest, ReadAllWithSmallBuffer)
{
    std::vector<std::byte> small(2);
    std::vector<std::byte> result;
    size_t n;
    while ((n = stream_->read(small, 0, 2)) > 0)
    {
        result.insert(result.end(), small.begin(), small.begin() + static_cast<std::ptrdiff_t>(n));
    }
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], std::byte{0x10});
    EXPECT_EQ(result[4], std::byte{0x50});
}

/** @brief Test available() returns the remaining byte count. @details Verifies that available() reports the correct number of unread bytes from the underlying stream. */
TEST_F(BufferedInputStreamTest, Available)
{
    const auto avail = stream_->available();
    EXPECT_EQ(avail, 5);
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position by the specified number of bytes. */
TEST_F(BufferedInputStreamTest, Skip)
{
    const auto skipped = stream_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(stream_->read(), std::byte{0x40});
}

/** @brief Test mark and reset functionality. @details Verifies that mark() records a position and reset() restores the stream to that position for re-reading. */
TEST_F(BufferedInputStreamTest, MarkAndReset)
{
    EXPECT_TRUE(stream_->markSupported());
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

/** @brief Test the close operation. @details Verifies that close() transitions the stream to the closed state. */
TEST_F(BufferedInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

/** @brief Test read() after the stream has been closed. @details Verifies that read() returns EOF (-1) and isEof() returns true when called on a closed stream. */
TEST_F(BufferedInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/** @brief Test construction with a custom buffer size. @details Verifies that BufferedInputStream accepts an explicit buffer size smaller than the data and still reads correctly. */
TEST_F(BufferedInputStreamTest, CustomBufferSize)
{
    auto inner = std::make_unique<ByteArrayInputStream>(std::vector{std::byte{0x01}, std::byte{0x02}});
    BufferedInputStream custom(std::move(inner), 4);
    EXPECT_EQ(custom.read(), std::byte{0x01});
}

/** @brief Test buffer read after the stream has been closed. @details Verifies that read(buf, off, len) returns 0 when called on a closed stream. */
TEST_F(BufferedInputStreamTest, ReadBufAfterClose)
{
    stream_->close();
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 0);
}

/** @brief Test skip beyond the end of the stream. @details Verifies that skip() returns the actual number of bytes skipped (5) when the requested amount (100) exceeds available data. */
TEST_F(BufferedInputStreamTest, SkipBeyondEnd)
{
    const auto skipped = stream_->skip(100);
    EXPECT_EQ(skipped, 5);
}

/** @brief Test skip after the stream has been closed. @details Verifies that skip() returns 0 when called on a closed stream. */
TEST_F(BufferedInputStreamTest, SkipAfterClose)
{
    stream_->close();
    const auto skipped = stream_->skip(5);
    EXPECT_EQ(skipped, 0);
}

/** @brief Test available() after reading some bytes. @details Verifies that available() decreases by the number of bytes consumed via read(). */
TEST_F(BufferedInputStreamTest, AvailableAfterRead)
{
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 4);
}

/** @brief Test available() after the stream has been closed. @details Verifies that available() returns 0 on a closed stream. */
TEST_F(BufferedInputStreamTest, AvailableAfterClose)
{
    stream_->close();
    EXPECT_EQ(stream_->available(), 0);
}

/** @brief Test that close() is idempotent. @details Verifies that calling close() multiple times does not throw any exception. */
TEST_F(BufferedInputStreamTest, CloseIsIdempotent)
{
    stream_->close();
    EXPECT_NO_THROW(stream_->close());
}

/** @brief Test read() after all bytes have been consumed. @details Verifies that read() returns EOF (-1) once the entire stream has been read. */
TEST_F(BufferedInputStreamTest, ReadSingleAfterExhaustion)
{
    for (int i = 0; i < 5; ++i) (void)stream_->read();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
}

/** @brief Test reset after a mark. @details Verifies that reset() restores the stream position to the location recorded by the preceding mark() call. */
TEST_F(BufferedInputStreamTest, ResetAfterMark)
{
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

/** @brief Test multiple mark/reset cycles. @details Verifies that the stream correctly handles repeated mark() and reset() sequences without data corruption. */
TEST_F(BufferedInputStreamTest, MultipleMarkReset)
{
    stream_->mark(10);
    (void)stream_->read();
    stream_->reset();
    (void)stream_->read();
    stream_->mark(10);
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x20});
}

/** @brief Test buffer read after stream exhaustion. @details Verifies that read(buf, off, len) returns 0 when all data has been consumed by a prior read. */
TEST_F(BufferedInputStreamTest, ReadBufAfterExhaustion)
{
    std::vector<std::byte> buf(5);
    auto n = stream_->read(buf, 0, 5);
    EXPECT_EQ(n, 5);
    n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 0);
}

/** @brief Test skip with zero length. @details Verifies that skip(0) returns 0 without advancing the stream read position. */
TEST_F(BufferedInputStreamTest, SkipZero)
{
    EXPECT_EQ(stream_->skip(0), 0);
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

/** @brief Test skip with a negative value. @details Verifies that skip(-1) returns 0 and does not advance the read position. */
TEST_F(BufferedInputStreamTest, SkipNegative)
{
    EXPECT_EQ(stream_->skip(-1), 0);
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

/** @brief Test reset without a prior mark. @details Verifies that calling reset() without a preceding mark() does not throw an exception. */
TEST_F(BufferedInputStreamTest, ResetWithoutMark)
{
    stream_->read();
    EXPECT_NO_THROW(stream_->reset());
}

/** @brief Test mark with a negative read limit. @details Verifies that mark(-1) is accepted without throwing and that reset() still restores the marked position. */
TEST_F(BufferedInputStreamTest, MarkWithNegativeReadlimit)
{
    EXPECT_NO_THROW(stream_->mark(-1));
    stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

/** @brief Test available() on an empty stream. @details Verifies that a BufferedInputStream wrapping an empty ByteArrayInputStream reports zero available bytes. */
TEST_F(BufferedInputStreamTest, AvailableOnEmptyStream)
{
    auto inner = std::make_unique<ByteArrayInputStream>(std::vector<std::byte>{});
    auto stream = std::make_unique<BufferedInputStream>(std::move(inner));
    EXPECT_EQ(stream->available(), 0);
}

/** @brief Test reset after exceeding the mark read limit. @details Verifies that reset() still succeeds without throwing after reading more bytes than the limit passed to mark(). */
TEST_F(BufferedInputStreamTest, MarkExceededResetFails)
{
    stream_->mark(1);
    stream_->read();
    stream_->read();
    EXPECT_NO_THROW(stream_->reset());
}
