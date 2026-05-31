/**
 * @file AuthRpcService.hpp
 * @brief RPC client for authentication service communication
 * @details This header defines the AuthRpcClient class that provides methods
 *          to interact with the server's authentication service via gRPC,
 *          including user registration, authentication, password management,
 *          and user deletion operations.
 */

#pragma once
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include "generated/RpcService.grpc.pb.h"
#include "rpc/GrpcConnectivityState.hpp"

namespace client_app::auth
{
    /// @brief RPC client for communicating with the server.
    /// @details This class provides methods to interact with the RPC service.
    class AuthRpcService
    {
    public:
        /// @brief Default constructor explicitly deleted to enforce parameterized construction
        AuthRpcService() = delete;

        /// @brief Construct a new AuthRpcManager object
        /// @param channel The gRPC channel to use for communication
        explicit AuthRpcService(const std::shared_ptr<grpc::Channel>& channel) ;

        /// @brief Copy constructor deleted to enforce unique ownership semantics
        AuthRpcService(const AuthRpcService&) = delete;

        /// @brief Move constructor with  guarantee for efficient resource transfer
        AuthRpcService(AuthRpcService&&)  = default;

        /// @brief Copy assignment operator deleted to prevent unintended resource duplication
        auto operator=(const AuthRpcService&) -> AuthRpcService& = delete;

        /// @brief Move assignment operator with  guarantee
        auto operator=(AuthRpcService&&)  -> AuthRpcService& = default;

        /// @brief Virtual destructor with default implementation for proper polymorphic cleanup
        virtual ~AuthRpcService()  = default;

        /// @brief Register a new user with username and password
        /// @param[in] username The username to register
        /// @param[in] password The password for the user
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse RegisterUser(const std::string& username, const std::string& password) const ;

        /// @brief Authenticate a user with username and password
        /// @param[in] username The username to authenticate
        /// @param[in] password The password for the user
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse AuthenticateUser(const std::string& username, const std::string& password) const ;

        /// @brief Check if a user exists
        /// @param[in] username The username to check
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse UserExists(const std::string& username) const ;

        /// @brief Change password for an authenticated user
        /// @param[in] username The username whose password to change
        /// @param[in] current_password The current password
        /// @param[in] new_password The new password to set
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse ChangePassword(const std::string& username, const std::string& current_password, const std::string& new_password) const ;

        /// @brief Reset password for a user (admin function)
        /// @param[in] username The username whose password to reset
        /// @param[in] new_password The new password to set
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse ResetPassword(const std::string& username, const std::string& new_password) const ;

        /// @brief Delete a user
        /// @param[in] username The username to delete
        /// @return rpc::AuthResponse containing operation result
        [[nodiscard]] rpc::AuthResponse DeleteUser(const std::string& username) const ;

        /// @brief Get the underlying channel's current connectivity state
        /// @return Current GrpcConnectivityState of the channel
        [[nodiscard]] common::rpc::GrpcConnectivityState getConnectivityState() const ;

        /// @brief Check if the client channel is ready for RPC calls
        /// @return True if channel is in READY state
        [[nodiscard]] bool isReady() const ;

    private:
        /// @brief Execute RPC call with error handling and logging
        /// @tparam RequestType Type of the request message
        /// @tparam ResponseType Type of the response message
        /// @param[in] operation_name Name of the operation for logging
        /// @param[in] request The request message to send
        /// @param[in] rpc_call Function that performs the actual RPC call
        /// @return rpc::AuthResponse containing operation result
        template <typename RequestType, typename ResponseType>
        [[nodiscard]] ResponseType ExecuteRpcCall(const std::string& operation_name, const RequestType& request, const std::function<grpc::Status(grpc::ClientContext *, const RequestType &, ResponseType *)>& rpc_call) const ;

        /// @brief gRPC stub for making RPC calls
        std::unique_ptr<rpc::AuthService::Stub> stub_;

        /// @brief Store reference to the original channel to access connectivity state
        std::shared_ptr<grpc::Channel> channel_;
    };
}