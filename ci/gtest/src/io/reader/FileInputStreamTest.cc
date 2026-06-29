/**
 * @file FileInputStreamTest.cc
 * @brief Unit tests for the FileInputStream class
 * @details Tests cover file reading operations including construction from
 *          different path types, single/bulk reads, skip, available, close,
 *          and error handling for non-existent files.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

#include <cppforge/io/reader/FileInputStream.hpp>

using namespace cppforge::io::reader;

/**
 * @brief Test fixture for FileInputStreamTest tests
 * @details Creates a temporary file with known content for each test.
 */
class FileInputStreamTest : public testing::Test
{
protected:
    std::string temp_file_;
    std::string content_;

    void SetUp() override
    {
        temp_file_ = "test_file_input_stream_temp.bin";
        content_ = "Hello, FileInputStream!";

        std::ofstream ofs(temp_file_, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs.write(content_.c_str(), static_cast<std::streamsize>(content_.size()));
        ofs.close();
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(temp_file_, ec);
    }
};

/**
 * @brief Test construction from std::string
 * @details Verifies that a file can be opened with string path
 */
TEST_F(FileInputStreamTest, ConstructFromString)
{
    EXPECT_NO_THROW(FileInputStream f(temp_file_));
}

/**
 * @brief Test construction from const char*
 * @details Verifies that a file can be opened with C-string path
 */
TEST_F(FileInputStreamTest, ConstructFromCString)
{
    EXPECT_NO_THROW(FileInputStream f(temp_file_.c_str()));
}

/**
 * @brief Test construction from std::filesystem::path
 * @details Verifies that a file can be opened with filesystem path
 */
TEST_F(FileInputStreamTest, ConstructFromPath)
{
    const std::filesystem::path p(temp_file_);
    EXPECT_NO_THROW(FileInputStream f(p));
}

/**
 * @brief Test construction with non-existent file throws
 * @details Opening a non-existent file should throw invalid_argument
 */
TEST_F(FileInputStreamTest, ConstructNonExistentThrows)
{
    EXPECT_THROW(FileInputStream f("nonexistent_file_xyz.bin"), std::invalid_argument);
}

/**
 * @brief Test read single byte
 * @details Verifies reading one byte at a time
 */
TEST_F(FileInputStreamTest, ReadSingleByte)
{
    FileInputStream f(temp_file_);
    for (const char c : content_)
    {
        EXPECT_EQ(f.read(), static_cast<std::byte>(c));
    }
    // EOF triggered on next read beyond end
    EXPECT_FALSE(f.isEof());
    EXPECT_EQ(f.read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(f.isEof());
}

/**
 * @brief Test read into buffer
 * @details Verifies reading into a byte buffer
 */
TEST_F(FileInputStreamTest, ReadIntoBuffer)
{
    FileInputStream f(temp_file_);
    std::vector<std::byte> buf(content_.size());
    const auto n = f.read(buf);
    EXPECT_EQ(n, content_.size());
    for (size_t i = 0; i < content_.size(); ++i)
    {
        EXPECT_EQ(buf[i], static_cast<std::byte>(content_[i]));
    }
}

/**
 * @brief Test read into buffer with offset
 * @details Verifies reading into a buffer at a specific offset
 */
TEST_F(FileInputStreamTest, ReadIntoBufferWithOffset)
{
    FileInputStream f(temp_file_);
    std::vector<std::byte> buf(content_.size() + 5);
    const auto n = f.read(buf, 2, content_.size());
    EXPECT_EQ(n, content_.size());
    for (size_t i = 0; i < content_.size(); ++i)
    {
        EXPECT_EQ(buf[i + 2], static_cast<std::byte>(content_[i]));
    }
}

/**
 * @brief Test skip
 * @details Verifies that skipping bytes works correctly
 */
TEST_F(FileInputStreamTest, Skip)
{
    FileInputStream f(temp_file_);
    const auto skipped = f.skip(7);
    EXPECT_EQ(skipped, 7);
    EXPECT_EQ(f.read(), static_cast<std::byte>('F'));
}

/**
 * @brief Test skip to end
 * @details Skipping beyond available stops when seekg fails.
 *          FileInputStream::skip uses seekg, not read(), to skip bytes.
 */
TEST_F(FileInputStreamTest, SkipToEnd)
{
    FileInputStream f(temp_file_);
    const auto content_len = static_cast<size_t>(content_.size());
    // FileInputStream::skip seeks forward; seeking beyond EOF returns 0
    // First skip covers the full file
    const auto skipped = f.skip(static_cast<int64_t>(content_len));
    EXPECT_EQ(skipped, static_cast<int64_t>(content_len));
    // EOF triggered on next read
    EXPECT_EQ(f.read(), static_cast<std::byte>(-1));
    EXPECT_TRUE(f.isEof());
}

/**
 * @brief Test available
 * @details Verifies available returns correct remaining bytes
 */
TEST_F(FileInputStreamTest, Available)
{
    FileInputStream f(temp_file_);
    const auto content_len = static_cast<size_t>(content_.size());
    EXPECT_EQ(f.available(), content_len);
    (void)f.read();
    EXPECT_EQ(f.available(), content_len - 1);
}

/**
 * @brief Test markSupported returns false
 * @details FileInputStream does not support mark/reset
 */
TEST_F(FileInputStreamTest, MarkSupportedFalse)
{
    FileInputStream f(temp_file_);
    EXPECT_FALSE(f.markSupported());
}

/**
 * @brief Test close and isClosed
 * @details Verifies close/isClosed contract
 */
TEST_F(FileInputStreamTest, Close)
{
    FileInputStream f(temp_file_);
    EXPECT_FALSE(f.isClosed());
    f.close();
    EXPECT_TRUE(f.isClosed());
}

/**
 * @brief Test read after close returns EOF
 * @details After close, read returns EOF marker
 */
TEST_F(FileInputStreamTest, ReadAfterClose)
{
    FileInputStream f(temp_file_);
    f.close();
    const auto byte = f.read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(f.isEof());
}

/**
 * @brief Test read(buf) after close returns 0
 * @details After close, read(buf) returns 0
 */
TEST_F(FileInputStreamTest, ReadBufAfterClose)
{
    FileInputStream f(temp_file_);
    f.close();
    std::vector<std::byte> buf(10);
    const auto n = f.read(buf);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test skip after close returns 0
 * @details After close, skip returns 0
 */
TEST_F(FileInputStreamTest, SkipAfterClose)
{
    FileInputStream f(temp_file_);
    f.close();
    const auto skipped = f.skip(5);
    EXPECT_EQ(skipped, 0);
}

/**
 * @brief Test available after close returns 0
 * @details After close, available returns 0
 */
TEST_F(FileInputStreamTest, AvailableAfterClose)
{
    FileInputStream f(temp_file_);
    f.close();
    EXPECT_EQ(f.available(), 0);
}

/**
 * @brief Test destructor closes the file
 * @details FileInputStream destructor should close the file automatically
 */
TEST_F(FileInputStreamTest, DestructorCloses)
{
    {
        FileInputStream f(temp_file_);
        EXPECT_FALSE(f.isClosed());
    }
    // File should be closed after destruction (verified by no crash/leak)
}

/**
 * @brief Test read with invalid buffer offset throws
 * @details Validates buffer bounds checking
 */
TEST_F(FileInputStreamTest, ReadInvalidOffsetThrows)
{
    FileInputStream f(temp_file_);
    std::vector<std::byte> buf(3);
    EXPECT_THROW(f.read(buf, 5, 1), std::out_of_range);
}

/**
 * @brief Test read with overflow length throws
 * @details Validates length against buffer capacity check
 */
TEST_F(FileInputStreamTest, ReadOverflowLenThrows)
{
    FileInputStream f(temp_file_);
    std::vector<std::byte> buf(3);
    EXPECT_THROW(f.read(buf, 0, 10), std::out_of_range);
}

/**
 * @brief Test constructor with directory path throws
 * @details Opening a directory as a file should throw
 */
TEST_F(FileInputStreamTest, ConstructDirectoryThrows)
{
    EXPECT_THROW(FileInputStream f("."), std::invalid_argument);
}
