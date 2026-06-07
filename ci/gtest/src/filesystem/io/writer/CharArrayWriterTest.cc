/**
 * @file CharArrayWriterTest.cc
 * @brief Unit tests for CharArrayWriter
 * @details Tests cover write and append operations, conversion to string/array, reset, and edge cases.
 */

#include <gtest/gtest.h>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "filesystem/io/writer/CharArrayWriter.hpp"

using namespace common::filesystem::io::writer;

class CharArrayWriterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        writer_ = std::make_unique<CharArrayWriter>();
    }

    std::unique_ptr<CharArrayWriter> writer_;
};

TEST_F(CharArrayWriterTest, InitiallyEmpty)
{
    EXPECT_EQ(writer_->size(), 0);
    EXPECT_EQ(writer_->toString(), "");
    EXPECT_TRUE(writer_->toCharArray().empty());
}

TEST_F(CharArrayWriterTest, WriteChar)
{
    writer_->write('A');
    EXPECT_EQ(writer_->toString(), "A");
    EXPECT_EQ(writer_->size(), 1);
}

TEST_F(CharArrayWriterTest, WriteMultipleChars)
{
    writer_->write('a');
    writer_->write('b');
    writer_->write('c');
    EXPECT_EQ(writer_->toString(), "abc");
    EXPECT_EQ(writer_->size(), 3);
}

TEST_F(CharArrayWriterTest, WriteVector)
{
    const std::vector<char> buf = {'h', 'e', 'l', 'l', 'o'};
    writer_->write(buf);
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, WriteVectorPartial)
{
    const std::vector<char> buf = {'w', 'x', 'y', 'z'};
    writer_->write(buf, 1, 2);
    EXPECT_EQ(writer_->toString(), "xy");
}

TEST_F(CharArrayWriterTest, WriteVectorOffsetOutOfRange)
{
    const std::vector<char> buf = {'a', 'b'};
    EXPECT_THROW(writer_->write(buf, 5, 1), std::out_of_range);
}

TEST_F(CharArrayWriterTest, WriteVectorLengthExceeds)
{
    const std::vector<char> buf = {'a', 'b'};
    EXPECT_THROW(writer_->write(buf, 1, 5), std::out_of_range);
}

TEST_F(CharArrayWriterTest, WriteString)
{
    writer_->write(std::string("hello"));
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, WriteStringPartial)
{
    writer_->write(std::string("hello world"), 0, 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, WriteStringOffsetOutOfRange)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 5, 1), std::out_of_range);
}

TEST_F(CharArrayWriterTest, WriteStringLengthExceeds)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 1, 5), std::out_of_range);
}

TEST_F(CharArrayWriterTest, AppendChar)
{
    writer_->append('Z');
    EXPECT_EQ(writer_->toString(), "Z");
}

TEST_F(CharArrayWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, AppendStringView)
{
    writer_->append(std::string_view("world"));
    EXPECT_EQ(writer_->toString(), "world");
}

TEST_F(CharArrayWriterTest, AppendCString)
{
    writer_->append("c-string");
    EXPECT_EQ(writer_->toString(), "c-string");
}

TEST_F(CharArrayWriterTest, AppendInitializerList)
{
    writer_->append({'a', 'b', 'c'});
    EXPECT_EQ(writer_->toString(), "abc");
}

TEST_F(CharArrayWriterTest, AppendCharArrayWithCount)
{
    writer_->append("hello world", 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

TEST_F(CharArrayWriterTest, AppendCharRepeated)
{
    writer_->append('Z', 5);
    EXPECT_EQ(writer_->toString(), "ZZZZZ");
}

TEST_F(CharArrayWriterTest, AppendSpan)
{
    const char arr[] = {'x', 'y', 'z'};
    writer_->append(std::span<const char>(arr));
    EXPECT_EQ(writer_->toString(), "xyz");
}

TEST_F(CharArrayWriterTest, AppendEmptyStringView)
{
    writer_->append(std::string_view());
    EXPECT_TRUE(writer_->toString().empty());
}

TEST_F(CharArrayWriterTest, AppendNullCString)
{
    const char* null_str = nullptr;
    writer_->append(null_str);
    EXPECT_TRUE(writer_->toString().empty());
}

TEST_F(CharArrayWriterTest, AppendEmptySpan)
{
    writer_->append(std::span<const char>());
    EXPECT_TRUE(writer_->toString().empty());
}

TEST_F(CharArrayWriterTest, MethodChaining)
{
    writer_->append('A').append(std::string("BC")).append('D');
    EXPECT_EQ(writer_->toString(), "ABCD");
}

TEST_F(CharArrayWriterTest, ToCharArray)
{
    writer_->append("hello");
    const auto arr = writer_->toCharArray();
    ASSERT_EQ(arr.size(), 5);
    EXPECT_EQ(arr[0], 'h');
    EXPECT_EQ(arr[4], 'o');
}

TEST_F(CharArrayWriterTest, SizeReflectsWrittenCount)
{
    EXPECT_EQ(writer_->size(), 0);
    writer_->write('a');
    EXPECT_EQ(writer_->size(), 1);
    writer_->write(std::string("bc"));
    EXPECT_EQ(writer_->size(), 3);
}

TEST_F(CharArrayWriterTest, ResetClearsContent)
{
    writer_->append("hello");
    ASSERT_EQ(writer_->size(), 5);
    writer_->reset();
    EXPECT_EQ(writer_->size(), 0);
    EXPECT_EQ(writer_->toString(), "");
}

TEST_F(CharArrayWriterTest, WriteToTransfersContent)
{
    CharArrayWriter target;
    writer_->append("transfer");
    writer_->writeTo(target);
    EXPECT_EQ(target.toString(), "transfer");
}

TEST_F(CharArrayWriterTest, WriteToEmptyDoesNothing)
{
    CharArrayWriter target;
    writer_->writeTo(target);
    EXPECT_TRUE(target.toString().empty());
}

TEST_F(CharArrayWriterTest, WriteZeroLengthVector)
{
    const std::vector<char> empty;
    writer_->write(empty, 0, 0);
    EXPECT_TRUE(writer_->toString().empty());
}

TEST_F(CharArrayWriterTest, WriteZeroLengthString)
{
    writer_->write(std::string("abc"), 0, 0);
    EXPECT_TRUE(writer_->toString().empty());
}

TEST_F(CharArrayWriterTest, IsClosed)
{
    EXPECT_FALSE(writer_->isClosed());
}

TEST_F(CharArrayWriterTest, CloseIsNoOp)
{
    writer_->close();
    writer_->append("after-close");
    EXPECT_EQ(writer_->toString(), "after-close");
}

TEST_F(CharArrayWriterTest, FlushIsNoOp)
{
    writer_->append("data");
    writer_->flush();
    EXPECT_EQ(writer_->toString(), "data");
}

TEST_F(CharArrayWriterTest, ConstructorWithInitialSize)
{
    CharArrayWriter w(64);
    EXPECT_EQ(w.size(), 0);
    w.append("test");
    EXPECT_EQ(w.toString(), "test");
}

TEST_F(CharArrayWriterTest, ConstructorThrowsOnNegativeSize)
{
    EXPECT_THROW(CharArrayWriter(-1), std::invalid_argument);
}

TEST_F(CharArrayWriterTest, LargeWriteExpandsBuffer)
{
    const std::string large(10000, 'X');
    writer_->write(large);
    EXPECT_EQ(writer_->toString(), large);
    EXPECT_EQ(writer_->size(), 10000);
}
