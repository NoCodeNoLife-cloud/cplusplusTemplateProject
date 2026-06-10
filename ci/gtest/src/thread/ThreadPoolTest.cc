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

#include "thread/ThreadPool.hpp"

using namespace common::thread;

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

TEST_F(ThreadPoolTest, SubmitAndExecute_SingleTask)
{
    ThreadPool pool(2, 4, 100, std::chrono::milliseconds(100));
    auto future = pool.submit([] { return 42; });
    EXPECT_EQ(future.get(), 42);
    pool.shutdown();
}

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

TEST_F(ThreadPoolTest, ActiveThreadCount_ReturnsCorrectValue)
{
    ThreadPool pool(3, 5, 10, std::chrono::milliseconds(100));
    EXPECT_EQ(pool.getActiveThreadCount(), 3);
    pool.shutdown();
}

TEST_F(ThreadPoolTest, QueueSize_ReturnsCorrectValue)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));

    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    pool.submit([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_GE(pool.getQueueSize(), 1);
    pool.shutdown();
}

TEST_F(ThreadPoolTest, Shutdown_Twice_NoCrash)
{
    ThreadPool pool(2, 2, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    pool.shutdown();
}

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

TEST_F(ThreadPoolTest, Constructor_CoreThreadsZero_Throws)
{
    EXPECT_THROW(
        ThreadPool(0, 1, 10, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

TEST_F(ThreadPoolTest, Constructor_MaxLessThanCore_Throws)
{
    EXPECT_THROW(
        ThreadPool(3, 2, 10, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

TEST_F(ThreadPoolTest, Constructor_QueueSizeZero_Throws)
{
    EXPECT_THROW(
        ThreadPool(1, 1, 0, std::chrono::milliseconds(100)),
        std::invalid_argument
    );
}

TEST_F(ThreadPoolTest, SubmitAfterShutdown_Throws)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    EXPECT_THROW(
        (void)pool.submit([] { return 42; }),
        std::runtime_error
    );
}

TEST_F(ThreadPoolTest, ExceptionIsolation)
{
    ThreadPool pool(2, 2, 10, std::chrono::milliseconds(100));
    auto f1 = pool.submit([] { throw std::runtime_error("task error"); return 1; });
    auto f2 = pool.submit([] { return 42; });
    EXPECT_THROW(f1.get(), std::runtime_error);
    EXPECT_EQ(f2.get(), 42);
    pool.shutdown();
}

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

TEST_F(ThreadPoolTest, MultipleShutdownNow_NoCrash)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdownNow();
    pool.shutdownNow();
    pool.shutdownNow();
}

TEST_F(ThreadPoolTest, ShutdownNowOnShutdownPool_NoCrash)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    pool.shutdown();
    pool.shutdownNow();
}

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

TEST_F(ThreadPoolTest, FutureGetAfterPoolDestruction)
{
    std::future<int> result;
    {
        ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
        result = pool.submit([] { return 42; });
    }
    EXPECT_EQ(result.get(), 42);
}

TEST_F(ThreadPoolTest, SubmitNullFunction)
{
    ThreadPool pool(1, 1, 10, std::chrono::milliseconds(100));
    std::function<void()> null_func;
    EXPECT_THROW(pool.submit(null_func), std::exception);
}

TEST_F(ThreadPoolTest, ConstructorZeroKeepalime)
{
    EXPECT_NO_THROW(ThreadPool pool(1, 1, 10, std::chrono::milliseconds(0)));
}

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
