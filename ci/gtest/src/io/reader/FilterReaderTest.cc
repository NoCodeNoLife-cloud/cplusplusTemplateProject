/**
 * @file FilterReaderTest.cc
 * @brief Unit tests for the FilterReader class
 * @details Tests cover delegation to wrapped reader, including read operations,
 *          skip, mark/reset passthrough, ready, and close propagation.
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/FilterReader.hpp"
#include "io/reader/CharArrayReader.hpp"

using namespace cppforge::io::reader;

/**
 * @brief Test fixture for FilterReaderTest tests
 */
class FilterReaderTest : public testing::Test
{
protected:
    std::vector<char> data_;
    std::shared_ptr<CharArrayReader> inner_;
    std::unique_ptr<FilterReader> reader_;

    void SetUp() override
    {
        data_ = {'H', 'e', 'l', 'l', 'o'};
        inner_ = std::make_shared<CharArrayReader>(data_);
        reader_ = std::make_unique<FilterReader>(inner_);
    }
};

/**
 * @brief Test read() delegates to inner reader
 * @details Verifies single-character read passthrough
 */
TEST_F(FilterReaderTest, ReadSingle)
{
    auto ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'H');

    ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'e');
}

/**
 * @brief Test read(buf, off, len) delegates to inner reader
 * @details Verifies offset-based buffer read passthrough
 */
TEST_F(FilterReaderTest, ReadIntoBufferWithOffset)
{
    std::vector<char> buf(6);
    const int n = reader_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], 'H');
    EXPECT_EQ(buf[2], 'e');
}

/**
 * @brief Test read(buf) delegates to inner reader
 * @details Verifies full buffer read passthrough
 */
TEST_F(FilterReaderTest, ReadIntoBuffer)
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
 * @brief Test read() returns nullopt at end of stream
 * @details When all data is consumed, read() returns nullopt
 */
TEST_F(FilterReaderTest, ReadReturnsNulloptAtEnd)
{
    for (int i = 0; i < 5; ++i)
    {
        (void)reader_->read();
    }
    auto ch = reader_->read();
    EXPECT_FALSE(ch.has_value());
}

/**
 * @brief Test skip delegates to inner reader
 * @details Verifies skip passthrough
 */
TEST_F(FilterReaderTest, Skip)
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
TEST_F(FilterReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(reader_->read().has_value());
}

/**
 * @brief Test markSupported delegates to inner reader
 * @details CharArrayReader supports mark, so FilterReader should too
 */
TEST_F(FilterReaderTest, MarkSupported)
{
    EXPECT_TRUE(reader_->markSupported());
}

/**
 * @brief Test mark and reset passthrough
 * @details Verifies mark/reset are delegated correctly
 */
TEST_F(FilterReaderTest, MarkAndReset)
{
    reader_->mark(10);
    (void)reader_->read();
    (void)reader_->read();
    reader_->reset();
    auto ch = reader_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'H');
}

/**
 * @brief Test ready delegates to inner reader
 * @details CharArrayReader returns true if data is available
 */
TEST_F(FilterReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
}

/**
 * @brief Test close propagates to inner reader
 * @details Closing the filter should close the underlying reader
 */
TEST_F(FilterReaderTest, Close)
{
    EXPECT_FALSE(reader_->isClosed());
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_TRUE(inner_->isClosed());
}

/**
 * @brief Test read after close returns nullopt
 * @details After close, read delegates to closed CharArrayReader which returns nullopt
 */
TEST_F(FilterReaderTest, ReadAfterClose)
{
    reader_->close();
    auto ch = reader_->read();
    EXPECT_FALSE(ch.has_value());
}

/**
 * @brief Test read(buf, off, len) with invalid offset throws
 * @details FilterReader validates buffer bounds before delegating
 */
TEST_F(FilterReaderTest, ReadInvalidOffsetThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 5, 1), std::out_of_range);
}

/**
 * @brief Test close is idempotent
 * @details Multiple close calls should not throw
 */
TEST_F(FilterReaderTest, CloseIsIdempotent)
{
    reader_->close();
    EXPECT_NO_THROW(reader_->close());
}

/**
 * @brief Test read(buf) after close returns -1
 * @details After close, read(buf) delegates to closed CharArrayReader which returns -1
 */
TEST_F(FilterReaderTest, ReadBufAfterClose)
{
    reader_->close();
    std::vector<char> buf(3);
    const int n = reader_->read(buf);
    EXPECT_EQ(n, -1);
}
