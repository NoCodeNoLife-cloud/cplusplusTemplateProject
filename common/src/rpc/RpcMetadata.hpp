/**
 * @file RpcMetadata.hpp
 * @brief RpcMetadata class declaration
 * @details This header defines the RpcMetadata class that provides functionality for RPC metadata and connectivity management.
 */

#pragma once
#include <string>
#include <grpc/impl/connectivity_state.h>
#include "GrpcConnectivityState.hpp"

namespace common::rpc
{
    /// @brief Utility class for RPC metadata operations
    class RpcMetadata
    {
    public:
        /// @brief Converts gRPC connectivity state to human-readable string representation
        /// @param[in] state The gRPC connectivity state to convert
        /// @return String representation of the connectivity state
        [[nodiscard]] static std::string grpcStateToString(grpc_connectivity_state state);

        /// @brief Converts gRPC connectivity state to internal GrpcConnectivityState enum
        /// @param[in] state The gRPC connectivity state to convert
        /// @return GrpcConnectivityState enum value
        [[nodiscard]] static GrpcConnectivityState grpcStateToEnum(grpc_connectivity_state state);

        /// @brief Converts internal GrpcConnectivityState enum to string representation
        /// @param[in] state The GrpcConnectivityState enum to convert
        /// @return String representation of the connectivity state
        [[nodiscard]] static std::string grpcStateToString(GrpcConnectivityState state);
    };
}