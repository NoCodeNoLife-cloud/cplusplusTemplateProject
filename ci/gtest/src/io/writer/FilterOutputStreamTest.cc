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

using namespace cppforge::io::writer;

/// @brief Test fixture for FilterOutputStream tests using ByteArrayOutputStream.
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

/** @brief Constructor throws on null inner stream.
    @details Passing nullptr triggers std::invalid_argument. */
TEST_F(FilterOutputStreamTest, ConstructorThrowsOnNull)
{
    EXPECT_THROW(FilterOutputStream(nullptr), std::invalid_argument);
}

/** @brief Write a single byte delegates to inner stream.
    @details Writes 0x41 and verifies it appears in the ByteArrayOutputStream. */
TEST_F(FilterOutputStreamTest, WriteSingleByteDelegates)
{
    filter_->write(std::byte{0x41});
    EXPECT_EQ(inner_->size(), 1);
    const auto arr = inner_->toByteArray();
    EXPECT_EQ(arr[0], std::byte{0x41});
}

/** @brief Write a full byte vector delegates to inner stream.
    @details Writes {0x10,0x20,0x30} and verifies size in inner stream. */
TEST_F(FilterOutputStreamTest, WriteVector)
{
    const std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    filter_->write(data);
    EXPECT_EQ(inner_->size(), 3);
}

/** @brief Write a sub-range of a byte vector delegates to inner stream.
    @details Writes offset=1, length=2 from {0x00,0x11,0x22} and expects {0x11}. */
TEST_F(FilterOutputStreamTest, WriteVectorPartial)
{
    const std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    filter_->write(data, 1, 2);
    const auto arr = inner_->toByteArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0], std::byte{0x11});
}

/** @brief Out-of-bounds vector offset throws.
    @details Offset 5 on a size-2 buffer triggers std::out_of_range. */
TEST_F(FilterOutputStreamTest, WriteVectorBoundsCheckThrows)
{
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 5, 1), std::out_of_range);
}

/** @brief Write a raw byte buffer delegates to inner stream.
    @details Writes a constexpr array {0x01,0x02} with explicit length 2. */
TEST_F(FilterOutputStreamTest, WriteRawBuffer)
{
    constexpr std::byte buf[] = {std::byte{0x01}, std::byte{0x02}};
    filter_->write(buf, 2);
    EXPECT_EQ(inner_->size(), 2);
}

/** @brief Null raw buffer with non-zero length throws.
    @details Passing nullptr with length 3 triggers std::invalid_argument. */
TEST_F(FilterOutputStreamTest, WriteRawBufferNullThrows)
{
    EXPECT_THROW(filter_->write(nullptr, 3), std::invalid_argument);
}

/** @brief Flush delegates without throwing.
    @details Verifies flush() on the filter succeeds. */
TEST_F(FilterOutputStreamTest, FlushDelegates)
{
    EXPECT_NO_THROW(filter_->flush());
}

/** @brief Close flushes the inner stream and marks closed.
    @details Writes data, closes, and verifies the filter is in closed state. */
TEST_F(FilterOutputStreamTest, CloseFlushesAndDelegates)
{
    filter_->write(std::byte{0x41});
    EXPECT_NO_THROW(filter_->close());
    EXPECT_TRUE(filter_->isClosed());
}

/** @brief Close sets closed state on the filter.
    @details Verifies isClosed transitions from false to true after close. */
TEST_F(FilterOutputStreamTest, CloseSetsClosedState)
{
    EXPECT_FALSE(filter_->isClosed());
    filter_->close();
    EXPECT_TRUE(filter_->isClosed());
}

/** @brief Write after close throws.
    @details Closing then writing triggers std::runtime_error. */
TEST_F(FilterOutputStreamTest, WriteAfterCloseThrows)
{
    filter_->close();
    EXPECT_THROW(filter_->write(std::byte{0x00}), std::runtime_error);
}

/** @brief Flush after close throws.
    @details Closing then flushing triggers std::runtime_error. */
TEST_F(FilterOutputStreamTest, FlushAfterCloseThrows)
{
    filter_->close();
    EXPECT_THROW(filter_->flush(), std::runtime_error);
}

/** @brief Empty vector write does nothing.
    @details Writing an empty vector leaves inner stream empty. */
TEST_F(FilterOutputStreamTest, WriteEmptyVectorDoesNothing)
{
    const std::vector<std::byte> empty;
    filter_->write(empty);
    EXPECT_TRUE(inner_->toByteArray().empty());
}

/** @brief Zero-length raw buffer write does nothing even with null.
    @details Null pointer with length 0 gracefully returns without writing. */
TEST_F(FilterOutputStreamTest, WriteZeroLengthRawDoesNothing)
{
    filter_->write(nullptr, 0);
    EXPECT_TRUE(inner_->toByteArray().empty());
}

/** @brief Multiple writes accumulate in the inner stream.
    @details Writes individual bytes then a vector, expects total size 4. */
TEST_F(FilterOutputStreamTest, MultipleWritesAccumulate)
{
    filter_->write(std::byte{0x01});
    filter_->write(std::byte{0x02});

    const std::vector<std::byte> more = {std::byte{0x03}, std::byte{0x04}};
    filter_->write(more);

    EXPECT_EQ(inner_->size(), 4);
}

/** @brief Write vector after close throws.
    @details Closing then writing a full vector triggers std::runtime_error. */
TEST_F(FilterOutputStreamTest, WriteVectorAfterCloseThrows)
{
    filter_->close();
    const std::vector<std::byte> data = {std::byte{0x01}};
    EXPECT_THROW(filter_->write(data), std::runtime_error);
}

/** @brief Write vector partial after close throws.
    @details Closing then writing a vector sub-range triggers std::runtime_error. */
TEST_F(FilterOutputStreamTest, WriteVectorPartialAfterCloseThrows)
{
    filter_->close();
    const std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 0, 1), std::runtime_error);
}

/** @brief Write raw buffer after close throws.
    @details Closing then writing a raw buffer triggers std::runtime_error. */
TEST_F(FilterOutputStreamTest, WriteRawAfterCloseThrows)
{
    filter_->close();
    constexpr std::byte buf[] = {std::byte{0x01}};
    EXPECT_THROW(filter_->write(buf, 1), std::runtime_error);
}

/** @brief Close is idempotent.
    @details Double close does not throw. */
TEST_F(FilterOutputStreamTest, CloseIsIdempotent)
{
    filter_->close();
    EXPECT_NO_THROW(filter_->close());
}

/** @brief Destruction does not close the inner stream.
    @details Creates a temporary filter, writes data, then verifies inner stream remains open and usable. */
TEST_F(FilterOutputStreamTest, DestructionDoesNotCloseInner)
{
    {
        FilterOutputStream temp(inner_);
        temp.write(std::byte{0xFF});
    }
    EXPECT_EQ(inner_->size(), 1);
    EXPECT_FALSE(inner_->isClosed());
}

/** @brief Write after flush accumulates correctly.
    @details Writes two bytes with flushes in between; expects total size 2. */
TEST_F(FilterOutputStreamTest, WriteAfterFlush)
{
    filter_->write(std::byte{0x10});
    filter_->flush();
    filter_->write(std::byte{0x20});
    filter_->flush();
    EXPECT_EQ(inner_->size(), 2);
}

/** @brief Negative vector offset throws.
    @details Offset -1 with any length triggers std::out_of_range. */
TEST_F(FilterOutputStreamTest, WriteVectorNegativeOffset)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, -1, 1), std::out_of_range);
}

/** @brief Negative vector length throws.
    @details Length -1 at offset 0 triggers std::out_of_range. */
TEST_F(FilterOutputStreamTest, WriteVectorNegativeLength)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 0, -1), std::out_of_range);
}

/** @brief Offset at vector size with non-zero length throws.
    @details Offset=2 on size-2 buffer with length=1 triggers std::out_of_range. */
TEST_F(FilterOutputStreamTest, WriteVectorOffsetAtSize)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 2, 1), std::out_of_range);
}

/** @brief Offset exceeding vector size throws.
    @details Offset 5 on a size-2 buffer triggers std::out_of_range. */
TEST_F(FilterOutputStreamTest, WriteVectorOffsetExceedsSize)
{
    std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
    EXPECT_THROW(filter_->write(data, 5, 1), std::out_of_range);
}
