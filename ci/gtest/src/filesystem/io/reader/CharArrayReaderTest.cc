#include <gtest/gtest.h>
#include <vector>

#include "filesystem/io/reader/CharArrayReader.hpp"

using namespace common::filesystem::io::reader;

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
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(CharArrayReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    const auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
}

TEST_F(CharArrayReaderTest, ReadWithOffsetAndLength)
{
    std::vector<char> buf(5);
    const auto n = reader_->read(buf, 1, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[1], 'h');
    EXPECT_EQ(buf[2], 'e');
    EXPECT_EQ(buf[3], 'l');
}

TEST_F(CharArrayReaderTest, Skip)
{
    const auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
}

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

TEST_F(CharArrayReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) (void)reader_->read();
    EXPECT_FALSE(reader_->ready());
}

TEST_F(CharArrayReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(CharArrayReaderTest, ConstructorWithOffsetAndLength)
{
    CharArrayReader partial(data_, 1, 3);
    EXPECT_EQ(partial.read(), 'e');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_EQ(partial.read(), 'l');
    EXPECT_FALSE(partial.read().has_value());
}

TEST_F(CharArrayReaderTest, ReadEmptyBufferReturnsNullopt)
{
    CharArrayReader empty(std::vector<char>{});
    EXPECT_FALSE(empty.read().has_value());
}
