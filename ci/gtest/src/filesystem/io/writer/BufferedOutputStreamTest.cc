/**
 * @file BufferedOutputStreamTest.cc
 * @brief Unit tests for BufferedOutputStream
 * @details Tests cover write operations, buffer flushing, close, and edge cases using ByteArrayOutputStream.
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "filesystem/io/writer/BufferedOutputStream.hpp"
#include "filesystem/io/writer/ByteArrayOutputStream.hpp"

using namespace common::filesystem::io::writer;

class BufferedOutputStreamTest : public testing::Test
{
protected:
    ByteArrayOutputStream* inner_;
    std::unique_ptr<BufferedOutputStream> stream_;

    void SetUp() override
    {
        auto inner = std::make_unique<ByteArrayOutputStream>();
        inner_ = inner.get();
        stream_ = std::make_unique<BufferedOutputStream>(std::move(inner));
    }
};

TEST_F(BufferedOutputStreamTest, WriteSingleByte)
{
    stream_->write(std::byte{0x41});
    stream_->flush();
    auto data = inner_->toByteArray();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], std::byte{0x41});
}

TEST_F(BufferedOutputStreamTest, WriteMultipleBytes)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    stream_->write(data, 0, data.size());
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{0x10});
    EXPECT_EQ(result[1], std::byte{0x20});
    EXPECT_EQ(result[2], std::byte{0x30});
}

TEST_F(BufferedOutputStreamTest, WriteRawBuffer)
{
    const std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    stream_->write(buf, 3);
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{0x01});
}

TEST_F(BufferedOutputStreamTest, FlushEmptiesBuffer)
{
    stream_->write(std::byte{0xFF});
    stream_->flush();
    EXPECT_EQ(stream_->getBufferedDataSize(), 0);
}

TEST_F(BufferedOutputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

TEST_F(BufferedOutputStreamTest, CloseIsIdempotent)
{
    stream_->close();
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

TEST_F(BufferedOutputStreamTest, GetBufferSize)
{
    EXPECT_EQ(stream_->getBufferSize(), 8192);
}

TEST_F(BufferedOutputStreamTest, CustomBufferSize)
{
    auto inner = std::make_unique<ByteArrayOutputStream>();
    auto custom = std::make_unique<BufferedOutputStream>(std::move(inner), 64);
    EXPECT_EQ(custom->getBufferSize(), 64);
}

TEST_F(BufferedOutputStreamTest, LargeWriteTriggersAutoFlush)
{
    std::vector<std::byte> large(16384, std::byte{0xAA});
    stream_->write(large, 0, large.size());
    auto result = inner_->toByteArray();
    ASSERT_GE(result.size(), 8192);
}

TEST_F(BufferedOutputStreamTest, WriteAfterClose)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

TEST_F(BufferedOutputStreamTest, WriteByteArrayWithOffset)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    stream_->write(data, 1, 2);
    stream_->flush();
    auto result = inner_->toByteArray();
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], std::byte{0x11});
    EXPECT_EQ(result[1], std::byte{0x22});
}

TEST_F(BufferedOutputStreamTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(BufferedOutputStream(nullptr), std::invalid_argument);
}

TEST_F(BufferedOutputStreamTest, ConstructorThrowsOnZeroSize)
{
    auto inner = std::make_unique<ByteArrayOutputStream>();
    EXPECT_THROW(BufferedOutputStream(std::move(inner), 0), std::invalid_argument);
}