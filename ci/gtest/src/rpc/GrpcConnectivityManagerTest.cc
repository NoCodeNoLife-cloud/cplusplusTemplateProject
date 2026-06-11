/**
 * @file GrpcConnectivityManagerTest.cc
 * @brief Unit tests for the GrpcConnectivityManager class
 * @details Tests cover construction with null and valid channels, state queries,
 *          monitoring lifecycle, and timeout behavior.
 */

#include <atomic>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "rpc/GrpcConnectivityManager.hpp"

using namespace common::rpc;

/**
 * @brief Test fixture for GrpcConnectivityManagerTest tests
 */
class GrpcConnectivityManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test construction with null channel
 * @details Verifies that a null channel is handled gracefully
 */
TEST_F(GrpcConnectivityManagerTest, ConstructWithNullChannel)
{
    EXPECT_NO_THROW(GrpcConnectivityManager manager(nullptr));
}

/**
 * @brief Test construction with a valid channel (unconnected)
 * @details Verifies that a valid channel can be managed
 */
TEST_F(GrpcConnectivityManagerTest, ConstructWithValidChannel)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    EXPECT_NO_THROW(GrpcConnectivityManager manager(channel));
}

/**
 * @brief Test getCurrentState with null channel returns SHUTDOWN
 * @details A null channel should report SHUTDOWN state
 */
TEST_F(GrpcConnectivityManagerTest, GetCurrentState_NullChannel)
{
    const GrpcConnectivityManager manager(nullptr);
    EXPECT_EQ(manager.getCurrentState(), GrpcConnectivityState::SHUTDOWN);
}

/**
 * @brief Test getCurrentStateString with null channel
 * @details A null channel should report "SHUTDOWN"
 */
TEST_F(GrpcConnectivityManagerTest, GetCurrentStateString_NullChannel)
{
    const GrpcConnectivityManager manager(nullptr);
    EXPECT_EQ(manager.getCurrentStateString(), "SHUTDOWN");
}

/**
 * @brief Test isReady with null channel returns false
 * @details A null channel is never ready
 */
TEST_F(GrpcConnectivityManagerTest, IsReady_NullChannel)
{
    const GrpcConnectivityManager manager(nullptr);
    EXPECT_FALSE(manager.isReady());
}

/**
 * @brief Test isReady with non-null channel returns false (no server)
 * @details An unconnected channel is not ready
 */
TEST_F(GrpcConnectivityManagerTest, IsReady_UnconnectedChannel)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    const GrpcConnectivityManager manager(channel);
    EXPECT_FALSE(manager.isReady());
}

/**
 * @brief Test getCurrentState with channel returns non-SHUTDOWN state
 * @details A valid channel should return some connectivity state
 */
TEST_F(GrpcConnectivityManagerTest, GetCurrentState_ValidChannel)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    const GrpcConnectivityManager manager(channel);
    const auto state = manager.getCurrentState();
    EXPECT_NE(state, GrpcConnectivityState::SHUTDOWN);
}

/**
 * @brief Test getCurrentStateString with valid channel
 * @details Non-null channel should return a non-empty state string
 */
TEST_F(GrpcConnectivityManagerTest, GetCurrentStateString_ValidChannel)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    const GrpcConnectivityManager manager(channel);
    const auto stateStr = manager.getCurrentStateString();
    EXPECT_FALSE(stateStr.empty());
    EXPECT_NE(stateStr, "SHUTDOWN");
}

/**
 * @brief Test waitForState with unreachable state times out
 * @details Waiting for READY on an unreachable server should time out
 */
TEST_F(GrpcConnectivityManagerTest, WaitForState_TimesOut)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    const GrpcConnectivityManager manager(channel);
    const bool result = manager.waitForState(GrpcConnectivityState::SHUTDOWN, 1);
    EXPECT_FALSE(result);
}

/**
 * @brief Test waitForState with current state returns immediately
 * @details Waiting for the current state should succeed immediately
 */
TEST_F(GrpcConnectivityManagerTest, WaitForState_CurrentState)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    const GrpcConnectivityManager manager(channel);
    const auto currentState = manager.getCurrentState();
    const bool result = manager.waitForState(currentState, 5);
    EXPECT_TRUE(result);
}

/**
 * @brief Test start and stop monitoring lifecycle
 * @details Verifies that monitoring can be started and stopped
 */
TEST_F(GrpcConnectivityManagerTest, StartStopMonitoring)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    GrpcConnectivityManager manager(channel);

    std::atomic<int> callbackCount{0};

    manager.startMonitoring([&callbackCount](GrpcConnectivityState)
    {
        ++callbackCount;
    }, 100);

    // Let monitoring run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    manager.stopMonitoring();

    // Should have stopped without crash
    SUCCEED();
}

/**
 * @brief Test double start monitoring is idempotent
 * @details Starting monitoring twice should not crash
 */
TEST_F(GrpcConnectivityManagerTest, DoubleStartMonitoring)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    GrpcConnectivityManager manager(channel);

    manager.startMonitoring([](GrpcConnectivityState) {}, 500);
    // Second start should be ignored (already monitoring)
    manager.startMonitoring([](GrpcConnectivityState) {}, 500);
    manager.stopMonitoring();

    SUCCEED();
}

/**
 * @brief Test double stop monitoring is idempotent
 * @details Stopping monitoring twice should not crash
 */
TEST_F(GrpcConnectivityManagerTest, DoubleStopMonitoring)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    GrpcConnectivityManager manager(channel);

    manager.startMonitoring([](GrpcConnectivityState) {}, 500);
    manager.stopMonitoring();
    // Second stop should be ignored (not monitoring)
    manager.stopMonitoring();

    SUCCEED();
}

/**
 * @brief Test destructor while monitoring stops cleanly
 * @details Manager destruction should stop monitoring automatically
 */
TEST_F(GrpcConnectivityManagerTest, DestructorWhileMonitoring)
{
    const auto channel = grpc::CreateChannel("localhost:1", grpc::InsecureChannelCredentials());
    {
        GrpcConnectivityManager manager(channel);
        manager.startMonitoring([](GrpcConnectivityState) {}, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } // Destructor called here - should stop monitoring
    SUCCEED();
}
