/**
 * @file IFlushableTest.cc
 * @brief Unit tests for IFlushable interface
 * @details Tests cover flush, flushSafe, and isFlushNeeded operations via a mock implementation.
 */

#include <gtest/gtest.h>

#include "interface/io/IFlushable.hpp"

using namespace common::interfaces::io;

namespace
{
    /// @brief Mock implementation of IFlushable for testing
    class MockFlushable : public IFlushable
    {
    public:
        int flushCount_ = 0;
        bool throwOnFlush_ = false;
        bool flushNeeded_ = true;

        void flush() override
        {
            flushCount_++;
            if (throwOnFlush_)
                throw std::runtime_error("flush failed");
        }

        [[nodiscard]] bool isFlushNeeded() const override
        {
            return flushNeeded_;
        }
    };
}

class IFlushableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_.reset(new MockFlushable());
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockFlushable> mock_;
};

TEST_F(IFlushableTest, FlushIncrementsCount)
{
    mock_->flush();
    EXPECT_EQ(mock_->flushCount_, 1);
}

TEST_F(IFlushableTest, FlushMultipleTimes)
{
    mock_->flush();
    mock_->flush();
    mock_->flush();
    EXPECT_EQ(mock_->flushCount_, 3);
}

TEST_F(IFlushableTest, FlushSafeSuccess)
{
    bool result = mock_->flushSafe();
    EXPECT_TRUE(result);
    EXPECT_EQ(mock_->flushCount_, 1);
}

TEST_F(IFlushableTest, FlushSafeHandlesException)
{
    mock_->throwOnFlush_ = true;
    bool result = mock_->flushSafe();
    EXPECT_FALSE(result);
    EXPECT_EQ(mock_->flushCount_, 1);
}

TEST_F(IFlushableTest, IsFlushNeededDefault)
{
    MockFlushable defaultMock;
    EXPECT_TRUE(defaultMock.isFlushNeeded());
}

TEST_F(IFlushableTest, IsFlushNeededFalse)
{
    mock_->flushNeeded_ = false;
    EXPECT_FALSE(mock_->isFlushNeeded());
}
