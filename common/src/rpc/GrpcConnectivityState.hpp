/**
 * @file GrpcConnectivityState.hpp
 * @brief gRPC connectivity state enum and string conversion
 * @description Defines the gRPC channel connectivity states: IDLE, CONNECTING,
 *          READY, TRANSIENT_FAILURE, SHUTDOWN.  Provides enum-to-string
 *          conversion for logging and diagnostic output.
 *
 * @par Thread Safety
 * This enum is const-only thread-safe (immutable).
 */

#pragma once

namespace common::rpc
{
    /// @brief Enumeration representing gRPC channel connectivity states
    /// @details This enum maps to the underlying gRPC connectivity state machine,
    ///          indicating the current status of a gRPC channel connection.
    enum class GrpcConnectivityState
    {
        IDLE,
        ///< Channel is idle, no active calls and not attempting to connect
        CONNECTING,
        ///< Channel is attempting to establish a connection
        READY,
        ///< Channel is connected and ready for RPC calls
        TRANSIENT_FAILURE,
        ///< Channel encountered a temporary failure, will retry
        SHUTDOWN ///< Channel has been shut down and cannot be used
    };
}