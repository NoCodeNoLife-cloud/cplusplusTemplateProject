/**
 * @file IReadableTest.cc
 * @brief Unit tests for IReadable interface
 * @details Tests cover single byte reading and EOF detection via a mock implementation.
 */

#include <gtest/gtest.h>

#include "filesystem/io/interface/IReadable.hpp"

using namespace common::interfaces;

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

class IReadableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_.reset(new MockReadable("hello"));
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockReadable> mock_;
};

TEST_F(IReadableTest, ReadSingleByte)
{
    auto ch = mock_->read();
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'h');
}

TEST_F(IReadableTest, ReadMultipleBytes)
{
    EXPECT_EQ(mock_->read(), 'h');
    EXPECT_EQ(mock_->read(), 'e');
    EXPECT_EQ(mock_->read(), 'l');
    EXPECT_EQ(mock_->read(), 'l');
    EXPECT_EQ(mock_->read(), 'o');
}

TEST_F(IReadableTest, ReadEof)
{
    for (int i = 0; i < 5; i++)
        mock_->read();

    auto ch = mock_->read();
    EXPECT_FALSE(ch.has_value());
}

TEST_F(IReadableTest, ReadEofOnEmpty)
{
    MockReadable empty("");
    auto ch = empty.read();
    EXPECT_FALSE(ch.has_value());
}

TEST_F(IReadableTest, ReadSequential)
{
    std::string result;
    while (auto ch = mock_->read())
        result += ch.value();

    EXPECT_EQ(result, "hello");
}

TEST_F(IReadableTest, ReadAfterEof)
{
    for (int i = 0; i < 5; i++)
        mock_->read();

    EXPECT_FALSE(mock_->read().has_value());
    EXPECT_FALSE(mock_->read().has_value());
}
