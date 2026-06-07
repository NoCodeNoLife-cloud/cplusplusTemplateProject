#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "filesystem/io/reader/BufferedReader.hpp"
#include "filesystem/io/reader/StringReader.hpp"

using namespace common::filesystem::io::reader;

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

TEST_F(BufferedReaderTest, ReadSingleChar)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 'e');
}

TEST_F(BufferedReaderTest, ReadLine)
{
    EXPECT_EQ(reader_->readLine(), "hello");
    EXPECT_EQ(reader_->readLine(), "world");
    EXPECT_EQ(reader_->readLine(), "!");
}

TEST_F(BufferedReaderTest, ReadLineAtEnd)
{
    (void)reader_->readLine();
    (void)reader_->readLine();
    (void)reader_->readLine();
    EXPECT_TRUE(reader_->readLine().empty());
}

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

TEST_F(BufferedReaderTest, MarkAndReset)
{
    EXPECT_TRUE(reader_->markSupported());
    reader_->read();
    reader_->mark(10);
    reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'e');
}

TEST_F(BufferedReaderTest, Skip)
{
    const auto skipped = reader_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader_->read(), 'l');
}

TEST_F(BufferedReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
}

TEST_F(BufferedReaderTest, ReadLineWithCarriageReturn)
{
    auto inner = std::make_unique<StringReader>("ab\r\ncd");
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), "ab");
    EXPECT_EQ(br.readLine(), "cd");
}
