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

#include "filesystem/io/writer/FileOutputStream.hpp"

using namespace common::filesystem::io::writer;

namespace fs = std::filesystem;

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

TEST_F(FileOutputStreamTest, WriteVectorOffsetOutOfRange)
{
    FileOutputStream fos(test_path_.string(), false);
    const std::vector<std::byte> data = {std::byte{0x01}};
    EXPECT_THROW(fos.write(data, 5, 1), std::out_of_range);
}

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

TEST_F(FileOutputStreamTest, WriteRawBufferNullThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    EXPECT_THROW(fos.write(nullptr, 3), std::invalid_argument);
}

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

TEST_F(FileOutputStreamTest, Flush)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x41});
    EXPECT_NO_THROW(fos.flush());
    fos.close();
}

TEST_F(FileOutputStreamTest, CloseIsIdempotent)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.write(std::byte{0x41});
    fos.close();
    EXPECT_NO_THROW(fos.close());
}

TEST_F(FileOutputStreamTest, IsClosedAfterClose)
{
    FileOutputStream fos(test_path_.string(), false);
    EXPECT_FALSE(fos.isClosed());
    fos.close();
    EXPECT_TRUE(fos.isClosed());
}

TEST_F(FileOutputStreamTest, CStringConstructor)
{
    {
        FileOutputStream fos(test_path_.string().c_str(), false);
        fos.write(std::byte{0x42});
    }
    EXPECT_TRUE(fs::exists(test_path_));
}

TEST_F(FileOutputStreamTest, FilesystemPathConstructor)
{
    {
        FileOutputStream fos(test_path_, false);
        fos.write(std::byte{0x43});
    }
    EXPECT_TRUE(fs::exists(test_path_));
}

TEST_F(FileOutputStreamTest, WriteAfterCloseThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.close();
    EXPECT_THROW(fos.write(std::byte{0x00}), std::ios_base::failure);
}

TEST_F(FileOutputStreamTest, FlushAfterCloseThrows)
{
    FileOutputStream fos(test_path_.string(), false);
    fos.close();
    EXPECT_THROW(fos.flush(), std::ios_base::failure);
}

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
