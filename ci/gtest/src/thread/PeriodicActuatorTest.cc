/**
 * @file PeriodicActuatorTest.cc
 * @brief Unit tests for PeriodicActuator class
 * @details Tests cover start/stop lifecycle, periodic execution, exception
 *          handling in tasks, and restart capability.
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>
#include <gtest/gtest.h>

#include "thread/PeriodicActuator.hpp"
#include "interface/task/ITimerTask.hpp"

using namespace common::thread;
using namespace common::interfaces::task;

namespace
{
    class MockTimerTask : public ITimerTask
    {
    public:
        explicit MockTimerTask(std::atomic<int>& counter) : counter_(counter)
        {
        }

        void execute() override
        {
            ++counter_;
        }

    private:
        std::atomic<int>& counter_;
    };

    class ThrowingTimerTask : public ITimerTask
    {
    public:
        explicit ThrowingTimerTask(std::atomic<int>& counter) : counter_(counter)
        {
        }

        void execute() override
        {
            ++counter_;
            throw std::runtime_error("intentional task failure");
        }

    private:
        std::atomic<int>& counter_;
    };
}

class PeriodicActuatorTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(PeriodicActuatorTest, Start_IsRunning)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(20));
    actuator.start();
    EXPECT_TRUE(actuator.isRunning());
    actuator.stop();
}

TEST_F(PeriodicActuatorTest, Stop_IsNotRunning)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(20));
    actuator.start();
    actuator.stop();
    EXPECT_FALSE(actuator.isRunning());
}

TEST_F(PeriodicActuatorTest, PeriodicExecution_IncrementsCounter)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(10));
    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    actuator.stop();
    EXPECT_GE(counter.load(), 3);
}

TEST_F(PeriodicActuatorTest, ThrowingTask_DoesNotStopScheduler)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<ThrowingTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(10));
    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    actuator.stop();
    EXPECT_GE(counter.load(), 3);
}

TEST_F(PeriodicActuatorTest, StartTwice_Throws)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(50));
    actuator.start();
    EXPECT_THROW(actuator.start(), std::runtime_error);
    actuator.stop();
}

TEST_F(PeriodicActuatorTest, StopWithoutStart_NoOp)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(50));
    EXPECT_NO_THROW(actuator.stop());
}

TEST_F(PeriodicActuatorTest, RestartAfterStop_ResumesExecution)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(10));

    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    actuator.stop();
    int countAfterFirst = counter.load();
    EXPECT_FALSE(actuator.isRunning());

    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    actuator.stop();
    EXPECT_GT(counter.load(), countAfterFirst);
}
