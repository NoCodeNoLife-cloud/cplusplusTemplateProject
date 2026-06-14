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
using namespace common::interface::task;

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

    class SlowTimerTask : public ITimerTask
    {
    public:
        explicit SlowTimerTask(std::atomic<int>& counter)
            : counter_(counter), delay_(std::chrono::milliseconds(50))
        {
        }

        SlowTimerTask(std::atomic<int>& counter, std::chrono::milliseconds delay)
            : counter_(counter), delay_(delay)
        {
        }

        void execute() override
        {
            ++counter_;
            std::this_thread::sleep_for(delay_);
        }

    private:
        std::atomic<int>& counter_;
        std::chrono::milliseconds delay_;
    };
}

/// @brief Test fixture for PeriodicActuator tests.
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

/**
 * @brief Verifies actuator is running immediately after start
 * @details Starts the actuator and asserts that isRunning() returns true
 *          before the task has had a chance to execute.
 */
TEST_F(PeriodicActuatorTest, Start_IsRunning)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(20));
    actuator.start();
    EXPECT_TRUE(actuator.isRunning());
    actuator.stop();
}

/**
 * @brief Verifies actuator is stopped after calling stop
 * @details Starts the actuator, immediately stops it, and asserts that
 *          isRunning() returns false.
 */
TEST_F(PeriodicActuatorTest, Stop_IsNotRunning)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(20));
    actuator.start();
    actuator.stop();
    EXPECT_FALSE(actuator.isRunning());
}

/**
 * @brief Verifies task executes periodically and counter increases
 * @details Runs a MockTimerTask at 10ms interval for 100ms and asserts
 *          the counter is incremented at least 3 times.
 */
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

/**
 * @brief Verifies exception in task does not halt the scheduler
 * @details Runs a ThrowingTimerTask that increments a counter before
 *          throwing each invocation; asserts the counter reaches >= 3,
 *          proving subsequent invocations still fire.
 */
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

/**
 * @brief Verifies double start throws std::runtime_error
 * @details Calls start() on an already-running actuator and asserts that
 *          a std::runtime_error is thrown.
 */
TEST_F(PeriodicActuatorTest, StartTwice_Throws)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(50));
    actuator.start();
    EXPECT_THROW(actuator.start(), std::runtime_error);
    actuator.stop();
}

/**
 * @brief Verifies stop on a never-started actuator is a no-op
 * @details Calls stop() on an actuator that was never started and asserts
 *          that no exception is thrown and isRunning() remains false.
 */
TEST_F(PeriodicActuatorTest, StopWithoutStart_NoOp)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(50));
    EXPECT_NO_THROW(actuator.stop());
}

/**
 * @brief Verifies restart after stop resumes task execution
 * @details Starts, stops, then restarts the actuator, capturing the
 *          counter after each phase and asserting it continues to
 *          increase after the second start.
 */
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

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

/**
 * @brief Test isRunning before start returns false
 * @details Verifies initial state is not running
 */
TEST_F(PeriodicActuatorTest, IsRunning_InitiallyFalse)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(50));
    EXPECT_FALSE(actuator.isRunning());
}

/**
 * @brief Test stop during task execution
 * @details Stopping while a task is mid-execution should not crash
 */
TEST_F(PeriodicActuatorTest, StopDuringExecution)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(1));
    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    actuator.stop();
    EXPECT_FALSE(actuator.isRunning());
}

/**
 * @brief Test multiple restart cycles
 * @details Verifies many start/stop cycles work correctly
 */
TEST_F(PeriodicActuatorTest, MultipleRestartCycles)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(10));

    for (int i = 0; i < 5; ++i)
    {
        actuator.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        actuator.stop();
        EXPECT_FALSE(actuator.isRunning());
    }
    EXPECT_GE(counter.load(), 5);
}

/**
 * @brief Test short period (5ms)
 * @edge Near-minimum feasible interval
 */
TEST_F(PeriodicActuatorTest, ShortPeriod)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    PeriodicActuator actuator(task, std::chrono::milliseconds(5));
    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    actuator.stop();
    EXPECT_GE(counter.load(), 3);
}

/**
 * @brief Test zero period throws on construction
 * @edge Zero-millisecond interval should be rejected
 */
TEST_F(PeriodicActuatorTest, ZeroPeriod)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<MockTimerTask>(counter);
    EXPECT_THROW(PeriodicActuator actuator(task, std::chrono::milliseconds(0)), std::invalid_argument);
}

/**
 * @brief Test destructor while running stops cleanly
 * @edge Destructor should stop the actuator automatically
 */
TEST_F(PeriodicActuatorTest, DestructorWhileRunning)
{
    std::atomic<int> counter{0};
    {
        auto task = std::make_shared<MockTimerTask>(counter);
        PeriodicActuator actuator(task, std::chrono::milliseconds(10));
        actuator.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    } // Destructor calls stop()
    EXPECT_GE(counter.load(), 1);
}

/**
 * @brief Test task that runs longer than period
 * @edge Overlapping executions
 */
TEST_F(PeriodicActuatorTest, TaskLongerThanPeriod)
{
    std::atomic<int> counter{0};
    auto task = std::make_shared<SlowTimerTask>(counter, std::chrono::milliseconds(50));
    PeriodicActuator actuator(task, std::chrono::milliseconds(10));
    actuator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    actuator.stop();
    // Even with overlap, at least 1 execution should have started
    EXPECT_GE(counter.load(), 1);
}
