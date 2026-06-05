#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "filesystem/io/reader/StringReader.hpp"

using namespace common::filesystem;

class StringReaderTest : public testing::Test
{
protected:
    std::unique_ptr<StringReader> reader_;

    void SetUp() override
    {
        reader_ = std::make_unique<StringReader>("hello");
    }
};

TEST_F(StringReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(StringReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], 'e');
    EXPECT_EQ(buf[2], 'l');
}

TEST_F(StringReaderTest, ReadBeyondString)
{
    std::vector<char> buf(10);
    auto n = reader_->read(buf, 0, 10);
    EXPECT_EQ(n, 5);
}

TEST_F(StringReaderTest, Skip)
{
    auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
}

TEST_F(StringReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    reader_->read();
    reader_->read();
    reader_->mark(10);
    reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

TEST_F(StringReaderTest, MarkResetWithoutExplicitMark)
{
    reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'h');
}

TEST_F(StringReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
    for (int i = 0; i < 5; ++i) reader_->read();
    EXPECT_FALSE(reader_->ready());
}

TEST_F(StringReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(StringReaderTest, ReadEmptyString)
{
    StringReader empty("");
    EXPECT_EQ(empty.read(), -1);
    EXPECT_FALSE(empty.ready());
}

TEST_F(StringReaderTest, SkipZero)
{
    EXPECT_EQ(reader_->skip(0), 0);
    EXPECT_EQ(reader_->read(), 'h');
}
