/**
 * @file GrpcConnectivityManager.hpp
 * @brief GrpcConnectivityManager class declaration
 * @details This header defines the GrpcConnectivityManager class that provides functionality for RPC metadata and connectivity management.
 */

#pragma once

#include <grpcpp/grpcpp.h>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

#include "GrpcConnectivityState.hpp"
#include "RpcMetadata.hpp"

namespace common::rpc
{
    /// @brief Manages gRPC channel connectivity state and provides monitoring capabilities
    class GrpcConnectivityManager
    {
    public:
        /// @brief Callback type for state change notifications
        using StateChangeCallback = std::function<void(GrpcConnectivityState)>;

        /// @brief Constructor
        /// @param channel The gRPC channel to monitor
        explicit GrpcConnectivityManager(const std::shared_ptr<grpc::Channel>& channel)  : channel_(channel), is_monitoring_(false)
        {
        }

        /// @brief Destructor - stops monitoring if running
        ~GrpcConnectivityManager()
        {
            stopMonitoring();
        }

        /// @brief Get the current connectivity state of the channel
        /// @return Current GrpcConnectivityState
        [[nodiscard]] GrpcConnectivityState getCurrentState() const
        {
            if (!channel_)
            {
                return GrpcConnectivityState::SHUTDOWN;
            }

            const grpc_connectivity_state raw_state = channel_->GetState(false);
            return RpcMetadata::grpcStateToEnum(raw_state);
        }

        /// @brief Get the current connectivity state as string
        /// @return String representation of current state
        [[nodiscard]] std::string getCurrentStateString() const
        {
            const auto state = getCurrentState();
            return RpcMetadata::grpcStateToString(state);
        }

        /// @brief Start monitoring the channel's connectivity state
        /// @param callback Called when state changes
        /// @param poll_interval_ms Interval in milliseconds to check for state changes
        void startMonitoring(StateChangeCallback callback, int poll_interval_ms = 1000)
        {
            if (is_monitoring_.exchange(true))
            {
                return; // Already monitoring
            }

            callback_ = std::move(callback);
            last_known_state_ = getCurrentState();

            monitor_thread_ = std::thread([this, poll_interval_ms]()
            {
                while (is_monitoring_.load())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));

                    const auto current_state = getCurrentState();
                    if (current_state != last_known_state_)
                    {
                        last_known_state_ = current_state;

                        if (callback_)
                        {
                            callback_(current_state);
                        }
                    }
                }
            });
        }

        /// @brief Stop monitoring the channel's connectivity state
        void stopMonitoring()
        {
            if (!is_monitoring_.exchange(false))
            {
                return; // Not monitoring
            }

            if (monitor_thread_.joinable())
            {
                monitor_thread_.join();
            }
        }

        /// @brief Wait for a specific connectivity state with timeout
        /// @param target_state The state to wait for
        /// @param timeout_seconds Maximum time to wait in seconds
        /// @return True if state was reached within timeout, false otherwise
        [[nodiscard]] bool waitForState(GrpcConnectivityState target_state, int timeout_seconds = 10) const
        {
            const auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_seconds);

            while (std::chrono::system_clock::now() < deadline)
            {
                if (getCurrentState() == target_state)
                {
                    return true;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            return false;
        }

        /// @brief Check if the channel is ready for RPC calls
        /// @return True if channel is in READY state
        [[nodiscard]] bool isReady() const
        {
            return getCurrentState() == GrpcConnectivityState::READY;
        }

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