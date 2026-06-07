/**
 * @file ICloseableTest.cc
 * @brief Unit tests for ICloseable interface
 * @details Tests cover close, isClosed, and closeSafe operations via a mock implementation.
 */

#include <gtest/gtest.h>

#include "interface/io/ICloseable.hpp"

using namespace common::interfaces::io;

namespace
{
    /// @brief Mock implementation of ICloseable for testing
    class MockCloseable : public ICloseable
    {
    public:
        bool closeCalled_ = false;
        bool closed_ = false;
        bool throwOnClose_ = false;

        void close() override
        {
            closeCalled_ = true;
            if (throwOnClose_)
                throw std::runtime_error("close failed");
            closed_ = true;
        }

        [[nodiscard]] bool isClosed() const override
        {
            return closed_;
        }
    };
}

class ICloseableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_.reset(new MockCloseable());
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockCloseable> mock_;
};

TEST_F(ICloseableTest, InitiallyNotClosed)
{
    EXPECT_FALSE(mock_->isClosed());
}

TEST_F(ICloseableTest, CloseSetsState)
{
    mock_->close();
    EXPECT_TRUE(mock_->closeCalled_);
    EXPECT_TRUE(mock_->isClosed());
}

TEST_F(ICloseableTest, CloseSafeSuccess)
{
    bool result = mock_->closeSafe();
    EXPECT_TRUE(result);
    EXPECT_TRUE(mock_->isClosed());
}

TEST_F(ICloseableTest, CloseSafeHandlesException)
{
    mock_->throwOnClose_ = true;
    bool result = mock_->closeSafe();
    EXPECT_FALSE(result);
    EXPECT_FALSE(mock_->isClosed());
}

TEST_F(ICloseableTest, CloseSafeCalledClose)
{
    mock_->closeSafe();
    EXPECT_TRUE(mock_->closeCalled_);
}

TEST_F(ICloseableTest, IdempotentClose)
{
    mock_->close();
    mock_->close();
    EXPECT_TRUE(mock_->isClosed());
}
