/**
 * @file IFlushableTest.cc
 * @brief Unit tests for IFlushable interface
 * @details Tests cover flush, flushSafe, and isFlushNeeded operations via a mock implementation.
 */

#include <gtest/gtest.h>

#include <cppforge/interface/io/IFlushable.hpp>

using namespace cppforge::interface::io;

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

/// @brief Test fixture for IFlushable tests.
class IFlushableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        mock_ = std::make_unique<MockFlushable>();
    }

    void TearDown() override
    {
        mock_.reset();
    }

    std::unique_ptr<MockFlushable> mock_;
};

/** @brief Verifies flush() increments the flush counter. */
TEST_F(IFlushableTest, FlushIncrementsCount)
{
    mock_->flush();
    EXPECT_EQ(mock_->flushCount_, 1);
}

/** @brief Verifies flush() can be called multiple times. */
TEST_F(IFlushableTest, FlushMultipleTimes)
{
    mock_->flush();
    mock_->flush();
    mock_->flush();
    EXPECT_EQ(mock_->flushCount_, 3);
}

/** @brief Verifies flushSafe flushes successfully and returns true. */
TEST_F(IFlushableTest, FlushSafeSuccess)
{
    const bool result = mock_->flushSafe();
    EXPECT_TRUE(result);
    EXPECT_EQ(mock_->flushCount_, 1);
}

/** @brief Verifies flushSafe catches exceptions and returns false. */
TEST_F(IFlushableTest, FlushSafeHandlesException)
{
    mock_->throwOnFlush_ = true;
    const bool result = mock_->flushSafe();
    EXPECT_FALSE(result);
    EXPECT_EQ(mock_->flushCount_, 1);
}

/** @brief Verifies isFlushNeeded returns true by default. */
TEST_F(IFlushableTest, IsFlushNeededDefault)
{
    const MockFlushable defaultMock;
    EXPECT_TRUE(defaultMock.isFlushNeeded());
}

/** @brief Verifies isFlushNeeded returns false when set. */
TEST_F(IFlushableTest, IsFlushNeededFalse)
{
    mock_->flushNeeded_ = false;
    EXPECT_FALSE(mock_->isFlushNeeded());
}
