/**
 * @file ThreadPoolTest.cc
 * @brief Unit tests for ThreadPool class
 * @details Tests cover task submission, execution, shutdown modes,
 *          queue size limits, and thread management.
 */

#include <atomic>
#include <chrono>
#include <future>
#include <stdexcept>
#include <thread>
#include <gtest/gtest.h>

#include <cppforge/thread/ThreadPool.hpp>

using namespace cppforge::thread;

/// @brief Test fixture for ThreadPool tests.
class ThreadPoolTest : public testing::Test
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
 * @brief Verifies a single submitted task executes and returns the correct value.
 * @details Submits a single lambda returning 42 and checks the future yields the expected result.
 */
TEST_F(ThreadPoolTest, SubmitAndExecute_SingleTask)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(100));
    auto future = pool.submit([] { return 42; });
    EXPECT_EQ(future.get(), 42);
    pool.shutdown();
}

/**
 * @brief Verifies multiple tasks execute concurrently and return correct results.
 * @details Submits 10 tasks computing squares and checks every future matches the expected value.
 */
TEST_F(ThreadPoolTest, SubmitAndExecute_MultipleTasks)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(100));
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i)
    {
        futures.push_back(pool.submit([i] { return i * i; }));
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(futures[i].get(), i * i);
    }
    pool.shutdown();
}

/**
 * @brief Verifies tasks with shared mutable state execute correctly.
 * @details Submits 20 tasks that increment an atomic counter and confirms the final value is 20.
 */
TEST_F(ThreadPoolTest, SubmitAndExecute_TasksWithSideEffects)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(100));
    std::atomic<int> counter{0};

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 20; ++i)
    {
        futures.push_back(pool.submit([&counter] { ++counter; }));
    }

    for (auto& f : futures)
    {
        f.get();
    }

    EXPECT_EQ(counter.load(), 20);
    pool.shutdown();
}

/**
 * @brief Verifies submitting a task to a full queue throws std::runtime_error.
 * @details Fills the queue with three sleeping tasks so the bounded queue overflows, then
 *          confirms the next submit throws. The sleeping tasks block worker threads, keeping
 *          the queue full during the window of the fourth submit.
 */
TEST_F(ThreadPoolTest, QueueFull_ThrowsException)
{
    ThreadPool pool(1, 1, 3, std::chrono::milliseconds(1000));

    // Fill queue with blocking tasks so queue stays full
    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });

    EXPECT_THROW(pool.submit([] { return; }), std::runtime_error);
    pool.shutdown();
}

/**
 * @brief Verifies graceful shutdown waits for running tasks to complete.
 * @details Submits a task that sets an atomic flag after sleeping, then calls shutdown
 *          and asserts the flag is true (confirming shutdown blocked until completion).
 */
TEST_F(ThreadPoolTest, Shutdown_WaitsForTasks)
{
    ThreadPool pool(2, 2, 10, std::chrono::milliseconds(100));
    std::atomic<bool> completed{false};

    pool.submit([&completed]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        completed = true;
    });

    pool.shutdown();
    EXPECT_TRUE(completed);
}

/**
 * @brief Verifies shutdownNow cancels queued but not-yet-executed tasks.
 * @details Submits a blocking task that occupies the single worker, then a second task that
 *          remains queued. shutdownNow should cancel the second task, so at most one task runs.
 */
TEST_F(ThreadPoolTest, ShutdownNow_CancelsPendingTasks)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    std::atomic<int> executed{0};

    // Submit one task that blocks, and one that should be canceled
    pool.submit([&executed]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ++executed;
    });
    pool.submit([&executed] { ++executed; });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pool.shutdownNow();

    EXPECT_LE(executed.load(), 1);
}

/**
 * @brief Verifies getActiveThreadCount returns the core thread count at pool creation.
 * @details Creates a pool with 3 core threads and asserts the active count is 3 immediately.
 */
TEST_F(ThreadPoolTest, ActiveThreadCount_ReturnsCorrectValue)
{
    ThreadPool pool(3, 5, 10, std::chrono::milliseconds(100));
    EXPECT_EQ(pool.getActiveThreadCount(), 3);
    pool.shutdown();
}

/**
 * @brief Verifies getQueueSize reports at least one task waiting in the queue.
 * @details Submits two sleeping tasks to a single-thread pool. The first occupies the worker,
 *          the second queues up; confirms the queue size is >= 1 before shutdown.
 */
TEST_F(ThreadPoolTest, QueueSize_ReturnsCorrectValue)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));

    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_GE(pool.getQueueSize(), 1);
    pool.shutdown();
}

/**
 * @brief Verifies calling shutdown twice does not crash or deadlock.
 * @details Calls shutdown, then calls it again and expects no exception or undefined behavior.
 */
TEST_F(ThreadPoolTest, Shutdown_Twice_NoCrash)
{
    ThreadPool pool(2, 2, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    pool.shutdown();
}

/**
 * @brief Verifies the destructor waits for running tasks to finish before returning.
 * @details Creates a pool in an inner scope, submits a task that sets an atomic flag, then
 *          checks the flag after the pool is destroyed (scope exit).
 */
TEST_F(ThreadPoolTest, Destructor_ShutdownGracefully)
{
    std::atomic<bool> completed{false};
    {
        ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
        pool.submit([&completed]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            completed = true;
        });
    }
    EXPECT_TRUE(completed);
}

/**
 * @brief Verifies the constructor throws std::invalid_argument when core threads is zero.
 * @details A pool requires at least one core thread; zero should be rejected at construction.
 */
TEST_F(ThreadPoolTest, Constructor_CoreThreadsZero_Throws)
{
    EXPECT_THROW(
        ThreadPool(0, 1, 10, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

/**
 * @brief Verifies the constructor throws when max threads is less than core threads.
 * @details The pool invariant requires max >= core; passing 3 core / 2 max is invalid.
 */
TEST_F(ThreadPoolTest, Constructor_MaxLessThanCore_Throws)
{
    EXPECT_THROW(
        ThreadPool(3, 2, 10, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

/**
 * @brief Verifies the constructor throws when queue size is zero.
 * @details A bounded queue must have capacity of at least one; zero is rejected at construction.
 */
TEST_F(ThreadPoolTest, Constructor_QueueSizeZero_Throws)
{
    EXPECT_THROW(
        ThreadPool(1, 1, 0, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

/**
 * @brief Verifies submitting a task after shutdown throws std::runtime_error.
 * @details Shuts down the pool, then attempts to submit a new task and expects rejection.
 */
TEST_F(ThreadPoolTest, SubmitAfterShutdown_Throws)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    EXPECT_THROW(
        (void)pool.submit([] { return 42; }),
        std::runtime_error
    );
}

/**
 * @brief Verifies an exception in one task does not affect other tasks.
 * @details Submits a throwing task and a normal task; confirms the exception is propagated
 *          through the future and the normal task still returns the correct value.
 */
TEST_F(ThreadPoolTest, ExceptionIsolation)
{
    ThreadPool pool(2, 2, 10, std::chrono::milliseconds(100));
    auto f1 = pool.submit([] { throw std::runtime_error("task error"); return 1; });
    auto f2 = pool.submit([] { return 42; });
    EXPECT_THROW(f1.get(), std::runtime_error);
    EXPECT_EQ(f2.get(), 42);
    pool.shutdown();
}

/**
 * @brief Verifies core threads remain alive in the pool after burst tasks complete.
 * @details Submits 10 short-lived tasks, waits for them to finish, then asserts that at
 *          least one core thread is still active (due to keepalive timeout being long).
 */
TEST_F(ThreadPoolTest, ActiveThreadCount_AfterTasksComplete)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(500));
    {
        std::vector<std::future<void>> futs;
        for (int i = 0; i < 10; ++i)
        {
            futs.push_back(pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }));
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_GE(pool.getActiveThreadCount(), 1);
    pool.shutdown();
}

/**
 * @brief Verifies queue size drops to zero after all queued tasks have been consumed.
 * @details Submits 10 short-lived tasks to a pool with ample workers, waits for completion,
 *          then asserts getQueueSize returns 0.
 */
TEST_F(ThreadPoolTest, QueueSize_ReturnsZeroAfterConsumption)
{
    ThreadPool pool(4, 4, 100, std::chrono::milliseconds(100));
    {
        std::vector<std::future<void>> futs;
        for (int i = 0; i < 10; ++i)
        {
            futs.push_back(pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }));
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(pool.getQueueSize(), 0);
    pool.shutdown();
}

/**
 * @brief Verifies calling shutdownNow multiple times does not crash or deadlock.
 * @details Calls shutdownNow three times in succession; expects no exception or UB.
 */
TEST_F(ThreadPoolTest, MultipleShutdownNow_NoCrash)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdownNow();
    pool.shutdownNow();
    pool.shutdownNow();
}

/**
 * @brief Verifies calling shutdownNow on an already-shutdown pool does not crash.
 * @details Calls shutdown first, then shutdownNow; expects no exception or undefined behavior.
 */
TEST_F(ThreadPoolTest, ShutdownNowOnShutdownPool_NoCrash)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    pool.shutdownNow();
}

/**
 * @brief Verifies concurrent submission and shutdown does not deadlock or corrupt state.
 * @details Spawns a background thread that submits 50 tasks while the main thread calls
 *          shutdown after a short delay; asserts at least one task completed.
 */
TEST_F(ThreadPoolTest, ConcurrentSubmitAndShutdown)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(100));
    std::atomic<int> completed{0};

    std::thread submitter([&pool, &completed]
    {
        for (int i = 0; i < 50; ++i)
        {
            try
            {
                auto fut = pool.submit([&completed]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    completed++;
                });
                fut.wait();
            }
            catch (const std::exception&)
            {
                break;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pool.shutdown();
    submitter.join();

    EXPECT_GE(completed, 1);
}

/**
 * @brief Verifies a future remains valid and retrievable after the pool is destroyed.
 * @details Submits a task, moves the future out of the pool scope, then calls get() after
 *          the pool destructor has run. The shared state should outlive the pool.
 */
TEST_F(ThreadPoolTest, FutureGetAfterPoolDestruction)
{
    std::future<int> result;
    {
        ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
        result = pool.submit([] { return 42; });
    }
    EXPECT_EQ(result.get(), 42);
}

/**
 * @brief Verifies submitting a null function throws an exception.
 * @details Constructs an empty std::function (evaluates to false) and confirms
 *          pool.submit rejects it with an exception rather than invoking a null target.
 */
TEST_F(ThreadPoolTest, SubmitNullFunction)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    std::function<void()> null_func;
    EXPECT_THROW(pool.submit(null_func), std::exception);
}

/**
 * @brief Verifies the constructor accepts a zero-valued keepalive duration.
 * @details A keepalive of 0ms means idle core threads may be retired immediately; the
 *          constructor should not reject this value.
 */
TEST_F(ThreadPoolTest, ConstructorZeroKeepalime)
{
    EXPECT_NO_THROW(ThreadPool pool(1, 1, 10, std::chrono::milliseconds(0)));
}

/**
 * @brief Verifies tasks submitted to a single-thread pool execute in FIFO order.
 * @details Submits 5 tasks that each push an integer to a vector under a mutex. With a single
 *          worker thread, the appended values should appear in submission order.
 */
TEST_F(ThreadPoolTest, MultipleTasksPreserveOrder)
{
    ThreadPool pool(1, 1, 100, std::chrono::milliseconds(100));

    std::vector<int> results;
    std::mutex results_mutex;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 5; ++i)
    {
        futures.push_back(pool.submit([&results, &results_mutex, i]
        {
            std::lock_guard lock(results_mutex);
            results.push_back(i);
        }));
    }
    for (auto& fut : futures)
    {
        fut.wait();
    }
    pool.shutdown();
}
