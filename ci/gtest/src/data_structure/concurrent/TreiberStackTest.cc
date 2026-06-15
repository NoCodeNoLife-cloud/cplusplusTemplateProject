/**
 * @file TreiberStackTest.cc
 * @brief Unit tests for TreiberStack — single-threaded functional and
 *        multithreaded concurrent tests
 * @details Comprehensive tests covering construction, basic operations,
 *          LIFO ordering, edge cases, move semantics, and concurrent
 *          producer/consumer correctness for the lock-free MPMC stack.
 *
 * @par Threading patterns
 * - Threads are synchronized via a spin barrier (std::atomic<bool> go_flag)
 *   to maximize contention at the start of each concurrent test.
 * - std::atomic counters track per-thread progress.
 * - Sanitizers (AddressSanitizer / ThreadSanitizer) are expected to pass.
 */

#include "data_structure/concurrent/TreiberStack.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace common::data_structure::concurrent;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class TreiberStackTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Construction / destruction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed stack is empty.
 * @details Verifies the post-conditions of the no-argument constructor.
 */
TEST_F(TreiberStackTest, DefaultConstructor_Empty)
{
    const TreiberStack<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

/**
 * @brief Move constructor transfers data and leaves source empty.
 * @details After moving, the target owns all pushed elements and the source
 *          is in a valid empty state (can be destroyed safely).
 */
TEST_F(TreiberStackTest, MoveConstructor_TransfersData)
{
    TreiberStack<int> src;
    src.push(1);
    src.push(2);
    src.push(3);

    TreiberStack<int> dst(std::move(src));

    EXPECT_EQ(dst.size(), 3);
    EXPECT_EQ(*dst.try_pop(), 3);
    EXPECT_EQ(*dst.try_pop(), 2);
    EXPECT_EQ(*dst.try_pop(), 1);
    EXPECT_TRUE(dst.empty());

    // NOLINTNEXTLINE(bugprone-use-after-move)
    // After move, source is documented to be in a valid empty state.
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);
}

/**
 * @brief Move assignment transfers data and releases old resources.
 * @details Assigning from another stack transfers ownership; the target's old
 *          elements are freed and the source becomes empty.
 */
TEST_F(TreiberStackTest, MoveAssignment_TransfersData)
{
    TreiberStack<int> dst;
    dst.push(99);  // Will be discarded.

    TreiberStack<int> src;
    src.push(1);
    src.push(2);

    dst = std::move(src);

    EXPECT_EQ(dst.size(), 2);
    EXPECT_EQ(*dst.try_pop(), 2);
    EXPECT_EQ(*dst.try_pop(), 1);
    EXPECT_TRUE(dst.empty());

    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0);

    // Source must survive destruction (already tested by scoping).
}

/**
 * @brief Self-move-assignment is a no-op (sanity).
 * @details Move-assigning to self should not corrupt the stack.
 */
TEST_F(TreiberStackTest, MoveAssignment_SelfAssign_NoOp)
{
    TreiberStack<int> s;
    s.push(1);
    s.push(2);

    // Self-assignment through a reference to avoid compiler warning.
    auto& ref = s;
    s = std::move(ref);

    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(*s.try_pop(), 2);
    EXPECT_EQ(*s.try_pop(), 1);
    EXPECT_TRUE(s.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Basic LIFO ordering
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Push then pop preserves LIFO order.
 * @details Verifies that elements come out in reverse order they were put in.
 */
TEST_F(TreiberStackTest, PushPop_LifoOrder)
{
    TreiberStack<int> s;
    s.push(10);
    s.push(20);
    s.push(30);

    EXPECT_EQ(*s.try_pop(), 30);
    EXPECT_EQ(*s.try_pop(), 20);
    EXPECT_EQ(*s.try_pop(), 10);
    EXPECT_TRUE(s.empty());
}

/**
 * @brief Interleaved push/pop preserves LIFO ordering.
 * @details Partially drain the stack, push more items, then drain again.
 *          Order must be preserved across the interleaving.
 */
TEST_F(TreiberStackTest, InterleavedPushPop)
{
    TreiberStack<int> s;

    // Push initial values.
    s.push(1);
    s.push(2);
    s.push(3);

    // Pop some — should see 3, then 2.
    EXPECT_EQ(*s.try_pop(), 3);
    EXPECT_EQ(*s.try_pop(), 2);

    // Push more values.
    s.push(4);
    s.push(5);

    // Pop remaining — LIFO: 5, 4, 1.
    EXPECT_EQ(*s.try_pop(), 5);
    EXPECT_EQ(*s.try_pop(), 4);
    EXPECT_EQ(*s.try_pop(), 1);

    EXPECT_TRUE(s.empty());
}

/**
 * @brief Push N items, pop N items — verify reverse order and values.
 * @details Exercises bulk insertion and full drain on a stack.
 */
TEST_F(TreiberStackTest, BulkPushPop)
{
    constexpr int N = 1000;
    TreiberStack<int> s;

    for (int i = 0; i < N; ++i)
    {
        s.push(i);
    }
    EXPECT_EQ(s.size(), static_cast<std::size_t>(N));

    // LIFO order: last pushed (N-1) must come out first.
    for (int i = N - 1; i >= 0; --i)
    {
        const auto val = s.try_pop();
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(*val, i);
    }
    EXPECT_TRUE(s.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Empty stack behavior
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief try_pop on an empty stack returns std::nullopt.
 * @details Newly constructed stack returns nullopt.
 */
TEST_F(TreiberStackTest, TryPop_Empty_ReturnsNullopt)
{
    TreiberStack<int> empty_s;
    const auto val = empty_s.try_pop();
    EXPECT_FALSE(val.has_value());
}

/**
 * @brief Draining all elements then popping returns nullopt.
 * @details After a full drain, the stack should behave as empty.
 */
TEST_F(TreiberStackTest, TryPop_AfterFullDrain_ReturnsNullopt)
{
    TreiberStack<int> s;
    s.push(1);
    std::ignore = s.try_pop();  // Drain.
    EXPECT_TRUE(s.empty());

    const auto val = s.try_pop();
    EXPECT_FALSE(val.has_value());
}

/**
 * @brief Multiple push/pop cycles leave stack in consistent empty state.
 * @details Exercises repeated fill-and-drain to verify state transitions.
 */
TEST_F(TreiberStackTest, PushPopCycle_MultipleTimes)
{
    TreiberStack<int> s;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        // Push N values.
        for (int i = 0; i < 100; ++i)
        {
            s.push(i);
        }
        EXPECT_EQ(s.size(), 100);

        // Pop all values.
        for (int i = 99; i >= 0; --i)
        {
            const auto val = s.try_pop();
            ASSERT_TRUE(val.has_value());
            EXPECT_EQ(*val, i);
        }

        // Must be empty after drain.
        EXPECT_TRUE(s.empty());
        EXPECT_EQ(s.size(), 0);

        // Pop again — must still be nullopt.
        EXPECT_FALSE(s.try_pop().has_value());
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  4. String values
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Stack of std::string works correctly.
 * @details Strings are moved in and out.  Verifies LIFO ordering and value
 *          integrity.
 */
TEST_F(TreiberStackTest, StringValues)
{
    TreiberStack<std::string> s;
    s.push("alpha");
    s.push("bravo");
    s.push("charlie");

    EXPECT_EQ(*s.try_pop(), "charlie");
    EXPECT_EQ(*s.try_pop(), "bravo");
    EXPECT_EQ(*s.try_pop(), "alpha");
    EXPECT_TRUE(s.empty());
}

/**
 * @brief Non-trivial move-only types work (e.g., std::unique_ptr<int>).
 */
TEST_F(TreiberStackTest, MoveOnlyType)
{
    TreiberStack<std::unique_ptr<int>> s;

    s.push(std::make_unique<int>(42));
    s.push(std::make_unique<int>(100));

    const auto a = s.try_pop();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(**a, 100);

    const auto b = s.try_pop();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(**b, 42);

    EXPECT_TRUE(s.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Move semantics on value types
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Large movable struct type push/pop.
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

TEST_F(TreiberStackTest, LargeMovableType)
{
    TreiberStack<LargeMovable> s;
    s.push(LargeMovable("hello", 1));
    s.push(LargeMovable("world", 2));

    const auto a = s.try_pop();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(a->data, "world");
    EXPECT_EQ(a->id, 2);

    const auto b = s.try_pop();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(b->data, "hello");
    EXPECT_EQ(b->id, 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Interface completeness — IConcurrentStack methods are callable
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verifies that the full IConcurrentStack interface is accessible and
 *        works in a typical usage scenario.
 * @details Smoke-test covering push, try_pop, size, empty.
 */
TEST_F(TreiberStackTest, FullInterfaceSmoke)
{
    TreiberStack<int> s;

    // Initially empty.
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);

    // Push.
    s.push(1);
    s.push(2);
    EXPECT_EQ(s.size(), 2);

    // Pop — LIFO gives last-pushed first.
    EXPECT_EQ(*s.try_pop(), 2);
    EXPECT_EQ(s.size(), 1);

    // Not empty after partial pop.
    EXPECT_FALSE(s.empty());

    // Pop remaining.
    EXPECT_EQ(*s.try_pop(), 1);
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Noexcept guarantees
// ══════════════════════════════════════════════════════════════════════════

/// @brief The destructor is noexcept.
static_assert(std::is_nothrow_destructible_v<TreiberStack<int>>,
              "TreiberStack destructor must be noexcept");

/// @brief Move constructor is noexcept.
static_assert(std::is_nothrow_move_constructible_v<TreiberStack<int>>,
              "TreiberStack move constructor must be noexcept");

/// @brief Move assignment is noexcept.
static_assert(std::is_nothrow_move_assignable_v<TreiberStack<int>>,
              "TreiberStack move assignment must be noexcept");

/// @brief Copy constructor is deleted.
static_assert(!std::is_copy_constructible_v<TreiberStack<int>>,
              "TreiberStack copy constructor must be deleted");

/// @brief Copy assignment is deleted.
static_assert(!std::is_copy_assignable_v<TreiberStack<int>>,
              "TreiberStack copy assignment must be deleted");

// ══════════════════════════════════════════════════════════════════════════
//  Concurrent tests
// ══════════════════════════════════════════════════════════════════════════

/// @brief Number of threads used in concurrent tests.
constexpr int CONCUR_NUM_THREADS = 8;

/// @brief Total items pushed in bulk concurrent tests.
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
//  8. Multi-threaded: single producer, multiple consumers (LIFO ordering)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Single producer pushes CONCUR_TOTAL_ITEMS values; 8 consumers pop.
 * @details Under a Treiber stack, strict LIFO order cannot be guaranteed
 *          under concurrent pops, but every value must be consumed exactly
 *          once.  Validates no data loss.
 */
TEST_F(TreiberStackTest, ConcurrentPushPop_LifoOrder)
{
    constexpr int NUM_CONSUMERS = CONCUR_NUM_THREADS;

    TreiberStack<int>    stack;
    std::atomic<int>     produced{0};
    std::atomic<int>     consumed{0};
    std::atomic<bool>    done{false};
    std::vector<int>     consumed_items(CONCUR_TOTAL_ITEMS, 0);
    std::mutex           consumed_mutex;

    SpinBarrier          barrier(NUM_CONSUMERS + 1);
    std::vector<std::thread> threads;

    // One producer thread pushes all items.
    threads.emplace_back([&]() {
        barrier.arrive_and_wait();

        for (int i = 0; i < CONCUR_TOTAL_ITEMS; ++i)
        {
            stack.push(i);
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // Consumer threads.
    for (int t = 0; t < NUM_CONSUMERS; ++t)
    {
        threads.emplace_back([&]() {
            barrier.arrive_and_wait();

            while (true)
            {
                const auto val = stack.try_pop();
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
                    const auto last = stack.try_pop();
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

    // Wait for the producer to finish.
    threads[0].join();

    // Signal consumers to stop.
    done.store(true, std::memory_order_release);

    // Wait for consumers.
    for (int t = 1; t <= CONCUR_NUM_THREADS; ++t)
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

    // Stack must be empty.
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Multi-threaded: all producers, all consumers (stress)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief All 8 threads are both producers and consumers.
 * @details Each thread enqueues 1000 items and then dequeues items until the
 *          total consumed count reaches CONCUR_TOTAL_ITEMS.  Validates that
 *          no deadlock, data corruption, or memory error occurs when every
 *          thread competes for the stack top.
 */
TEST_F(TreiberStackTest, ConcurrentAllPushPop_Stress)
{
    constexpr int ITEMS_PER_THREAD = 1000;
    constexpr int TOTAL = CONCUR_NUM_THREADS * ITEMS_PER_THREAD;  // 8000

    TreiberStack<int>    stack;
    std::atomic<int>     total_consumed{0};
    std::atomic<bool>    stop{false};

    SpinBarrier              barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    std::vector<std::atomic<int>> consumed_counts(TOTAL);

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
                stack.push(val);
            }

            // Consume items until stop.
            while (!stop.load(std::memory_order_relaxed))
            {
                const auto val = stack.try_pop();
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

    // Wait until all items have been consumed or timeout (60 s).
    constexpr auto TIMEOUT = std::chrono::seconds(60);
    const auto     deadline = std::chrono::steady_clock::now() + TIMEOUT;

    while (total_consumed.load(std::memory_order_relaxed) < TOTAL)
    {
        if (std::chrono::steady_clock::now() >= deadline)
        {
            break;  // Timed out — will fail below.
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

    // Stack should be empty (or nearly so — allow a small race window).
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  10. High-contention push-only test
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief All threads push concurrently — validate final size and content.
 * @details 8 threads each push 1000 items.  After all join, pop
 *          and verify that all items are present exactly once.
 */
TEST_F(TreiberStackTest, ConcurrentPushOnly_SizeCorrect)
{
    constexpr int ITEMS_PER_THREAD = 1000;
    constexpr int TOTAL = CONCUR_NUM_THREADS * ITEMS_PER_THREAD;

    TreiberStack<int>     stack;
    SpinBarrier            barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            const int start = t * ITEMS_PER_THREAD;

            barrier.arrive_and_wait();

            for (int i = 0; i < ITEMS_PER_THREAD; ++i)
            {
                stack.push(start + i);
            }
        });
    }

    barrier.release();

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(stack.size(), static_cast<std::size_t>(TOTAL));

    // Pop all items and verify presence.
    std::vector<int> seen(TOTAL, 0);
    for (int i = 0; i < TOTAL; ++i)
    {
        const auto val = stack.try_pop();
        if (!val.has_value())
        {
            break;  // Stack empty before all items drained — will fail below.
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
//  11. Mixed push/pop workload
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concurrent mixed push/pop — no crash or data corruption.
 * @details Each thread repeatedly pushes or pops from a shared stack
 *          for a fixed duration (~2 s).  After all threads stop, the total
 *          produced vs consumed counts are validated (may not be exact due
 *          to items still in the stack, but the stack state must be
 *          consistent).
 */
TEST_F(TreiberStackTest, MixedPushPop_NoCorruption)
{
    constexpr auto     TEST_DURATION = std::chrono::seconds(2);

    TreiberStack<int>    stack;
    std::atomic<bool>    stop{false};
    std::atomic<int>     ops_done{0};

    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<unsigned int>(t) * 5555 + 3333); // NOLINT

            while (!stop.load(std::memory_order_relaxed))
            {
                const int op = static_cast<int>(rng() % 2);

                if (op == 0)
                {
                    // Push.
                    stack.push(static_cast<int>(rng() % 10000));
                }
                else
                {
                    // Try pop.
                    std::ignore = stack.try_pop();
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

    // Stack must be in a valid state (no crash, size is consistent).
    const std::size_t s = stack.size();
    // size() is approximate, but it should be a non-negative value consistent
    // with internal invariants (no wraparound, no corruption).
    EXPECT_LE(s, static_cast<std::size_t>(ops_done.load() / 2 + CONCUR_NUM_THREADS));

    // Drain remaining items — must not crash.
    int drain_count = 0;
    while (stack.try_pop().has_value())
    {
        ++drain_count;
    }
    // Drain count plus final size should be 0 (all drained).
    EXPECT_EQ(stack.size(), 0);
    EXPECT_TRUE(stack.empty());

    // Total operations should approximately balance (allow slack).
    // We only validate that drain succeeded without crashing.
    EXPECT_GE(drain_count, 0);
}
