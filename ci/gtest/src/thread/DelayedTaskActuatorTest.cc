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

#include "thread/DelayedTaskActuator.hpp"

using namespace common::thread;

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

TEST_F(DelayedTaskActuatorTest, ScheduleTask_ReturnsNonNegativeId)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 42; });
    EXPECT_GE(id, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(DelayedTaskActuatorTest, GetTaskResult_ReturnsCorrectValue)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, [] { return 99; });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_EQ(result.get(), 99);
}

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

TEST_F(DelayedTaskActuatorTest, IsTaskPending_ReturnsTrueBeforeExecution)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(100, [] { return 0; });
    EXPECT_TRUE(actuator.isTaskPending(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

TEST_F(DelayedTaskActuatorTest, CancelTask_RemovesPendingTask)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(100, [] { return 0; });
    EXPECT_TRUE(actuator.cancelTask(id));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

TEST_F(DelayedTaskActuatorTest, CancelTask_AlreadyExecuted_ReturnsFalse)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 0; });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(actuator.cancelTask(id));
}

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

TEST_F(DelayedTaskActuatorTest, GetTaskResult_ThrowsIfAlreadyRetrieved)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(1, [] { return 42; });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_EQ(result.get(), 42);

    EXPECT_THROW(static_cast<void>(actuator.getTaskResult(id)), std::runtime_error);
}

TEST_F(DelayedTaskActuatorTest, ScheduleTask_NegativeDelay_Throws)
{
    DelayedTaskActuator<int> actuator;
    EXPECT_THROW(static_cast<void>(actuator.scheduleTask(-1, [] { return 0; })), std::invalid_argument);
}

TEST_F(DelayedTaskActuatorTest, ScheduleTask_NullTask_Throws)
{
    DelayedTaskActuator<int> actuator;
    std::function<int()> nullTask = nullptr;
    EXPECT_THROW(static_cast<void>(actuator.scheduleTask(10, nullTask)), std::invalid_argument);
}

TEST_F(DelayedTaskActuatorTest, TaskWithException_GetThrows)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, []() -> int { throw std::runtime_error("task error"); });
    std::future<int> result = actuator.getTaskResult(id);
    EXPECT_THROW(result.get(), std::runtime_error);
}

TEST_F(DelayedTaskActuatorTest, GetTaskResult_IdempotentForCompletedTask)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(10, [] { return 77; });
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_FALSE(actuator.isTaskPending(id));
}

TEST_F(DelayedTaskActuatorTest, CancelTask_NonExistentId_ReturnsFalse)
{
    DelayedTaskActuator<int> actuator;
    EXPECT_FALSE(actuator.cancelTask(999));
}

TEST_F(DelayedTaskActuatorTest, ScheduleZeroDelay_ExecutesImmediately)
{
    DelayedTaskActuator<int> actuator;
    int32_t id = actuator.scheduleTask(0, [] { return 100; });
    EXPECT_EQ(actuator.getTaskResult(id).get(), 100);
}
