/**
 * @file IStartupTaskTest.cc
 * @brief Unit tests for IStartupTask interface
 * @details Tests cover task execution and return values.
 */

#include <gtest/gtest.h>

#include "interface/task/IStartupTask.hpp"

using namespace common::interface::task;

namespace
{
    /// @brief Mock startup task that tracks execution
    class MockStartupTask : public IStartupTask
    {
    public:
        int executeCount_ = 0;
        bool returnValue_ = true;

        bool execute() override
        {
            executeCount_++;
            return returnValue_;
        }
    };
}

class IStartupTaskTest : public testing::Test
{
protected:
    void SetUp() override
    {
        task_.reset(new MockStartupTask());
    }

    void TearDown() override
    {
        task_.reset();
    }

    std::unique_ptr<MockStartupTask> task_;
};

TEST_F(IStartupTaskTest, ExecuteReturnsTrue)
{
    bool result = task_->execute();

    EXPECT_TRUE(result);
    EXPECT_EQ(task_->executeCount_, 1);
}

TEST_F(IStartupTaskTest, ExecuteCalledOnceReturnsTrue)
{
    task_->execute();

    EXPECT_EQ(task_->executeCount_, 1);
}

TEST_F(IStartupTaskTest, ExecuteReturnsFalseOnFailure)
{
    task_->returnValue_ = false;

    bool result = task_->execute();

    EXPECT_FALSE(result);
    EXPECT_EQ(task_->executeCount_, 1);
}

TEST_F(IStartupTaskTest, ExecuteCanBeCalledMultipleTimes)
{
    task_->execute();
    task_->execute();
    task_->execute();

    EXPECT_EQ(task_->executeCount_, 3);
}

TEST_F(IStartupTaskTest, ExecuteDoesNotThrow)
{
    EXPECT_NO_THROW(task_->execute());
}

TEST_F(IStartupTaskTest, ExecuteIsNoDiscard)
{
    auto result = task_->execute();
    EXPECT_TRUE(result);
}
