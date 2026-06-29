#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <cppforge/io/reader/StringReader.hpp>

using namespace cppforge::io::reader;

/// @brief Test fixture for StringReader tests.
class StringReaderTest : public testing::Test
{
protected:
    std::unique_ptr<StringReader> reader_;

    void SetUp() override
    {
        reader_ = std::make_unique<StringReader>("hello");
    }
};

/** @brief Test reading single characters from the reader. @details Verifies that sequential read() calls return each character from the string followed by std::nullopt. */
TEST_F(StringReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test reading into a character buffer. @details Verifies that read(buf, off, len) fills the buffer with the first three characters. */
TEST_F(StringReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    const auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
}

/** @brief Test reading beyond the string length. @details Verifies that read() returns only the actual number of characters available when the buffer exceeds the string size. */
TEST_F(StringReaderTest, ReadBeyondString)
{
    std::vector<char> buf(10);
    const auto n = reader_->read(buf, 0, 10);
    EXPECT_EQ(n, 5);
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position by the specified number of characters. */
TEST_F(StringReaderTest, Skip)
{
    const auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
}

/** @brief Test mark and reset functionality. @details Verifies that mark() records a position and reset() restores the reader to that position. */
TEST_F(StringReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    (void)reader_->read();
    (void)reader_->read();
    reader_->mark(10);
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

/** @brief Test reset without an explicit mark. @details Verifies that reset() without a prior mark() returns to the beginning of the string. */
TEST_F(StringReaderTest, MarkResetWithoutExplicitMark)
{
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'h');
}

/** @brief Test the ready() method. @details Verifies that ready() returns true when data is available and false after all characters are consumed. */
TEST_F(StringReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->ready());
}

/** @brief Test the close operation. @details Verifies that close() transitions the reader to closed state and subsequent read() returns std::nullopt. */
TEST_F(StringReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test reading from an empty string. @details Verifies that a StringReader wrapping an empty string immediately returns std::nullopt and ready() is false. */
TEST_F(StringReaderTest, ReadEmptyString)
{
    StringReader empty("");
    EXPECT_FALSE(empty.read().has_value());
    EXPECT_FALSE(empty.ready());
}

/** @brief Test skip with zero length. @details Verifies that skip(0) returns 0 without advancing the read position. */
TEST_F(StringReaderTest, SkipZero)
{
    EXPECT_EQ(reader_->skip(0), 0);
    EXPECT_EQ(reader_->read(), 'h');
}
