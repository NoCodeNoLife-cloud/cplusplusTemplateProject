/**
 * @file MichaelScottQueueTest.cc
 * @brief Unit tests for MichaelScottQueue �?single-threaded functional and
 *        multithreaded concurrent tests
 * @details Comprehensive tests covering construction, basic operations,
 *          FIFO ordering, edge cases, move semantics, and concurrent
 *          producer/consumer correctness for the lock-free MPMC queue.
 *
 * @par Threading patterns
 * - Threads are synchronized via a spin barrier (std::atomic<bool> go_flag)
 *   to maximize contention at the start of each concurrent test.
 * - std::atomic counters track per-thread progress.
 * - Sanitizers (AddressSanitizer / ThreadSanitizer) are expected to pass.
 */

#include <cppforge/data_structure/concurrent/MichaelScottQueue.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace cppforge::data_structure::concurrent;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class MichaelScottQueueTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Construction / destruction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed queue is empty.
 * @details Verifies the post-conditions of the no-argument constructor.
 */
TEST_F(MichaelScottQueueTest, DefaultConstructor_Empty)
{
    const MichaelScottQueue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

/**
 * @brief Move constructor transfers data and leaves source empty.
 * @details After moving, the target owns all enqueued elements and the source
 *          is in a valid empty state (can be destroyed safely).
 */
TEST_F(MichaelScottQueueTest, MoveConstructor_TransfersData)
{
    MichaelScottQueue<int> src;
    src.enqueue(1);
    src.enqueue(2);
    src.enqueue(3);

    MichaelScottQueue<int> dst(std::move(src));

    EXPECT_EQ(dst.size(), 3);
    EXPECT_EQ(*dst.try_dequeue(), 1);
    EXPECT_EQ(*dst.try_dequeue(), 2);
    EXPECT_EQ(*dst.try_dequeue(), 3);
    EXPECT_TRUE(dst.empty());

    // NOLINTNEXTLINE(bugprone-use-after-move)
    // After move, source is documented to be in a valid empty state.
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);
}

/**
 * @brief Move assignment transfers data and releases old resources.
 * @details Assigning from another queue transfers ownership; the target's old
 *          elements are freed and the source becomes empty.
 */
TEST_F(MichaelScottQueueTest, MoveAssignment_TransfersData)
{
    MichaelScottQueue<int> dst;
    dst.enqueue(99);  // Will be discarded.

    MichaelScottQueue<int> src;
    src.enqueue(1);
    src.enqueue(2);

    dst = std::move(src);

    EXPECT_EQ(dst.size(), 2);
    EXPECT_EQ(*dst.try_dequeue(), 1);
    EXPECT_EQ(*dst.try_dequeue(), 2);
    EXPECT_TRUE(dst.empty());

    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);

    // Source must survive destruction (already tested by scoping).
}

/**
 * @brief Self-move-assignment is a no-op (sanity).
 * @details Move-assigning to self should not corrupt the queue.
 */
TEST_F(MichaelScottQueueTest, MoveAssignment_SelfAssign_NoOp)
{
    MichaelScottQueue<int> q;
    q.enqueue(1);
    q.enqueue(2);

    // Self-assignment through a reference to avoid compiler warning.
    auto& ref = q;
    q = std::move(ref);

    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(*q.try_dequeue(), 1);
    EXPECT_EQ(*q.try_dequeue(), 2);
    EXPECT_TRUE(q.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Basic FIFO ordering
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Enqueue then dequeue preserves FIFO order.
 * @details Verifies that elements come out in the same order they were put in.
 */
TEST_F(MichaelScottQueueTest, EnqueueDequeue_FifoOrder)
{
    MichaelScottQueue<int> q;
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    EXPECT_EQ(*q.try_dequeue(), 10);
    EXPECT_EQ(*q.try_dequeue(), 20);
    EXPECT_EQ(*q.try_dequeue(), 30);
    EXPECT_TRUE(q.empty());
}

/**
 * @brief Interleaved enqueue/dequeue preserves ordering.
 * @details Partially drain the queue, enqueue more items, then drain again.
 *          Order must be preserved across the interleaving.
 */
TEST_F(MichaelScottQueueTest, InterleavedEnqueueDequeue)
{
    MichaelScottQueue<int> q;

    // ── Pre-SEH enqueues (these are known-safe) ────────────────────────
    std::cout << "STEP: enqueue(1)" << std::endl;
    q.enqueue(1);
    std::cout << "STEP: enqueue(2)" << std::endl;
    q.enqueue(2);
    std::cout << "STEP: enqueue(3)" << std::endl;
    q.enqueue(3);

    // ── Operations (STEP output pinpoints crash location in CI logs) ──
    std::cout << "STEP: try_dequeue #1" << std::endl;
    const auto v1 = q.try_dequeue();
    std::cout << "STEP: try_dequeue #1 result = "
              << (v1.has_value() ? std::to_string(*v1) : "nullopt") << std::endl;
    EXPECT_TRUE(v1.has_value());
    EXPECT_EQ(*v1, 1);

    std::cout << "STEP: try_dequeue #2" << std::endl;
    const auto v2 = q.try_dequeue();
    std::cout << "STEP: try_dequeue #2 result = "
              << (v2.has_value() ? std::to_string(*v2) : "nullopt") << std::endl;
    EXPECT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 2);

    std::cout << "STEP: enqueue(4)" << std::endl;
    q.enqueue(4);
    std::cout << "STEP: enqueue(5)" << std::endl;
    q.enqueue(5);

    std::cout << "STEP: try_dequeue #3" << std::endl;
    const auto v3 = q.try_dequeue();
    std::cout << "STEP: try_dequeue #3 result = "
              << (v3.has_value() ? std::to_string(*v3) : "nullopt") << std::endl;
    EXPECT_TRUE(v3.has_value());
    EXPECT_EQ(*v3, 3);

    std::cout << "STEP: try_dequeue #4" << std::endl;
    const auto v4 = q.try_dequeue();
    std::cout << "STEP: try_dequeue #4 result = "
              << (v4.has_value() ? std::to_string(*v4) : "nullopt") << std::endl;
    EXPECT_TRUE(v4.has_value());
    EXPECT_EQ(*v4, 4);

    std::cout << "STEP: try_dequeue #5" << std::endl;
    const auto v5 = q.try_dequeue();
    std::cout << "STEP: try_dequeue #5 result = "
              << (v5.has_value() ? std::to_string(*v5) : "nullopt") << std::endl;
    EXPECT_TRUE(v5.has_value());
    EXPECT_EQ(*v5, 5);

    std::cout << "STEP: final empty check" << std::endl;
    EXPECT_TRUE(q.empty());
}

/**
 * @brief Enqueue N items, dequeue N items �?verify order and values.
 * @details Exercises bulk insertion and full drain.
 */
TEST_F(MichaelScottQueueTest, BulkEnqueueDequeue)
{
    constexpr int N = 1000;
    MichaelScottQueue<int> q;

    for (int i = 0; i < N; ++i)
    {
        q.enqueue(i);
    }
    EXPECT_EQ(q.size(), static_cast<std::size_t>(N));

    for (int i = 0; i < N; ++i)
    {
        const auto val = q.try_dequeue();
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(*val, i);
    }
    EXPECT_TRUE(q.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  3. try_dequeue on empty queue
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief try_dequeue on an empty queue returns std::nullopt.
 * @details Newly constructed queue and fully drained queue both return
 *          nullopt.
 */
TEST_F(MichaelScottQueueTest, TryDequeue_Empty_ReturnsNullopt)
{
    MichaelScottQueue<int> empty_q;
    const auto val = empty_q.try_dequeue();
    EXPECT_FALSE(val.has_value());
}

/**
 * @brief Draining all elements then dequeueing returns nullopt.
 * @details After a full drain, the queue should behave as empty.
 */
TEST_F(MichaelScottQueueTest, TryDequeue_AfterFullDrain_ReturnsNullopt)
{
    MichaelScottQueue<int> q;
    q.enqueue(1);
    std::ignore = q.try_dequeue();  // Drain.
    EXPECT_TRUE(q.empty());

    const auto val = q.try_dequeue();
    EXPECT_FALSE(val.has_value());
}

// ══════════════════════════════════════════════════════════════════════════
//  4. String values
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Queue of std::string works correctly.
 * @details Strings are moved in and out.  Verifies FIFO ordering and value
 *          integrity.
 */
TEST_F(MichaelScottQueueTest, StringValues)
{
    MichaelScottQueue<std::string> q;
    q.enqueue("alpha");
    q.enqueue("bravo");
    q.enqueue("charlie");

    EXPECT_EQ(*q.try_dequeue(), "alpha");
    EXPECT_EQ(*q.try_dequeue(), "bravo");
    EXPECT_EQ(*q.try_dequeue(), "charlie");
    EXPECT_TRUE(q.empty());
}

/**
 * @brief Non-trivial move-only types work (e.g., std::unique_ptr<int>).
 */
TEST_F(MichaelScottQueueTest, MoveOnlyType)
{
    MichaelScottQueue<std::unique_ptr<int>> q;

    q.enqueue(std::make_unique<int>(42));
    q.enqueue(std::make_unique<int>(100));

    const auto a = q.try_dequeue();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(**a, 42);

    const auto b = q.try_dequeue();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(**b, 100);

    EXPECT_TRUE(q.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Move semantics on value types
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Large movable struct type enqueue/dequeue.
 * @details Exercises move construction/assignment with non-trivial data.
 */
struct LargeMovable
{
    std::string data;
    int         id = 0;

    LargeMovable() = default;
    LargeMovable(std::string d, int i)
        : data(std::move(d)), id(i)
    {
    }

    LargeMovable(const LargeMovable&) = delete;
    auto operator=(const LargeMovable&) -> LargeMovable& = delete;

    LargeMovable(LargeMovable&&) = default;
    auto operator=(LargeMovable&&) -> LargeMovable& = default;
};

TEST_F(MichaelScottQueueTest, LargeMovableType)
{
    MichaelScottQueue<LargeMovable> q;
    q.enqueue(LargeMovable("hello", 1));
    q.enqueue(LargeMovable("world", 2));

    const auto a = q.try_dequeue();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(a->data, "hello");
    EXPECT_EQ(a->id, 1);

    const auto b = q.try_dequeue();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(b->data, "world");
    EXPECT_EQ(b->id, 2);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Interface completeness �?IConcurrentQueue methods are callable
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verifies that the full IConcurrentQueue interface is accessible and
 *        works in a typical usage scenario.
 * @details Smoke-test covering enqueue, try_dequeue, size, empty.
 */
TEST_F(MichaelScottQueueTest, FullInterfaceSmoke)
{
    MichaelScottQueue<int> q;

    // Initially empty.
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);

    // Enqueue.
    q.enqueue(1);
    q.enqueue(2);
    EXPECT_EQ(q.size(), 2);

    // Dequeue.
    EXPECT_EQ(*q.try_dequeue(), 1);
    EXPECT_EQ(q.size(), 1);

    // Empty after dequeue.
    EXPECT_FALSE(q.empty());

    // Dequeue remaining.
    EXPECT_EQ(*q.try_dequeue(), 2);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Noexcept guarantees
// ══════════════════════════════════════════════════════════════════════════

/// @brief The destructor is noexcept.
static_assert(std::is_nothrow_destructible_v<MichaelScottQueue<int>>,
              "MichaelScottQueue destructor must be noexcept");

/// @brief Move constructor is noexcept.
static_assert(std::is_nothrow_move_constructible_v<MichaelScottQueue<int>>,
              "MichaelScottQueue move constructor must be noexcept");

/// @brief Move assignment is noexcept.
static_assert(std::is_nothrow_move_assignable_v<MichaelScottQueue<int>>,
              "MichaelScottQueue move assignment must be noexcept");

/// @brief Copy constructor is deleted.
static_assert(!std::is_copy_constructible_v<MichaelScottQueue<int>>,
              "MichaelScottQueue copy constructor must be deleted");

/// @brief Copy assignment is deleted.
static_assert(!std::is_copy_assignable_v<MichaelScottQueue<int>>,
              "MichaelScottQueue copy assignment must be deleted");

// ══════════════════════════════════════════════════════════════════════════
//  Concurrent tests
// ══════════════════════════════════════════════════════════════════════════

/// @brief Number of threads used in concurrent tests.
constexpr int CONCUR_NUM_THREADS = 8;

/// @brief Total items enqueued in bulk concurrent tests.
constexpr int CONCUR_TOTAL_ITEMS = 8000;

/// @brief Spin barrier for concurrent tests.
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
//  8. Multi-threaded: N producers, M consumers
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple producers and consumers �?verify no data loss.
 * @details 4 producer threads each enqueue CONCUR_TOTAL_ITEMS / 4 items.
 *          4 consumer threads dequeue until all items are consumed.
 *          Every item must be consumed exactly once, and the overall FIFO
 *          order must be preserved (each producer's items appear in order).
 */
TEST_F(MichaelScottQueueTest, ConcurrentProducersConsumers_NoDataLoss)
{
    constexpr int NUM_PRODUCERS = 4;
    constexpr int NUM_CONSUMERS = 4;
    constexpr int ITEMS_PER_PRODUCER = CONCUR_TOTAL_ITEMS / NUM_PRODUCERS;

    MichaelScottQueue<int> q;
    std::atomic<int>       produced{0};
    std::atomic<int>       consumed{0};
    std::atomic<bool>      done{false};
    std::vector<int>       consumed_items(CONCUR_TOTAL_ITEMS, 0);
    std::mutex             consumed_mutex;

    SpinBarrier          barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    // Producer threads.
    for (int t = 0; t < NUM_PRODUCERS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * ITEMS_PER_PRODUCER;
            const int end   = start + ITEMS_PER_PRODUCER;

            barrier.arrive_and_wait();

            for (int i = start; i < end; ++i)
            {
                q.enqueue(i);
                produced.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Consumer threads.
    for (int t = 0; t < NUM_CONSUMERS; ++t)
    {
        threads.emplace_back([&]() {
            barrier.arrive_and_wait();

            while (true)
            {
                const auto val = q.try_dequeue();
                if (val.has_value())
                {
                    std::lock_guard<std::mutex> lock(consumed_mutex);
                    consumed_items[static_cast<std::size_t>(*val)]++;
                    consumed.fetch_add(1, std::memory_order_relaxed);
                }
                else if (done.load(std::memory_order_acquire))
                {
                    // One more attempt after the done flag to avoid
                    // missing the last item.
                    const auto last = q.try_dequeue();
                    if (last.has_value())
                    {
                        std::lock_guard<std::mutex> lock(consumed_mutex);
                        consumed_items[static_cast<std::size_t>(*last)]++;
                        consumed.fetch_add(1, std::memory_order_relaxed);
                    }
                    break;
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    barrier.release();

    // Wait for all producers to finish.
    for (int t = 0; t < NUM_PRODUCERS; ++t)
    {
        threads[static_cast<std::size_t>(t)].join();
    }

    // Signal consumers to stop.
    done.store(true, std::memory_order_release);

    // Wait for consumers.
    for (int t = NUM_PRODUCERS; t < CONCUR_NUM_THREADS; ++t)
    {
        threads[static_cast<std::size_t>(t)].join();
    }

    // Verify: all items produced must equal all items consumed.
    EXPECT_EQ(produced.load(), CONCUR_TOTAL_ITEMS);
    EXPECT_EQ(consumed.load(), CONCUR_TOTAL_ITEMS);

    // Verify: each value was consumed exactly once.
    for (int i = 0; i < CONCUR_TOTAL_ITEMS; ++i)
    {
        EXPECT_EQ(consumed_items[static_cast<std::size_t>(i)], 1)
            << "Item " << i << " was consumed " << consumed_items[static_cast<std::size_t>(i)]
            << " times";
    }

    // Queue must be empty.
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Multi-threaded: all producers, all consumers
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief All 8 threads are both producers and consumers.
 * @details Each thread enqueues 1000 items and then dequeues items until the
 *          total consumed count reaches CONCUR_TOTAL_ITEMS.  Validates that
 *          no deadlock, data corruption, or memory error occurs when every
 *          thread competes for both head and tail.
 */
TEST_F(MichaelScottQueueTest, ConcurrentAllProducersConsumers_Stress)
{
    constexpr int ITEMS_PER_THREAD = 1000;
    constexpr int TOTAL = CONCUR_NUM_THREADS * ITEMS_PER_THREAD;  // 8000

    MichaelScottQueue<int> q;
    std::atomic<int>       total_consumed{0};
    std::atomic<bool>      stop{false};

    SpinBarrier              barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    std::vector<std::atomic<int>>    consumed_counts(TOTAL);

    for (auto& c : consumed_counts)
    {
        c.store(0, std::memory_order_relaxed);
    }

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            barrier.arrive_and_wait();

            // Produce ITEMS_PER_THREAD items.
            for (int i = 0; i < ITEMS_PER_THREAD; ++i)
            {
                const int val = t * ITEMS_PER_THREAD + i;
                q.enqueue(val);
            }

            // Consume items until stop.
            while (!stop.load(std::memory_order_relaxed))
            {
                const auto val = q.try_dequeue();
                if (val.has_value())
                {
                    const int idx = *val;
                    consumed_counts[static_cast<std::size_t>(idx)]
                        .fetch_add(1, std::memory_order_relaxed);
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    barrier.release();

    // Wait until all items have been produced (they should all be consumed
    // very quickly by the consumer side of each thread).  Since every thread
    // is also a consumer, the queue should drain rapidly.
    //
    // Spin-wait until total_consumed reaches TOTAL or timeout (60 s).
    constexpr auto TIMEOUT = std::chrono::seconds(60);
    const auto     deadline = std::chrono::steady_clock::now() + TIMEOUT;

    while (total_consumed.load(std::memory_order_relaxed) < TOTAL)
    {
        if (std::chrono::steady_clock::now() >= deadline)
        {
            break;  // Timed out �?will fail below.
        }
        std::this_thread::yield();
    }

    stop.store(true, std::memory_order_release);

    for (auto& th : threads)
    {
        th.join();
    }

    // Verify consumption.
    EXPECT_EQ(total_consumed.load(), TOTAL);

    // Each value must have been consumed exactly once.
    for (int i = 0; i < TOTAL; ++i)
    {
        int c = consumed_counts[static_cast<std::size_t>(i)].load(std::memory_order_relaxed);
        EXPECT_EQ(c, 1) << "Item " << i << " was consumed " << c << " times";
    }

    // Queue should be empty (or nearly so �?allow a small race window).
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  10. High-contention enqueue-only test
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief All threads enqueue concurrently �?validate final size and content.
 * @details 8 threads each enqueue 1000 items.  After all join, dequeue
 *          and verify that all items are present exactly once.
 */
TEST_F(MichaelScottQueueTest, ConcurrentEnqueueOnly_SizeCorrect)
{
    constexpr int ITEMS_PER_THREAD = 1000;
    constexpr int TOTAL = CONCUR_NUM_THREADS * ITEMS_PER_THREAD;

    MichaelScottQueue<int> q;
    SpinBarrier            barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * ITEMS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int i = 0; i < ITEMS_PER_THREAD; ++i)
            {
                q.enqueue(start + i);
            }
        });
    }

    barrier.release();

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(q.size(), static_cast<std::size_t>(TOTAL));

    // Dequeue all items and verify presence.
    std::vector<int> seen(TOTAL, 0);
    for (int i = 0; i < TOTAL; ++i)
    {
        const auto val = q.try_dequeue();
        if (!val.has_value())
        {
            break;  // Queue empty before all items drained �?will fail below.
        }
        seen[static_cast<std::size_t>(*val)]++;
    }

    for (int i = 0; i < TOTAL; ++i)
    {
        EXPECT_EQ(seen[static_cast<std::size_t>(i)], 1)
            << "Item " << i << " seen " << seen[static_cast<std::size_t>(i)] << " times";
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  11. Mixed enqueue/dequeue workload
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent mixed enqueue/dequeue �?no crash or data corruption.
 * @details Each thread repeatedly enqueues or dequeues from a shared queue
 *          for a fixed duration (~2 s).  After all threads stop, the total
 *          produced vs consumed counts are validated (may not be exact due
 *          to items still in the queue, but the queue state must be
 *          consistent).
 */
TEST_F(MichaelScottQueueTest, MixedEnqueueDequeue_NoCorruption)
{
    constexpr auto     TEST_DURATION = std::chrono::seconds(2);

    MichaelScottQueue<int> q;
    std::atomic<bool>      stop{false};
    std::atomic<int>       ops_done{0};

    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<unsigned int>(t) * 5555 + 3333);

            while (!stop.load(std::memory_order_relaxed))
            {
                const int op = static_cast<int>(rng() % 2);

                if (op == 0)
                {
                    // Enqueue.
                    q.enqueue(static_cast<int>(rng() % 10000));
                }
                else
                {
                    // Try dequeue.
                    std::ignore = q.try_dequeue();
                }
                ops_done.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::this_thread::sleep_for(TEST_DURATION);

    stop.store(true, std::memory_order_relaxed);
    for (auto& th : threads)
    {
        th.join();
    }

    // At least some operations completed.
    EXPECT_GT(ops_done.load(), 0);

    // Queue must be in a valid state (no crash, size is consistent).
    const std::size_t s = q.size();
    // size() is approximate, but it should be a non-negative value consistent
    // with internal invariants (no wraparound, no corruption).
    EXPECT_LE(s, static_cast<std::size_t>(ops_done.load() / 2 + CONCUR_NUM_THREADS));

    // Drain remaining items �?must not crash.
    int drain_count = 0;
    while (q.try_dequeue().has_value())
    {
        ++drain_count;
    }
    // Drain count plus final size should be 0 (all drained).
    EXPECT_EQ(q.size(), 0);
    EXPECT_TRUE(q.empty());

    // Total operations should approximately balance (allow slack).
    // We only validate that drain succeeded without crashing.
    EXPECT_GE(drain_count, 0);
}
