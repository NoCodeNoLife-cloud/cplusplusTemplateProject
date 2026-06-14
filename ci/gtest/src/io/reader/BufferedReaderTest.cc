#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "io/reader/BufferedReader.hpp"
#include "io/reader/StringReader.hpp"

using namespace common::io::reader;

/// @brief Test fixture for BufferedReader tests.
class BufferedReaderTest : public testing::Test
{
protected:
    std::unique_ptr<BufferedReader> reader_;

    void SetUp() override
    {
        auto inner = std::make_unique<StringReader>("hello\nworld\n!");
        reader_ = std::make_unique<BufferedReader>(std::move(inner));
    }
};

/** @brief Test reading a single character. @details Verifies that sequential read() calls return the expected characters from the underlying source. */
TEST_F(BufferedReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
}

/** @brief Test reading lines. @details Verifies that readLine() correctly splits the input on newline boundaries and returns each line. */
TEST_F(BufferedReaderTest, ReadLine)
{
    EXPECT_EQ(reader_->readLine(), "hello");
    EXPECT_EQ(reader_->readLine(), "world");
    EXPECT_EQ(reader_->readLine(), "!");
}

/** @brief Test readLine at the end of the stream. @details Verifies that readLine() returns an empty string when no more data is available. */
TEST_F(BufferedReaderTest, ReadLineAtEnd)
{
    (void)reader_->readLine();
    (void)reader_->readLine();
    (void)reader_->readLine();
    EXPECT_TRUE(reader_->readLine().empty());
}

/** @brief Test reading into a character buffer. @details Verifies that read(buf, off, len) fills the buffer with the correct characters and returns the count. */
TEST_F(BufferedReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(5);
    const auto n = reader_->read(buf, 0, 5);
    EXPECT_EQ(n, 5);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
    EXPECT_EQ(buf[3], 'l');
    EXPECT_EQ(buf[4], 'o');
}

/** @brief Test mark and reset functionality. @details Verifies that mark() records a position and reset() restores the reader to that position for re-reading. */
TEST_F(BufferedReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    reader_->read();
    reader_->mark(10);
    reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'e');
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position by the specified number of characters. */
TEST_F(BufferedReaderTest, Skip)
{
    const auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
}

/** @brief Test the close operation. @details Verifies that close() transitions the reader to the closed state. */
TEST_F(BufferedReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
}

/** @brief Test readLine with carriage return and newline. @details Verifies that readLine() correctly handles \r\n line endings as line separators. */
TEST_F(BufferedReaderTest, ReadLineWithCarriageReturn)
{
    auto inner = std::make_unique<StringReader>("ab\r\ncd");
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), "ab");
    EXPECT_EQ(br.readLine(), "cd");
}

/** @brief Test single-character read after the reader is closed. @details Verifies that read() throws std::runtime_error when called on a closed reader. */
TEST_F(BufferedReaderTest, ReadAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->read(), std::runtime_error);
}

/** @brief Test buffer read after the reader is closed. @details Verifies that read(buf, off, len) throws std::runtime_error when called on a closed reader. */
TEST_F(BufferedReaderTest, ReadBufAfterClose)
{
    reader_->close();
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 0, 3), std::runtime_error);
}

/** @brief Test skip beyond the end of the stream. @details Verifies that skip() returns the actual number of characters skipped when the requested amount exceeds available data. */
TEST_F(BufferedReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 13);
    EXPECT_TRUE(reader_->readLine().empty());
}

/** @brief Test the ready() method. @details Verifies that ready() returns true when the stream has data available. */
TEST_F(BufferedReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
}

/** @brief Test readLine with consecutive empty lines. @details Verifies that readLine() returns empty strings for consecutive newline characters. */
TEST_F(BufferedReaderTest, ReadLineEmptyLines)
{
    auto inner = std::make_unique<StringReader>("\n\n\n");
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), "");
    EXPECT_EQ(br.readLine(), "");
    EXPECT_EQ(br.readLine(), "");
    EXPECT_TRUE(br.readLine().empty());
}

/** @brief Test read into a buffer with an offset. @details Verifies that read(buf, off, len) inserts data starting at the specified buffer offset. */
TEST_F(BufferedReaderTest, ReadBufWithOffset)
{
    std::vector<char> buf(10);
    const auto n = reader_->read(buf, 2, 5);
    EXPECT_EQ(n, 5);
    EXPECT_EQ(buf[2], 'h');
    EXPECT_EQ(buf[6], 'o');
}

/** @brief Test reading a large input line. @details Verifies that readLine() correctly handles a single line of 10000 characters without truncation. */
TEST_F(BufferedReaderTest, LargeInput)
{
    const std::string large(10000, 'x');
    auto inner = std::make_unique<StringReader>(large);
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), large);
}

/** @brief Test readLine with all carriage-return-newline pairs. @details Verifies that readLine() returns empty strings for consecutive \r\n line separators. */
TEST_F(BufferedReaderTest, ReadLineAllNewlines)
{
    auto inner = std::make_unique<StringReader>("\r\n\r\n\r\n");
    BufferedReader br(std::move(inner));
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
}

/** @brief Test read with zero length. @details Verifies that read(buf, off, 0) returns 0 without attempting to read any data. */
TEST_F(BufferedReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
