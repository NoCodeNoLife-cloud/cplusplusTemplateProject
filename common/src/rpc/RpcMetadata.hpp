/**
 * @file RpcMetadata.hpp
 * @brief RPC metadata key-value container for gRPC custom headers
 * @description Stores and manages RPC metadata as key-value pairs (similar to
 *          gRPC's metadata facility).  Supports iteration, lookup, and
 *          serialisation.  Used for passing custom headers (auth tokens,
 *          tracing IDs) in gRPC calls.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
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
