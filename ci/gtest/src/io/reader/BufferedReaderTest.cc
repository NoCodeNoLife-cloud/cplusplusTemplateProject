#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "io/reader/BufferedReader.hpp"
#include "io/reader/StringReader.hpp"

using namespace common::io::reader;

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

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(BufferedReaderTest, ReadAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->read(), std::runtime_error);
}

TEST_F(BufferedReaderTest, ReadBufAfterClose)
{
    reader_->close();
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 0, 3), std::runtime_error);
}

TEST_F(BufferedReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 13);
    EXPECT_TRUE(reader_->readLine().empty());
}

TEST_F(BufferedReaderTest, Ready)
{
    EXPECT_TRUE(reader_->ready());
}

TEST_F(BufferedReaderTest, ReadLineEmptyLines)
{
    auto inner = std::make_unique<StringReader>("\n\n\n");
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), "");
    EXPECT_EQ(br.readLine(), "");
    EXPECT_EQ(br.readLine(), "");
    EXPECT_TRUE(br.readLine().empty());
}

TEST_F(BufferedReaderTest, ReadBufWithOffset)
{
    std::vector<char> buf(10);
    const auto n = reader_->read(buf, 2, 5);
    EXPECT_EQ(n, 5);
    EXPECT_EQ(buf[2], 'h');
    EXPECT_EQ(buf[6], 'o');
}

TEST_F(BufferedReaderTest, LargeInput)
{
    const std::string large(10000, 'x');
    auto inner = std::make_unique<StringReader>(large);
    BufferedReader br(std::move(inner));
    EXPECT_EQ(br.readLine(), large);
}

TEST_F(BufferedReaderTest, ReadLineAllNewlines)
{
    auto inner = std::make_unique<StringReader>("\r\n\r\n\r\n");
    BufferedReader br(std::move(inner));
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
    EXPECT_TRUE(br.readLine().empty());
}

TEST_F(BufferedReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
