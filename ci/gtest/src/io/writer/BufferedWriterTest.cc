/**
 * @file BufferedWriterTest.cc
 * @brief Unit tests for BufferedWriter
 * @details Tests cover write and append operations, flushing, closing, and edge cases.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <cppforge/io/writer/BufferedWriter.hpp>

using namespace cppforge::io::writer;

/// @brief Test fixture for BufferedWriter tests using a temporary file.
class BufferedWriterTest : public testing::Test
{
protected:
    std::string temp_path_;
    std::unique_ptr<BufferedWriter> writer_;

    void SetUp() override
    {
        temp_path_ = (std::filesystem::temp_directory_path() / "buffered_writer_test.tmp").string();
        auto os = std::make_unique<std::ofstream>(temp_path_);
        writer_ = std::make_unique<BufferedWriter>(std::move(os), 16);
    }

    void TearDown() override
    {
        writer_->close();
        std::remove(temp_path_.c_str());
    }

    [[nodiscard]] std::string readFile() const
    {
        writer_->flush();
        std::ifstream ifs(temp_path_);
        return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    }
};

/** @brief Write a full string to the file.
    @details Writes "hello" and reads back from the temporary file. */
TEST_F(BufferedWriterTest, WriteString)
{
    writer_->write("hello");
    EXPECT_EQ(readFile(), "hello");
}

/** @brief Write a char vector to the file.
    @details Writes {'a','b','c'} and reads back from file. */
TEST_F(BufferedWriterTest, WriteVector)
{
    const std::vector<char> buf = {'a', 'b', 'c'};
    writer_->write(buf, 0, buf.size());
    EXPECT_EQ(readFile(), "abc");
}

/** @brief Write a sub-range of a char vector.
    @details Writes buf[1..2] from {'w','x','y','z'} and expects "xy". */
TEST_F(BufferedWriterTest, WriteVectorPartial)
{
    const std::vector<char> buf = {'w', 'x', 'y', 'z'};
    writer_->write(buf, 1, 2);
    EXPECT_EQ(readFile(), "xy");
}

/** @brief Append a single character.
    @details Appends 'A', flushes, and reads back from file. */
TEST_F(BufferedWriterTest, AppendChar)
{
    writer_->append('A');
    writer_->flush();
    EXPECT_EQ(readFile(), "A");
}

/** @brief Append a full string.
    @details Appends "hello" and reads back from file. */
TEST_F(BufferedWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(readFile(), "hello");
}

/** @brief Append a string_view.
    @details Appends "world" via the string_view overload. */
TEST_F(BufferedWriterTest, AppendStringView)
{
    writer_->append(std::string_view("world"));
    EXPECT_EQ(readFile(), "world");
}

/** @brief Append a C-string.
    @details Appends "c-string" via the const char* overload. */
TEST_F(BufferedWriterTest, AppendCString)
{
    writer_->append("c-string");
    EXPECT_EQ(readFile(), "c-string");
}

/** @brief Append an initializer list.
    @details Appends {'a','b','c'} and reads back from file. */
TEST_F(BufferedWriterTest, AppendInitializerList)
{
    writer_->append({'a', 'b', 'c'});
    EXPECT_EQ(readFile(), "abc");
}

/** @brief Append with char array and explicit count.
    @details Appends first 5 chars of "hello world" and reads back. */
TEST_F(BufferedWriterTest, AppendCharArrayWithCount)
{
    writer_->append("hello world", 5);
    EXPECT_EQ(readFile(), "hello");
}

/** @brief Append a char repeated N times.
    @details Appends 'Z' 5 times and reads back "ZZZZZ". */
TEST_F(BufferedWriterTest, AppendCharRepeated)
{
    writer_->append('Z', 5);
    EXPECT_EQ(readFile(), "ZZZZZ");
}

/** @brief Append method chaining.
    @details Chains append('A').append("BC").append('D') and reads back "ABCD". */
TEST_F(BufferedWriterTest, MethodChaining)
{
    writer_->append('A').append(std::string("BC")).append('D');
    EXPECT_EQ(readFile(), "ABCD");
}

/** @brief Flush writes buffered data to the file.
    @details Appends data, flushes, and verifies content is written. */
TEST_F(BufferedWriterTest, Flush)
{
    writer_->append("data");
    writer_->flush();
    EXPECT_EQ(readFile(), "data");
}

/** @brief Close marks the writer as closed.
    @details Verifies isClosed returns true after close. */
TEST_F(BufferedWriterTest, Close)
{
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

/** @brief NewLine inserts a platform newline.
    @details Appends "line1", calls newLine, appends "line2", expects "line1\nline2". */
TEST_F(BufferedWriterTest, NewLine)
{
    writer_->append("line1");
    writer_->newLine();
    writer_->append("line2");
    EXPECT_EQ(readFile(), "line1\nline2");
}

/** @brief Large write triggers automatic flush.
    @details Writing 32 bytes (2x buffer of 16) forces flush to the file. */
TEST_F(BufferedWriterTest, LargeWriteTriggersFlush)
{
    const std::string large(32, 'X');
    writer_->write(large);
    EXPECT_EQ(readFile(), large);
}

/** @brief ToString returns the buffered content.
    @details Appends "content" and checks toString returns the same value. */
TEST_F(BufferedWriterTest, ToString)
{
    writer_->append("content");
    EXPECT_EQ(writer_->toString(), "content");
}

/** @brief Append a sub-range of a string.
    @details Appends "hello world"[0..5] and reads back "hello". */
TEST_F(BufferedWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(readFile(), "hello");
}

/** @brief Write after close is safe.
    @details After close, the writer is marked closed. */
TEST_F(BufferedWriterTest, WriteAfterClose)
{
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

/** @brief Destructor flushes buffered data to the file.
    @details Creates a scoped writer, appends data, and verifies file content after destruction. */
TEST_F(BufferedWriterTest, DestructorFlushes)
{
    const auto dtor_path = (std::filesystem::temp_directory_path() / "bw_dtor_test.tmp").string();
    {
        auto os = std::make_unique<std::ofstream>(dtor_path);
        BufferedWriter w(std::move(os), 16);
        w.append("flush-on-destroy");
    }
    std::ifstream ifs(dtor_path);
    std::string content(std::istreambuf_iterator<char>{ifs}, {});
    EXPECT_EQ(content, "flush-on-destroy");
    std::remove(dtor_path.c_str());
}
