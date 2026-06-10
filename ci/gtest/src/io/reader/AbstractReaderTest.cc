/**
 * @file AbstractReaderTest.cc
 * @brief Unit tests for the AbstractReader base class
 * @details Tests cover the default implementations of read(), read(buf), skip,
 *          markSupported, ready, and close via a minimal concrete stub.
 */

#include <gtest/gtest.h>
#include <vector>

#include "io/reader/AbstractReader.hpp"

using namespace common::io::reader;

/**
 * @brief Minimal concrete stub for testing AbstractReader base class behavior.
 * @details Only overrides read(buf,off,len), close, mark, reset, isClosed.
 *          All other methods (read(), read(buf), skip, markSupported, ready)
 *          are tested through AbstractReader's default implementations.
 */
class TestReader final : public AbstractReader
{
public:
    explicit TestReader(std::vector<char> data)
        : data_(std::move(data)), pos_(0), closed_(false)
    {
    }

    using AbstractReader::read;

    int read(std::vector<char>& cBuf, size_t off, size_t len) override
    {
        if (closed_)
        {
            return -1;
        }
        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("Buffer overflow detected");
        }
        if (pos_ >= data_.size())
        {
            return -1;
        }
        size_t toRead = std::min(len, data_.size() - pos_);
        for (size_t i = 0; i < toRead; ++i)
        {
            cBuf[off + i] = data_[pos_++];
        }
        return static_cast<int>(toRead);
    }

    void mark(size_t readAheadLimit) override
    {
        mark_ = pos_;
    }

    void reset() override
    {
        if (mark_ == static_cast<size_t>(-1))
        {
            throw std::runtime_error("No mark set");
        }
        pos_ = mark_;
    }

    void close() override
    {
        closed_ = true;
    }

    [[nodiscard]] bool isClosed() const override
    {
        return closed_;
    }

private:
    std::vector<char> data_;
    size_t pos_{0};
    size_t mark_{static_cast<size_t>(-1)};
    bool closed_{false};
};

/**
 * @brief Test fixture for AbstractReaderTest tests
 */
class AbstractReaderTest : public testing::Test
{
protected:
    std::vector<char> data_;
    std::unique_ptr<TestReader> reader_;

    void SetUp() override
    {
        data_ = {'H', 'e', 'l', 'l', 'o'};
        reader_ = std::make_unique<TestReader>(data_);
    }
};

/**
 * @brief Test read() single character via base class
 * @details The base read() calls read(buf, 0, 1) internally
 */
TEST_F(AbstractReaderTest, ReadSingleCharacter)
{
    auto ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'H');

    ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'e');
}

/**
 * @brief Test read() returns nullopt at end of stream
 * @details When all data is consumed, read() should return nullopt
 */
TEST_F(AbstractReaderTest, ReadReturnsNulloptAtEnd)
{
    for (int i = 0; i < 5; ++i)
    {
        (void)reader_->read();
    }
    auto ch = reader_->read();
    EXPECT_FALSE(ch.has_value());
}

/**
 * @brief Test read(buf) via base class implementation
 * @details The base read(buf) delegates to read(buf, 0, buf.size())
 */
TEST_F(AbstractReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(5);
    const int n = reader_->read(buf);
    EXPECT_EQ(n, 5);
    EXPECT_EQ(buf[0], 'H');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
    EXPECT_EQ(buf[3], 'l');
    EXPECT_EQ(buf[4], 'o');
}

/**
 * @brief Test read(buf, off, len) directly on stub
 * @direct Verifies the concrete implementation works correctly
 */
TEST_F(AbstractReaderTest, ReadIntoBufferWithOffset)
{
    std::vector<char> buf(6);
    const int n = reader_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], 'H');
    EXPECT_EQ(buf[2], 'e');
}

/**
 * @brief Test read(buf) with empty buffer
 * @details Reading into an empty buffer should return 0
 */
TEST_F(AbstractReaderTest, ReadIntoEmptyBuffer)
{
    std::vector<char> buf;
    const int n = reader_->read(buf);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test skip via base class implementation
 * @details The base skip(n) reads and discards n characters
 */
TEST_F(AbstractReaderTest, Skip)
{
    const auto skipped = reader_->skip(2);
    EXPECT_EQ(skipped, 2);
    auto ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'l');
}

/**
 * @brief Test skip beyond available data
 * @details Skipping more than available should stop at end
 */
TEST_F(AbstractReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(reader_->read().has_value());
}

/**
 * @brief Test skip with zero
 * @details Skipping 0 bytes returns 0
 */
TEST_F(AbstractReaderTest, SkipZero)
{
    const auto skipped = reader_->skip(0);
    EXPECT_EQ(skipped, 0);
}

/**
 * @brief Test markSupported returns false by default
 * @details The base implementation returns false
 */
TEST_F(AbstractReaderTest, MarkSupported_DefaultFalse)
{
    EXPECT_FALSE(reader_->markSupported());
}

/**
 * @brief Test ready returns false by default
 * @details The base implementation returns false
 */
TEST_F(AbstractReaderTest, Ready_DefaultFalse)
{
    EXPECT_FALSE(reader_->ready());
}

/**
 * @brief Test close and isClosed
 * @details Verifies the close/isClosed contract
 */
TEST_F(AbstractReaderTest, Close)
{
    EXPECT_FALSE(reader_->isClosed());
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
}

/**
 * @brief Test read after close returns -1
 * @details After close, read(buf, off, len) returns -1
 */
TEST_F(AbstractReaderTest, ReadAfterClose)
{
    reader_->close();
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, -1);
}

/**
 * @brief Test mark and reset
 * @details Verifies that mark and reset work through the stub
 */
TEST_F(AbstractReaderTest, MarkAndReset)
{
    auto ch1 = reader_->read();
    ASSERT_TRUE(ch1.has_value());
    EXPECT_EQ(ch1.value(), 'H');

    reader_->mark(10);

    auto ch2 = reader_->read();
    ASSERT_TRUE(ch2.has_value());
    EXPECT_EQ(ch2.value(), 'e');

    reader_->reset();
    auto ch3 = reader_->read();
    ASSERT_TRUE(ch3.has_value());
    EXPECT_EQ(ch3.value(), 'e');
}

/**
 * @brief Test reset without mark throws
 * @details Calling reset without a prior mark throws
 */
TEST_F(AbstractReaderTest, ResetWithoutMark)
{
    reader_->read();
    EXPECT_THROW(reader_->reset(), std::runtime_error);
}

/**
 * @brief Test read(buf) with zero-length buffer returns 0
 * @details The base read(buf) calls read(buf, 0, 0) which returns 0
 */
TEST_F(AbstractReaderTest, ReadZeroLengthBuffer)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test read(buf) with invalid offset throws
 * @details The implemented read(buf,off,len) validates buffer bounds
 */
TEST_F(AbstractReaderTest, ReadInvalidOffsetThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 5, 1), std::out_of_range);
}

/**
 * @brief Test multiple reads consume stream sequentially
 * @details Verifies sequential read operations
 */
TEST_F(AbstractReaderTest, SequentialReads)
{
    std::vector<char> buf1(2);
    auto n1 = reader_->read(buf1);
    EXPECT_EQ(n1, 2);
    EXPECT_EQ(buf1[0], 'H');
    EXPECT_EQ(buf1[1], 'e');

    std::vector<char> buf2(3);
    auto n2 = reader_->read(buf2);
    EXPECT_EQ(n2, 3);
    EXPECT_EQ(buf2[0], 'l');
    EXPECT_EQ(buf2[1], 'l');
    EXPECT_EQ(buf2[2], 'o');
}
