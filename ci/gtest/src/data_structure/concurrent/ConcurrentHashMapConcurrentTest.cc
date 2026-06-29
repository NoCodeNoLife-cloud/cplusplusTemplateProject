/**
 * @file ConcurrentHashMapConcurrentTest.cc
 * @brief Concurrency stress tests for ConcurrentHashMap
 * @details Multi-threaded tests verifying thread safety, data-race freedom,
 *          and correctness under concurrent read, write, erase, and mixed
 *          workloads.  Each test uses std::atomic synchronisation primitives
 *          and std::thread for deterministic thread management.
 *
 * @par Threading patterns
 * - Threads are synchronised via a spin barrier (std::atomic<bool> go_flag)
 *   to maximise contention at the start of each test.
 * - std::atomic counters track per-thread progress.
 * - Sanitizers (AddressSanitizer / ThreadSanitizer) are expected to pass.
 */

#include <cppforge/data_structure/concurrent/ConcurrentHashMap.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace cppforge::data_structure::concurrent;

// ══════════════════════════════════════════════════════════════════════════
//  Helpers
// ══════════════════════════════════════════════════════════════════════════

/// @brief Number of threads used in most concurrent tests.
constexpr int NUM_THREADS = 8;

/// @brief Total entries inserted in bulk-concurrent tests.
constexpr int TOTAL_ENTRIES = 2000;

/// @brief Spin barrier: all threads spin until the flag becomes true.
///        This maximises the chance of true concurrent execution.
class SpinBarrier
{
public:
    explicit SpinBarrier(int num_threads)
        : num_threads_(num_threads)
        , ready_{0}
        , go_{false}
    {
    }

    /// @brief Register this thread and wait for the go signal.
    void arrive_and_wait()
    {
        // Signal readiness.
        ready_.fetch_add(1, std::memory_order_release);

        // Spin until released.
        while (!go_.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
    }

    /// @brief Release all waiting threads.
    void release()
    {
        // Wait for all threads to be ready.
        while (ready_.load(std::memory_order_acquire) < num_threads_)
        {
            std::this_thread::yield();
        }

        go_.store(true, std::memory_order_release);
    }

private:
    int                  num_threads_;
    std::atomic<int>     ready_;
    std::atomic<bool>    go_;
};

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class ConcurrentHashMapConcurrentTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Concurrent read (get) �?no data race
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple threads read concurrently without data races.
 * @details Pre-insert 1000 entries, then 8 threads each perform 2000
 *          random lookups.  Only shared_lock is used, so all reads run
 *          in parallel.  No thread should observe a missing key or a
 *          corrupted value (AddressSanitizer / ThreadSanitizer pass).
 */
TEST_F(ConcurrentHashMapConcurrentTest, ConcurrentGet_NoDataRace)
{
    constexpr int PRE_INSERT = 1000;
    constexpr int LOOKUPS_PER_THREAD = 2000;

    ConcurrentHashMap<int, int> map;

    // Pre-populate.
    for (int i = 0; i < PRE_INSERT; ++i)
    {
        ASSERT_TRUE(map.insert(i, i * 3));
    }

    std::atomic<bool> fail{false};
    SpinBarrier       barrier(NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            // Local RNG seeded per thread to avoid contention on std::mt19937.
            std::mt19937 rng(static_cast<unsigned int>(t) + 12345);
            barrier.arrive_and_wait();

            for (int i = 0; i < LOOKUPS_PER_THREAD; ++i)
            {
                const int key = rng() % PRE_INSERT;
                auto      val = map.get(key);
                if (!val.has_value())
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
                if (*val != key * 3)
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(fail.load()) << "A thread observed a missing or wrong value";
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Concurrent insert of disjoint keys
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple threads insert disjoint key ranges concurrently.
 * @details 8 threads each insert TOTAL_ENTRIES/NUM_THREADS keys.  Since
 *          the key ranges are non-overlapping, every insert should succeed
 *          and the final size must equal TOTAL_ENTRIES.
 */
TEST_F(ConcurrentHashMapConcurrentTest, ConcurrentInsert_DisjointKeys_CorrectSize)
{
    constexpr int KEYS_PER_THREAD = TOTAL_ENTRIES / NUM_THREADS;

    ConcurrentHashMap<int, int> map;
    SpinBarrier                 barrier(NUM_THREADS);
    std::vector<std::thread>    threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * KEYS_PER_THREAD;
            const int end   = start + KEYS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int k = start; k < end; ++k)
            {
                // Every insert should succeed because ranges are disjoint.
                EXPECT_TRUE(map.insert(k, k * 2));
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(map.size(), static_cast<std::size_t>(TOTAL_ENTRIES));

    // Verify all entries.
    for (int i = 0; i < TOTAL_ENTRIES; ++i)
    {
        ASSERT_TRUE(map.contains(i));
        EXPECT_EQ(*map.get(i), i * 2);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Concurrent insert of the same key
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief All threads race to insert the same key �?only one should succeed.
 * @details 8 threads all call insert("shared", value) on the same key.
 *          Only the first insertion (winner of the lock race) should return
 *          true; subsequent attempts return false.  Final size must be 1.
 */
TEST_F(ConcurrentHashMapConcurrentTest, ConcurrentInsert_SameKey_OnlyOneSucceeds)
{
    constexpr int            KEY = 42;
    ConcurrentHashMap<int, int> map;
    SpinBarrier                 barrier(NUM_THREADS);
    std::atomic<int>            success_count{0};
    std::vector<std::thread>    threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            barrier.arrive_and_wait();
            if (map.insert(KEY, t))
            {
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    // Exactly one thread should have succeeded.
    EXPECT_EQ(success_count.load(), 1);
    EXPECT_EQ(map.size(), 1);

    // The value is whatever the winner inserted; we don't know which, but
    // it must be in [0, NUM_THREADS).
    auto val = map.get(KEY);
    ASSERT_TRUE(val.has_value());
    EXPECT_GE(*val, 0);
    EXPECT_LT(*val, NUM_THREADS);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Mixed read / write workload
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent insert, get, and erase from all threads.
 * @details Each thread repeatedly performs a random operation (insert,
 *          insert_or_assign, get, contains, erase) from a bounded key
 *          space for a fixed duration (~2 s).  After all threads stop,
 *          the map state is validated �?no crashes, no memory errors.
 */
TEST_F(ConcurrentHashMapConcurrentTest, MixedReadWrite_NoCrash)
{
    constexpr int    KEY_SPACE = 500;
    constexpr auto   TEST_DURATION = std::chrono::seconds(2);

    ConcurrentHashMap<int, std::string> map;
    std::atomic<bool>                   stop{false};
    std::atomic<int>                    ops_done{0};
    std::vector<std::thread>            threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<unsigned int>(t) * 7777 + 1111);

            while (!stop.load(std::memory_order_relaxed))
            {
                const int key = rng() % KEY_SPACE;
                const int op  = rng() % 5;

                switch (op)
                {
                case 0: // insert
                    map.insert(key, "v" + std::to_string(key));
                    break;
                case 1: // insert_or_assign
                    map.insert_or_assign(key, "v" + std::to_string(key));
                    break;
                case 2: // get
                    static_cast<void>(map.get(key));
                    break;
                case 3: // contains
                    static_cast<void>(map.contains(key));
                    break;
                case 4: // erase
                    static_cast<void>(map.erase(key));
                    break;
                }
                ops_done.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Let the threads run for the test duration.
    std::this_thread::sleep_for(TEST_DURATION);

    stop.store(true, std::memory_order_relaxed);
    for (auto& th : threads)
    {
        th.join();
    }

    // At least some operations completed.
    EXPECT_GT(ops_done.load(), 0);

    // The map must still be in a consistent state (no crash).
    std::size_t final_size = map.size();
    EXPECT_LE(final_size, static_cast<std::size_t>(KEY_SPACE));

    // Verify that size() matches what for_each sees.
    std::size_t counted = 0;
    map.for_each([&counted](int, const std::string&) { ++counted; });
    EXPECT_EQ(counted, final_size);
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Concurrent erase �?no double-free
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple threads erase disjoint key sets from a shared map.
 * @details Pre-insert 2000 entries, then 4 threads each erase 500 disjoint
 *          keys.  After all threads complete, the map must be empty.  This
 *          validates that concurrent erase operations on different segments
 *          (or different chains within a segment) do not cause double-free
 *          or use-after-free.
 */
TEST_F(ConcurrentHashMapConcurrentTest, ConcurrentErase_Disjoint_NoDoubleFree)
{
    constexpr int            NUM_ERASER_THREADS = 4;
    constexpr int            KEYS_PER_THREAD = TOTAL_ENTRIES / NUM_ERASER_THREADS;

    ConcurrentHashMap<int, int> map;

    // Pre-insert.
    for (int i = 0; i < TOTAL_ENTRIES; ++i)
    {
        ASSERT_TRUE(map.insert(i, i));
    }
    ASSERT_EQ(map.size(), static_cast<std::size_t>(TOTAL_ENTRIES));

    SpinBarrier              barrier(NUM_ERASER_THREADS);
    std::atomic<bool>        error{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_ERASER_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * KEYS_PER_THREAD;
            const int end   = start + KEYS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int k = start; k < end; ++k)
            {
                if (!map.erase(k))
                {
                    error.store(true, std::memory_order_relaxed);
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(error.load()) << "One or more erases returned false unexpectedly";
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Clear concurrent with inserts
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief One thread repeatedly clears while others insert �?no crash.
 * @details A writer thread calls clear() in a tight loop, while 4 producer
 *          threads continuously insert new entries.  After a short duration,
 *          all threads stop.  The test validates that no deadlock,
 *          segmentation fault, or memory corruption occurs.
 */
TEST_F(ConcurrentHashMapConcurrentTest, ClearDuringInsert_NoCrash)
{
    constexpr int      NUM_WRITERS = 4;
    constexpr auto     TEST_DURATION = std::chrono::seconds(2);

    ConcurrentHashMap<int, int> map;
    std::atomic<bool>           stop{false};
    std::atomic<int>            clear_count{0};
    std::vector<std::thread>    threads;

    // One thread repeatedly clears.
    threads.emplace_back([&]() {
        while (!stop.load(std::memory_order_relaxed))
        {
            map.clear();
            clear_count.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // Several threads insert.
    for (int t = 0; t < NUM_WRITERS; ++t)
    {
        threads.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<unsigned int>(t) * 3333);
            while (!stop.load(std::memory_order_relaxed))
            {
                const int key = rng() % 10000;
                map.insert(key, key);
            }
        });
    }

    std::this_thread::sleep_for(TEST_DURATION);
    stop.store(true, std::memory_order_relaxed);

    for (auto& th : threads)
    {
        th.join();
    }

    // At least one clear happened (likely many more).
    EXPECT_GT(clear_count.load(), 0);

    // Map is in a valid state (no crash, size is consistent).
    std::size_t s = map.size();
    std::size_t counted = 0;
    map.for_each([&counted](int, int) { ++counted; });
    EXPECT_EQ(counted, s);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. for_each concurrent with modifications
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief for_each traversal while other threads insert/erase �?no corruption.
 * @details One thread repeatedly calls for_each (which takes shared_lock on
 *          each segment), while other threads insert and erase keys.  The
 *          traversal should never crash or produce an inconsistent view
 *          (segments are locked one at a time, so modifications to segments
 *          not yet visited are invisible, and modifications to already-visited
 *          segments are reflected in the next for_each call).
 */
TEST_F(ConcurrentHashMapConcurrentTest, ForEachDuringModifications_NoCrash)
{
    constexpr int      NUM_MODIFIERS = 4;
    constexpr int      KEY_SPACE = 500;
    constexpr auto     TEST_DURATION = std::chrono::seconds(2);

    ConcurrentHashMap<int, int> map;
    std::atomic<bool>           stop{false};
    std::atomic<int>            for_each_count{0};

    // Pre-populate some data so for_each has something to traverse.
    for (int i = 0; i < KEY_SPACE; ++i)
    {
        map.insert(i, i);
    }

    std::vector<std::thread> threads;

    // for_each thread.
    threads.emplace_back([&]() {
        while (!stop.load(std::memory_order_relaxed))
        {
            map.for_each([](int, int) {
                // No-op; just traverse.
            });
            for_each_count.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // Modifier threads.
    for (int t = 0; t < NUM_MODIFIERS; ++t)
    {
        threads.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<unsigned int>(t) * 5555);
            while (!stop.load(std::memory_order_relaxed))
            {
                const int key = rng() % KEY_SPACE;
                if (rng() % 2 == 0)
                {
                    map.insert(key, key);
                }
                else
                {
                    static_cast<void>(map.erase(key));
                }
            }
        });
    }

    std::this_thread::sleep_for(TEST_DURATION);
    stop.store(true, std::memory_order_relaxed);

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_GT(for_each_count.load(), 0);
    // Map must be internally consistent.
    std::size_t s = map.size();
    std::size_t counted = 0;
    map.for_each([&counted](int, int) { ++counted; });
    EXPECT_EQ(counted, s);
}

// ══════════════════════════════════════════════════════════════════════════
//  8. High contention with few segments (SegmentBits = 1)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Extreme contention: only 2 segments, many threads insert/read.
 * @details With SegmentBits=1, only 2 segments guard the entire map.  This
 *          maximises lock contention.  8 threads insert 1000 disjoint keys
 *          and verify lookup correctness.
 */
TEST_F(ConcurrentHashMapConcurrentTest, HighContention_TwoSegments_Correctness)
{
    constexpr int            KEYS_PER_THREAD = 250;
    constexpr int            TOTAL = NUM_THREADS * KEYS_PER_THREAD;

    // Only 2 segments �?high contention.
    ConcurrentHashMap<int, int, std::hash<int>, std::equal_to<int>, 1> map;

    SpinBarrier              barrier(NUM_THREADS);
    std::atomic<bool>        error{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * KEYS_PER_THREAD;
            const int end   = start + KEYS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int k = start; k < end; ++k)
            {
                if (!map.insert(k, k * 10))
                {
                    error.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(error.load()) << "Some inserts failed unexpectedly";
    EXPECT_EQ(map.size(), static_cast<std::size_t>(TOTAL));

    // Verify all keys are present with correct values.
    for (int i = 0; i < TOTAL; ++i)
    {
        ASSERT_TRUE(map.contains(i));
        EXPECT_EQ(*map.get(i), i * 10);
    }
}

/**
 * @brief High contention with 2 segments and concurrent erase.
 * @details Insert 496 entries, then erase them concurrently with 8 threads
 *          erasing disjoint ranges.  Validates that segment-level locking
 *          works correctly even under extreme lock pressure.
 */
TEST_F(ConcurrentHashMapConcurrentTest, HighContention_TwoSegments_Erase)
{
    constexpr int TOTAL_KEYS = 496;  // 8 × 62, evenly divisible by NUM_THREADS
    constexpr int KEYS_PER_THREAD = TOTAL_KEYS / NUM_THREADS;

    ConcurrentHashMap<int, int, std::hash<int>, std::equal_to<int>, 1> map;

    for (int i = 0; i < TOTAL_KEYS; ++i)
    {
        ASSERT_TRUE(map.insert(i, i));
    }
    ASSERT_EQ(map.size(), static_cast<std::size_t>(TOTAL_KEYS));

    SpinBarrier              barrier(NUM_THREADS);
    std::atomic<bool>        error{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * KEYS_PER_THREAD;
            const int end   = start + KEYS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int k = start; k < end; ++k)
            {
                if (!map.erase(k))
                {
                    error.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(error.load()) << "Some erases returned false unexpectedly";
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}
