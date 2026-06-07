/**
 * @file IAppendableTest.cc
 * @brief Unit tests for IAppendable interface
 * @details Tests cover all append overloads and method chaining via a mock implementation.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "interface/io/IAppendable.hpp"

using namespace common::interfaces::io;

namespace
{
    /// @brief Mock implementation of IAppendable for testing
    class MockAppendable : public IAppendable<MockAppendable>
    {
    public:
        [[nodiscard]] const std::string& value() const { return data_; }

        MockAppendable& append(char c) override
        {
            data_.push_back(c);
            return *this;
        }

        MockAppendable& append(const std::string& str) override
        {
            data_ += str;
            return *this;
        }

        MockAppendable& append(const std::string& str, size_t start, size_t end) override
        {
            data_ += str.substr(start, end - start);
            return *this;
        }

        MockAppendable& append(std::string_view str) override
        {
            data_ += str;
            return *this;
        }

        MockAppendable& append(const char* str) override
        {
            data_ += str;
            return *this;
        }

        MockAppendable& append(std::initializer_list<char> chars) override
        {
            for (auto c : chars)
                data_.push_back(c);
            return *this;
        }

        MockAppendable& append(const char* chars, size_t count) override
        {
            data_.append(chars, count);
            return *this;
        }

        MockAppendable& append(char c, size_t count) override
        {
            data_.append(count, c);
            return *this;
        }

        MockAppendable& append(std::span<const char> chars) override
        {
            for (auto c : chars)
                data_.push_back(c);
            return *this;
        }

    private:
        std::string data_;
    };
}

class IAppendableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_.reset(new MockAppendable());
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockAppendable> mock_;
};

TEST_F(IAppendableTest, AppendChar)
{
    mock_->append('A');
    EXPECT_EQ(mock_->value(), "A");
}

TEST_F(IAppendableTest, AppendString)
{
    mock_->append(std::string("hello"));
    EXPECT_EQ(mock_->value(), "hello");
}

TEST_F(IAppendableTest, AppendSubstring)
{
    mock_->append(std::string("hello world"), 0, 5);
    EXPECT_EQ(mock_->value(), "hello");
}

TEST_F(IAppendableTest, AppendStringView)
{
    mock_->append(std::string_view("world"));
    EXPECT_EQ(mock_->value(), "world");
}

TEST_F(IAppendableTest, AppendCString)
{
    mock_->append("c-string");
    EXPECT_EQ(mock_->value(), "c-string");
}

TEST_F(IAppendableTest, AppendInitializerList)
{
    mock_->append({'a', 'b', 'c'});
    EXPECT_EQ(mock_->value(), "abc");
}

TEST_F(IAppendableTest, AppendCharArrayWithCount)
{
    mock_->append("hello world", 5);
    EXPECT_EQ(mock_->value(), "hello");
}

TEST_F(IAppendableTest, AppendCharRepeated)
{
    mock_->append('Z', 5);
    EXPECT_EQ(mock_->value(), "ZZZZZ");
}

TEST_F(IAppendableTest, AppendSpan)
{
    const char arr[] = {'x', 'y', 'z'};
    mock_->append(std::span<const char>(arr));
    EXPECT_EQ(mock_->value(), "xyz");
}

TEST_F(IAppendableTest, MethodChaining)
{
    mock_->append('A').append(std::string("BC")).append('D');
    EXPECT_EQ(mock_->value(), "ABCD");
}

TEST_F(IAppendableTest, AppendEmptyString)
{
    mock_->append(std::string());
    EXPECT_EQ(mock_->value(), "");
}

TEST_F(IAppendableTest, AppendEmptySpan)
{
    mock_->append(std::span<const char>());
    EXPECT_EQ(mock_->value(), "");
}

TEST_F(IAppendableTest, MultipleAppends)
{
    mock_->append("Hello, ");
    mock_->append(std::string_view("World"));
    mock_->append('!');
    EXPECT_EQ(mock_->value(), "Hello, World!");
}
