/**
 * @file ITaskTest.cc
 * @brief Unit tests for ITask interface
 * @details Tests cover task execution via a simple mock.
 */

#include <gtest/gtest.h>

#include "interface/task/ITask.hpp"

using namespace common::interface::task;

namespace
{
    /// @brief Mock task that tracks execution
    class MockTask : public ITask
    {
    public:
        int runCount_ = 0;

        void run() override
        {
            runCount_++;
        }
    };
}

/// @brief Test fixture for ITask tests.
class ITaskTest : public testing::Test
{
protected:
    void SetUp() override
    {
        task_ = std::make_unique<MockTask>();
    }

    void TearDown() override
    {
        task_.reset();
    }

    std::unique_ptr<MockTask> task_;
};

/** @brief Tests run executes the task once.
 *  @details Verifies runCount_ increments to 1 after calling run().
 */
TEST_F(ITaskTest, RunExecutesTask)
{
    task_->run();
    EXPECT_EQ(task_->runCount_, 1);
}

/** @brief Tests run can be called multiple times.
 *  @details Verifies runCount_ reaches 3 after three sequential calls.
 */
TEST_F(ITaskTest, RunMultipleTimes)
{
    task_->run();
    task_->run();
    task_->run();

    EXPECT_EQ(task_->runCount_, 3);
}

TEST_F(ITaskTest, RunDoesNotThrow)
{
    EXPECT_NO_THROW(task_->run());
}

TEST_F(ITaskTest, RunIsIdempotent)
{
    task_->run();
    task_->run();

    EXPECT_EQ(task_->runCount_, 2);
}
