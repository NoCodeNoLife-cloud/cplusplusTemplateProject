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
