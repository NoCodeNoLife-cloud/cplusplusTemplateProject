/**
 * @file ConcurrentQueueTest.cc
 * @brief Unit tests for the ConcurrentQueue class
 * @details Tests cover single-threaded basic operations, blocking/non-blocking
 *          push/pop semantics, bound enforcement, and multithreaded
 *          producer-consumer scenarios.
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>

#include "data_structure/queue/ConcurrentQueue.hpp"

using namespace cppforge::data_structure;

/**
 * @brief Test fixture for ConcurrentQueue tests
 */
class ConcurrentQueueTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ============================================================================
// Construction Tests
// ============================================================================

/**
 * @brief Test default constructor creates unbounded queue
 * @details Verifies that default queue uses maximum capacity
 */
TEST_F(ConcurrentQueueTest, DefaultConstructor_Unbounded)
{
    const ConcurrentQueue<int> queue;
    EXPECT_EQ(queue.capacity(), std::numeric_limits<ConcurrentQueue<int>::size_type>::max());
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test constructor with explicit capacity
 * @details Verifies that bounded queue reports correct capacity
 */
TEST_F(ConcurrentQueueTest, BoundedConstructor_CorrectCapacity)
{
    const ConcurrentQueue<int> queue(10);
    EXPECT_EQ(queue.capacity(), 10);
}

// ============================================================================
// Basic Blocking Operations
// ============================================================================

/**
 * @brief Test blocking push and pop with single element
 * @details Verifies basic push/pop cycle works correctly
 */
TEST_F(ConcurrentQueueTest, PushPop_SingleElement)
{
    ConcurrentQueue<int> queue;
    queue.push(42);
    EXPECT_EQ(queue.size(), 1);
    EXPECT_FALSE(queue.empty());

    const int value = queue.pop();
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test blocking push and pop with multiple elements
 * @details Verifies FIFO ordering is maintained
 */
TEST_F(ConcurrentQueueTest, PushPop_MultipleElements)
{
    ConcurrentQueue<int> queue;
    for (int i = 0; i < 100; ++i)
    {
        queue.push(i);
    }
    EXPECT_EQ(queue.size(), 100);

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(queue.pop(), i);
    }
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test blocking push with move semantics
 * @details Verifies that rvalue push moves element correctly
 */
TEST_F(ConcurrentQueueTest, PushMove_MovesElement)
{
    ConcurrentQueue<std::unique_ptr<int>> queue;
    auto ptr = std::make_unique<int>(42);
    queue.push(std::move(ptr));
    EXPECT_EQ(ptr, nullptr);

    auto result = queue.pop();
    EXPECT_EQ(*result, 42);
}

/**
 * @brief Test blocking emplace constructs element in-place
 * @details Verifies argument forwarding to T's constructor
 */
TEST_F(ConcurrentQueueTest, Emplace_ConstructsInPlace)
{
    ConcurrentQueue<std::pair<int, int>> queue;
    queue.emplace(1, 2);
    EXPECT_EQ(queue.size(), 1);

    const auto result = queue.pop();
    EXPECT_EQ(result.first, 1);
    EXPECT_EQ(result.second, 2);
}

// ============================================================================
// Non-blocking Operations
// ============================================================================

/**
 * @brief Test try_pop on empty queue returns false
 * @details Verifies non-blocking pop does not block on empty queue
 */
TEST_F(ConcurrentQueueTest, TryPop_EmptyQueue_ReturnsFalse)
{
    ConcurrentQueue<int> queue;
    int value = -1;
    EXPECT_FALSE(queue.try_pop(value));
    EXPECT_EQ(value, -1);
}

/**
 * @brief Test try_pop on non-empty queue succeeds
 * @details Verifies non-blocking pop retrieves element correctly
 */
TEST_F(ConcurrentQueueTest, TryPop_NonEmpty_ReturnsTrue)
{
    ConcurrentQueue<int> queue;
    queue.push(42);

    int value = 0;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test try_push on bounded full queue returns false
 * @details Verifies non-blocking push rejects when queue is at capacity
 */
TEST_F(ConcurrentQueueTest, TryPush_FullQueue_ReturnsFalse)
{
    ConcurrentQueue<int> queue(2);
    EXPECT_TRUE(queue.try_push(1));
    EXPECT_TRUE(queue.try_push(2));
    EXPECT_FALSE(queue.try_push(3));
    EXPECT_EQ(queue.size(), 2);
}

/**
 * @brief Test try_push with move semantics on full queue
 * @details Verifies move variant also rejects correctly
 */
TEST_F(ConcurrentQueueTest, TryPushMove_FullQueue_ReturnsFalse)
{
    ConcurrentQueue<std::unique_ptr<int>> queue(1);
    auto ptr1 = std::make_unique<int>(1);
    EXPECT_TRUE(queue.try_push(std::move(ptr1)));
    EXPECT_EQ(ptr1, nullptr);

    auto ptr2 = std::make_unique<int>(2);
    EXPECT_FALSE(queue.try_push(std::move(ptr2)));
    EXPECT_NE(ptr2, nullptr); // not moved because push failed
}

/**
 * @brief Test try_emplace on bounded full queue returns false
 * @details Verifies non-blocking emplace rejects when queue is full
 */
TEST_F(ConcurrentQueueTest, TryEmplace_FullQueue_ReturnsFalse)
{
    ConcurrentQueue<std::pair<int, int>> queue(1);
    EXPECT_TRUE(queue.try_emplace(1, 2));
    EXPECT_FALSE(queue.try_emplace(3, 4));
    EXPECT_EQ(queue.size(), 1);
}

/**
 * @brief Test try_pop_for with timeout on empty queue
 * @details Verifies timeout behavior when no element is available
 */
TEST_F(ConcurrentQueueTest, TryPopFor_Timeout_ReturnsFalse)
{
    ConcurrentQueue<int> queue;
    int value = -1;
    const bool result = queue.try_pop_for(value, std::chrono::milliseconds(10));
    EXPECT_FALSE(result);
    EXPECT_EQ(value, -1);
}

/**
 * @brief Test try_pop_for with available element
 * @details Verifies successful timed pop when element is present
 */
TEST_F(ConcurrentQueueTest, TryPopFor_ElementAvailable_ReturnsTrue)
{
    ConcurrentQueue<int> queue;
    queue.push(42);

    int value = 0;
    const bool result = queue.try_pop_for(value, std::chrono::milliseconds(100));
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 42);
}

// ============================================================================
// Capacity and Status
// ============================================================================

/**
 * @brief Test size returns correct count
 * @details Verifies accurate element counting
 */
TEST_F(ConcurrentQueueTest, Size_CorrectCount)
{
    ConcurrentQueue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(10);
    EXPECT_EQ(queue.size(), 1);

    queue.push(20);
    EXPECT_EQ(queue.size(), 2);

    queue.pop();
    EXPECT_EQ(queue.size(), 1);

    queue.pop();
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test empty returns correct state
 * @details Verifies empty detection throughout lifecycle
 */
TEST_F(ConcurrentQueueTest, Empty_CorrectState)
{
    ConcurrentQueue<int> queue;
    EXPECT_TRUE(queue.empty());

    queue.push(10);
    EXPECT_FALSE(queue.empty());

    queue.pop();
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test clear removes all elements
 * @details Verifies clear empties the queue
 */
TEST_F(ConcurrentQueueTest, Clear_RemovesAllElements)
{
    ConcurrentQueue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);
    EXPECT_EQ(queue.size(), 3);

    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

/**
 * @brief Test clear on empty queue is safe
 * @details Verifies no-op behavior on already empty queue
 */
TEST_F(ConcurrentQueueTest, Clear_EmptyQueue_Safe)
{
    ConcurrentQueue<int> queue;
    queue.clear();
    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// String Type Tests
// ============================================================================

/**
 * @brief Test queue with string elements
 * @details Verifies correct behavior with non-trivial types
 */
TEST_F(ConcurrentQueueTest, StringElements_CorrectBehavior)
{
    ConcurrentQueue<std::string> queue;
    queue.push("hello");
    queue.push("world");

    EXPECT_EQ(queue.pop(), "hello");
    EXPECT_EQ(queue.pop(), "world");
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test queue with move-only string elements
 * @details Verifies move semantics with string values
 */
TEST_F(ConcurrentQueueTest, StringElements_MoveSemantics)
{
    ConcurrentQueue<std::string> queue;
    queue.push(std::string("test"));

    EXPECT_EQ(queue.pop(), "test");
}

// ============================================================================
// Multi-threaded Tests
// ============================================================================

/**
 * @brief Test single producer, single consumer
 * @details Verifies basic producer-consumer pattern works correctly
 */
TEST_F(ConcurrentQueueTest, SingleProducerSingleConsumer)
{
    ConcurrentQueue<int> queue;
    constexpr int kElementCount = 1000;
    std::atomic<int> sum{0};

    std::thread producer([&queue]
    {
        for (int i = 0; i < kElementCount; ++i)
        {
            queue.push(i);
        }
    });

    std::thread consumer([&queue, &sum]
    {
        for (int i = 0; i < kElementCount; ++i)
        {
            sum += queue.pop();
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(sum.load(), kElementCount * (kElementCount - 1) / 2);
}

/**
 * @brief Test multiple producers, multiple consumers
 * @details Verifies concurrent queue handles MPMC correctly
 */
TEST_F(ConcurrentQueueTest, MultipleProducersMultipleConsumers)
{
    ConcurrentQueue<int> queue(64);
    constexpr int kElementsPerProducer = 500;
    constexpr int kProducerCount = 4;
    constexpr int kConsumerCount = 4;
    constexpr int kTotalElements = kElementsPerProducer * kProducerCount;

    std::atomic<int> total_sum{0};

    std::vector<std::thread> producers;
    for (int p = 0; p < kProducerCount; ++p)
    {
        producers.emplace_back([&queue, p]
        {
            for (int i = 0; i < kElementsPerProducer; ++i)
            {
                queue.push(p * kElementsPerProducer + i);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < kConsumerCount; ++c)
    {
        consumers.emplace_back([&queue, &total_sum]
        {
            for (int i = 0; i < kTotalElements / kConsumerCount; ++i)
            {
                total_sum += queue.pop();
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    const long long expected_sum = static_cast<long long>(kTotalElements) * (kTotalElements - 1) / 2;
    EXPECT_EQ(total_sum.load(), expected_sum);
}

/**
 * @brief Test blocking behavior on bounded queue
 * @details Verifies that push blocks when queue is full and unblocks after pop
 */
TEST_F(ConcurrentQueueTest, BlockingPush_WaitsUntilSpaceAvailable)
{
    ConcurrentQueue<int> queue(2);
    std::atomic<bool> producer_done{false};

    // Fill the queue
    queue.push(1);
    queue.push(2);

    // Start a producer that will block (queue is full)
    std::thread producer([&queue, &producer_done]
    {
        queue.push(3); // should block until space is available
        producer_done = true;
    });

    // Give producer time to block
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(producer_done);

    // Consume one element, unblocking the producer
    EXPECT_EQ(queue.pop(), 1);

    // Wait for producer to finish
    producer.join();
    EXPECT_TRUE(producer_done);
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
}

/**
 * @brief Test blocking pop waits on empty queue
 * @details Verifies that pop blocks until element is pushed
 */
TEST_F(ConcurrentQueueTest, BlockingPop_WaitsUntilElementAvailable)
{
    ConcurrentQueue<int> queue(2);
    std::atomic<int> received{-1};

    // Start a consumer that will block (queue is empty)
    std::thread consumer([&queue, &received]
    {
        received = queue.pop();
    });

    // Give consumer time to block
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(received.load(), -1);

    // Push an element to unblock the consumer
    queue.push(42);

    consumer.join();
    EXPECT_EQ(received.load(), 42);
}

/**
 * @brief Test try_pop_for with actual blocking then timeout
 * @details Verifies timed pop correctly times out
 */
TEST_F(ConcurrentQueueTest, TryPopFor_ActualTimeout)
{
    ConcurrentQueue<int> queue;
    int value = -1;

    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.try_pop_for(value, std::chrono::milliseconds(30));
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_FALSE(result);
    EXPECT_EQ(value, -1);
    EXPECT_GE(elapsed.count(), 20); // should have waited at least ~20ms
}

/**
 * @brief Test try_pop_for wakes up when element becomes available
 * @details Verifies timed pop can be satisfied before timeout
 */
TEST_F(ConcurrentQueueTest, TryPopFor_WakesOnElementAvailable)
{
    ConcurrentQueue<int> queue;

    std::thread producer([&queue]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(99);
    });

    int value = 0;
    const bool result = queue.try_pop_for(value, std::chrono::seconds(5));
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 99);

    producer.join();
}

/**
 * @brief Test concurrent clear while producers are waiting
 * @details Verifies clear wakes blocked producers
 */
TEST_F(ConcurrentQueueTest, Clear_WakesWaitingProducers)
{
    ConcurrentQueue<int> queue(2);
    std::atomic<bool> producer_woke{false};

    // Fill the queue
    queue.push(1);
    queue.push(2);

    // Start a producer that will block
    std::thread producer([&queue, &producer_woke]
    {
        queue.push(3);
        producer_woke = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Clear should wake the blocked producer, allowing push(3) to complete
    queue.clear();

    producer.join();
    EXPECT_TRUE(producer_woke);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.pop(), 3);
}

/**
 * @brief Test concurrent stress with many threads
 * @details Verifies thread safety under high concurrency
 */
TEST_F(ConcurrentQueueTest, ConcurrentStress_HighContention)
{
    ConcurrentQueue<int> queue(16);
    constexpr int kIterations = 200;
    constexpr int kThreadPairs = 4;

    std::atomic<int> total_produced{0};
    std::atomic<int> total_consumed{0};
    std::atomic<bool> done{false};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int i = 0; i < kThreadPairs; ++i)
    {
        producers.emplace_back([&queue, &total_produced]
        {
            for (int j = 0; j < kIterations; ++j)
            {
                queue.push(1);
                total_produced.fetch_add(1, std::memory_order_relaxed);
            }
        });

        consumers.emplace_back([&queue, &total_consumed, &done]
        {
            int value = 0;
            while (true)
            {
                if (queue.try_pop_for(value, std::chrono::milliseconds(50)))
                {
                    total_consumed.fetch_add(value, std::memory_order_relaxed);
                }
                else if (done.load(std::memory_order_acquire))
                {
                    break;
                }
            }
            // Drain remaining
            while (queue.try_pop(value))
            {
                total_consumed.fetch_add(value, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) t.join();
    done.store(true, std::memory_order_release);

    for (auto& t : consumers) t.join();

    EXPECT_EQ(total_produced.load(), total_consumed.load());
}

/**
 * @brief Test try_pop_for with timeout on concurrent queue
 * @details Verifies that timeout returns false correctly under concurrent access
 */
TEST_F(ConcurrentQueueTest, TryPopFor_ConcurrentTimeout)
{
    ConcurrentQueue<int> queue(8);

    // Fill then drain to ensure empty
    queue.push(1);
    queue.pop();

    int value = -1;
    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.try_pop_for(value, std::chrono::milliseconds(20));
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed.count(), 15);
}

/**
 * @brief Test try_push and try_pop cycle on bounded queue
 * @details Verifies non-blocking operations work correctly with bounded capacity
 */
TEST_F(ConcurrentQueueTest, TryPushTryPop_CycleOnBoundedQueue)
{
    ConcurrentQueue<int> queue(3);
    std::vector<int> results;

    // Fill the queue
    EXPECT_TRUE(queue.try_push(10));
    EXPECT_TRUE(queue.try_push(20));
    EXPECT_TRUE(queue.try_push(30));
    EXPECT_FALSE(queue.try_push(40));

    // Drain the queue
    int value = 0;
    while (queue.try_pop(value))
    {
        results.push_back(value);
    }

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 10);
    EXPECT_EQ(results[1], 20);
    EXPECT_EQ(results[2], 30);
}
