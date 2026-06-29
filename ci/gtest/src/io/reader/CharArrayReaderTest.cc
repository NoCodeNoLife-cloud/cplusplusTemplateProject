#include <gtest/gtest.h>
#include <vector>

#include <cppforge/io/reader/CharArrayReader.hpp>

using namespace cppforge::io::reader;

/// @brief Test fixture for CharArrayReader tests.
class CharArrayReaderTest : public testing::Test
{
protected:
    std::vector<char> data_;
    std::unique_ptr<CharArrayReader> reader_;

    void SetUp() override
    {
        data_ = {'h', 'e', 'l', 'l', 'o'};
        reader_ = std::make_unique<CharArrayReader>(data_);
    }
};

/** @brief Test reading single characters from the reader. @details Verifies that sequential read() calls return each character followed by std::nullopt at EOF. */
TEST_F(CharArrayReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test reading into a character buffer. @details Verifies that read(buf, off, len) fills the buffer with the correct characters. */
TEST_F(CharArrayReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    const auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
}

/** @brief Test reading with offset and length parameters. @details Verifies that read(buf, off, len) places data at the correct buffer offset. */
TEST_F(CharArrayReaderTest, ReadWithOffsetAndLength)
{
    std::vector<char> buf(5);
    const auto n = reader_->read(buf, 1, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[1], 'h');
    EXPECT_EQ(buf[2], 'e');
    EXPECT_EQ(buf[3], 'l');
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position by the specified number of characters. */
TEST_F(CharArrayReaderTest, Skip)
{
    const auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
}

/** @brief Test mark and reset functionality. @details Verifies that mark() records a position and reset() restores the reader to that position. */
TEST_F(CharArrayReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    (void)reader_->read();
    (void)reader_->read();
    reader_->mark(10);
    const auto c = reader_->read();
    EXPECT_EQ(c, 'l');
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

/** @brief Test the ready() method. @details Verifies that ready() returns true when data is available and false after all data is consumed. */
TEST_F(CharArrayReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->ready());
}

/** @brief Test the close operation. @details Verifies that close() transitions the reader to closed state and subsequent read() returns std::nullopt. */
TEST_F(CharArrayReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test constructor with offset and length parameters. @details Verifies that a sub-range CharArrayReader returns only the specified slice of the backing array. */
TEST_F(CharArrayReaderTest, ConstructorWithOffsetAndLength)
{
    CharArrayReader partial(data_, 1, 3);
    EXPECT_EQ(partial.read(), 'e');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_FALSE(partial.read().has_value());
}

/** @brief Test reading from an empty buffer. @details Verifies that a CharArrayReader wrapping an empty vector immediately returns std::nullopt. */
TEST_F(CharArrayReaderTest, ReadEmptyBufferReturnsNullopt)
{
    CharArrayReader empty(std::vector<char>{});
    EXPECT_FALSE(empty.read().has_value());
}

/** @brief Test buffer read after close. @details Verifies that read(buf, off, len) returns -1 when the reader is closed. */
TEST_F(CharArrayReaderTest, ReadAfterClose_Buffer)
{
    reader_->close();
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, -1);
}

/** @brief Test skip beyond the end of the data. @details Verifies that skip() returns the actual number of characters skipped (5) when the requested amount exceeds available data. */
TEST_F(CharArrayReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test skip with zero length. @details Verifies that skip(0) returns 0 without advancing the read position. */
TEST_F(CharArrayReaderTest, SkipZero)
{
    const auto skipped = reader_->skip(0);
    EXPECT_EQ(skipped, 0);
}

/** @brief Test mark and reset after close. @details Verifies that mark() and reset() throw std::runtime_error when called on a closed reader. */
TEST_F(CharArrayReaderTest, MarkAndResetAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->mark(10), std::runtime_error);
    EXPECT_THROW(reader_->reset(), std::runtime_error);
}

/** @brief Test read with an invalid offset. @details Verifies that read(buf, off, len) throws std::out_of_range when the offset exceeds the buffer size. */
TEST_F(CharArrayReaderTest, ReadInvalidOffsetThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 5, 1), std::out_of_range);
}

/** @brief Test read with an overflow length. @details Verifies that read(buf, off, len) throws std::out_of_range when the requested length exceeds buffer capacity. */
TEST_F(CharArrayReaderTest, ReadOverflowLengthThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 0, 10), std::out_of_range);
}

/** @brief Test constructor with an invalid offset. @details Verifies that constructing CharArrayReader with an out-of-range offset throws std::invalid_argument. */
TEST_F(CharArrayReaderTest, ConstructorInvalidOffset)
{
    EXPECT_THROW(CharArrayReader(data_, 10, 3), std::invalid_argument);
}

/** @brief Test a large skip operation. @details Verifies that skip() over the full range of a sub-range reader returns the exact count and reaches EOF. */
TEST_F(CharArrayReaderTest, LargeSkip)
{
    CharArrayReader large(data_, 0, 5);
    const auto skipped = large.skip(5);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(large.read().has_value());
}

/** @brief Test ready() after close. @details Verifies that ready() returns false when the reader is closed. */
TEST_F(CharArrayReaderTest, ReadyAfterClose)
{
    reader_->close();
    EXPECT_FALSE(reader_->ready());
}

/** @brief Test multiple mark and reset cycles. @details Verifies that the reader correctly handles repeated mark() and reset() sequences without data corruption. */
TEST_F(CharArrayReaderTest, MultipleMarkResetCycles)
{
    reader_->mark(10);
    (void)reader_->read();
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'h');
    (void)reader_->read();
    reader_->mark(10);
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

/** @brief Test read with zero length. @details Verifies that read(buf, off, 0) returns 0 without attempting to read any characters. */
TEST_F(CharArrayReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
