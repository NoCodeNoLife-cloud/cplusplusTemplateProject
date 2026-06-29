/**
 * @file IStartupTaskTest.cc
 * @brief Unit tests for IStartupTask interface
 * @details Tests cover task execution and return values.
 */

#include <gtest/gtest.h>

#include <cppforge/interface/task/IStartupTask.hpp>

using namespace cppforge::interface::task;

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

/// @brief Test fixture for IStartupTask tests.
class IStartupTaskTest : public testing::Test
{
protected:
    void SetUp() override
    {
        task_ = std::make_unique<MockStartupTask>();
    }

    void TearDown() override
    {
        task_.reset();
    }

    std::unique_ptr<MockStartupTask> task_;
};

/** @brief Tests execute returns true by default.
 *  @details Verifies return value is true and execute count increments to 1.
 */
TEST_F(IStartupTaskTest, ExecuteReturnsTrue)
{
    const bool result = task_->execute();

    EXPECT_TRUE(result);
    EXPECT_EQ(task_->executeCount_, 1);
}

/** @brief Tests execute increments count on single call.
 *  @details Verifies executeCount_ is 1 after executing once.
 */
TEST_F(IStartupTaskTest, ExecuteCalledOnceReturnsTrue)
{
    task_->execute();

    EXPECT_EQ(task_->executeCount_, 1);
}

/** @brief Tests execute returns false on configured failure.
 *  @details Verifies return value reflects mock's returnValue_ flag when set to false.
 */
TEST_F(IStartupTaskTest, ExecuteReturnsFalseOnFailure)
{
    task_->returnValue_ = false;

    const bool result = task_->execute();

    EXPECT_FALSE(result);
    EXPECT_EQ(task_->executeCount_, 1);
}

/** @brief Tests execute can be invoked multiple times.
 *  @details Verifies executeCount_ reaches 3 after three sequential calls.
 */
TEST_F(IStartupTaskTest, ExecuteCanBeCalledMultipleTimes)
{
    task_->execute();
    task_->execute();
    task_->execute();

    EXPECT_EQ(task_->executeCount_, 3);
}

/** @brief Tests execute does not throw exceptions.
 *  @details Verifies execution completes without throwing any exception.
 */
TEST_F(IStartupTaskTest, ExecuteDoesNotThrow)
{
    EXPECT_NO_THROW(task_->execute());
}

/** @brief Tests execute return value is marked nodiscard.
 *  @details Verifies captured return value is true when default mock returns true.
 */
TEST_F(IStartupTaskTest, ExecuteIsNoDiscard)
{
    const auto result = task_->execute();
    EXPECT_TRUE(result);
}
