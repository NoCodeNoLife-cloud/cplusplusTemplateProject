/**
 * @file IReadableTest.cc
 * @brief Unit tests for IReadable interface
 * @details Tests cover single byte reading and EOF detection via a mock implementation.
 */

#include <gtest/gtest.h>

#include <cppforge/interface/io/IReadable.hpp>

using namespace cppforge::interface::io;

namespace
{
    /// @brief Mock implementation of IReadable for testing
    class MockReadable : public IReadable
    {
    public:
        std::string data_;
        size_t pos_ = 0;

        explicit MockReadable(std::string data) : data_(std::move(data)) {}

        [[nodiscard]] std::optional<char> read() override
        {
            if (pos_ >= data_.size())
                return std::nullopt;
            return data_[pos_++];
        }
    };
}

/// @brief Test fixture for IReadable tests.
class IReadableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_ = std::make_unique<MockReadable>("hello");
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockReadable> mock_;
};

/** @brief Tests reading a single byte from stream.
 *  @details Verifies the first character 'h' is returned and has_value is true.
 */
TEST_F(IReadableTest, ReadSingleByte)
{
    const auto ch = mock_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'h');
}

/** @brief Tests sequential reading of all bytes.
 *  @details Verifies each character in "hello" is read in order.
 */
TEST_F(IReadableTest, ReadMultipleBytes)
{
    EXPECT_EQ(mock_->read(), 'h');
    EXPECT_EQ(mock_->read(), 'e');
    EXPECT_EQ(mock_->read(), 'l');
    EXPECT_EQ(mock_->read(), 'l');
    EXPECT_EQ(mock_->read(), 'o');
}

/** @brief Tests EOF detection after reading all bytes.
 *  @details Verifies read returns nullopt after exhausting the stream.
 */
TEST_F(IReadableTest, ReadEof)
{
    for (int i = 0; i < 5; i++)
        static_cast<void>(mock_->read());

    const auto ch = mock_->read();
    EXPECT_FALSE(ch.has_value());
}

/** @brief Tests EOF on empty stream.
 *  @details Verifies read returns nullopt immediately for empty data.
 */
TEST_F(IReadableTest, ReadEofOnEmpty)
{
    MockReadable empty("");
    const auto ch = empty.read();
    EXPECT_FALSE(ch.has_value());
}

/** @brief Tests reading entire stream via loop.
 *  @details Verifies loop-based reading reconstructs the original string "hello".
 */
TEST_F(IReadableTest, ReadSequential)
{
    std::string result;
    while (auto ch = mock_->read())
        result += ch.value();

    EXPECT_EQ(result, "hello");
}

/** @brief Tests repeated reads after EOF.
 *  @details Verifies subsequent reads after exhaustion consistently return nullopt.
 */
TEST_F(IReadableTest, ReadAfterEof)
{
    for (int i = 0; i < 5; i++)
        static_cast<void>(mock_->read());

    EXPECT_FALSE(mock_->read().has_value());
    EXPECT_FALSE(mock_->read().has_value());
}
