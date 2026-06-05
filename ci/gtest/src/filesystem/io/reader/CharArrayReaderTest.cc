#include <gtest/gtest.h>
#include <vector>

#include "filesystem/io/reader/CharArrayReader.hpp"

using namespace common::filesystem;

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

TEST_F(CharArrayReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(CharArrayReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
}

TEST_F(CharArrayReaderTest, ReadWithOffsetAndLength)
{
    std::vector<char> buf(5);
    auto n = reader_->read(buf, 1, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[1], 'h');
    EXPECT_EQ(buf[2], 'e');
    EXPECT_EQ(buf[3], 'l');
}

TEST_F(CharArrayReaderTest, Skip)
{
    auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
}

TEST_F(CharArrayReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    reader_->read();
    reader_->read();
    reader_->mark(10);
    auto c = reader_->read();
    EXPECT_EQ(c, 'l');
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

TEST_F(CharArrayReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) reader_->read();
    EXPECT_FALSE(reader_->ready());
}

TEST_F(CharArrayReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(CharArrayReaderTest, ConstructorWithOffsetAndLength)
{
    CharArrayReader partial(data_, 1, 3);
    EXPECT_EQ(partial.read(), 'e');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_EQ(partial.read(), -1);
}

TEST_F(CharArrayReaderTest, ReadEmptyBufferReturnsMinusOne)
{
    CharArrayReader empty(std::vector<char>{});
    EXPECT_EQ(empty.read(), -1);
}
