/**
 * @file AbstractInputStreamTest.cc
 * @brief Unit tests for the AbstractInputStream base class
 * @details Tests cover the default implementations of read(buf), read(buf,off,len),
 *          skip, mark, markSupported, reset, and EOF tracking via a minimal stub.
 */

#include <gtest/gtest.h>
#include <vector>

#include <cppforge/io/reader/AbstractInputStream.hpp>

using namespace cppforge::io::reader;

/**
 * @brief Minimal concrete stub for testing AbstractInputStream base class behavior.
 * @details Only overrides read(), available(), close(), and isClosed().
 *          All other methods (read(buf), read(buf,off,len), skip, mark, reset)
 *          are tested through AbstractInputStream's default implementations.
 */
class TestInputStream final : public AbstractInputStream
{
public:
    explicit TestInputStream(std::vector<std::byte> data)
        : data_(std::move(data)), pos_(0), closed_(false)
    {
    }

    using AbstractInputStream::read;

    [[nodiscard]] std::byte read() override
    {
        if (closed_ || pos_ >= data_.size())
        {
            setEof();
            return static_cast<std::byte>(-1);
        }
        return data_[pos_++];
    }

    [[nodiscard]] size_t available() override
    {
        if (closed_)
        {
            return 0;
        }
        return data_.size() - pos_;
    }

    void close() override
    {
        closed_ = true;
        setEof();
    }

    [[nodiscard]] bool isClosed() const override
    {
        return closed_;
    }

private:
    std::vector<std::byte> data_;
    size_t pos_;
    bool closed_;
};

/**
 * @brief Test fixture for AbstractInputStreamTest tests
 */
class AbstractInputStreamTest : public testing::Test
{
protected:
    std::vector<std::byte> data_;
    std::unique_ptr<TestInputStream> stream_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}, std::byte{0x44}};
        stream_ = std::make_unique<TestInputStream>(data_);
    }
};

/**
 * @brief Test read(buf) via base class implementation
 * @details The base read(buf) delegates to read(buf, 0, buf.size())
 */
TEST_F(AbstractInputStreamTest, ReadIntoBuffer)
{
    std::vector<std::byte> buf(4);
    const auto n = stream_->read(buf);
    EXPECT_EQ(n, 4);
    EXPECT_EQ(buf[0], std::byte{0x41});
    EXPECT_EQ(buf[1], std::byte{0x42});
    EXPECT_EQ(buf[2], std::byte{0x43});
    EXPECT_EQ(buf[3], std::byte{0x44});
    // EOF not yet set â€?base read(buf) resets eof_ and no extra read() triggers it
    EXPECT_FALSE(stream_->isEof());
    // One more read() triggers EOF
    EXPECT_EQ(stream_->read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read(buf, offset, len) via base class implementation
 * @details The base method calls read() in a loop for len bytes
 */
TEST_F(AbstractInputStreamTest, ReadIntoBufferWithOffset)
{
    std::vector<std::byte> buf(6);
    const auto n = stream_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], std::byte{0x41});
    EXPECT_EQ(buf[2], std::byte{0x42});
}

/**
 * @brief Test read(buf, offset, len) with zero length
 * @details Reading 0 bytes should return 0 immediately
 */
TEST_F(AbstractInputStreamTest, ReadZeroLenReturnsZero)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test read(buf) with empty buffer
 * @details Reading into an empty buffer returns 0
 */
TEST_F(AbstractInputStreamTest, ReadIntoEmptyBuffer)
{
    std::vector<std::byte> buf;
    const auto n = stream_->read(buf);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test skip via base class implementation
 * @details The base skip(n) calls read() in a loop for n bytes
 */
TEST_F(AbstractInputStreamTest, Skip)
{
    const auto skipped = stream_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

/**
 * @brief Test skip beyond available data
 * @details Skipping more than available calls read() until eof.
 *          The base skip() increments counter even for the read() that triggers EOF.
 */
TEST_F(AbstractInputStreamTest, SkipBeyondEnd)
{
    // With 4 bytes, skip reads all 4 plus one more that triggers EOF
    const auto skipped = stream_->skip(100);
    EXPECT_EQ(skipped, 5);
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test skip with zero
 * @details Skipping 0 bytes should return 0
 */
TEST_F(AbstractInputStreamTest, SkipZero)
{
    const auto skipped = stream_->skip(0);
    EXPECT_EQ(skipped, 0);
}

/**
 * @brief Test available returns correct count
 * @details Verifies available() decreases as bytes are read
 */
TEST_F(AbstractInputStreamTest, Available)
{
    EXPECT_EQ(stream_->available(), 4);
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 3);
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 2);
}

/**
 * @brief Test mark is not supported by default
 * @details The base markSupported() returns false
 */
TEST_F(AbstractInputStreamTest, MarkSupported_DefaultFalse)
{
    EXPECT_FALSE(stream_->markSupported());
}

/**
 * @brief Test mark throws by default
 * @details The base mark() throws runtime_error
 */
TEST_F(AbstractInputStreamTest, Mark_ThrowsByDefault)
{
    EXPECT_THROW(stream_->mark(10), std::runtime_error);
}

/**
 * @brief Test reset throws by default
 * @details The base reset() throws runtime_error
 */
TEST_F(AbstractInputStreamTest, Reset_ThrowsByDefault)
{
    EXPECT_THROW(stream_->reset(), std::runtime_error);
}

/**
 * @brief Test isEof returns false initially
 * @details EOF flag should start as false
 */
TEST_F(AbstractInputStreamTest, InitialEofIsFalse)
{
    EXPECT_FALSE(stream_->isEof());
}

/**
 * @brief Test isEof after reading all data
 * @details EOF should be set when all bytes are consumed (on the extra read beyond end)
 */
TEST_F(AbstractInputStreamTest, EofAfterReadingAll)
{
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_FALSE(stream_->isEof());
        (void)stream_->read();
    }
    // 5th read triggers EOF
    EXPECT_EQ(stream_->read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test close sets EOF
 * @details Closing the stream should set the EOF flag
 */
TEST_F(AbstractInputStreamTest, CloseSetsEof)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read after close
 * @details After close, read should return EOF (-1)
 */
TEST_F(AbstractInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test read(buf) after close
 * @details Base read(buf) resets eof_ then calls read() which triggers EOF again.
 *          Returns 1 because the single read() call is counted.
 */
TEST_F(AbstractInputStreamTest, ReadBufAfterClose)
{
    stream_->close();
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf);
    // Base class counts the single read() call that triggers EOF
    EXPECT_EQ(n, 1);
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test skip after close
 * @details Base skip resets eof_ then calls read() which triggers EOF again.
 *          Returns 1 because the single read() call is counted.
 */
TEST_F(AbstractInputStreamTest, SkipAfterClose)
{
    stream_->close();
    const auto skipped = stream_->skip(5);
    EXPECT_EQ(skipped, 1);
    EXPECT_TRUE(stream_->isEof());
}

/**
 * @brief Test available after close returns 0
 * @details After close, available should return 0
 */
TEST_F(AbstractInputStreamTest, AvailableAfterClose)
{
    stream_->close();
    EXPECT_EQ(stream_->available(), 0);
}

/**
 * @brief Test read(buf, off, len) with invalid offset throws
 * @details The base implementation validates buffer bounds
 */
TEST_F(AbstractInputStreamTest, ReadInvalidOffsetThrows)
{
    std::vector<std::byte> buf(3);
    EXPECT_THROW(stream_->read(buf, 5, 1), std::out_of_range);
}

/**
 * @brief Test read(buf, off, len) with overflow length throws
 * @details The base implementation validates length against buffer capacity
 */
TEST_F(AbstractInputStreamTest, ReadOverflowLenThrows)
{
    std::vector<std::byte> buf(3);
    EXPECT_THROW(stream_->read(buf, 0, 10), std::out_of_range);
}

/**
 * @brief Test read(buf) on empty stream
 * @details Reading from a stream with no data
 */
TEST_F(AbstractInputStreamTest, ReadFromEmptyStream)
{
    TestInputStream empty(std::vector<std::byte>{});
    // Initially not eof (eof_ starts false)
    EXPECT_FALSE(empty.isEof());
    // First read triggers EOF
    EXPECT_EQ(empty.read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(empty.isEof());
}

/**
 * @brief Test multiple read(buf) calls consume stream sequentially
 * @details Verifies sequential read(buf) calls work correctly
 */
TEST_F(AbstractInputStreamTest, SequentialReadBuffer)
{
    std::vector<std::byte> buf1(2);
    auto n1 = stream_->read(buf1);
    EXPECT_EQ(n1, 2);
    EXPECT_EQ(buf1[0], std::byte{0x41});
    EXPECT_EQ(buf1[1], std::byte{0x42});

    std::vector<std::byte> buf2(2);
    auto n2 = stream_->read(buf2);
    EXPECT_EQ(n2, 2);
    EXPECT_EQ(buf2[0], std::byte{0x43});
    EXPECT_EQ(buf2[1], std::byte{0x44});

    // Not yet eof â€?an extra read() triggers it
    EXPECT_FALSE(stream_->isEof());
    EXPECT_EQ(stream_->read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}
