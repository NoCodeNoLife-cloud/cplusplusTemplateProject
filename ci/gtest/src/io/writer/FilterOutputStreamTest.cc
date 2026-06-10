/**
 * @file FilterOutputStreamTest.cc
 * @brief Unit tests for FilterOutputStream
 * @details Tests cover delegation to underlying stream, null handling, close and flush propagation.
 */

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <vector>

#include "io/writer/FilterOutputStream.hpp"
#include "io/writer/ByteArrayOutputStream.hpp"

using namespace common::io::writer;

class FilterOutputStreamTest : public testing::Test
{
protected:
    void SetUp() override
    {
        inner_ = std::make_shared<ByteArrayOutputStream>();
        filter_ = std::make_unique<FilterOutputStream>(inner_);
    }

    std::shared_ptr<ByteArrayOutputStream> inner_;
    std::unique_ptr<FilterOutputStream> filter_;
};

TEST_F(FilterOutputStreamTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(FilterOutputStream(nullptr), std::invalid_argument);
}

TEST_F(FilterOutputStreamTest, WriteSingleByteDelegates)
{
    filter_->write(std::byte{0x41});
    EXPECT_EQ(inner_->size(), 1);
    const auto arr = inner_->toByteArray();
    EXPECT_EQ(arr[0], std::byte{0x41});
}

TEST_F(FilterOutputStreamTest, WriteVector)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    filter_->write(data);
    EXPECT_EQ(inner_->size(), 3);
}

TEST_F(FilterOutputStreamTest, WriteVectorPartial)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    filter_->write(data, 1, 2);
    const auto arr = inner_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x11});
}

TEST_F(FilterOutputStreamTest, WriteVectorBoundsCheckThrows)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 5, 1), std::out_of_range);
}

TEST_F(FilterOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}};
    filter_->write(buf, 2);
    EXPECT_EQ(inner_->size(), 2);
}

TEST_F(FilterOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(filter_->write(nullptr, 3), std::invalid_argument);
}

TEST_F(FilterOutputStreamTest, FlushDelegates)
{
    EXPECT_NO_THROW(filter_->flush());
}

TEST_F(FilterOutputStreamTest, CloseFlushesAndDelegates)
{
    filter_->write(std::byte{0x41});
    EXPECT_NO_THROW(filter_->close());
    EXPECT_TRUE(filter_->isClosed());
}

TEST_F(FilterOutputStreamTest, CloseSetsClosedState)
{
    EXPECT_FALSE(filter_->isClosed());
    filter_->close();
    EXPECT_TRUE(filter_->isClosed());
}

TEST_F(FilterOutputStreamTest, WriteAfterCloseThrows)
{
    filter_->close();
    EXPECT_THROW(filter_->write(std::byte{0x00}), std::runtime_error);
}

TEST_F(FilterOutputStreamTest, FlushAfterCloseThrows)
{
    filter_->close();
    EXPECT_THROW(filter_->flush(), std::runtime_error);
}

TEST_F(FilterOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    filter_->write(empty);
    EXPECT_TRUE(inner_->toByteArray().empty());
}

TEST_F(FilterOutputStreamTest, WriteZeroLengthRawDoesNothing)
{
    filter_->write(nullptr, 0);
    EXPECT_TRUE(inner_->toByteArray().empty());
}

TEST_F(FilterOutputStreamTest, MultipleWritesAccumulate)
{
    filter_->write(std::byte{0x01});
    filter_->write(std::byte{0x02});

    const std::vector<std::byte> more = {std::byte{0x03}, std::byte{0x04}};
    filter_->write(more);

    EXPECT_EQ(inner_->size(), 4);
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(FilterOutputStreamTest, WriteVectorAfterCloseThrows)
{
    filter_->close();
    const std::vector<std::byte> data = {std::byte{0x01}};
    EXPECT_THROW(filter_->write(data), std::runtime_error);
}

TEST_F(FilterOutputStreamTest, WriteVectorPartialAfterCloseThrows)
{
    filter_->close();
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 0, 1), std::runtime_error);
}

TEST_F(FilterOutputStreamTest, WriteRawAfterCloseThrows)
{
    filter_->close();
    constexpr std::byte buf[] = {std::byte{0x01}};
    EXPECT_THROW(filter_->write(buf, 1), std::runtime_error);
}

TEST_F(FilterOutputStreamTest, CloseIsIdempotent)
{
    filter_->close();
    EXPECT_NO_THROW(filter_->close());
}

TEST_F(FilterOutputStreamTest, DestructionDoesNotCloseInner)
{
    {
        FilterOutputStream temp(inner_);
        temp.write(std::byte{0xFF});
    }
    // Inner stream should still be usable after filter destruction
    EXPECT_EQ(inner_->size(), 1);
    EXPECT_FALSE(inner_->isClosed());
}

TEST_F(FilterOutputStreamTest, WriteAfterFlush)
{
    filter_->write(std::byte{0x10});
    filter_->flush();
    filter_->write(std::byte{0x20});
    filter_->flush();
    EXPECT_EQ(inner_->size(), 2);
}

TEST_F(FilterOutputStreamTest, WriteVectorNegativeOffset)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, -1, 1), std::out_of_range);
}

TEST_F(FilterOutputStreamTest, WriteVectorNegativeLength)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 0, -1), std::out_of_range);
}

TEST_F(FilterOutputStreamTest, WriteVectorOffsetAtSize)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 2, 1), std::out_of_range);
}

TEST_F(FilterOutputStreamTest, WriteVectorOffsetExceedsSize)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 5, 1), std::out_of_range);
}
