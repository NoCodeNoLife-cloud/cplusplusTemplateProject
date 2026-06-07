/**
 * @file AbstractOutputStreamTest.cc
 * @brief Unit tests for AbstractOutputStream
 * @details Tests cover write operations, bounds checking, flush and close via a mock implementation.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <vector>

#include "filesystem/io/writer/AbstractOutputStream.hpp"

using namespace common::filesystem;

namespace
{
    class MockOutputStream : public AbstractOutputStream
    {
    public:
        using AbstractOutputStream::write;

        std::vector<std::byte> written_;
        int flush_count_ = 0;
        bool closed_ = false;

        void write(const std::byte b) override
        {
            written_.push_back(b);
        }

        void write(const std::byte* buffer, const size_t length) override
        {
            if (length == 0)
            {
                return;
            }
            if (!buffer)
            {
                throw std::invalid_argument("Buffer cannot be null");
            }
            written_.insert(written_.end(), buffer, buffer + length);
        }

        void flush() override
        {
            ++flush_count_;
        }

        void close() override
        {
            closed_ = true;
        }

        [[nodiscard]] bool isClosed() const override
        {
            return closed_;
        }
    };
}

class AbstractOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_ = std::make_unique<MockOutputStream>();
    }

    std::unique_ptr<MockOutputStream> mock_;
};

TEST_F(AbstractOutputStreamTest, WriteSingleByte)
{
    mock_->write(std::byte{0x41});
    ASSERT_EQ(mock_->written_.size(), 1);
    EXPECT_EQ(mock_->written_[0], std::byte{0x41});
}

TEST_F(AbstractOutputStreamTest, WriteVectorFull)
{
    const std::vector data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    mock_->write(data);
    ASSERT_EQ(mock_->written_.size(), 3);
    EXPECT_EQ(mock_->written_[0], std::byte{0x10});
    EXPECT_EQ(mock_->written_[1], std::byte{0x20});
    EXPECT_EQ(mock_->written_[2], std::byte{0x30});
}

TEST_F(AbstractOutputStreamTest, WriteVectorPartial)
{
    const std::vector data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    mock_->write(data, 1, 2);
    ASSERT_EQ(mock_->written_.size(), 2);
    EXPECT_EQ(mock_->written_[0], std::byte{0x11});
    EXPECT_EQ(mock_->written_[1], std::byte{0x22});
}

TEST_F(AbstractOutputStreamTest, WriteVectorBoundsCheckThrows)
{
    const std::vector data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(mock_->write(data, 5, 1), std::out_of_range);
}

TEST_F(AbstractOutputStreamTest, WriteVectorLengthExceedsThrows)
{
    const std::vector data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(mock_->write(data, 1, 5), std::out_of_range);
}

TEST_F(AbstractOutputStreamTest, WriteVectorOffsetEqualsSizeThrows)
{
    const std::vector data = {std::byte{0x01}};
    EXPECT_THROW(mock_->write(data, 1, 1), std::out_of_range);
}

TEST_F(AbstractOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    mock_->write(buf, 3);
    ASSERT_EQ(mock_->written_.size(), 3);
    EXPECT_EQ(mock_->written_[0], std::byte{0x01});
}

TEST_F(AbstractOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(mock_->write(nullptr, 3), std::invalid_argument);
}

TEST_F(AbstractOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    mock_->write(empty);
    EXPECT_TRUE(mock_->written_.empty());
}

TEST_F(AbstractOutputStreamTest, WriteEmptyRawBufferDoesNothing)
{
    const std::byte* buf = nullptr;
    mock_->write(buf, 0);
    // length 0 should not throw even with null
    EXPECT_TRUE(mock_->written_.empty());
}

TEST_F(AbstractOutputStreamTest, FlushCalled)
{
    EXPECT_EQ(mock_->flush_count_, 0);
    mock_->flush();
    EXPECT_EQ(mock_->flush_count_, 1);
}

TEST_F(AbstractOutputStreamTest, CloseCalled)
{
    EXPECT_FALSE(mock_->isClosed());
    mock_->close();
    EXPECT_TRUE(mock_->isClosed());
}
