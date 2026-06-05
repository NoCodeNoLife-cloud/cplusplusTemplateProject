#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "filesystem/io/reader/PushbackReader.hpp"
#include "filesystem/io/reader/StringReader.hpp"

using namespace common::filesystem;

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
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(PushbackReaderTest, UnreadSingleAndReRead)
{
    auto c = reader_->read();
    EXPECT_EQ(c, 'h');
    reader_->unread(c);
    EXPECT_EQ(reader_->read(), 'h');
}

TEST_F(PushbackReaderTest, UnreadBufferAndReRead)
{
    reader_->read();
    reader_->unread(std::vector<char>{'X', 'Y'});
    EXPECT_EQ(reader_->read(), 'X');
    EXPECT_EQ(reader_->read(), 'Y');
    EXPECT_EQ(reader_->read(), 'e');
}

TEST_F(PushbackReaderTest, UnreadOverflow)
{
    std::vector<char> large(2000);
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
    reader_->read();
    reader_->read();
    auto skipped = reader_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(reader_->read(), 'o');
}

TEST_F(PushbackReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_THROW(reader_->read(), std::runtime_error);
}

TEST_F(PushbackReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) reader_->read();
    EXPECT_FALSE(reader_->ready());
}
