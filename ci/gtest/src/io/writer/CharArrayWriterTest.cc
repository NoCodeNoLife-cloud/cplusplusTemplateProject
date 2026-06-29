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

#include "io/writer/CharArrayWriter.hpp"

using namespace cppforge::io::writer;

/// @brief Test fixture for CharArrayWriter tests.
class CharArrayWriterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        writer_ = std::make_unique<CharArrayWriter>();
    }

    std::unique_ptr<CharArrayWriter> writer_;
};

/** @brief A newly created writer is empty.
    @details Checks size=0, toString is "", and toCharArray is empty. */
TEST_F(CharArrayWriterTest, InitiallyEmpty)
{
    EXPECT_EQ(writer_->size(), 0);
    EXPECT_EQ(writer_->toString(), "");
    EXPECT_TRUE(writer_->toCharArray().empty());
}

/** @brief Write a single character.
    @details Writes 'A' and verifies toString and size. */
TEST_F(CharArrayWriterTest, WriteChar)
{
    writer_->write('A');
    EXPECT_EQ(writer_->toString(), "A");
    EXPECT_EQ(writer_->size(), 1);
}

/** @brief Write multiple characters sequentially.
    @details Writes 'a','b','c' and verifies concatenation and size. */
TEST_F(CharArrayWriterTest, WriteMultipleChars)
{
    writer_->write('a');
    writer_->write('b');
    writer_->write('c');
    EXPECT_EQ(writer_->toString(), "abc");
    EXPECT_EQ(writer_->size(), 3);
}

/** @brief Write a char vector.
    @details Writes {'h','e','l','l','o'} and expects "hello". */
TEST_F(CharArrayWriterTest, WriteVector)
{
    const std::vector<char> buf = {'h', 'e', 'l', 'l', 'o'};
    writer_->write(buf);
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Write a sub-range of a char vector.
    @details Writes buf[1..2] from {'w','x','y','z'} and expects "xy". */
TEST_F(CharArrayWriterTest, WriteVectorPartial)
{
    const std::vector<char> buf = {'w', 'x', 'y', 'z'};
    writer_->write(buf, 1, 2);
    EXPECT_EQ(writer_->toString(), "xy");
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-2 vector triggers std::out_of_range. */
TEST_F(CharArrayWriterTest, WriteVectorOffsetOutOfRange)
{
    const std::vector<char> buf = {'a', 'b'};
    EXPECT_THROW(writer_->write(buf, 5, 1), std::out_of_range);
}

/** @brief Vector length exceeding buffer size throws.
    @details Length 5 at offset 1 on a size-2 vector triggers std::out_of_range. */
TEST_F(CharArrayWriterTest, WriteVectorLengthExceeds)
{
    const std::vector<char> buf = {'a', 'b'};
    EXPECT_THROW(writer_->write(buf, 1, 5), std::out_of_range);
}

/** @brief Write a full string.
    @details Writes "hello" and verifies toString. */
TEST_F(CharArrayWriterTest, WriteString)
{
    writer_->write(std::string("hello"));
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Write a sub-range of a string.
    @details Writes "hello world"[0..5] and expects "hello". */
TEST_F(CharArrayWriterTest, WriteStringPartial)
{
    writer_->write(std::string("hello world"), 0, 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Out-of-bounds string offset throws.
    @details Offset 5 on a length-2 string triggers std::out_of_range. */
TEST_F(CharArrayWriterTest, WriteStringOffsetOutOfRange)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 5, 1), std::out_of_range);
}

/** @brief String length exceeding buffer size throws.
    @details Length 5 at offset 1 on a length-2 string triggers std::out_of_range. */
TEST_F(CharArrayWriterTest, WriteStringLengthExceeds)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 1, 5), std::out_of_range);
}

/** @brief Append a single character.
    @details Appends 'Z' and verifies toString is "Z". */
TEST_F(CharArrayWriterTest, AppendChar)
{
    writer_->append('Z');
    EXPECT_EQ(writer_->toString(), "Z");
}

/** @brief Append a full string.
    @details Appends "hello" and verifies toString. */
TEST_F(CharArrayWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Append a sub-range of a string.
    @details Appends "hello world"[0..5] and expects "hello". */
TEST_F(CharArrayWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Append a string_view.
    @details Appends "world" via the string_view overload. */
TEST_F(CharArrayWriterTest, AppendStringView)
{
    writer_->append(std::string_view("world"));
    EXPECT_EQ(writer_->toString(), "world");
}

/** @brief Append a C-string.
    @details Appends "c-string" via the const char* overload. */
TEST_F(CharArrayWriterTest, AppendCString)
{
    writer_->append("c-string");
    EXPECT_EQ(writer_->toString(), "c-string");
}

/** @brief Append an initializer list.
    @details Appends {'a','b','c'} and expects "abc". */
TEST_F(CharArrayWriterTest, AppendInitializerList)
{
    writer_->append({'a', 'b', 'c'});
    EXPECT_EQ(writer_->toString(), "abc");
}

/** @brief Append with char array and explicit count.
    @details Appends first 5 chars of "hello world" and expects "hello". */
TEST_F(CharArrayWriterTest, AppendCharArrayWithCount)
{
    writer_->append("hello world", 5);
    EXPECT_EQ(writer_->toString(), "hello");
}

/** @brief Append a char repeated N times.
    @details Appends 'Z' 5 times and expects "ZZZZZ". */
TEST_F(CharArrayWriterTest, AppendCharRepeated)
{
    writer_->append('Z', 5);
    EXPECT_EQ(writer_->toString(), "ZZZZZ");
}

/** @brief Append a std::span of chars.
    @details Appends {'x','y','z'} via the span overload. */
TEST_F(CharArrayWriterTest, AppendSpan)
{
    const char arr[] = {'x', 'y', 'z'};
    writer_->append(std::span<const char>(arr));
    EXPECT_EQ(writer_->toString(), "xyz");
}

/** @brief Append an empty string_view produces empty content.
    @details Appending a default-constructed string_view leaves the buffer empty. */
TEST_F(CharArrayWriterTest, AppendEmptyStringView)
{
    writer_->append(std::string_view());
    EXPECT_TRUE(writer_->toString().empty());
}

/** @brief Append a null C-string produces empty content.
    @details Appending nullptr gracefully adds nothing. */
TEST_F(CharArrayWriterTest, AppendNullCString)
{
    const char* null_str = nullptr;
    writer_->append(null_str);
    EXPECT_TRUE(writer_->toString().empty());
}

/** @brief Append an empty span produces empty content.
    @details Appending a default-constructed span adds nothing. */
TEST_F(CharArrayWriterTest, AppendEmptySpan)
{
    writer_->append(std::span<const char>());
    EXPECT_TRUE(writer_->toString().empty());
}

/** @brief Append method chaining.
    @details Chains append('A').append("BC").append('D') and expects "ABCD". */
TEST_F(CharArrayWriterTest, MethodChaining)
{
    writer_->append('A').append(std::string("BC")).append('D');
    EXPECT_EQ(writer_->toString(), "ABCD");
}

/** @brief ToCharArray returns the written characters.
    @details Appends "hello" and checks individual elements in the returned array. */
TEST_F(CharArrayWriterTest, ToCharArray)
{
    writer_->append("hello");
    const auto arr = writer_->toCharArray();
    ASSERT_EQ(arr.size(), 5);
    EXPECT_EQ(arr[0], 'h');
    EXPECT_EQ(arr[4], 'o');
}

/** @brief Size reflects the count of written characters.
    @details Tracks size after each write operation (char, string). */
TEST_F(CharArrayWriterTest, SizeReflectsWrittenCount)
{
    EXPECT_EQ(writer_->size(), 0);
    writer_->write('a');
    EXPECT_EQ(writer_->size(), 1);
    writer_->write(std::string("bc"));
    EXPECT_EQ(writer_->size(), 3);
}

/** @brief Reset clears all buffered content.
    @details Appends "hello", resets, and verifies size=0 and empty string. */
TEST_F(CharArrayWriterTest, ResetClearsContent)
{
    writer_->append("hello");
    ASSERT_EQ(writer_->size(), 5);
    writer_->reset();
    EXPECT_EQ(writer_->size(), 0);
    EXPECT_EQ(writer_->toString(), "");
}

/** @brief writeTo transfers content to another writer.
    @details Writes "transfer" and uses writeTo to copy to a target. */
TEST_F(CharArrayWriterTest, WriteToTransfersContent)
{
    CharArrayWriter target;
    writer_->append("transfer");
    writer_->writeTo(target);
    EXPECT_EQ(target.toString(), "transfer");
}

/** @brief writeTo on an empty writer does nothing.
    @details Calling writeTo immediately after construction leaves target empty. */
TEST_F(CharArrayWriterTest, WriteToEmptyDoesNothing)
{
    CharArrayWriter target;
    writer_->writeTo(target);
    EXPECT_TRUE(target.toString().empty());
}

/** @brief Write a zero-length vector does nothing.
    @details write(empty_vec, 0, 0) leaves the buffer empty. */
TEST_F(CharArrayWriterTest, WriteZeroLengthVector)
{
    const std::vector<char> empty;
    writer_->write(empty, 0, 0);
    EXPECT_TRUE(writer_->toString().empty());
}

/** @brief Write a zero-length string subrange does nothing.
    @details write("abc", 0, 0) leaves the buffer empty. */
TEST_F(CharArrayWriterTest, WriteZeroLengthString)
{
    writer_->write(std::string("abc"), 0, 0);
    EXPECT_TRUE(writer_->toString().empty());
}

/** @brief isClosed returns false for a new writer.
    @brief Verifies the default closed state. */
TEST_F(CharArrayWriterTest, IsClosed)
{
    EXPECT_FALSE(writer_->isClosed());
}

/** @brief Close is a no-op; the writer remains usable.
    @details After close, appends still succeed and toString returns content. */
TEST_F(CharArrayWriterTest, CloseIsNoOp)
{
    writer_->close();
    writer_->append("after-close");
    EXPECT_EQ(writer_->toString(), "after-close");
}

/** @brief Flush is a no-op for this in-memory writer.
    @details After append+flush, toString returns the same content. */
TEST_F(CharArrayWriterTest, FlushIsNoOp)
{
    writer_->append("data");
    writer_->flush();
    EXPECT_EQ(writer_->toString(), "data");
}

/** @brief Constructor with explicit initial size.
    @details Creates a writer with capacity 64 and verifies writes still work. */
TEST_F(CharArrayWriterTest, ConstructorWithInitialSize)
{
    CharArrayWriter w(64);
    EXPECT_EQ(w.size(), 0);
    w.append("test");
    EXPECT_EQ(w.toString(), "test");
}

/** @brief Constructor throws on negative initial size.
    @details Passing -1 triggers std::invalid_argument. */
TEST_F(CharArrayWriterTest, ConstructorThrowsOnNegativeSize)
{
    EXPECT_THROW(CharArrayWriter(-1), std::invalid_argument);
}

/** @brief Large write expands the internal buffer.
    @details Writes 10000 characters and verifies the full content is stored. */
TEST_F(CharArrayWriterTest, LargeWriteExpandsBuffer)
{
    const std::string large(10000, 'X');
    writer_->write(large);
    EXPECT_EQ(writer_->toString(), large);
    EXPECT_EQ(writer_->size(), 10000);
}
