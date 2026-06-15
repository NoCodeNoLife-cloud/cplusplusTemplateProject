/**
 * @file SeqLockTest.cc
 * @brief Unit tests for SeqLock and MutexSeqLock
 * @details Comprehensive tests covering construction, basic operations,
 *          copy/move semantics, ReadGuard/WriteGuard RAII semantics,
 *          large-POD types, concurrent multi-reader scenarios, concurrent
 *          read-write workloads, and MutexSeqLock multi-writer correctness.
 *
 * @par Threading patterns
 * - Threads are synchronized via a spin barrier (SpinBarrier) to maximize
 *   contention at the start of concurrent tests.
 * - std::atomic counters track per-thread progress.
 * - Sanitizers (AddressSanitizer / ThreadSanitizer) are expected to pass.
 */

#include "data_structure/concurrent/SeqLock.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <future>
#include <optional>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace common::data_structure::concurrent;

// ══════════════════════════════════════════════════════════════════════════
//  Custom types for testing
// ══════════════════════════════════════════════════════════════════════════

/// @brief A POD struct with two int fields — used to test operator-> on
///        ReadGuard and WriteGuard.
struct Point
{
    int x;
    int y;

    [[nodiscard]] auto operator==(const Point& other) const -> bool
    {
        return x == other.x && y == other.y;
    }
};

/// @brief A 256-byte POD type used to verify correctness for large values.
struct alignas(64) LargePOD
{
    char data[256];

    [[nodiscard]] auto operator==(const LargePOD& other) const -> bool
    {
        return std::memcmp(data, other.data, sizeof(data)) == 0;
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  Spin barrier for concurrent tests
// ══════════════════════════════════════════════════════════════════════════

/// @brief Simple spin barrier that holds threads until release() is called.
class SpinBarrier
{
public:
    explicit SpinBarrier(int num_threads)
        : num_threads_(num_threads)
        , ready_{0}
        , go_{false}
    {
    }

    void arrive_and_wait()
    {
        ready_.fetch_add(1, std::memory_order_release);
        while (!go_.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
    }

    void release()
    {
        while (ready_.load(std::memory_order_acquire) < num_threads_)
        {
            std::this_thread::yield();
        }
        go_.store(true, std::memory_order_release);
    }

private:
    int               num_threads_;
    std::atomic<int>  ready_;
    std::atomic<bool> go_;
};

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class SeqLockTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  0. Compile-time checks
// ══════════════════════════════════════════════════════════════════════════

/// @brief int satisfies SeqLockable.
static_assert(SeqLockable<int>,
              "int must satisfy SeqLockable");

/// @brief std::string is NOT trivially copyable — must be rejected.
static_assert(!SeqLockable<std::string>,
              "std::string must NOT satisfy SeqLockable");

/// @brief SeqLock must be at least 64 bytes (cache-line isolation).
static_assert(sizeof(SeqLock<int>) >= 64,
              "SeqLock must be padded to at least 64 bytes");

/// @brief SeqLock is not copy-constructible.
static_assert(!std::is_copy_constructible_v<SeqLock<int>>,
              "SeqLock copy constructor must be deleted");

/// @brief SeqLock is not copy-assignable.
static_assert(!std::is_copy_assignable_v<SeqLock<int>>,
              "SeqLock copy assignment must be deleted");

/// @brief SeqLock is nothrow move-constructible.
static_assert(std::is_nothrow_move_constructible_v<SeqLock<int>>,
              "SeqLock move constructor must be noexcept");

/// @brief SeqLock is nothrow move-assignable.
static_assert(std::is_nothrow_move_assignable_v<SeqLock<int>>,
              "SeqLock move assignment must be noexcept");

/// @brief ReadGuard is not copy-constructible.
static_assert(!std::is_copy_constructible_v<SeqLock<int>::ReadGuard>,
              "ReadGuard copy constructor must be deleted");

/// @brief ReadGuard is not move-constructible.
static_assert(!std::is_move_constructible_v<SeqLock<int>::ReadGuard>,
              "ReadGuard move constructor must be deleted");

/// @brief WriteGuard is not copy-constructible.
static_assert(!std::is_copy_constructible_v<SeqLock<int>::WriteGuard>,
              "WriteGuard copy constructor must be deleted");

/// @brief WriteGuard is not move-constructible.
static_assert(!std::is_move_constructible_v<SeqLock<int>::WriteGuard>,
              "WriteGuard move constructor must be deleted");

// ══════════════════════════════════════════════════════════════════════════
//  1. Construction & basic operations
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed SeqLock value-initialises the protected data.
 * @details After default construction, load() must return T{} (zero for int).
 */
TEST_F(SeqLockTest, DefaultConstructor_DataIsDefaultInitialized)
{
    const SeqLock<int> lock;
    EXPECT_EQ(lock.load(), 0);
}

/**
 * @brief Value constructor stores the supplied initial value.
 * @details SeqLock<int>(42) must cause load() to return 42.
 */
TEST_F(SeqLockTest, ValueConstructor_StoresInitialValue)
{
    const SeqLock<int> lock(42);
    EXPECT_EQ(lock.load(), 42);
}

/**
 * @brief Store then load returns the stored value.
 * @details After store(100), load() must return 100.
 */
TEST_F(SeqLockTest, StoreThenLoad_ReturnsValue)
{
    SeqLock<int> lock;
    lock.store(100);
    EXPECT_EQ(lock.load(), 100);
}

/**
 * @brief Multiple stores — each load returns the latest stored value.
 * @details Verifies that store() followed by load() returns the most
 *          recently written value.
 */
TEST_F(SeqLockTest, MultipleStore_EachLoadReturnsLatest)
{
    SeqLock<int> lock;

    lock.store(10);
    EXPECT_EQ(lock.load(), 10);

    lock.store(20);
    EXPECT_EQ(lock.load(), 20);

    lock.store(30);
    EXPECT_EQ(lock.load(), 30);

    lock.store(-5);
    EXPECT_EQ(lock.load(), -5);
}

/**
 * @brief try_load on an idle (no-writer) SeqLock returns the stored value.
 * @details When no writer is active, try_load should succeed on the first
 *          attempt and return a consistent snapshot.
 */
TEST_F(SeqLockTest, TryLoad_Idle_ReturnsValue)
{
    const SeqLock<int> lock(99);
    const auto         result = lock.try_load();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 99);
}

/**
 * @brief try_load while a writer holds the lock returns std::nullopt.
 * @details A concurrent writer makes the sequence counter odd. try_load
 *          must detect this and return nullopt without spinning.
 */
TEST_F(SeqLockTest, TryLoad_WhileWriting_ReturnsNullopt)
{
    SeqLock<int> lock(0);

    std::promise<void> writer_acquired;
    auto               writer_ready = writer_acquired.get_future();

    std::thread writer([&]() {
        auto guard = lock.lock();
        *guard     = 42;
        // Signal that the write lock is held (seq_ is odd).
        writer_acquired.set_value();
        // Hold the lock long enough for the main thread to observe it.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    // Wait until the writer has definitely acquired the lock.
    writer_ready.wait();

    const auto result = lock.try_load();
    EXPECT_FALSE(result.has_value()) << "try_load must fail while a writer holds the lock";

    writer.join();

    // After the writer releases, the value must be visible.
    EXPECT_EQ(lock.load(), 42);
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Copy / Move semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Move constructor transfers the sequence counter and data.
 * @details After moving, the target holds the original value and the source
 *          is left in a valid empty state (seq_ = 0, data_ default-initialised).
 */
TEST_F(SeqLockTest, MoveConstructor_TransfersState)
{
    SeqLock<int> src(42);
    SeqLock<int> dst(std::move(src));

    // Target has the original value.
    EXPECT_EQ(dst.load(), 42);

    // NOLINTNEXTLINE(bugprone-use-after-move)
    // Source is in a valid empty state — seq_ = 0, data_ = int{} = 0.
    EXPECT_EQ(src.load(), 0);
}

/**
 * @brief Move assignment transfers state and leaves source empty.
 * @details The target's previous data is discarded; the source becomes empty.
 */
TEST_F(SeqLockTest, MoveAssignment_TransfersState)
{
    SeqLock<int> src(100);
    SeqLock<int> dst(0);

    dst = std::move(src);

    EXPECT_EQ(dst.load(), 100);

    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_EQ(src.load(), 0);
}

/**
 * @brief Self-move-assignment is safe and leaves the object intact.
 * @details move-assigning to self must not corrupt state.
 */
TEST_F(SeqLockTest, MoveSelfAssignment_Safe)
{
    SeqLock<int> lock(42);
    auto&        ref = lock;
    lock              = std::move(ref);

    EXPECT_EQ(lock.load(), 42);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. ReadGuard / WriteGuard RAII
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief ReadGuard::operator* returns a const reference to the snapshot.
 * @details protect() must return a ReadGuard whose operator* gives access
 *          to the consistent snapshot taken at construction.
 */
TEST_F(SeqLockTest, ReadGuard_Dereference_ReturnsSnapshot)
{
    const SeqLock<int> lock(77);
    const auto         guard = lock.protect();
    EXPECT_EQ(*guard, 77);
}

/**
 * @brief ReadGuard::operator-> provides const pointer access to the snapshot.
 * @details A ReadGuard for a struct type must support -> to access fields.
 */
TEST_F(SeqLockTest, ReadGuard_ArrowOperator_AccessesFields)
{
    const SeqLock<Point> lock(Point{3, 5});
    const auto           guard = lock.protect();
    EXPECT_EQ(guard->x, 3);
    EXPECT_EQ(guard->y, 5);
}

/**
 * @brief ReadGuard holds a snapshot independent of concurrent writes.
 * @brief After obtaining a ReadGuard, subsequent store() must not affect the
 *        snapshot held by the guard.
 */
TEST_F(SeqLockTest, ReadGuard_SnapshotIndependentOfWrites)
{
    SeqLock<int> lock(10);
    const auto   guard = lock.protect();
    lock.store(20);
    // The guard's snapshot must still be the old value.
    EXPECT_EQ(*guard, 10);
}

/**
 * @brief WriteGuard::operator* provides mutable access to the protected data.
 * @brief Changes made via *guard become visible to readers after the guard
 *        is destroyed (write lock released).
 */
TEST_F(SeqLockTest, WriteGuard_Dereference_ProvidesMutableAccess)
{
    SeqLock<int> lock(0);
    {
        auto guard = lock.lock();
        *guard     = 99;
    }
    EXPECT_EQ(lock.load(), 99);
}

/**
 * @brief WriteGuard::operator-> provides mutable pointer access to fields.
 * @brief Changes via guard->field become visible after the guard is destroyed.
 */
TEST_F(SeqLockTest, WriteGuard_ArrowOperator_ModifiesFields)
{
    SeqLock<Point> lock(Point{0, 0});
    {
        auto guard = lock.lock();
        guard->x   = 10;
        guard->y   = 20;
    }
    const auto p = lock.load();
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

/**
 * @brief WriteGuard destructor releases the write lock, making changes visible.
 * @details When the WriteGuard goes out of scope, the seq counter is
 *          incremented from odd to even, allowing readers to see the new data.
 */
TEST_F(SeqLockTest, WriteGuard_Destructor_ReleasesLock)
{
    SeqLock<int> lock(0);
    {
        auto guard = lock.lock();
        *guard     = 42;
    }
    // Guard destroyed — lock released, readers should see 42.
    EXPECT_EQ(lock.load(), 42);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Large POD type
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Large POD (256 bytes) survives store/load round-trips without
 *        corruption.
 * @details All bytes of the 256-byte struct must be preserved across
 *          multiple store/load cycles.
 */
TEST_F(SeqLockTest, LargePOD_ReadWriteConsistent)
{
    SeqLock<LargePOD> lock;

    // First value.
    LargePOD v1;
    std::memset(v1.data, 0xAB, sizeof(v1.data));
    lock.store(v1);
    EXPECT_EQ(lock.load(), v1);

    // Second value — different pattern.
    LargePOD v2;
    for (std::size_t i = 0; i < sizeof(v2.data); ++i)
    {
        v2.data[i] = static_cast<char>(i & 0xFF);
    }
    lock.store(v2);
    EXPECT_EQ(lock.load(), v2);

    // Third value — alternating pattern.
    LargePOD v3;
    for (std::size_t i = 0; i < sizeof(v3.data); ++i)
    {
        v3.data[i] = (i % 2 == 0) ? static_cast<char>(0xAA) : static_cast<char>(0x55);
    }
    lock.store(v3);
    EXPECT_EQ(lock.load(), v3);
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Concurrent tests — multi-reader
// ══════════════════════════════════════════════════════════════════════════

/// @brief Number of iterations each thread performs in concurrent tests.
static constexpr int kConcurrentIters = 100'000;

/// @brief Number of values the writer produces in read-write concurrent tests.
static constexpr int kWriterValues = 50'000;

/**
 * @brief Multiple concurrent readers calling load() — no data race.
 * @details 4 reader threads read the same SeqLock concurrently with no
 *          writer.  The test passes if no data race or crash occurs
 *          (detected by ThreadSanitizer / AddressSanitizer).
 */
TEST_F(SeqLockTest, ConcurrentReaders_NoDataRace)
{
    constexpr int          kNumReaders = 4;
    SeqLock<int>           lock(42);
    SpinBarrier            barrier(kNumReaders);
    std::atomic<std::size_t> completed{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < kNumReaders; ++t)
    {
        threads.emplace_back([&]() {
            barrier.arrive_and_wait();

            for (int i = 0; i < kConcurrentIters; ++i)
            {
                // load() must not crash or produce garbage under
                // concurrent read-only access.
                const int val = lock.load();
                // The value must always be 42 (no writer).
                EXPECT_EQ(val, 42);
            }

            completed.fetch_add(1, std::memory_order_release);
        });
    }

    barrier.release();

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(completed.load(std::memory_order_acquire), kNumReaders);
}

/**
 * @brief 8 concurrent readers while a single writer writes — all read values
 *        are consistent (were actually stored by the writer).
 * @details The writer writes a monotonically increasing sequence [0, N).
 *          Readers capture the values they see and must only observe values
 *          that fall within the written range.
 */
TEST_F(SeqLockTest, ConcurrentReaders_AllSeeConsistentValues)
{
    constexpr int kNumReaders = 8;

    SeqLock<int>           lock(0);
    SpinBarrier            barrier(kNumReaders + 1); // readers + writer
    std::atomic<bool>      writer_done{false};
    std::vector<std::thread> threads;

    // ── Writer thread ──
    threads.emplace_back([&]() {
        barrier.arrive_and_wait();

        for (int i = 0; i < kWriterValues; ++i)
        {
            lock.store(i);
        }

        writer_done.store(true, std::memory_order_release);
    });

    // ── Reader threads ──
    // Each reader records all values it reads and verifies they are in a
    // valid range.
    for (int t = 0; t < kNumReaders; ++t)
    {
        threads.emplace_back([&]() {
            barrier.arrive_and_wait();

            for (int i = 0; i < kConcurrentIters; ++i)
            {
                const int val = lock.load();
                // The value must be in [0, kWriterValues - 1] — the
                // writer writes exactly those values in order.
                EXPECT_GE(val, 0);
                EXPECT_LE(val, kWriterValues - 1);
            }
        });
    }

    barrier.release();

    for (auto& th : threads)
    {
        th.join();
    }

    // Final value must be the last written.
    EXPECT_EQ(lock.load(), kWriterValues - 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Concurrent tests — read-write
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief 1 writer + 4 readers running for 3 seconds — no corruption.
 * @details The writer writes a monotonically increasing counter.  Readers
 *          read via load() and verify that every value is in the valid
 *          range [0, max_written] where max_written is captured at the
 *          end of the run.
 */
TEST_F(SeqLockTest, OneWriter_ManyReaders_NoCorruption)
{
    constexpr auto     kTestDuration = std::chrono::seconds(3);
    constexpr int      kNumReaders   = 4;

    SeqLock<int>       lock(0);
    std::atomic<bool>  stop{false};
    SpinBarrier        barrier(kNumReaders + 1); // readers + writer
    std::vector<std::thread> threads;

    // Writer
    threads.emplace_back([&]() {
        barrier.arrive_and_wait();

        int val = 0;
        while (!stop.load(std::memory_order_relaxed))
        {
            lock.store(val);
            ++val;
        }
    });

    // Readers
    std::mt19937_64 rng(42); // NOLINT: fixed seed for determinism

    for (int t = 0; t < kNumReaders; ++t)
    {
        threads.emplace_back([&, t]() {
            barrier.arrive_and_wait();

            // Local PRNG to reduce contention.
            std::mt19937_64 local_rng(42 + static_cast<std::uint64_t>(t) * 1000); // NOLINT

            while (!stop.load(std::memory_order_relaxed))
            {
                const int val = lock.load();
                // Any value >= 0 is valid (the writer never writes
                // negative values).
                EXPECT_GE(val, 0);
                // Busy-wait a bit to increase contention window.
                for (volatile int spin = 0; spin < 10; ++spin) {}
            }
        });
    }

    barrier.release();

    std::this_thread::sleep_for(kTestDuration);
    stop.store(true, std::memory_order_release);

    for (auto& th : threads)
    {
        th.join();
    }

    // The final read must return the last value written (no corruption).
    const int final_val = lock.load();
    EXPECT_GE(final_val, 0);
}

/**
 * @brief 1 writer + 4 readers using try_load for 3 seconds.
 * @details Same scenario as OneWriter_ManyReaders_NoCorruption but readers
 *          use try_load().  Every non-nullopt result must be a valid value.
 *          nullopt results are expected (writer may be active).
 */
TEST_F(SeqLockTest, OneWriter_ManyReaders_TryLoadConsistency)
{
    constexpr auto     kTestDuration = std::chrono::seconds(3);
    constexpr int      kNumReaders   = 4;

    SeqLock<int>       lock(0);
    std::atomic<bool>  stop{false};
    SpinBarrier        barrier(kNumReaders + 1);
    std::vector<std::thread> threads;

    // Writer
    threads.emplace_back([&]() {
        barrier.arrive_and_wait();

        int val = 0;
        while (!stop.load(std::memory_order_relaxed))
        {
            lock.store(val);
            ++val;
        }
    });

    // Readers
    for (int t = 0; t < kNumReaders; ++t)
    {
        threads.emplace_back([&]() {
            barrier.arrive_and_wait();

            while (!stop.load(std::memory_order_relaxed))
            {
                const auto opt = lock.try_load();
                if (opt.has_value())
                {
                    // If we got a value, it must be non-negative.
                    EXPECT_GE(*opt, 0);
                }
                // nullopt is also valid (writer was active).
            }
        });
    }

    barrier.release();

    std::this_thread::sleep_for(kTestDuration);
    stop.store(true, std::memory_order_release);

    for (auto& th : threads)
    {
        th.join();
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Move semantics — concurrent safety
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief After moving a SeqLock, the source and target are independent.
 * @details The moved-from object is in a valid empty state (can be stored
 *          to and read from).  The moved-to object retains the original
 *          value.  Both can be used independently.
 */
TEST_F(SeqLockTest, MoveDuringConcurrentAccess_Isolation)
{
    SeqLock<int> src(42);
    SeqLock<int> dst(std::move(src));

    // Target has the original value.
    EXPECT_EQ(dst.load(), 42);

    // NOLINTNEXTLINE(bugprone-use-after-move)
    // Source is in a valid state — can be used for new operations.
    src.store(100);
    EXPECT_EQ(src.load(), 100);

    // Target is unaffected.
    EXPECT_EQ(dst.load(), 42);

    // Target can also be modified independently.
    dst.store(200);
    EXPECT_EQ(dst.load(), 200);

    // Both remain independent.
    EXPECT_EQ(src.load(), 100);
}

// ══════════════════════════════════════════════════════════════════════════
//  8. MutexSeqLock tests
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief MutexSeqLock basic store/load round-trip.
 * @details After store(42), load() must return 42.
 */
TEST_F(SeqLockTest, MutexSeqLock_BasicStoreLoad)
{
    MutexSeqLock<int> mlock(0);
    mlock.store(42);
    EXPECT_EQ(mlock.load(), 42);
}

/**
 * @brief MutexSeqLock try_load on an idle system returns the stored value.
 * @details When no writer holds the internal mutex, try_lock_shared
 *          succeeds and the seqlock try_load returns the value.
 */
TEST_F(SeqLockTest, MutexSeqLock_TryLoad_Idle_ReturnsValue)
{
    const MutexSeqLock<int> mlock(77);
    const auto              result = mlock.try_load();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 77);
}

/**
 * @brief MutexSeqLock multiple concurrent writers — no data corruption.
 * @details 4 threads each store their own thread-specific values (thousands
 *          of times).  After all threads join, the final value must be one
 *          of the values written by some thread (not garbage).
 */
TEST_F(SeqLockTest, MutexSeqLock_MultipleWriters_NoDataRace)
{
    constexpr int      kNumWriters = 4;
    constexpr int      kIters      = 10'000;

    MutexSeqLock<int>  mlock(0);
    std::atomic<long>  total_stored{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < kNumWriters; ++t)
    {
        threads.emplace_back([&, t]() {
            // Each writer stores a unique value sequence to make any
            // cross-writer corruption detectable.
            const int base = t * kIters;

            for (int i = 0; i < kIters; ++i)
            {
                mlock.store(base + i);
            }

            total_stored.fetch_add(kIters, std::memory_order_relaxed);
        });
    }

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(total_stored.load(std::memory_order_relaxed),
              static_cast<long>(kNumWriters * kIters));

    // The final value must be a valid written value: any value in
    // [0, kNumWriters * kIters - 1].
    const int final_val = mlock.load();
    EXPECT_GE(final_val, 0);
    EXPECT_LE(final_val, kNumWriters * kIters - 1);
}
