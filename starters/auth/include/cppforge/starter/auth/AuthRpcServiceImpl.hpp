/**
 * @file AuthRpcServiceImpl.hpp
 * @brief Authentication RPC service implementation
 * @details This header defines the AuthRpcServiceImpl class that implements the gRPC
 *          authentication service, providing user registration, authentication,
 *          password management, and account operations via remote procedure calls.
 */

#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <grpcpp/server_builder.h>
#include <cppforge/auth/AuthenticationException.hpp>
#include <generated/AuthService.grpc.pb.h>

#include <cppforge/starter/auth/AuthConfig.hpp>
#include <cppforge/auth/UserAuthenticator.hpp>


namespace cppforge::starter::auth
{
    class AuthServiceFactory;
    /// @brief RPC service implementation for handling remote procedure calls
    class AuthRpcServiceImpl final : public cppforge::starter::auth::AuthService::Service
    {
    public:
        /// @brief Default constructor deleted (singleton)
        AuthRpcServiceImpl() = delete;

        /// @brief Singleton instance access
        /// @return Reference to the singleton AuthRpcServiceImpl instance
        /// @note Must call init() before the first call to getInstance()
        static AuthRpcServiceImpl& getInstance();

        /// @brief Initialize the singleton with application config path
        /// @param config_path Path to the application YAML config file
        /// @note Must be called once before any call to getInstance()
        static void init(const std::filesystem::path& config_path);

        /// @brief Build and start the gRPC server, then block on Wait()
        /// @note init() must have been called before this
        void start();

        /// @brief Shutdown the gRPC server
        void shutdown();

        /// @brief Get the loaded RPC options
        /// @return Reference to the loaded AuthConfig
        [[nodiscard]] const AuthConfig& getOptions() const;

        /// @brief Copy constructor (deleted)
        AuthRpcServiceImpl(const AuthRpcServiceImpl&) = delete;

        /// @brief Copy assignment operator (deleted)
        void operator=(const AuthRpcServiceImpl&) = delete;

        /// @brief Move constructor (deleted)
        AuthRpcServiceImpl(AuthRpcServiceImpl&&) = delete;

        /// @brief Move assignment operator (deleted)
        void operator=(AuthRpcServiceImpl&&) = delete;

        /// @brief Default destructor
        ~AuthRpcServiceImpl() override;

        /// @brief Register new user account
        [[nodiscard]] grpc::Status RegisterUser(grpc::ServerContext* context, const cppforge::starter::auth::RegisterUserRequest* request, cppforge::starter::auth::AuthResponse* response) override;

        /// @brief Authenticate user credentials
        [[nodiscard]] grpc::Status AuthenticateUser(grpc::ServerContext* context, const cppforge::starter::auth::AuthenticateUserRequest* request, cppforge::starter::auth::AuthResponse* response) override;

        /// @brief Change user password
        [[nodiscard]] grpc::Status ChangePassword(grpc::ServerContext* context, const cppforge::starter::auth::ChangePasswordRequest* request, cppforge::starter::auth::AuthResponse* response) override;

        /// @brief Reset user password (administrative)
        [[nodiscard]] grpc::Status ResetPassword(grpc::ServerContext* context, const cppforge::starter::auth::ResetPasswordRequest* request, cppforge::starter::auth::AuthResponse* response) override;

        /// @brief Delete user account
        [[nodiscard]] grpc::Status DeleteUser(grpc::ServerContext* context, const cppforge::starter::auth::DeleteUserRequest* request, cppforge::starter::auth::AuthResponse* response) override;

        /// @brief Check if user exists
        [[nodiscard]] grpc::Status UserExists(grpc::ServerContext* context, const cppforge::starter::auth::UserExistsRequest* request, cppforge::starter::auth::AuthResponse* response) override;

    private:
        friend class AuthServiceFactory;

        /// @brief Private constructor
        /// @param db_path Path to SQLite database file
        explicit AuthRpcServiceImpl(const std::string& db_path);

        /// @brief Authenticator instance for managing user accounts
        cppforge::auth::UserAuthenticator authenticator_;

        /// @brief Map exception types to error codes using table-driven approach
        static const std::unordered_map<std::string_view, int> error_map_;

        /// @brief Convert AuthenticationException to grpc::Status
        /// @param e AuthenticationException to handle
        /// @param response Response to populate with error details
        /// @return Appropriate gRPC status
        [[nodiscard]] static grpc::Status HandleAuthException(const cppforge::auth::AuthenticationException& e, cppforge::starter::auth::AuthResponse* response);

        /// @brief Loaded RPC configuration options
        AuthConfig options_;

        /// @brief gRPC server instance
        std::unique_ptr<grpc::Server> server_;
    };
}
