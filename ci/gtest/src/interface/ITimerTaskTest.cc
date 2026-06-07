/**
 * @file ITimerTaskTest.cc
 * @brief Unit tests for ITimerTask interface
 * @details Tests cover timer task execution via a mock implementation.
 */

#include <gtest/gtest.h>

#include "interface/ITimerTask.hpp"

using namespace common::interfaces;

namespace
{
    /// @brief Mock timer task that tracks execution
    class MockTimerTask : public ITimerTask
    {
    public:
        int executeCount_ = 0;

        void execute() override
        {
            executeCount_++;
        }
    };
}

class ITimerTaskTest : public testing::Test
{
protected:
    void SetUp() override
    {
        task_.reset(new MockTimerTask());
    }

    void TearDown() override
    {
        task_.reset();
    }

    std::unique_ptr<MockTimerTask> task_;
};

TEST_F(ITimerTaskTest, ExecuteRunsTask)
{
    task_->execute();
    EXPECT_EQ(task_->executeCount_, 1);
}

TEST_F(ITimerTaskTest, ExecuteMultipleTimes)
{
    task_->execute();
    task_->execute();
    task_->execute();

    EXPECT_EQ(task_->executeCount_, 3);
}

TEST_F(ITimerTaskTest, ExecuteDoesNotThrow)
{
    EXPECT_NO_THROW(task_->execute());
}

TEST_F(ITimerTaskTest, ExecuteCalledSequentially)
{
    for (int i = 0; i < 5; i++)
        task_->execute();

    EXPECT_EQ(task_->executeCount_, 5);
}
