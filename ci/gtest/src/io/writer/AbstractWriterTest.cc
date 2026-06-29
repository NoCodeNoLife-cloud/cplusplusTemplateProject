/**
 * @file AbstractWriterTest.cc
 * @brief Unit tests for the AbstractWriter base class
 * @details Tests cover the default implementations of write(char), write(buf),
 *          write(string), and all append() overloads via a minimal concrete stub.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "io/writer/AbstractWriter.hpp"

using namespace cppforge::io::writer;

/**
 * @brief Minimal concrete stub for testing AbstractWriter base class behavior.
 * @details Only overrides write(buf,off,len), toString, flush, close, isClosed.
 *          All other methods (write(char), write(string), append() overloads)
 *          are tested through AbstractWriter's default implementations.
 */
class TestWriter final : public AbstractWriter
{
public:
    using AbstractWriter::write;

    void write(const std::vector<char>& cBuf, size_t off, size_t len) override
    {
        if (closed_)
        {
            throw std::runtime_error("Stream closed");
        }
        for (size_t i = 0; i < len; ++i)
        {
            if (off + i < cBuf.size())
            {
                buffer_.push_back(cBuf[off + i]);
            }
        }
    }

    void flush() override
    {
        flushed_ = true;
    }

    void close() override
    {
        closed_ = true;
    }

    [[nodiscard]] std::string toString() const override
    {
        return std::string(buffer_.begin(), buffer_.end());
    }

    [[nodiscard]] bool isClosed() const override
    {
        return closed_;
    }

    [[nodiscard]] bool isFlushed() const
    {
        return flushed_;
    }

    void reset()
    {
        buffer_.clear();
        flushed_ = false;
        closed_ = false;
    }

private:
    std::vector<char> buffer_;
    bool flushed_{false};
    bool closed_{false};
};

/**
 * @brief Test fixture for AbstractWriterTest tests
 */
class AbstractWriterTest : public testing::Test
{
protected:
    TestWriter writer_;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test write single character via base class
 * @details The base write(char c) calls write(vector<char>{c}, 0, 1)
 */
TEST_F(AbstractWriterTest, WriteSingleChar)
{
    writer_.write('a');
    EXPECT_EQ(writer_.toString(), "a");
}

/**
 * @brief Test write string via base class
 * @details The base write(string) calls write(str, 0, str.size())
 */
TEST_F(AbstractWriterTest, WriteString)
{
    writer_.write("hello");
    EXPECT_EQ(writer_.toString(), "hello");
}

/**
 * @brief Test write string with offset and length
 * @details Writes a substring of the given string
 */
TEST_F(AbstractWriterTest, WriteStringWithOffset)
{
    writer_.write("hello world", 6, 5);
    EXPECT_EQ(writer_.toString(), "world");
}

/**
 * @brief Test write string with offset beyond size
 * @details Should write nothing
 */
TEST_F(AbstractWriterTest, WriteStringOffsetBeyondEnd)
{
    writer_.write("hello", 10, 5);
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test write vector via base class
 * @details The base write(vector) calls write(buf, 0, buf.size())
 */
TEST_F(AbstractWriterTest, WriteVector)
{
    const std::vector<char> buf = {'a', 'b', 'c'};
    writer_.write(buf);
    EXPECT_EQ(writer_.toString(), "abc");
}

/**
 * @brief Test write empty string
 * @edge Empty input should produce no output
 */
TEST_F(AbstractWriterTest, WriteEmptyString)
{
    writer_.write("");
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test write empty vector
 * @edge Empty vector should produce no output
 */
TEST_F(AbstractWriterTest, WriteEmptyVector)
{
    const std::vector<char> empty;
    writer_.write(empty);
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test append single character
 * @details append(char) returns reference for chaining
 */
TEST_F(AbstractWriterTest, AppendChar)
{
    AbstractWriter& ref = writer_.append('x');
    EXPECT_EQ(&ref, &writer_);
    EXPECT_EQ(writer_.toString(), "x");
}

/**
 * @brief Test append string
 * @details append(string) calls write(string) internally
 */
TEST_F(AbstractWriterTest, AppendString)
{
    writer_.append("hello");
    EXPECT_EQ(writer_.toString(), "hello");
}

/**
 * @brief Test append string subsequence
 * @details append(string, start, end) appends a portion
 */
TEST_F(AbstractWriterTest, AppendStringSubsequence)
{
    writer_.append("hello world", 6, 11);
    EXPECT_EQ(writer_.toString(), "world");
}

/**
 * @brief Test append string_view
 * @details append(string_view) delegates to write(string)
 */
TEST_F(AbstractWriterTest, AppendStringView)
{
    const std::string_view sv = "test";
    writer_.append(sv);
    EXPECT_EQ(writer_.toString(), "test");
}

/**
 * @brief Test append C-string
 * @details append(const char*) handles null safely
 */
TEST_F(AbstractWriterTest, AppendCString)
{
    writer_.append("c-string");
    EXPECT_EQ(writer_.toString(), "c-string");
}

/**
 * @brief Test append nullptr C-string
 * @edge Null pointer should not crash
 */
TEST_F(AbstractWriterTest, AppendNullptr)
{
    writer_.append(static_cast<const char*>(nullptr));
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test append initializer list
 * @details append(initializer_list<char>) appends chars from list
 */
TEST_F(AbstractWriterTest, AppendInitializerList)
{
    writer_.append({'a', 'b', 'c'});
    EXPECT_EQ(writer_.toString(), "abc");
}

/**
 * @brief Test append empty initializer list
 * @edge Empty list produces no output
 */
TEST_F(AbstractWriterTest, AppendEmptyInitializerList)
{
    writer_.append(std::initializer_list<char>{});
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test append char with count
 * @details append(char, count) repeats the char
 */
TEST_F(AbstractWriterTest, AppendCharRepeated)
{
    writer_.append('z', 5);
    EXPECT_EQ(writer_.toString(), "zzzzz");
}

/**
 * @brief Test append char with zero count
 * @edge Zero count produces no output
 */
TEST_F(AbstractWriterTest, AppendCharZeroCount)
{
    writer_.append('x', 0);
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test append span
 * @details append(span<const char>) uses contiguous char range
 */
TEST_F(AbstractWriterTest, AppendSpan)
{
    const char arr[] = {'s', 'p', 'a', 'n'};
    writer_.append(std::span<const char>(arr, 4));
    EXPECT_EQ(writer_.toString(), "span");
}

/**
 * @brief Test append empty span
 * @edge Empty span produces no output
 */
TEST_F(AbstractWriterTest, AppendEmptySpan)
{
    writer_.append(std::span<const char>());
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test chained append calls
 * @details Multiple appends should chain correctly
 */
TEST_F(AbstractWriterTest, ChainedAppends)
{
    writer_.append('a').append("bc").append({'d', 'e'});
    EXPECT_EQ(writer_.toString(), "abcde");
}

/**
 * @brief Test flush
 * @details Verifies flush delegates to concrete implementation
 */
TEST_F(AbstractWriterTest, Flush)
{
    writer_.write("data");
    writer_.flush();
    EXPECT_TRUE(writer_.isFlushed());
}

/**
 * @brief Test close and isClosed
 * @details Verifies close/isClosed contract
 */
TEST_F(AbstractWriterTest, Close)
{
    EXPECT_FALSE(writer_.isClosed());
    writer_.close();
    EXPECT_TRUE(writer_.isClosed());
}

/**
 * @brief Test write after close throws
 * @details After close, write should throw
 */
TEST_F(AbstractWriterTest, WriteAfterClose)
{
    writer_.close();
    EXPECT_THROW(writer_.write('x'), std::runtime_error);
}

/**
 * @brief Test append empty string_view
 * @edge Empty view produces no output
 */
TEST_F(AbstractWriterTest, AppendEmptyStringView)
{
    writer_.append(std::string_view());
    EXPECT_TRUE(writer_.toString().empty());
}

/**
 * @brief Test append with start > end
 * @edge Invalid subsequence bounds should produce no output
 */
TEST_F(AbstractWriterTest, AppendInvalidSubsequence)
{
    writer_.append(std::string("hello"), 5, 3);
    EXPECT_TRUE(writer_.toString().empty());
}
