/**
 * @file AuthRpcService.cc
 * @brief Implementation of the authentication RPC client
 * @details This file contains the implementation of AuthRpcClient class methods,
 *          including RPC call execution, error handling, and connectivity state management.
 */


#include "AuthRpcService.hpp"

#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <glog/logging.h>

#include "auth/AuthRpcParam.hpp"
#include "rpc/RpcMetadata.hpp"

namespace client_app::auth
{
    namespace
    {
        std::unique_ptr<AuthRpcService> s_instance = nullptr;
    }

    /// @brief Initialize the singleton with application config path
    void AuthRpcService::init(const std::filesystem::path& config_path)
    {
        LOG_IF(FATAL, s_instance) << "AuthRpcService already initialized";

        DLOG(INFO) << "Setting up gRPC client configuration";

        // Load RPC options from config
        AuthRpcParam rpc_options;
        rpc_options.deserializedFromYamlFile(config_path);

        DLOG(INFO) << fmt::format("gRPC configuration loaded successfully - Keepalive Time: {}ms, Keepalive Timeout: {}ms, Permit Without Calls: {}, Server Address: {}", rpc_options.keepaliveTimeMs(), rpc_options.keepaliveTimeoutMs(), rpc_options.keepalivePermitWithoutCalls(), rpc_options.serverAddress());

        // Setup gRPC channel with custom arguments
        DLOG(INFO) << "Setting up gRPC channel with custom arguments";
        grpc::ChannelArguments channel_args;
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, rpc_options.keepaliveTimeMs());
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, rpc_options.keepaliveTimeoutMs());
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, rpc_options.keepalivePermitWithoutCalls());

        const std::string server_address = rpc_options.serverAddress();

        DLOG(INFO) << fmt::format("Channel arguments set - Time: {}ms, Timeout: {}ms, Permit without calls: {}", rpc_options.keepaliveTimeMs(), rpc_options.keepaliveTimeoutMs(), rpc_options.keepalivePermitWithoutCalls());
        DLOG(INFO) << fmt::format("Creating channel to server at: {}", server_address);

        auto channel = grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), channel_args);

        // Log initial channel state
        const auto initial_state = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(true));
        DLOG(INFO) << fmt::format("Channel state after creation: {}", common::rpc::RpcMetadata::grpcStateToString(initial_state));

        // Wait for connection with timeout
        if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5)))
        {
            const auto final_state = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
            const auto error_msg = fmt::format("Failed to connect to gRPC server at {} within timeout period. Final state: {}", server_address, common::rpc::RpcMetadata::grpcStateToString(final_state));
            LOG(ERROR) << error_msg;
            throw std::runtime_error(error_msg);
        }

        DLOG(INFO) << fmt::format("Successfully connected to gRPC server at {}", server_address);

        const auto final_state = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
        DLOG(INFO) << fmt::format("Final connection state: {}", common::rpc::RpcMetadata::grpcStateToString(final_state));

        s_instance = std::unique_ptr<AuthRpcService>(new AuthRpcService(channel));
    }

    /// @brief Get the singleton instance
    AuthRpcService& AuthRpcService::getInstance()
    {
        LOG_IF(FATAL, !s_instance) << "AuthRpcService not initialized. Call init() first.";
        return *s_instance;
    }

    /// @brief Construct a new AuthRpcClient object
    /// @param channel The gRPC channel to use for communication
    AuthRpcService::AuthRpcService(const std::shared_ptr<grpc::Channel>& channel) : stub_(rpc::AuthService::NewStub(channel)), channel_(channel)
    {
    }

    /// @brief Register a new user with username and password
    /// @param[in] username The username to register
    /// @param[in] password The password for the user
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::RegisterUser(const std::string& username, const std::string& password) const
    {
        rpc::RegisterUserRequest request{};
        request.set_username(username);
        request.set_password(password);

        return ExecuteRpcCall<rpc::RegisterUserRequest, rpc::AuthResponse>("RegisterUser", request, [this](grpc::ClientContext* context, const rpc::RegisterUserRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->RegisterUser(context, req, response);
        });
    }

    /// @brief Authenticate a user with username and password
    /// @param[in] username The username to authenticate
    /// @param[in] password The password for the user
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::AuthenticateUser(const std::string& username, const std::string& password) const
    {
        rpc::AuthenticateUserRequest request{};
        request.set_username(username);
        request.set_password(password);

        return ExecuteRpcCall<rpc::AuthenticateUserRequest, rpc::AuthResponse>("AuthenticateUser", request, [this](grpc::ClientContext* context, const rpc::AuthenticateUserRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->AuthenticateUser(context, req, response);
        });
    }

    /// @brief Check if a user exists
    /// @param[in] username The username to check
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::UserExists(const std::string& username) const
    {
        rpc::UserExistsRequest request{};
        request.set_username(username);

        return ExecuteRpcCall<rpc::UserExistsRequest, rpc::AuthResponse>("UserExists", request, [this](grpc::ClientContext* context, const rpc::UserExistsRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->UserExists(context, req, response);
        });
    }

    /// @brief Change password for an authenticated user
    /// @param[in] username The username whose password to change
    /// @param[in] current_password The current password
    /// @param[in] new_password The new password to set
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::ChangePassword(const std::string& username, const std::string& current_password, const std::string& new_password) const
    {
        rpc::ChangePasswordRequest request{};
        request.set_username(username);
        request.set_current_password(current_password);
        request.set_new_password(new_password);

        return ExecuteRpcCall<rpc::ChangePasswordRequest, rpc::AuthResponse>("ChangePassword", request, [this](grpc::ClientContext* context, const rpc::ChangePasswordRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->ChangePassword(context, req, response);
        });
    }

    /// @brief Reset password for a user (admin function)
    /// @param[in] username The username whose password to reset
    /// @param[in] new_password The new password to set
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::ResetPassword(const std::string& username, const std::string& new_password) const
    {
        rpc::ResetPasswordRequest request{};
        request.set_username(username);
        request.set_new_password(new_password);

        return ExecuteRpcCall<rpc::ResetPasswordRequest, rpc::AuthResponse>("ResetPassword", request, [this](grpc::ClientContext* context, const rpc::ResetPasswordRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->ResetPassword(context, req, response);
        });
    }

    /// @brief Delete a user
    /// @param[in] username The username to delete
    /// @return rpc::AuthResponse containing operation result
    [[nodiscard]] rpc::AuthResponse AuthRpcService::DeleteUser(const std::string& username) const
    {
        rpc::DeleteUserRequest request{};
        request.set_username(username);

        return ExecuteRpcCall<rpc::DeleteUserRequest, rpc::AuthResponse>("DeleteUser", request, [this](grpc::ClientContext* context, const rpc::DeleteUserRequest& req, rpc::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->DeleteUser(context, req, response);
        });
    }

    /// @brief Get the underlying channel's current connectivity state
    /// @return Current GrpcConnectivityState of the channel
    common::rpc::GrpcConnectivityState AuthRpcService::getConnectivityState() const
    {
        const grpc_connectivity_state raw_state = channel_->GetState(false);
        return common::rpc::RpcMetadata::grpcStateToEnum(raw_state);
    }

    /// @brief Check if the client channel is ready for RPC calls
    /// @return True if channel is in READY state
    bool AuthRpcService::isReady() const
    {
        return getConnectivityState() == common::rpc::GrpcConnectivityState::READY;
    }

    /// @brief Execute RPC call with error handling and logging
    /// @tparam RequestType Type of the request message
    /// @tparam ResponseType Type of the response message
    /// @param[in] operation_name Name of the operation for logging
    /// @param[in] request The request message to send
    /// @param[in] rpc_call Function that performs the actual RPC call
    /// @return rpc::AuthResponse containing operation result
    template <typename RequestType, typename ResponseType>
    [[nodiscard]] ResponseType AuthRpcService::ExecuteRpcCall(const std::string& operation_name, const RequestType& request, const std::function<grpc::Status(grpc::ClientContext*, const RequestType&, ResponseType*)>& rpc_call) const
    {
        ResponseType response{};
        grpc::ClientContext context{};

        if (const grpc::Status status = rpc_call(&context, request, &response); !status.ok())
        {
            LOG_IF(WARNING, !status.ok()) << "RPC " << operation_name << " failed: " << status.error_message();
            response.set_success(false);
            response.set_message("RPC failed: " + status.error_message());
            response.set_error_code(status.error_code());
        }
        else
        {
            LOG_IF(INFO, status.ok()) << "RPC " << operation_name << " succeeded";
        }
        return response;
    }
}
