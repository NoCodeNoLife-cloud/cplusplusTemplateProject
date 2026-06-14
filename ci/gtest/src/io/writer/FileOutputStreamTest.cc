/**
 * @file FileOutputStreamTest.cc
 * @brief Unit tests for FileOutputStream
 * @details Tests cover file creation, writing, flush, close, idempotent close, and error handling.
 *          Temporary files are cleaned up after each test.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "io/writer/FileOutputStream.hpp"

using namespace common::io::writer;

namespace fs = std::filesystem;

/// @brief Test fixture for FileOutputStream tests using temporary files.
class FileOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        test_path_ = fs::temp_directory_path() / "fos_test_temp.bin";
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove(test_path_, ec);
    }

    fs::path test_path_;
};

/** @brief Create a file, write a single byte, and verify on disk.
    @details Writes 0x41, closes, then reads back from the file and confirms the byte value. */
TEST_F(FileOutputStreamTest, CreateAndWriteSingleByte)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x41});
    fos.close();

    std::ifstream in(test_path_, std::ios::binary);
    ASSERT_TRUE(in.is_open());
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 1);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x41);
}

/** @brief Write multiple bytes sequentially to a file.
    @details Writes {0x01,0x02,0x03}, closes, and reads back to verify all three bytes. */
TEST_F(FileOutputStreamTest, WriteMultipleBytes)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x01});
    fos.write(std::byte{0x02});
    fos.write(std::byte{0x03});
    fos.close();

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 3);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(content[1]), 0x02);
    EXPECT_EQ(static_cast<unsigned char>(content[2]), 0x03);
}

/** @brief Write a full byte vector to a file.
    @details Writes {0x10,0x20,0x30} and reads back from file. */
TEST_F(FileOutputStreamTest, WriteVector)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(data);
    }

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 3);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x10);
}

/** @brief Write a sub-range of a byte vector to a file.
    @details Writes offset=1, length=2 from {0x00,0x11,0x22,0x33} and reads back. */
TEST_F(FileOutputStreamTest, WriteVectorPartial)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(data, 1, 2);
    }

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x11);
    EXPECT_EQ(static_cast<unsigned char>(content[1]), 0x22);
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-1 buffer triggers std::out_of_range. */
TEST_F(FileOutputStreamTest, WriteVectorOffsetOutOfRange)
{
    FileOutputStream fos(test_path_.string(), false);
    const std::vector<std::byte> data = {std::byte{0x01}};
    EXPECT_THROW(fos.write(data, 5, 1), std::out_of_range);
}

/** @brief Write a raw C-style byte buffer to a file.
    @details Writes a constexpr array {0x41,0x42} with explicit length and reads back. */
TEST_F(FileOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x41}, std::byte{0x42}};
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(buf, 2);
    }

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 2);
}

/** @brief Null raw buffer with non-zero length throws.
    @details Passing nullptr with length 3 triggers std::invalid_argument. */
TEST_F(FileOutputStreamTest, WriteRawBufferNullThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    EXPECT_THROW(fos.write(nullptr, 3), std::invalid_argument);
}

/** @brief Append mode preserves existing file content.
    @details Writes 0x01 in non-append mode, then 0x02 in append mode, verifies both bytes. */
TEST_F(FileOutputStreamTest, AppendMode)
{
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(std::byte{0x01});
    }
    {
        FileOutputStream fos(test_path_.string(), true);
        fos.write(std::byte{0x02});
    }

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(content[1]), 0x02);
}

/** @brief Flush does not throw.
    @details Writes a byte, calls flush, and verifies no exception is thrown. */
TEST_F(FileOutputStreamTest, Flush)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x41});
    EXPECT_NO_THROW(fos.flush());
    fos.close();
}

/** @brief Close is idempotent.
    @details Calling close twice does not throw and leaves the stream closed. */
TEST_F(FileOutputStreamTest, CloseIsIdempotent)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x41});
    fos.close();
    EXPECT_NO_THROW(fos.close());
}

/** @brief isClosed returns correct state before and after close.
    @details Returns false initially and true after close. */
TEST_F(FileOutputStreamTest, IsClosedAfterClose)
{
    FileOutputStream fos(test_path_.string(), false);
    EXPECT_FALSE(fos.isClosed());
    fos.close();
    EXPECT_TRUE(fos.isClosed());
}

/** @brief Constructor accepts a C-string path.
    @details Creates a FileOutputStream with .c_str() path and verifies file is created. */
TEST_F(FileOutputStreamTest, CStringConstructor)
{
    {
        FileOutputStream fos(test_path_.string().c_str(), false);
        fos.write(std::byte{0x42});
    }
    EXPECT_TRUE(fs::exists(test_path_));
}

/** @brief Constructor accepts a std::filesystem::path.
    @details Creates a FileOutputStream with fs::path and verifies file is created. */
TEST_F(FileOutputStreamTest, FilesystemPathConstructor)
{
    {
        FileOutputStream fos(test_path_, false);
        fos.write(std::byte{0x43});
    }
    EXPECT_TRUE(fs::exists(test_path_));
}

/** @brief Write after close throws.
    @details Closing then writing triggers std::ios_base::failure. */
TEST_F(FileOutputStreamTest, WriteAfterCloseThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.close();
    EXPECT_THROW(fos.write(std::byte{0x00}), std::ios_base::failure);
}

/** @brief Flush after close throws.
    @details Closing then flushing triggers std::ios_base::failure. */
TEST_F(FileOutputStreamTest, FlushAfterCloseThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.close();
    EXPECT_THROW(fos.flush(), std::ios_base::failure);
}

/** @brief Truncate mode overwrites existing file content.
    @details Writes two bytes, then reopens in non-append mode and writes one byte; only the last byte survives. */
TEST_F(FileOutputStreamTest, TruncateMode)
{
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(std::byte{0x01});
        fos.write(std::byte{0x02});
    }
    {
        FileOutputStream fos(test_path_.string(), false);
        fos.write(std::byte{0x03});
    }

    std::ifstream in(test_path_, std::ios::binary);
    std::vector<char> content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ASSERT_EQ(content.size(), 1);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x03);
}
