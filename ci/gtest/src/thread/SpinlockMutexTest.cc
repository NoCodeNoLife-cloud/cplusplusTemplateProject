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

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

/**
 * @brief Test multiple threads contending for the same lock
 * @details Verifies that many threads can safely use the spinlock
 */
TEST_F(SpinlockMutexTest, MultiThreadContention)
{
    int shared = 0;
    constexpr int numThreads = 10;
    constexpr int incrementsPerThread = 1000;
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&]
        {
            for (int i = 0; i < incrementsPerThread; ++i)
            {
                std::lock_guard<SpinlockMutex> lock(mutex_);
                ++shared;
            }
        });
    }

    for (auto& t : threads) t.join();

    EXPECT_EQ(shared, numThreads * incrementsPerThread);
}

/**
 * @brief Test concurrent lock and try_lock interleaving
 * @details Threads alternating between lock and try_lock
 */
TEST_F(SpinlockMutexTest, ConcurrentLockAndTryLock)
{
    std::atomic<int> lockedCount{0};
    constexpr int iterations = 5000;

    std::thread t1([&]
    {
        for (int i = 0; i < iterations; ++i)
        {
            mutex_.lock();
            ++lockedCount;
            mutex_.unlock();
        }
    });

    std::thread t2([&]
    {
        for (int i = 0; i < iterations; ++i)
        {
            if (mutex_.try_lock())
            {
                ++lockedCount;
                mutex_.unlock();
            }
        }
    });

    t1.join();
    t2.join();

    // At least t1's iterations must have completed
    EXPECT_GE(lockedCount.load(), iterations);
    EXPECT_LE(lockedCount.load(), iterations * 2);
}

/**
 * @brief Test try_lock_for with zero timeout
 * @edge Immediate timeout should behave like try_lock
 */
TEST_F(SpinlockMutexTest, TryLockFor_ZeroTimeout)
{
    EXPECT_TRUE(mutex_.try_lock_for(std::chrono::milliseconds(0)));
    mutex_.unlock();
}

/**
 * @brief Test try_lock_for with zero timeout when locked
 * @edge Immediate timeout should fail when already locked
 */
TEST_F(SpinlockMutexTest, TryLockFor_ZeroTimeout_WhenLocked)
{
    mutex_.lock();
    EXPECT_FALSE(mutex_.try_lock_for(std::chrono::milliseconds(0)));
    mutex_.unlock();
}

/**
 * @brief Test scoped_lock with multiple spinlocks
 * @edge Verify compatibility with C++17 scoped_lock
 */
TEST_F(SpinlockMutexTest, ScopedLock)
{
    SpinlockMutex mutex2;
    {
        std::scoped_lock lock(mutex_, mutex2);
        EXPECT_TRUE(true); // Should not deadlock
    }
}

/**
 * @brief Test stress test: many threads, many iterations
 * @edge Heavy contention stress
 */
TEST_F(SpinlockMutexTest, StressTest)
{
    int shared = 0;
    constexpr int numThreads = 20;
    constexpr int iterations = 5000;

    auto worker = [&](int id)
    {
        for (int i = 0; i < iterations; ++i)
        {
            if (id % 2 == 0)
            {
                std::lock_guard<SpinlockMutex> lock(mutex_);
                ++shared;
            }
            else
            {
                while (!mutex_.try_lock())
                {
                    std::this_thread::yield();
                }
                ++shared;
                mutex_.unlock();
            }
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back(worker, t);
    }
    for (auto& t : threads) t.join();

    EXPECT_EQ(shared, numThreads * iterations);
}

/**
 * @brief Test lock/unlock from different threads
 * @edge Verifies that one thread can lock and another can unlock
 */
TEST_F(SpinlockMutexTest, LockUnlockAcrossThreads)
{
    mutex_.lock();
    std::thread unlocker([&]
    {
        mutex_.unlock();
    });
    unlocker.join();

    // Should be able to re-lock after unlock from different thread
    EXPECT_TRUE(mutex_.try_lock());
    mutex_.unlock();
}
