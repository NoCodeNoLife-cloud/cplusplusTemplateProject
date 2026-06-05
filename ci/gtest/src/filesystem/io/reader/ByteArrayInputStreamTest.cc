#include <gtest/gtest.h>
#include <vector>

#include "filesystem/io/reader/ByteArrayInputStream.hpp"

using namespace common::filesystem;

class ByteArrayInputStreamTest : public testing::Test
{
protected:
    std::vector<std::byte> data_;
    std::unique_ptr<ByteArrayInputStream> stream_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}};
        stream_ = std::make_unique<ByteArrayInputStream>(data_);
    }
};

TEST_F(ByteArrayInputStreamTest, ReadSingleByte)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
    EXPECT_TRUE(stream_->isEof());
}

TEST_F(ByteArrayInputStreamTest, ReadIntoBuffer)
{
    std::vector<std::byte> buf(3);
    auto n = stream_->read(buf);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], std::byte{0x41});
    EXPECT_EQ(buf[1], std::byte{0x42});
    EXPECT_EQ(buf[2], std::byte{0x43});
}

TEST_F(ByteArrayInputStreamTest, ReadIntoBufferWithOffset)
{
    std::vector<std::byte> buf(5);
    auto n = stream_->read(buf, 1, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[1], std::byte{0x41});
    EXPECT_EQ(buf[2], std::byte{0x42});
}

TEST_F(ByteArrayInputStreamTest, Available)
{
    EXPECT_EQ(stream_->available(), 3);
    stream_->read();
    EXPECT_EQ(stream_->available(), 2);
}

TEST_F(ByteArrayInputStreamTest, Skip)
{
    auto skipped = stream_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

TEST_F(ByteArrayInputStreamTest, MarkAndReset)
{
    EXPECT_TRUE(stream_->markSupported());
    stream_->mark(10);
    stream_->read();
    stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

TEST_F(ByteArrayInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
    EXPECT_TRUE(stream_->isEof());
}

TEST_F(ByteArrayInputStreamTest, ReadAfterClose)
{
    stream_->close();
    auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

TEST_F(ByteArrayInputStreamTest, EofOnEmptyBuffer)
{
    ByteArrayInputStream empty(std::vector<std::byte>{});
    EXPECT_TRUE(empty.isEof());
    EXPECT_EQ(empty.read(), static_cast<std::byte>(-1));
}

TEST_F(ByteArrayInputStreamTest, ReadZeroLenReturnsZero)
{
    std::vector<std::byte> buf(3);
    auto n = stream_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}
