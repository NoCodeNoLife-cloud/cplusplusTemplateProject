#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/PushbackReader.hpp"
#include "io/reader/StringReader.hpp"

using namespace common::io::reader;

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

TEST_F(PushbackReaderTest, ReadNormal)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(PushbackReaderTest, UnreadSingleAndReRead)
{
    const auto c = reader_->read();
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(c.value(), 'h');
    reader_->unread(c.value());
    EXPECT_EQ(reader_->read(), 'h');
}

TEST_F(PushbackReaderTest, UnreadBufferAndReRead)
{
    (void)reader_->read();
    reader_->unread(std::vector{'X', 'Y'});
    EXPECT_EQ(reader_->read(), 'X');
    EXPECT_EQ(reader_->read(), 'Y');
    EXPECT_EQ(reader_->read(), 'e');
}

TEST_F(PushbackReaderTest, UnreadOverflow)
{
    const std::vector<char> large(2000);
    EXPECT_THROW(reader_->unread(large), std::overflow_error);
}

TEST_F(PushbackReaderTest, MarkNotSupported)
{
    EXPECT_FALSE(reader_->markSupported());
    EXPECT_THROW(reader_->mark(10), std::runtime_error);
    EXPECT_THROW(reader_->reset(), std::runtime_error);
}

TEST_F(PushbackReaderTest, Skip)
{
    (void)reader_->read();
    (void)reader_->read();
    const auto skipped = reader_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(reader_->read(), 'o');
}

TEST_F(PushbackReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_THROW((void)reader_->read(), std::runtime_error);
}

TEST_F(PushbackReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->ready());
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(PushbackReaderTest, MultipleUnreadSingle)
{
    reader_->unread('X');
    reader_->unread('Y');
    EXPECT_EQ(reader_->read(), 'Y');
    EXPECT_EQ(reader_->read(), 'X');
    EXPECT_EQ(reader_->read(), 'h');
}

TEST_F(PushbackReaderTest, UnreadAfterExhaustion)
{
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->read().has_value());
    reader_->unread('Z');
    EXPECT_EQ(reader_->read(), 'Z');
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(PushbackReaderTest, UnreadAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->unread('X'), std::runtime_error);
}

TEST_F(PushbackReaderTest, ReadBufWithOffset)
{
    std::vector<char> buf(10);
    const int n = reader_->read(buf, 2, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[2], 'h');
    EXPECT_EQ(buf[3], 'e');
    EXPECT_EQ(buf[4], 'l');
}

TEST_F(PushbackReaderTest, UnreadBufferExactCapacity)
{
    const std::vector<char> exact(100, 'X');
    EXPECT_NO_THROW(reader_->unread(exact));
}

TEST_F(PushbackReaderTest, SkipAfterUnread)
{
    reader_->unread('X');
    reader_->unread('Y');
    const auto skipped = reader_->skip(1);
    EXPECT_EQ(skipped, 1);
    EXPECT_EQ(reader_->read(), 'X');
}

TEST_F(PushbackReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}

TEST_F(PushbackReaderTest, UnreadInvalidBufferThrows)
{
    EXPECT_THROW(reader_->unread(std::vector<char>(2000)), std::overflow_error);
}
