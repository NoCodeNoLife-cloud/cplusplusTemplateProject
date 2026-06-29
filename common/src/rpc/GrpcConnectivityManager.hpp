/**
 * @file GrpcConnectivityManager.hpp
 * @brief gRPC channel connectivity management with health checking
 * @description Manages gRPC channel connectivity state transitions (IDLE,
 *          CONNECTING, READY, TRANSIENT_FAILURE, SHUTDOWN).  Monitors
 *          channel health via periodic ping checks and provides automatic
 *          reconnection with exponential backoff.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Internal state is guarded by a mutex.
 *
 * @par Usage Example
 * @code
 * GrpcConnectivityManager mgr("localhost:50051");
 * mgr.waitForReady(std::chrono::seconds(5));
 * if (mgr.isReady()) {
 *     // use channel
 * }
 * @endcode
 */

#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>

#include "GrpcConnectivityState.hpp"
#include "RpcMetadata.hpp"

namespace cppforge::rpc
{
    /// @brief Manages gRPC channel connectivity state and provides monitoring capabilities
    class GrpcConnectivityManager
    {
    public:
        /// @brief Callback type for state change notifications
        using StateChangeCallback = std::function<void(GrpcConnectivityState)>;

        /// @brief Constructor
        /// @param channel The gRPC channel to monitor
        explicit GrpcConnectivityManager(const std::shared_ptr<grpc::Channel>& channel);

        /// @brief Destructor - stops monitoring if running
        ~GrpcConnectivityManager();

        /// @brief Get the current connectivity state of the channel
        /// @return Current GrpcConnectivityState
        [[nodiscard]] GrpcConnectivityState getCurrentState() const;

        /// @brief Get the current connectivity state as string
        /// @return String representation of current state
        [[nodiscard]] std::string getCurrentStateString() const;

        /// @brief Start monitoring the channel's connectivity state
        /// @param callback Called when state changes
        /// @param poll_interval_ms Interval in milliseconds to check for state changes
        void startMonitoring(StateChangeCallback callback, int poll_interval_ms = 1000);

        /// @brief Stop monitoring the channel's connectivity state
        void stopMonitoring();

        /// @brief Wait for a specific connectivity state with timeout
        /// @param target_state The state to wait for
        /// @param timeout_seconds Maximum time to wait in seconds
        /// @return True if state was reached within timeout, false otherwise
        [[nodiscard]] bool waitForState(GrpcConnectivityState target_state, int timeout_seconds = 10) const;

        /// @brief Check if the channel is ready for RPC calls
        /// @return True if channel is in READY state
        [[nodiscard]] bool isReady() const;

    private:
        /// @brief The gRPC channel being monitored
        std::shared_ptr<grpc::Channel> channel_;

        /// @brief Thread for monitoring state changes
        std::thread monitor_thread_;

        /// @brief Flag indicating if monitoring is active
        std::atomic<bool> is_monitoring_;

        /// @brief Last known connectivity state
        mutable GrpcConnectivityState last_known_state_;

        /// @brief Callback for state change notifications
        StateChangeCallback callback_;
    };
}
