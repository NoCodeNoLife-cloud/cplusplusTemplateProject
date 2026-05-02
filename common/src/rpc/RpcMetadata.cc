#include "RpcMetadata.hpp"

#include <fmt/format.h>
#include <unordered_map>
#include <utility>

namespace common::rpc {
    auto RpcMetadata::grpcStateToString(const grpc_connectivity_state state) -> std::string {
        // Using table-driven approach for better maintainability
        static const std::unordered_map<grpc_connectivity_state, std::string> stateToStringMap = {{GRPC_CHANNEL_IDLE, "IDLE"}, {GRPC_CHANNEL_CONNECTING, "CONNECTING"}, {GRPC_CHANNEL_READY, "READY"}, {GRPC_CHANNEL_TRANSIENT_FAILURE, "TRANSIENT_FAILURE"}, {GRPC_CHANNEL_SHUTDOWN, "SHUTDOWN"}};

        // Look up the state in our map
        const auto it = stateToStringMap.find(state);
        if (it != stateToStringMap.end()) {
            return it->second;
        }
        return "UNKNOWN";
    }

    auto RpcMetadata::grpcStateToEnum(const grpc_connectivity_state state) -> GrpcConnectivityState {
        // Map grpc_connectivity_state to our internal GrpcConnectivityState enum
        GrpcConnectivityState result;
        switch (state) {
            case GRPC_CHANNEL_IDLE:
                result = GrpcConnectivityState::IDLE;
                break;
            case GRPC_CHANNEL_CONNECTING:
                result = GrpcConnectivityState::CONNECTING;
                break;
            case GRPC_CHANNEL_READY:
                result = GrpcConnectivityState::READY;
                break;
            case GRPC_CHANNEL_TRANSIENT_FAILURE:
                result = GrpcConnectivityState::TRANSIENT_FAILURE;
                break;
            case GRPC_CHANNEL_SHUTDOWN:
                result = GrpcConnectivityState::SHUTDOWN;
                break;
            default:
                result = GrpcConnectivityState::IDLE;
        }
        return result;
    }

    auto RpcMetadata::grpcStateToString(const GrpcConnectivityState state) -> std::string {
        std::string result;
        // Convert our internal GrpcConnectivityState enum to string
        switch (state) {
            case GrpcConnectivityState::IDLE:
                result = "IDLE";
                break;
            case GrpcConnectivityState::CONNECTING:
                result = "CONNECTING";
                break;
            case GrpcConnectivityState::READY:
                result = "READY";
                break;
            case GrpcConnectivityState::TRANSIENT_FAILURE:
                result = "TRANSIENT_FAILURE";
                break;
            case GrpcConnectivityState::SHUTDOWN:
                result = "SHUTDOWN";
                break;
            default:
                result = "UNKNOWN";
        }
        return result;
    }
} // common
