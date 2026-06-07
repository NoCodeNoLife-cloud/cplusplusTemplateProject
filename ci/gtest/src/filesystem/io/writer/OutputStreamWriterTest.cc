/**
 * @file OutputStreamWriterTest.cc
 * @brief Unit tests for OutputStreamWriter
 * @details Tests cover construction with charset, write/append operations, flush, close, and error handling.
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "filesystem/io/writer/OutputStreamWriter.hpp"
#include "filesystem/io/writer/CharArrayWriter.hpp"

using namespace common::filesystem;

class OutputStreamWriterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        auto inner = std::make_unique<CharArrayWriter>();
        inner_ = inner.get();
        writer_ = std::make_unique<OutputStreamWriter>(std::move(inner));
    }

    CharArrayWriter* inner_;
    std::unique_ptr<OutputStreamWriter> writer_;
};

TEST_F(OutputStreamWriterTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(OutputStreamWriter(nullptr), std::invalid_argument);
}

TEST_F(OutputStreamWriterTest, ConstructorThrowsOnUnsupportedCharset)
{
    auto inner = std::make_unique<CharArrayWriter>();
    EXPECT_THROW(OutputStreamWriter(std::move(inner), "ASCII"), std::invalid_argument);
}

TEST_F(OutputStreamWriterTest, ConstructorDefaultCharsetIsUTF8)
{
    EXPECT_EQ(writer_->getEncoding(), "UTF-8");
}

TEST_F(OutputStreamWriterTest, ConstructorWithExplicitUTF8)
{
    auto inner = std::make_unique<CharArrayWriter>();
    auto w = OutputStreamWriter(std::move(inner), "UTF-8");
    EXPECT_EQ(w.getEncoding(), "UTF-8");
}

TEST_F(OutputStreamWriterTest, WriteSingleChar)
{
    writer_->write('A');
    EXPECT_EQ(inner_->toString(), "A");
}

TEST_F(OutputStreamWriterTest, WriteVector)
{
    const std::vector<char> data = {'h', 'e', 'l', 'l', 'o'};
    writer_->write(data);
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, WriteVectorPartial)
{
    const std::vector<char> data = {'w', 'x', 'y', 'z'};
    writer_->write(data, 1, 2);
    EXPECT_EQ(inner_->toString(), "xy");
}

TEST_F(OutputStreamWriterTest, WriteVectorOffsetOutOfRange)
{
    const std::vector<char> data = {'a', 'b'};
    EXPECT_THROW(writer_->write(data, 5, 1), std::out_of_range);
}

TEST_F(OutputStreamWriterTest, WriteString)
{
    writer_->write(std::string("hello"));
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, WriteStringPartial)
{
    writer_->write(std::string("hello world"), 0, 5);
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, WriteStringOffsetOutOfRange)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 5, 1), std::out_of_range);
}

TEST_F(OutputStreamWriterTest, AppendChar)
{
    writer_->append('Z');
    EXPECT_EQ(inner_->toString(), "Z");
}

TEST_F(OutputStreamWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, AppendSubstringStartGtEndThrows)
{
    EXPECT_THROW(writer_->append(std::string("abc"), 3, 1), std::out_of_range);
}

TEST_F(OutputStreamWriterTest, AppendSubstringStartOutOfBoundsThrows)
{
    EXPECT_THROW(writer_->append(std::string("abc"), 10, 12), std::out_of_range);
}

TEST_F(OutputStreamWriterTest, FlushDelegates)
{
    writer_->write("data");
    EXPECT_NO_THROW(writer_->flush());
    EXPECT_EQ(inner_->toString(), "data");
}

TEST_F(OutputStreamWriterTest, CloseFlushesAndCloses)
{
    writer_->write("before close");
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

TEST_F(OutputStreamWriterTest, CloseIsIdempotent)
{
    writer_->close();
    EXPECT_NO_THROW(writer_->close());
    EXPECT_TRUE(writer_->isClosed());
}

TEST_F(OutputStreamWriterTest, WriteAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->write('A'), std::ios_base::failure);
}

TEST_F(OutputStreamWriterTest, WriteStringAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->write(std::string("test")), std::ios_base::failure);
}

TEST_F(OutputStreamWriterTest, FlushAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->flush(), std::ios_base::failure);
}

TEST_F(OutputStreamWriterTest, AppendAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->append('X'), std::ios_base::failure);
}

TEST_F(OutputStreamWriterTest, ToStringAfterCloseReturnsEmpty)
{
    writer_->write("content");
    EXPECT_EQ(writer_->toString(), "content");
    writer_->close();
    EXPECT_EQ(writer_->toString(), "");
}

TEST_F(OutputStreamWriterTest, MethodChaining)
{
    writer_->write('A').write(std::string("BC")).flush();
    EXPECT_EQ(inner_->toString(), "ABC");
}

TEST_F(OutputStreamWriterTest, WriteEmptyStringDoesNothing)
{
    writer_->write(std::string(""));
    EXPECT_TRUE(inner_->toString().empty());
}

TEST_F(OutputStreamWriterTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<char> empty;
    writer_->write(empty);
    EXPECT_TRUE(inner_->toString().empty());
}

TEST_F(OutputStreamWriterTest, WriteUTF8String)
{
    const std::string utf8 = u8"hello";
    writer_->write(utf8);
    EXPECT_EQ(inner_->toString(), "hello");
}

TEST_F(OutputStreamWriterTest, InitiallyNotClosed)
{
    EXPECT_FALSE(writer_->isClosed());
}
