/**
 * @file AuthRpcService.hpp
 * @brief Authentication RPC service implementation
 * @details This header defines the AuthRpcService class that implements the gRPC
 *          authentication service, providing user registration, authentication,
 *          password management, and account operations via remote procedure calls.
 */

#pragma once
#include <src/auth/UserAuthenticator.hpp>
#include <src/exception/AuthenticationException.hpp>

#include "generated/RpcService.grpc.pb.h"
#include <string>
#include <string_view>
#include <unordered_map>

namespace server_app::auth
{
    /// @brief RPC service implementation for handling remote procedure calls
    /// @details This class implements the gRPC service interface defined in RpcService.grpc.pb.h
    /// and provides the actual business logic for handling RPC requests.
    class AuthRpcService final : public rpc::AuthService::Service
    {
    public:
        /// @brief Constructor with database path
        /// @param db_path Path to SQLite database file
        explicit AuthRpcService(const std::string& db_path) noexcept;

        /// @brief Default destructor
        ~AuthRpcService() noexcept override = default;

        /// @brief Copy constructor (deleted)
        AuthRpcService(const AuthRpcService&) = delete;

        /// @brief Copy assignment operator (deleted)
        void operator=(const AuthRpcService&) = delete;

        /// @brief Move constructor (deleted)
        AuthRpcService(AuthRpcService&&) = delete;

        /// @brief Move assignment operator (deleted)
        void operator=(AuthRpcService&&) = delete;

        /// @brief Register new user account
        [[nodiscard]] ::grpc::Status RegisterUser(::grpc::ServerContext* context, const ::rpc::RegisterUserRequest* request, ::rpc::AuthResponse* response) override;

        /// @brief Authenticate user credentials
        [[nodiscard]] ::grpc::Status AuthenticateUser(::grpc::ServerContext* context, const ::rpc::AuthenticateUserRequest* request, ::rpc::AuthResponse* response) override;

        /// @brief Change user password
        [[nodiscard]] ::grpc::Status ChangePassword(::grpc::ServerContext* context, const ::rpc::ChangePasswordRequest* request, ::rpc::AuthResponse* response) override;

        /// @brief Reset user password (administrative)
        [[nodiscard]] ::grpc::Status ResetPassword(::grpc::ServerContext* context, const ::rpc::ResetPasswordRequest* request, ::rpc::AuthResponse* response) override;

        /// @brief Delete user account
        [[nodiscard]] ::grpc::Status DeleteUser(::grpc::ServerContext* context, const ::rpc::DeleteUserRequest* request, ::rpc::AuthResponse* response) override;

        /// @brief Check if user exists
        [[nodiscard]] ::grpc::Status UserExists(::grpc::ServerContext* context, const ::rpc::UserExistsRequest* request, ::rpc::AuthResponse* response) override;

    private:
        /// @brief Authenticator instance for managing user accounts
        common::auth::UserAuthenticator authenticator_;

        /// @brief Map exception types to error codes using table-driven approach
        static const std::unordered_map<std::string_view, int> error_map_;

        /// @brief Convert AuthenticationException to grpc::Status
        /// @param e AuthenticationException to handle
        /// @param response Response to populate with error details
        /// @return Appropriate gRPC status
        [[nodiscard]] static ::grpc::Status HandleAuthException(const common::exception::AuthenticationException& e, ::rpc::AuthResponse* response) noexcept;
    };
}
