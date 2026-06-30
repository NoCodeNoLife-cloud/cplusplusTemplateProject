/**
 * @file AuthRpcClient.hpp
 * @brief RPC client for authentication service communication
 * @details This header defines the AuthRpcClient class that provides methods
 *          to interact with the server''s authentication service via gRPC,
 *          including user registration, authentication, password management,
 *          and user deletion operations.
 */

#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include <generated/AuthService.grpc.pb.h>
#include <cppforge/rpc/GrpcConnectivityState.hpp>

namespace cppforge::starter::auth
{
    class AuthServiceFactory;

    /// @brief RPC client for communicating with the server.
    class AuthRpcClient
    {
    public:
        /// @brief Default constructor deleted (singleton)
        AuthRpcClient() = delete;

        /// @brief Singleton instance access
        /// @return Reference to the singleton AuthRpcClient instance
        /// @note Must call init() before the first call to getInstance()
        static AuthRpcClient& getInstance();

        /// @brief Initialize the singleton with application config path
        /// @param config_path Path to the application YAML config file
        /// @note Must be called once before any call to getInstance()
        static void init(const std::filesystem::path& config_path);

        /// @brief Copy constructor deleted
        AuthRpcClient(const AuthRpcClient&) = delete;

        /// @brief Move constructor deleted
        AuthRpcClient(AuthRpcClient&&) = delete;

        /// @brief Copy assignment operator deleted
        auto operator=(const AuthRpcClient&) -> AuthRpcClient& = delete;

        /// @brief Move assignment operator deleted
        auto operator=(AuthRpcClient&&) -> AuthRpcClient& = delete;

        /// @brief Virtual destructor with default implementation
        virtual ~AuthRpcClient() = default;

        /// @brief Register a new user with username and password
        /// @param[in] username The username to register
        /// @param[in] password The password for the user
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse RegisterUser(const std::string& username, const std::string& password) const ;

        /// @brief Authenticate a user with username and password
        /// @param[in] username The username to authenticate
        /// @param[in] password The password for the user
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse AuthenticateUser(const std::string& username, const std::string& password) const ;

        /// @brief Check if a user exists
        /// @param[in] username The username to check
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse UserExists(const std::string& username) const ;

        /// @brief Change password for an authenticated user
        /// @param[in] username The username whose password to change
        /// @param[in] current_password The current password
        /// @param[in] new_password The new password to set
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse ChangePassword(const std::string& username, const std::string& current_password, const std::string& new_password) const ;

        /// @brief Reset password for a user (admin function)
        /// @param[in] username The username whose password to reset
        /// @param[in] new_password The new password to set
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse ResetPassword(const std::string& username, const std::string& new_password) const ;

        /// @brief Delete a user
        /// @param[in] username The username to delete
        /// @return AuthResponse containing operation result
        [[nodiscard]] cppforge::starter::auth::AuthResponse DeleteUser(const std::string& username) const ;

        /// @brief Get the underlying channel''s current connectivity state
        /// @return Current GrpcConnectivityState of the channel
        [[nodiscard]] cppforge::rpc::GrpcConnectivityState getConnectivityState() const ;

        /// @brief Check if the client channel is ready for RPC calls
        /// @return True if channel is in READY state
        [[nodiscard]] bool isReady() const ;

    private:
        friend class AuthServiceFactory;

        /// @brief Construct a new AuthRpcClient object
        /// @param channel The gRPC channel to use for communication
        explicit AuthRpcClient(const std::shared_ptr<grpc::Channel>& channel);

        /// @brief Execute RPC call with error handling and logging
        /// @tparam RequestType Type of the request message
        /// @tparam ResponseType Type of the response message
        /// @param[in] operation_name Name of the operation for logging
        /// @param[in] request The request message to send
        /// @param[in] rpc_call Function that performs the actual RPC call
        /// @return AuthResponse containing operation result
        template <typename RequestType, typename ResponseType>
        [[nodiscard]] ResponseType ExecuteRpcCall(const std::string& operation_name, const RequestType& request, const std::function<grpc::Status(grpc::ClientContext *, const RequestType &, ResponseType *)>& rpc_call) const ;

        /// @brief gRPC stub for making RPC calls
        std::unique_ptr<cppforge::starter::auth::AuthService::Stub> stub_;

        /// @brief Store reference to the original channel to access connectivity state
        std::shared_ptr<grpc::Channel> channel_;
    };
}
