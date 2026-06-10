#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/BufferedInputStream.hpp"
#include "io/reader/ByteArrayInputStream.hpp"

using namespace common::io::reader;

class BufferedInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<BufferedInputStream> stream_;

    void SetUp() override
    {
        auto inner = std::make_unique<ByteArrayInputStream>(
            std::vector{std::byte{0x10}, std::byte{0x20}, std::byte{0x30}, std::byte{0x40}, std::byte{0x50}});
        stream_ = std::make_unique<BufferedInputStream>(std::move(inner));
    }
};

TEST_F(BufferedInputStreamTest, ReadSingleByte)
{
    EXPECT_EQ(stream_->read(), std::byte{0x10});
    EXPECT_EQ(stream_->read(), std::byte{0x20});
}

TEST_F(BufferedInputStreamTest, ReadMultipleBytes)
{
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], std::byte{0x10});
    EXPECT_EQ(buf[1], std::byte{0x20});
    EXPECT_EQ(buf[2], std::byte{0x30});
}

TEST_F(BufferedInputStreamTest, ReadAllWithSmallBuffer)
{
    std::vector<std::byte> small(2);
    std::vector<std::byte> result;
    size_t n;
    while ((n = stream_->read(small, 0, 2)) > 0)
    {
        result.insert(result.end(), small.begin(), small.begin() + static_cast<std::ptrdiff_t>(n));
    }
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], std::byte{0x10});
    EXPECT_EQ(result[4], std::byte{0x50});
}

TEST_F(BufferedInputStreamTest, Available)
{
    const auto avail = stream_->available();
    EXPECT_EQ(avail, 5);
}

TEST_F(BufferedInputStreamTest, Skip)
{
    const auto skipped = stream_->skip(3);
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(stream_->read(), std::byte{0x40});
}

TEST_F(BufferedInputStreamTest, MarkAndReset)
{
    EXPECT_TRUE(stream_->markSupported());
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

TEST_F(BufferedInputStreamTest, Close)
{
    stream_->close();
    EXPECT_TRUE(stream_->isClosed());
}

TEST_F(BufferedInputStreamTest, ReadAfterClose)
{
    stream_->close();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
    EXPECT_TRUE(stream_->isEof());
}

TEST_F(BufferedInputStreamTest, CustomBufferSize)
{
    auto inner = std::make_unique<ByteArrayInputStream>(std::vector{std::byte{0x01}, std::byte{0x02}});
    BufferedInputStream custom(std::move(inner), 4);
    EXPECT_EQ(custom.read(), std::byte{0x01});
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(BufferedInputStreamTest, ReadBufAfterClose)
{
    stream_->close();
    std::vector<std::byte> buf(3);
    const auto n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 0);
}

TEST_F(BufferedInputStreamTest, SkipBeyondEnd)
{
    const auto skipped = stream_->skip(100);
    EXPECT_EQ(skipped, 5);
}

TEST_F(BufferedInputStreamTest, SkipAfterClose)
{
    stream_->close();
    const auto skipped = stream_->skip(5);
    EXPECT_EQ(skipped, 0);
}

TEST_F(BufferedInputStreamTest, AvailableAfterRead)
{
    (void)stream_->read();
    EXPECT_EQ(stream_->available(), 4);
}

TEST_F(BufferedInputStreamTest, AvailableAfterClose)
{
    stream_->close();
    EXPECT_EQ(stream_->available(), 0);
}

TEST_F(BufferedInputStreamTest, CloseIsIdempotent)
{
    stream_->close();
    EXPECT_NO_THROW(stream_->close());
}

TEST_F(BufferedInputStreamTest, ReadSingleAfterExhaustion)
{
    for (int i = 0; i < 5; ++i) (void)stream_->read();
    const auto byte = stream_->read();
    EXPECT_EQ(byte, static_cast<std::byte>(-1));
}

TEST_F(BufferedInputStreamTest, ResetAfterMark)
{
    stream_->mark(10);
    (void)stream_->read();
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x10});
}

TEST_F(BufferedInputStreamTest, MultipleMarkReset)
{
    stream_->mark(10);
    (void)stream_->read();
    stream_->reset();
    (void)stream_->read();
    stream_->mark(10);
    (void)stream_->read();
    stream_->reset();
    EXPECT_EQ(stream_->read(), std::byte{0x20});
}

TEST_F(BufferedInputStreamTest, ReadBufAfterExhaustion)
{
    std::vector<std::byte> buf(5);
    auto n = stream_->read(buf, 0, 5);
    EXPECT_EQ(n, 5);
    n = stream_->read(buf, 0, 3);
    EXPECT_EQ(n, 0);
}
