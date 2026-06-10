#include <gtest/gtest.h>
#include <vector>

#include "io/reader/CharArrayReader.hpp"

using namespace common::io::reader;

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

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(CharArrayReaderTest, ReadAfterClose_Buffer)
{
    reader_->close();
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, -1);
}

TEST_F(CharArrayReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(CharArrayReaderTest, SkipZero)
{
    const auto skipped = reader_->skip(0);
    EXPECT_EQ(skipped, 0);
}

TEST_F(CharArrayReaderTest, MarkAndResetAfterClose)
{
    reader_->close();
    EXPECT_THROW(reader_->mark(10), std::runtime_error);
    EXPECT_THROW(reader_->reset(), std::runtime_error);
}

TEST_F(CharArrayReaderTest, ReadInvalidOffsetThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 5, 1), std::out_of_range);
}

TEST_F(CharArrayReaderTest, ReadOverflowLengthThrows)
{
    std::vector<char> buf(3);
    EXPECT_THROW(reader_->read(buf, 0, 10), std::out_of_range);
}

TEST_F(CharArrayReaderTest, ConstructorInvalidOffset)
{
    EXPECT_THROW(CharArrayReader(data_, 10, 3), std::invalid_argument);
}

TEST_F(CharArrayReaderTest, LargeSkip)
{
    CharArrayReader large(data_, 0, 5);
    const auto skipped = large.skip(5);
    EXPECT_EQ(skipped, 5);
    EXPECT_FALSE(large.read().has_value());
}

TEST_F(CharArrayReaderTest, ReadyAfterClose)
{
    reader_->close();
    EXPECT_FALSE(reader_->ready());
}

TEST_F(CharArrayReaderTest, MultipleMarkResetCycles)
{
    reader_->mark(10);
    (void)reader_->read();
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'h');
    (void)reader_->read();
    reader_->mark(10);
    (void)reader_->read();
    reader_->reset();
    EXPECT_EQ(reader_->read(), 'l');
}

TEST_F(CharArrayReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const int n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
