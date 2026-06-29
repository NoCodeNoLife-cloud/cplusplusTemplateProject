/**
 * @file AbstractOutputStreamTest.cc
 * @brief Unit tests for AbstractOutputStream
 * @details Tests cover write operations, bounds checking, flush and close via a mock implementation.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <vector>

#include <cppforge/io/writer/AbstractOutputStream.hpp>

using namespace cppforge::io::writer;

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

/// @brief Test fixture for AbstractOutputStream tests using a MockOutputStream.
class AbstractOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_ = std::make_unique<MockOutputStream>();
    }

    std::unique_ptr<MockOutputStream> mock_;
};

/** @brief Write a single byte to the stream.
    @details Writes std::byte{0x41} and verifies it is stored in the mock's buffer. */
TEST_F(AbstractOutputStreamTest, WriteSingleByte)
{
    mock_->write(std::byte{0x41});
    ASSERT_EQ(mock_->written_.size(), 1);
    EXPECT_EQ(mock_->written_[0], std::byte{0x41});
}

/** @brief Write a full byte vector.
    @details Writes {0x10, 0x20, 0x30} and checks each element in the output. */
TEST_F(AbstractOutputStreamTest, WriteVectorFull)
{
    const std::vector data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    mock_->write(data);
    ASSERT_EQ(mock_->written_.size(), 3);
    EXPECT_EQ(mock_->written_[0], std::byte{0x10});
    EXPECT_EQ(mock_->written_[1], std::byte{0x20});
    EXPECT_EQ(mock_->written_[2], std::byte{0x30});
}

/** @brief Write a sub-range of a byte vector.
    @details Writes offset=1, length=2 from {0x00,0x11,0x22,0x33} and expects {0x11,0x22}. */
TEST_F(AbstractOutputStreamTest, WriteVectorPartial)
{
    const std::vector data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
    mock_->write(data, 1, 2);
    ASSERT_EQ(mock_->written_.size(), 2);
    EXPECT_EQ(mock_->written_[0], std::byte{0x11});
    EXPECT_EQ(mock_->written_[1], std::byte{0x22});
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-2 buffer triggers std::out_of_range. */
TEST_F(AbstractOutputStreamTest, WriteVectorBoundsCheckThrows)
{
    const std::vector data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(mock_->write(data, 5, 1), std::out_of_range);
}

/** @brief Vector length exceeding buffer size throws.
    @details Length 5 at offset 1 on a size-2 buffer triggers std::out_of_range. */
TEST_F(AbstractOutputStreamTest, WriteVectorLengthExceedsThrows)
{
    const std::vector data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(mock_->write(data, 1, 5), std::out_of_range);
}

/** @brief Offset equal to vector size with non-zero length throws.
    @details Offset=1 on size-1 buffer with length=1 triggers std::out_of_range. */
TEST_F(AbstractOutputStreamTest, WriteVectorOffsetEqualsSizeThrows)
{
    const std::vector data = {std::byte{0x01}};
    EXPECT_THROW(mock_->write(data, 1, 1), std::out_of_range);
}

/** @brief Write a raw C-style byte buffer.
    @details Writes a constexpr byte array {0x01,0x02,0x03} with explicit length 3. */
TEST_F(AbstractOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    mock_->write(buf, 3);
    ASSERT_EQ(mock_->written_.size(), 3);
    EXPECT_EQ(mock_->written_[0], std::byte{0x01});
}

/** @brief Null raw buffer with non-zero length throws.
    @details Passing nullptr with length 3 triggers std::invalid_argument. */
TEST_F(AbstractOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(mock_->write(nullptr, 3), std::invalid_argument);
}

/** @brief Empty vector write is a no-op.
    @details Writing an empty vector leaves the mock's buffer unchanged. */
TEST_F(AbstractOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    mock_->write(empty);
    EXPECT_TRUE(mock_->written_.empty());
}

/** @brief Zero-length raw buffer write is a no-op even with null pointer.
    @details Null pointer with length 0 gracefully returns without writing. */
TEST_F(AbstractOutputStreamTest, WriteEmptyRawBufferDoesNothing)
{
    const std::byte* buf = nullptr;
    mock_->write(buf, 0);
    EXPECT_TRUE(mock_->written_.empty());
}

/** @brief Flush increments the mock's flush counter.
    @details Calls flush and verifies the counter changes from 0 to 1. */
TEST_F(AbstractOutputStreamTest, FlushCalled)
{
    EXPECT_EQ(mock_->flush_count_, 0);
    mock_->flush();
    EXPECT_EQ(mock_->flush_count_, 1);
}

/** @brief Close marks the stream as closed.
    @details Verifies isClosed returns true after calling close. */
TEST_F(AbstractOutputStreamTest, CloseCalled)
{
    EXPECT_FALSE(mock_->isClosed());
    mock_->close();
    EXPECT_TRUE(mock_->isClosed());
}

/** @brief Close is idempotent.
    @details Double close does not throw and leaves the stream closed. */
TEST_F(AbstractOutputStreamTest, CloseIsIdempotent)
{
    mock_->close();
    EXPECT_NO_THROW(mock_->close());
    EXPECT_TRUE(mock_->isClosed());
}

/** @brief Multiple flush cycles accumulate the counter.
    @details Five consecutive flushes increment flush_count_ to 5. */
TEST_F(AbstractOutputStreamTest, MultipleFlushCycles)
{
    for (int i = 0; i < 5; ++i)
    {
        mock_->flush();
    }
    EXPECT_EQ(mock_->flush_count_, 5);
}

