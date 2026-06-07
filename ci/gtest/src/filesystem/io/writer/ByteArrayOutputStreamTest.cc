/**
 * @file ByteArrayOutputStreamTest.cc
 * @brief Unit tests for ByteArrayOutputStream
 * @details Tests cover write operations, buffer expansion, conversion to byte array/string, reset, and edge cases.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <vector>

#include "filesystem/io/writer/ByteArrayOutputStream.hpp"

using namespace common::filesystem::io::writer;

class ByteArrayOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stream_ = std::make_unique<ByteArrayOutputStream>();
    }

    std::unique_ptr<ByteArrayOutputStream> stream_;
};

TEST_F(ByteArrayOutputStreamTest, InitiallyEmpty)
{
    EXPECT_EQ(stream_->size(), 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
    EXPECT_EQ(stream_->toString(), "");
}

TEST_F(ByteArrayOutputStreamTest, WriteSingleByte)
{
    stream_->write(std::byte{0x41});
    EXPECT_EQ(stream_->size(), 1);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 1);
    EXPECT_EQ(arr[0], std::byte{0x41});
}

TEST_F(ByteArrayOutputStreamTest, WriteMultipleBytes)
{
    stream_->write(std::byte{0x01});
    stream_->write(std::byte{0x02});
    stream_->write(std::byte{0x03});
    EXPECT_EQ(stream_->size(), 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[1], std::byte{0x02});
    EXPECT_EQ(arr[2], std::byte{0x03});
}

TEST_F(ByteArrayOutputStreamTest, WriteVector)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    stream_->write(data);
    EXPECT_EQ(stream_->size(), 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x10});
    EXPECT_EQ(arr[1], std::byte{0x20});
    EXPECT_EQ(arr[2], std::byte{0x30});
}

TEST_F(ByteArrayOutputStreamTest, WriteVectorPartial)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    stream_->write(data, 1, 2);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x11});
    EXPECT_EQ(arr[1], std::byte{0x22});
}

TEST_F(ByteArrayOutputStreamTest, WriteVectorOffsetOutOfRange)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(stream_->write(data, 5, 1), std::out_of_range);
}

TEST_F(ByteArrayOutputStreamTest, WriteVectorLengthExceeds)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(stream_->write(data, 1, 5), std::out_of_range);
}

TEST_F(ByteArrayOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    stream_->write(buf, 3);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[2], std::byte{0x03});
}

TEST_F(ByteArrayOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(stream_->write(nullptr, 3), std::invalid_argument);
}

TEST_F(ByteArrayOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    stream_->write(empty);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

TEST_F(ByteArrayOutputStreamTest, WriteZeroLengthRawBufferDoesNothing)
{
    const std::byte* buf = nullptr;
    stream_->write(buf, 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

TEST_F(ByteArrayOutputStreamTest, WriteToTransfersContent)
{
    stream_->write(std::byte{0x41});
    stream_->write(std::byte{0x42});

    ByteArrayOutputStream target;
    stream_->writeTo(target);
    const auto arr = target.toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x41});
    EXPECT_EQ(arr[1], std::byte{0x42});
}

TEST_F(ByteArrayOutputStreamTest, WriteToEmptyDoesNothing)
{
    ByteArrayOutputStream target;
    stream_->writeTo(target);
    EXPECT_TRUE(target.toByteArray().empty());
}

TEST_F(ByteArrayOutputStreamTest, ResetClearsContent)
{
    stream_->write(std::byte{0x41});
    ASSERT_EQ(stream_->size(), 1);
    stream_->reset();
    EXPECT_EQ(stream_->size(), 0);
    EXPECT_TRUE(stream_->toByteArray().empty());
}

TEST_F(ByteArrayOutputStreamTest, ToString)
{
    stream_->write(std::byte{'H'});
    stream_->write(std::byte{'i'});
    EXPECT_EQ(stream_->toString(), "Hi");
}

TEST_F(ByteArrayOutputStreamTest, Capacity)
{
    const size_t cap = stream_->capacity();
    EXPECT_GE(cap, 32);
}

TEST_F(ByteArrayOutputStreamTest, ConstructorWithSize)
{
    auto s = ByteArrayOutputStream(64);
    EXPECT_EQ(s.size(), 0);
    EXPECT_GE(s.capacity(), 64);
}

TEST_F(ByteArrayOutputStreamTest, ConstructorThrowsOnZeroSize)
{
    EXPECT_THROW(ByteArrayOutputStream(0), std::invalid_argument);
}

TEST_F(ByteArrayOutputStreamTest, BufferExpandsAutomatically)
{
    const std::vector<std::byte> large(1000, std::byte{0xFF});
    stream_->write(large);
    EXPECT_EQ(stream_->size(), 1000);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 1000);
    EXPECT_EQ(arr[0], std::byte{0xFF});
    EXPECT_EQ(arr[999], std::byte{0xFF});
}

TEST_F(ByteArrayOutputStreamTest, FlushIsNoOp)
{
    stream_->write(std::byte{0x41});
    stream_->flush();
    EXPECT_EQ(stream_->size(), 1);
}

TEST_F(ByteArrayOutputStreamTest, CloseIsNoOp)
{
    stream_->write(std::byte{0x41});
    stream_->close();
    EXPECT_FALSE(stream_->isClosed());
    stream_->write(std::byte{0x42});
    EXPECT_EQ(stream_->size(), 2);
}

TEST_F(ByteArrayOutputStreamTest, IsClosed)
{
    EXPECT_FALSE(stream_->isClosed());
}

TEST_F(ByteArrayOutputStreamTest, WriteAfterWriteTo)
{
    stream_->write(std::byte{0x01});
    ByteArrayOutputStream target;
    stream_->writeTo(target);
    stream_->write(std::byte{0x02});
    EXPECT_EQ(stream_->size(), 2);
    const auto arr = stream_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x01});
    EXPECT_EQ(arr[1], std::byte{0x02});
}
