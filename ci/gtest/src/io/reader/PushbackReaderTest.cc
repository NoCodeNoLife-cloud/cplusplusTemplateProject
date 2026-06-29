#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <cppforge/io/reader/PushbackReader.hpp>
#include <cppforge/io/reader/StringReader.hpp>

using namespace cppforge::io::reader;

/// @brief Test fixture for PushbackReader tests.
class PushbackReaderTest : public testing::Test
{
protected:
    std::unique_ptr<PushbackReader> reader_;

    void SetUp() override
    {
        auto inner = std::make_shared<StringReader>("hello");
        reader_ = std::make_unique<PushbackReader>(inner);
    }
};

/** @brief Test normal read of all characters. @details Verifies that sequential read() calls return each character from the underlying reader followed by std::nullopt at EOF. */
TEST_F(PushbackReaderTest, ReadNormal)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test unread of a single character and re-read. @details Verifies that unread() pushes back one character and a subsequent read() returns it again. */
TEST_F(PushbackReaderTest, UnreadSingleAndReRead)
{
    const auto c = reader_->read();
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(c.value(), 'h');
    reader_->unread(c.value());
    EXPECT_EQ(reader_->read(), 'h');
}

/** @brief Test unread of a buffer and re-read. @details Verifies that unread(buf) pushes back multiple characters which are returned in LIFO order. */
TEST_F(PushbackReaderTest, UnreadBufferAndReRead)
{
    (void)reader_->read();
    reader_->unread(std::vector{'X', 'Y'});
    EXPECT_EQ(reader_->read(), 'X');
    EXPECT_EQ(reader_->read(), 'Y');
    EXPECT_EQ(reader_->read(), 'e');
}

/** @brief Test unread with overflow beyond pushback capacity. @details Verifies that unread() throws std::overflow_error when the buffer exceeds the default pushback size. */
TEST_F(PushbackReaderTest, UnreadOverflow)
{
    const std::vector<char> large(2000);
    EXPECT_THROW(reader_->unread(large), std::overflow_error);
}

/** @brief Test that mark is not supported. @details Verifies that markSupported() returns false and mark()/reset() throw std::runtime_error. */
TEST_F(PushbackReaderTest, MarkNotSupported)
{
    EXPECT_FALSE(reader_->markSupported());
    EXPECT_THROW(reader_->mark(10), std::runtime_error);
    EXPECT_THROW(reader_->reset(), std::runtime_error);
}

/** @brief Test the skip operation. @details Verifies that skip() advances the read position past the specified number of characters. */
TEST_F(PushbackReaderTest, Skip)
{
    (void)reader_->read();
    (void)reader_->read();
    const auto skipped = reader_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(reader_->read(), 'o');
}

/** @brief Test the close operation. @details Verifies that close() transitions the reader to closed state and subsequent read() throws std::runtime_error. */
TEST_F(PushbackReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_THROW((void)reader_->read(), std::runtime_error);
}

/** @brief Test the ready() method. @details Verifies that ready() returns true when data is available and false after all characters are consumed. */
TEST_F(PushbackReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->ready());
}

/** @brief Test multiple unread of single characters. @details Verifies that unread() pushes back characters in LIFO order and subsequent reads return them correctly. */
TEST_F(PushbackReaderTest, MultipleUnreadSingle)
{
    reader_->unread('X');
    reader_->unread('Y');
    EXPECT_EQ(reader_->read(), 'Y');
    EXPECT_EQ(reader_->read(), 'X');
    EXPECT_EQ(reader_->read(), 'h');
}

/** @brief Test unread after exhausting the underlying stream. @details Verifies that unread() works after the source is fully consumed, returning the pushed-back character. */
TEST_F(PushbackReaderTest, UnreadAfterExhaustion)
{
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->read().has_value());
    reader_->unread('Z');
    EXPECT_EQ(reader_->read(), 'Z');
    EXPECT_FALSE(reader_->read().has_value());
}

/** @brief Test unread after the reader is closed. @details Verifies that unread() throws std::runtime_error when called on a closed reader. */
TEST_F(PushbackReaderTest, UnreadAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->unread('X'), std::runtime_error);
}

/** @brief Test reading into a buffer with an offset. @details Verifies that read(buf, off, len) places data at the specified buffer offset. */
TEST_F(PushbackReaderTest, ReadBufWithOffset)
{
    std::vector<char> buf(10);
    const int n = reader_->read(buf, 2, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[2], 'h');
    EXPECT_EQ(buf[3], 'e');
    EXPECT_EQ(buf[4], 'l');
}

/** @brief Test unread at exact buffer capacity. @details Verifies that unread(buf) does not throw when the buffer size matches the pushback capacity. */
TEST_F(PushbackReaderTest, UnreadBufferExactCapacity)
{
    const std::vector<char> exact(100, 'X');
    EXPECT_NO_THROW(reader_->unread(exact));
}

/** @brief Test skip after an unread operation. @details Verifies that skip() correctly advances past pushed-back characters. */
TEST_F(PushbackReaderTest, SkipAfterUnread)
{
    reader_->unread('X');
    reader_->unread('Y');
    const auto skipped = reader_->skip(1);
    EXPECT_EQ(skipped, 1);
    EXPECT_EQ(reader_->read(), 'X');
}

/** @brief Test read with zero length. @details Verifies that read(buf, off, 0) returns 0 without attempting to read any characters. */
TEST_F(PushbackReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}

/** @brief Test unread with a buffer that exceeds capacity. @details Verifies that unread() throws std::overflow_error when the buffer is larger than the pushback capacity. */
TEST_F(PushbackReaderTest, UnreadInvalidBufferThrows)
{
    EXPECT_THROW(reader_->unread(std::vector<char>(2000)), std::overflow_error);
}
