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

#include "filesystem/io/writer/BufferedWriter.hpp"

using namespace common::filesystem;

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

TEST_F(BufferedWriterTest, WriteString)
{
    writer_->write("hello");
    EXPECT_EQ(readFile(), "hello");
}

TEST_F(BufferedWriterTest, WriteVector)
{
    const std::vector<char> buf = {'a', 'b', 'c'};
    writer_->write(buf, 0, buf.size());
    EXPECT_EQ(readFile(), "abc");
}

TEST_F(BufferedWriterTest, WriteVectorPartial)
{
    const std::vector<char> buf = {'w', 'x', 'y', 'z'};
    writer_->write(buf, 1, 2);
    EXPECT_EQ(readFile(), "xy");
}

TEST_F(BufferedWriterTest, AppendChar)
{
    writer_->append('A');
    writer_->flush();
    EXPECT_EQ(readFile(), "A");
}

TEST_F(BufferedWriterTest, AppendString)
{
    writer_->append(std::string("hello"));
    EXPECT_EQ(readFile(), "hello");
}

TEST_F(BufferedWriterTest, AppendStringView)
{
    writer_->append(std::string_view("world"));
    EXPECT_EQ(readFile(), "world");
}

TEST_F(BufferedWriterTest, AppendCString)
{
    writer_->append("c-string");
    EXPECT_EQ(readFile(), "c-string");
}

TEST_F(BufferedWriterTest, AppendInitializerList)
{
    writer_->append({'a', 'b', 'c'});
    EXPECT_EQ(readFile(), "abc");
}

TEST_F(BufferedWriterTest, AppendCharArrayWithCount)
{
    writer_->append("hello world", 5);
    EXPECT_EQ(readFile(), "hello");
}

TEST_F(BufferedWriterTest, AppendCharRepeated)
{
    writer_->append('Z', 5);
    EXPECT_EQ(readFile(), "ZZZZZ");
}

TEST_F(BufferedWriterTest, MethodChaining)
{
    writer_->append('A').append(std::string("BC")).append('D');
    EXPECT_EQ(readFile(), "ABCD");
}

TEST_F(BufferedWriterTest, Flush)
{
    writer_->append("data");
    writer_->flush();
    EXPECT_EQ(readFile(), "data");
}

TEST_F(BufferedWriterTest, Close)
{
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

TEST_F(BufferedWriterTest, NewLine)
{
    writer_->append("line1");
    writer_->newLine();
    writer_->append("line2");
    EXPECT_EQ(readFile(), "line1\nline2");
}

TEST_F(BufferedWriterTest, LargeWriteTriggersFlush)
{
    const std::string large(32, 'X');
    writer_->write(large);
    EXPECT_EQ(readFile(), large);
}

TEST_F(BufferedWriterTest, ToString)
{
    writer_->append("content");
    EXPECT_EQ(writer_->toString(), "content");
}

TEST_F(BufferedWriterTest, AppendSubstring)
{
    writer_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(readFile(), "hello");
}

TEST_F(BufferedWriterTest, WriteAfterClose)
{
    writer_->close();
    EXPECT_TRUE(writer_->isClosed());
}

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
