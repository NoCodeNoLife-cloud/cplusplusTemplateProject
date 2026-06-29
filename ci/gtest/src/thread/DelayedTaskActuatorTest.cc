/**
 * @file DelayedTaskActuatorTest.cc
 * @brief Unit tests for DelayedTaskActuator class
 * @details Tests cover task scheduling, result retrieval, cancellation,
 *          pending status checks, and edge cases.
 */

#include <chrono>
#include <future>
#include <stdexcept>
#include <thread>
#include <gtest/gtest.h>

#include <cppforge/thread/DelayedTaskActuator.hpp>

using namespace cppforge::thread;

/// @brief Test fixture for DelayedTaskActuator tests.
class DelayedTaskActuatorTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Verifies scheduleTask returns a non-negative task ID.
 * @details Ensures that any successfully scheduled task is assigned a valid
 *          non-negative identifier, even with a minimal delay.
 */
TEST_F(DelayedTaskActuatorTest, ScheduleTask_ReturnsNonNegativeId)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 42; });
    EXPECT_GE(id, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

/**
 * @brief Verifies getTaskResult returns the value produced by the task.
 * @details Schedules a task returning 99 with a 10ms delay and confirms
 *          the retrieved future resolves to that exact value.
 */
TEST_F(DelayedTaskActuatorTest, GetTaskResult_ReturnsCorrectValue)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, [] { return 99; });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_EQ(result.get(), 99);
}

/**
 * @brief Verifies getTaskResult blocks until the configured delay elapses.
 * @details Measures the wall-clock time for a 50ms-delayed task and asserts
 *          that at least 40ms have passed, confirming the delay is honored.
 */
TEST_F(DelayedTaskActuatorTest, GetTaskResult_WaitsForDelay)
{
    DelayedTaskActuator<int> actuator;
    auto start = std::chrono::steady_clock::now();
    int32_t id = actuator.scheduleTask(50, [] { return 1; });
    std::future<int> result = actuator.getTaskResult(id);
    result.get();
    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_GE(elapsed, std::chrono::milliseconds(40));
}

/**
 * @brief Verifies isTaskPending reflects correct state before and after execution.
 * @details Schedules a 100ms-delayed task and checks that isTaskPending returns
 *          true before, then false after, the delay has elapsed.
 */
TEST_F(DelayedTaskActuatorTest, IsTaskPending_ReturnsTrueBeforeExecution)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(100, [] { return 0; });
    EXPECT_TRUE(actuator.isTaskPending(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

/**
 * @brief Verifies cancelTask removes a pending task from the actuator.
 * @details Cancels a 100ms-delayed task before it executes and confirms
 *          the task is no longer reported as pending.
 */
TEST_F(DelayedTaskActuatorTest, CancelTask_RemovesPendingTask)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(100, [] { return 0; });
    EXPECT_TRUE(actuator.cancelTask(id));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

/**
 * @brief Verifies cancelTask returns false for a task that already ran.
 * @details Schedules a 1ms-delay task, waits for it to finish, then asserts
 *          that attempting to cancel it returns false.
 */
TEST_F(DelayedTaskActuatorTest, CancelTask_AlreadyExecuted_ReturnsFalse)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 0; });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(actuator.cancelTask(id));
}

/**
 * @brief Verifies multiple concurrently scheduled tasks produce independent results.
 * @details Schedules three tasks with staggered delays (5/10/20ms) and distinct
 *          return values, then confirms each future resolves to its own value.
 */
TEST_F(DelayedTaskActuatorTest, MultipleTasks_IndependentResults)
{
    DelayedTaskActuator<int> actuator;
    int32_t id1 = actuator.scheduleTask(10, [] { return 10; });
    int32_t id2 = actuator.scheduleTask(20, [] { return 20; });
    int32_t id3 = actuator.scheduleTask(5, [] { return 5; });

    EXPECT_EQ(actuator.getTaskResult(id1).get(), 10);
    EXPECT_EQ(actuator.getTaskResult(id2).get(), 20);
    EXPECT_EQ(actuator.getTaskResult(id3).get(), 5);
}

/**
 * @brief Verifies getTaskResult throws if the result was already retrieved.
 * @details Retrieves a task's future once, consumes it, then asserts that a
 *          second call to getTaskResult with the same ID throws std::runtime_error.
 */
TEST_F(DelayedTaskActuatorTest, GetTaskResult_ThrowsIfAlreadyRetrieved)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 42; });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_EQ(result.get(), 42);

    EXPECT_THROW(static_cast<void>(actuator.getTaskResult(id)), std::runtime_error);
}

/**
 * @brief Verifies scheduleTask rejects a negative delay with an exception.
 * @details Passes a delay of -1 and asserts that std::invalid_argument is thrown,
 *          enforcing the pre-condition that delay must be non-negative.
 */
TEST_F(DelayedTaskActuatorTest, ScheduleTask_NegativeDelay_Throws)
{
    DelayedTaskActuator<int> actuator;
    EXPECT_THROW(static_cast<void>(actuator.scheduleTask(-1, [] { return 0; })), std::invalid_argument);
}

/**
 * @brief Verifies scheduleTask rejects a null task with an exception.
 * @details Passes a null std::function as the task and asserts that
 *          std::invalid_argument is thrown to prevent a null dereference.
 */
TEST_F(DelayedTaskActuatorTest, ScheduleTask_NullTask_Throws)
{
    DelayedTaskActuator<int> actuator;
    std::function<int()> nullTask = nullptr;
    EXPECT_THROW(static_cast<void>(actuator.scheduleTask(10, nullTask)), std::invalid_argument);
}

/**
 * @brief Verifies that an exception thrown inside a task propagates to the caller.
 * @details Schedules a task that throws std::runtime_error and confirms that
 *          calling get() on the associated future re-throws that exception.
 */
TEST_F(DelayedTaskActuatorTest, TaskWithException_GetThrows)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, []() -> int { throw std::runtime_error("task error"); });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_THROW(result.get(), std::runtime_error);
}

/**
 * @brief Verifies a completed task is no longer reported as pending.
 * @details Schedules a 10ms task, waits beyond its delay, then asserts that
 *          isTaskPending returns false for that task ID.
 */
TEST_F(DelayedTaskActuatorTest, GetTaskResult_IdempotentForCompletedTask)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, [] { return 77; });
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

/**
 * @brief Verifies cancelTask returns false for a non-existent task ID.
 * @details Calls cancelTask with ID 999 on an empty actuator and asserts
 *          the return value is false, confirming no-op behavior for invalid IDs.
 */
TEST_F(DelayedTaskActuatorTest, CancelTask_NonExistentId_ReturnsFalse)
{
    DelayedTaskActuator<int> actuator;
    EXPECT_FALSE(actuator.cancelTask(999));
}

/**
 * @brief Verifies a task scheduled with zero delay executes immediately.
 * @details Schedules a task with delay 0 and confirms the result is available
 *          right away without any measurable blocking.
 */
TEST_F(DelayedTaskActuatorTest, ScheduleZeroDelay_ExecutesImmediately)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(0, [] { return 100; });
    EXPECT_EQ(actuator.getTaskResult(id).get(), 100);
}
