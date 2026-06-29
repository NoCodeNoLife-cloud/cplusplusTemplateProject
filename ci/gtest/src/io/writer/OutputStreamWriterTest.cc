/**
 * @file OutputStreamWriterTest.cc
 * @brief Unit tests for OutputStreamWriter
 * @details Tests cover construction with charset, write/append operations, flush, close, and error handling.
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include <cppforge/io/writer/OutputStreamWriter.hpp>
#include <cppforge/io/writer/CharArrayWriter.hpp>

using namespace cppforge::io::writer;

/// @brief Test fixture for OutputStreamWriter tests using CharArrayWriter.
class OutputStreamWriterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        auto inner = std::make_unique<CharArrayWriter>();
        inner_ = inner.get();
        writer_ = std::make_unique<OutputStreamWriter>(std::move(inner));
    }

    CharArrayWriter* inner_{nullptr};
    std::unique_ptr<OutputStreamWriter> writer_;
};

/** @brief Constructor throws on null inner writer.
    @details Passing nullptr triggers std::invalid_argument. */
TEST_F(OutputStreamWriterTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(OutputStreamWriter(nullptr), std::invalid_argument);
}

/** @brief Constructor throws on unsupported charset.
    @details Passing "ASCII" as the charset triggers std::invalid_argument. */
TEST_F(OutputStreamWriterTest, ConstructorThrowsOnUnsupportedCharset)
{
    auto inner = std::make_unique<CharArrayWriter>();
    EXPECT_THROW(OutputStreamWriter(std::move(inner), "ASCII"), std::invalid_argument);
}

/** @brief Default constructor charset is UTF-8.
    @details Verifies the default encoding returned by getEncoding is "UTF-8". */
TEST_F(OutputStreamWriterTest, ConstructorDefaultCharsetIsUTF8)
{
    EXPECT_EQ(writer_->getEncoding(), "UTF-8");
}

/** @brief Constructor with explicit UTF-8 charset.
    @details Creates a writer with "UTF-8" and verifies the encoding. */
TEST_F(OutputStreamWriterTest, ConstructorWithExplicitUTF8)
{
    auto inner = std::make_unique<CharArrayWriter>();
    auto w = OutputStreamWriter(std::move(inner), "UTF-8");
    EXPECT_EQ(w.getEncoding(), "UTF-8");
}

/** @brief Write a single character.
    @details Writes 'A' and verifies it appears in the inner CharArrayWriter. */
TEST_F(OutputStreamWriterTest, WriteSingleChar)
{
    writer_->write('A');
    EXPECT_EQ(inner_->toString(), "A");
}

/** @brief Write a full char vector.
    @details Writes {'h','e','l','l','o'} and expects "hello". */
TEST_F(OutputStreamWriterTest, WriteVector)
{
    const std::vector data = {'h', 'e', 'l', 'l', 'o'};
    writer_->write(data);
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief Write a sub-range of a char vector.
    @details Writes buf[1..2] from {'w','x','y','z'} and expects "xy". */
TEST_F(OutputStreamWriterTest, WriteVectorPartial)
{
    const std::vector data = {'w', 'x', 'y', 'z'};
    writer_->write(data, 1, 2);
    EXPECT_EQ(inner_->toString(), "xy");
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-2 vector triggers std::out_of_range. */
TEST_F(OutputStreamWriterTest, WriteVectorOffsetOutOfRange)
{
    const std::vector data = {'a', 'b'};
    EXPECT_THROW(writer_->write(data, 5, 1), std::out_of_range);
}

/** @brief Write a full string.
    @details Writes "hello" and verifies in the inner writer. */
TEST_F(OutputStreamWriterTest, WriteString)
{
    writer_->write(std::string("hello"));
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief Write a sub-range of a string.
    @details Writes "hello world"[0..5] and expects "hello". */
TEST_F(OutputStreamWriterTest, WriteStringPartial)
{
    writer_->write(std::string("hello world"), 0, 5);
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief Out-of-bounds string offset throws.
    @details Offset 5 on a length-2 string triggers std::out_of_range. */
TEST_F(OutputStreamWriterTest, WriteStringOffsetOutOfRange)
{
    EXPECT_THROW(writer_->write(std::string("ab"), 5, 1), std::out_of_range);
}

/** @brief Append a single character.
    @details Appends 'Z' and verifies in the inner writer. */
TEST_F(OutputStreamWriterTest, AppendChar)
{
    writer_->append('Z');
    EXPECT_EQ(inner_->toString(), "Z");
}

/** @brief Append a full string.
    @details Appends "hello" and verifies in the inner writer. */
TEST_F(OutputStreamWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief Append a sub-range of a string.
    @details Appends "hello world"[0..5] and expects "hello". */
TEST_F(OutputStreamWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief Append with start > end throws.
    @details Passing start=3, end=1 on "abc" triggers std::out_of_range. */
TEST_F(OutputStreamWriterTest, AppendSubstringStartGtEndThrows)
{
    EXPECT_THROW(writer_->append(std::string("abc"), 3, 1), std::out_of_range);
}

/** @brief Append with start out of bounds throws.
    @details Passing start=10, end=12 on "abc" triggers std::out_of_range. */
TEST_F(OutputStreamWriterTest, AppendSubstringStartOutOfBoundsThrows)
{
    EXPECT_THROW(writer_->append(std::string("abc"), 10, 12), std::out_of_range);
}

/** @brief Flush delegates to the inner writer.
    @details Writes "data", flushes, and verifies content reached the inner writer. */
TEST_F(OutputStreamWriterTest, FlushDelegates)
{
    writer_->write("data");
    EXPECT_NO_THROW(writer_->flush());
    EXPECT_EQ(inner_->toString(), "data");
}

/** @brief Close flushes the inner writer and marks closed.
    @details Writes data, closes, and verifies the closed state. */
TEST_F(OutputStreamWriterTest, CloseFlushesAndCloses)
{
    writer_->write("before close");
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

/** @brief Close is idempotent.
    @details Double close does not throw and leaves the writer closed. */
TEST_F(OutputStreamWriterTest, CloseIsIdempotent)
{
    writer_->close();
    EXPECT_NO_THROW(writer_->close());
    EXPECT_TRUE(writer_->isClosed());
}

/** @brief Write after close throws.
    @details Closing then writing a char triggers std::ios_base::failure. */
TEST_F(OutputStreamWriterTest, WriteAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->write('A'), std::ios_base::failure);
}

/** @brief Write string after close throws.
    @details Closing then writing a string triggers std::ios_base::failure. */
TEST_F(OutputStreamWriterTest, WriteStringAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->write(std::string("test")), std::ios_base::failure);
}

/** @brief Flush after close throws.
    @details Closing then flushing triggers std::ios_base::failure. */
TEST_F(OutputStreamWriterTest, FlushAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->flush(), std::ios_base::failure);
}

/** @brief Append after close throws.
    @details Closing then appending triggers std::ios_base::failure. */
TEST_F(OutputStreamWriterTest, AppendAfterCloseThrows)
{
    writer_->close();
    EXPECT_THROW(writer_->append('X'), std::ios_base::failure);
}

/** @brief toString returns empty after close.
    @details write+toString succeeds before close; toString returns "" after close. */
TEST_F(OutputStreamWriterTest, ToStringAfterCloseReturnsEmpty)
{
    writer_->write("content");
    EXPECT_EQ(writer_->toString(), "content");
    writer_->close();
    EXPECT_EQ(writer_->toString(), "");
}

/** @brief Write sequence of char and string.
    @details Writes 'A' then "BC", flushes, and expects "ABC". */
TEST_F(OutputStreamWriterTest, WriteSequence)
{
    writer_->write('A');
    writer_->write(std::string("BC"));
    writer_->flush();
    EXPECT_EQ(inner_->toString(), "ABC");
}

/** @brief Empty string write does nothing.
    @details Writing an empty string leaves the inner writer unchanged. */
TEST_F(OutputStreamWriterTest, WriteEmptyStringDoesNothing)
{
    writer_->write(std::string(""));
    EXPECT_TRUE(inner_->toString().empty());
}

/** @brief Empty vector write does nothing.
    @details Writing an empty vector leaves the inner writer unchanged. */
TEST_F(OutputStreamWriterTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<char> empty;
    writer_->write(empty);
    EXPECT_TRUE(inner_->toString().empty());
}

/** @brief Write a UTF-8 string.
    @details Writes "hello" and verifies correct encoding through the writer. */
TEST_F(OutputStreamWriterTest, WriteUTF8String)
{
    writer_->write(std::string("hello"));
    EXPECT_EQ(inner_->toString(), "hello");
}

/** @brief A newly created writer is not closed.
    @brief Verifies the initial closed state is false. */
TEST_F(OutputStreamWriterTest, InitiallyNotClosed)
{
    EXPECT_FALSE(writer_->isClosed());
}
