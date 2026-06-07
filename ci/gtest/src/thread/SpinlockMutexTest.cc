/**
 * @file SpinlockMutexTest.cc
 * @brief Unit tests for SpinlockMutex class
 * @details Tests cover lock/unlock, try_lock, try_lock_for, RAII compatibility,
 *          and mutual exclusion guarantees.
 */

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <gtest/gtest.h>

#include "thread/SpinlockMutex.hpp"

using namespace common::thread;

class SpinlockMutexTest : public testing::Test
{
protected:
    SpinlockMutex mutex_;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(SpinlockMutexTest, Lock_Unlock_NoContention)
{
    mutex_.lock();
    mutex_.unlock();
}

TEST_F(SpinlockMutexTest, TryLock_SucceedsWhenUnlocked)
{
    EXPECT_TRUE(mutex_.try_lock());
    mutex_.unlock();
}

TEST_F(SpinlockMutexTest, TryLock_FailsWhenLocked)
{
    mutex_.lock();
    EXPECT_FALSE(mutex_.try_lock());
    mutex_.unlock();
}

TEST_F(SpinlockMutexTest, LockGuard_CompilesAndWorks)
{
    {
        std::lock_guard<SpinlockMutex> lock(mutex_);
    }
}

TEST_F(SpinlockMutexTest, UniqueLock_CompilesAndWorks)
{
    {
        std::unique_lock<SpinlockMutex> lock(mutex_);
    }
}

TEST_F(SpinlockMutexTest, MutualExclusion_ProtectsSharedData)
{
    int shared = 0;
    constexpr int iterations = 10000;
    std::atomic<bool> start{false};

    std::thread t1([&]
    {
        while (!start) { std::this_thread::yield(); }
        for (int i = 0; i < iterations; ++i)
        {
            std::lock_guard<SpinlockMutex> lock(mutex_);
            ++shared;
        }
    });

    std::thread t2([&]
    {
        while (!start) { std::this_thread::yield(); }
        for (int i = 0; i < iterations; ++i)
        {
            std::lock_guard<SpinlockMutex> lock(mutex_);
            ++shared;
        }
    });

    start = true;
    t1.join();
    t2.join();

    EXPECT_EQ(shared, iterations * 2);
}

TEST_F(SpinlockMutexTest, TryLockFor_SucceedsWithinTimeout)
{
    EXPECT_TRUE(mutex_.try_lock_for(std::chrono::milliseconds(10)));
    mutex_.unlock();
}

TEST_F(SpinlockMutexTest, TryLockFor_TimesOutWhenLocked)
{
    mutex_.lock();
    EXPECT_FALSE(mutex_.try_lock_for(std::chrono::milliseconds(10)));
    mutex_.unlock();
}
