/**
 * @file EventBusTest.cc
 * @brief Unit tests for the EventBus publish/subscribe subsystem.
 * @details Tests cover single/multiple subscribers, event filtering, no-subscriber safety, and thread safety.
 */

#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>

#include <cppforge/observability/events/EventBus.hpp>

using namespace cppforge::observability::events;

/// @brief Test fixture for EventBus tests.
class EventBusTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}

    EventBus bus_;
};

/**
 * @brief Test single subscriber receives published event.
 */
TEST_F(EventBusTest, SingleSubscriberReceivesEvent)
{
    std::string received;
    bus_.subscribe("test.event", [&](const std::string &payload) { received = payload; });

    bus_.publish("test.event", "hello");

    EXPECT_EQ(received, "hello");
}

/**
 * @brief Test multiple subscribers all receive the event.
 */
TEST_F(EventBusTest, MultipleSubscribersAllReceiveEvent)
{
    std::string received1;
    std::string received2;
    std::string received3;

    bus_.subscribe("broadcast", [&](const std::string &payload) { received1 = payload; });
    bus_.subscribe("broadcast", [&](const std::string &payload) { received2 = payload; });
    bus_.subscribe("broadcast", [&](const std::string &payload) { received3 = payload; });

    bus_.publish("broadcast", "data");

    EXPECT_EQ(received1, "data");
    EXPECT_EQ(received2, "data");
    EXPECT_EQ(received3, "data");
}

/**
 * @brief Test subscriber only receives events it subscribed to.
 */
TEST_F(EventBusTest, SubscriberOnlyReceivesSubscribedEvent)
{
    std::string receivedA;
    std::string receivedB;

    bus_.subscribe("eventA", [&](const std::string &payload) { receivedA = payload; });
    bus_.subscribe("eventB", [&](const std::string &payload) { receivedB = payload; });

    bus_.publish("eventA", "payloadA");

    EXPECT_EQ(receivedA, "payloadA");
    EXPECT_EQ(receivedB, "");
}

/**
 * @brief Test publishing event with no subscribers does not crash.
 */
TEST_F(EventBusTest, PublishWithNoSubscribersDoesNotCrash)
{
    EXPECT_NO_THROW(bus_.publish("no.subscribers", "orphan"));
}

/**
 * @brief Test multiple publishes are all delivered.
 */
TEST_F(EventBusTest, MultiplePublishesDelivered)
{
    std::vector<std::string> received;
    bus_.subscribe("multi", [&](const std::string &payload) { received.push_back(payload); });

    bus_.publish("multi", "first");
    bus_.publish("multi", "second");
    bus_.publish("multi", "third");

    ASSERT_EQ(received.size(), 3u);
    EXPECT_EQ(received[0], "first");
    EXPECT_EQ(received[1], "second");
    EXPECT_EQ(received[2], "third");
}

/**
 * @brief Test thread safety - publish from multiple threads concurrently.
 */
TEST_F(EventBusTest, ThreadSafetyConcurrentPublish)
{
    std::atomic<int> callCount{0};
    bus_.subscribe("concurrent", [&](const std::string & /*payload*/) { callCount.fetch_add(1, std::memory_order_relaxed); });

    constexpr int kThreadCount = 8;
    constexpr int kPublishPerThread = 100;

    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (int t = 0; t < kThreadCount; ++t)
    {
        threads.emplace_back([this, kPublishPerThread]() {
            for (int i = 0; i < kPublishPerThread; ++i)
            {
                bus_.publish("concurrent", "payload");
            }
        });
    }

    for (auto &th : threads)
    {
        th.join();
    }

    EXPECT_EQ(callCount.load(), kThreadCount * kPublishPerThread);
}

/**
 * @brief Test thread safety - subscribe and publish concurrently.
 */
TEST_F(EventBusTest, ThreadSafetyConcurrentSubscribeAndPublish)
{
    std::atomic<int> callCount{0};
    constexpr int kSubscriberCount = 4;
    constexpr int kPublishCount = 50;

    std::vector<std::thread> threads;

    // Subscriber threads
    for (int t = 0; t < kSubscriberCount; ++t)
    {
        threads.emplace_back([this, &callCount]() {
            bus_.subscribe("mixed", [&](const std::string & /*payload*/) { callCount.fetch_add(1, std::memory_order_relaxed); });
        });
    }

    // Wait for subscribers to register
    for (auto &th : threads)
    {
        th.join();
    }
    threads.clear();

    // Publisher threads
    for (int t = 0; t < kSubscriberCount; ++t)
    {
        threads.emplace_back([this]() {
            for (int i = 0; i < kPublishCount; ++i)
            {
                bus_.publish("mixed", "data");
            }
        });
    }

    for (auto &th : threads)
    {
        th.join();
    }

    EXPECT_EQ(callCount.load(), kSubscriberCount * kPublishCount * kSubscriberCount);
}
