/**
 * @file GrpcConnectivityManager.cc
 * @brief GrpcConnectivityManager implementation â€?channel watch, health ping
 * @details Implements gRPC channel connectivity: watches state changes via
 *          grpc::Channel::GetState, runs health pings on idle channels, and
 *          manages reconnection with backoff timers.
 */

#include "GrpcConnectivityManager.hpp"

#include <chrono>
#include <thread>
#include <utility>

namespace cppforge::rpc
{
    GrpcConnectivityManager::GrpcConnectivityManager(const std::shared_ptr<grpc::Channel>& channel)
        : channel_(channel), is_monitoring_(false), last_known_state_(GrpcConnectivityState::IDLE)
    {
    }

    GrpcConnectivityManager::~GrpcConnectivityManager()
    {
        stopMonitoring();
    }

    GrpcConnectivityState GrpcConnectivityManager::getCurrentState() const
    {
        if (!channel_)
        {
            return GrpcConnectivityState::SHUTDOWN;
        }

        const grpc_connectivity_state raw_state = channel_->GetState(false);
        return RpcMetadata::grpcStateToEnum(raw_state);
    }

    std::string GrpcConnectivityManager::getCurrentStateString() const
    {
        const auto state = getCurrentState();
        return RpcMetadata::grpcStateToString(state);
    }

    void GrpcConnectivityManager::startMonitoring(StateChangeCallback callback, int poll_interval_ms)
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

    void GrpcConnectivityManager::stopMonitoring()
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

    bool GrpcConnectivityManager::waitForState(GrpcConnectivityState target_state, int timeout_seconds) const
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

    bool GrpcConnectivityManager::isReady() const
    {
        return getCurrentState() == GrpcConnectivityState::READY;
    }
} // namespace cppforge::rpc
