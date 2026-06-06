#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "filesystem/io/reader/PushbackInputStream.hpp"
#include "filesystem/io/reader/ByteArrayInputStream.hpp"

using namespace common::filesystem;

class PushbackInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<PushbackInputStream> stream_;
    std::vector<std::byte> data_;

    void SetUp() override
    {
        data_ = {std::byte{0x41}, std::byte{0x42}, std::byte{0x43}};
        auto inner = std::make_unique<ByteArrayInputStream>(data_);
        stream_ = std::make_unique<PushbackInputStream>(std::move(inner), 4);
    }
};

TEST_F(PushbackInputStreamTest, ReadNormal)
{
    EXPECT_EQ(stream_->read(), std::byte{0x41});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
    EXPECT_EQ(stream_->read(), std::byte{0x43});
}

TEST_F(PushbackInputStreamTest, UnreadSingleAndReRead)
{
    const auto byte = stream_->read();
    EXPECT_EQ(byte, std::byte{0x41});
    stream_->unread(byte);
    EXPECT_EQ(stream_->read(), std::byte{0x41});
}

TEST_F(PushbackInputStreamTest, UnreadBufferAndReRead)
{
    (void)stream_->read();
    stream_->unread(std::vector{std::byte{0xFF}, std::byte{0xFE}});
    EXPECT_EQ(stream_->read(), std::byte{0xFF});
    EXPECT_EQ(stream_->read(), std::byte{0xFE});
    EXPECT_EQ(stream_->read(), std::byte{0x42});
}

TEST_F(PushbackInputStreamTest, UnreadOverflow)
{
    const std::vector<std::byte> large(5);
    EXPECT_THROW(stream_->unread(large), std::overflow_error);
}

TEST_F(PushbackInputStreamTest, Available)
{
    const auto avail = stream_->available();
    EXPECT_GE(avail, 3);
}

TEST_F(PushbackInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}
