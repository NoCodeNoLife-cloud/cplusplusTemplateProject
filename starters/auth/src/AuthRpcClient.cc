/**
 * @file AuthRpcClient.cc
 * @brief Implementation of the authentication RPC client
 * @details This file contains the implementation of AuthRpcClient class methods,
 *          including RPC call execution, error handling, and connectivity state management.
 */


#include <cppforge/starter/auth/AuthRpcClient.hpp>

#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <glog/logging.h>

#include <cppforge/starter/auth/AuthConfig.hpp>
#include <cppforge/rpc/RpcMetadata.hpp>

namespace cppforge::starter::auth
{
    namespace
    {
        std::unique_ptr<AuthRpcClient> s_instance = nullptr;
    }

    void AuthRpcClient::init(const std::filesystem::path& config_path)
    {
        LOG_IF(FATAL, s_instance) << "AuthRpcClient already initialized";

        DLOG(INFO) << "Setting up gRPC client configuration";

        // Load RPC options from config
        AuthConfig rpc_options;
        rpc_options.deserializeFromYamlFile(config_path);

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

        const auto channel = grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), channel_args);

        // Log initial channel state
        const auto initial_state = cppforge::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(true));
        DLOG(INFO) << fmt::format("Channel state after creation: {}", cppforge::rpc::RpcMetadata::grpcStateToString(initial_state));

        // Wait for connection with timeout
        if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5)))
        {
            const auto final_state = cppforge::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
            const auto error_msg = fmt::format("Failed to connect to gRPC server at {} within timeout period. Final state: {}", server_address, cppforge::rpc::RpcMetadata::grpcStateToString(final_state));
            LOG(ERROR) << error_msg;
            throw std::runtime_error(error_msg);
        }

        DLOG(INFO) << fmt::format("Successfully connected to gRPC server at {}", server_address);

        const auto final_state = cppforge::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
        DLOG(INFO) << fmt::format("Final connection state: {}", cppforge::rpc::RpcMetadata::grpcStateToString(final_state));

        s_instance = std::unique_ptr<AuthRpcClient>(new AuthRpcClient(channel));
    }

    AuthRpcClient& AuthRpcClient::getInstance()
    {
        LOG_IF(FATAL, !s_instance) << "AuthRpcClient not initialized. Call init() first.";
        return *s_instance;
    }

    AuthRpcClient::AuthRpcClient(const std::shared_ptr<grpc::Channel>& channel) : stub_(cppforge::starter::auth::AuthService::NewStub(channel)), channel_(channel)
    {
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::RegisterUser(const std::string& username, const std::string& password) const
    {
        cppforge::starter::auth::RegisterUserRequest request{};
        request.set_username(username);
        request.set_password(password);

        return ExecuteRpcCall<cppforge::starter::auth::RegisterUserRequest, cppforge::starter::auth::AuthResponse>("RegisterUser", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::RegisterUserRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->RegisterUser(context, req, response);
        });
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::AuthenticateUser(const std::string& username, const std::string& password) const
    {
        cppforge::starter::auth::AuthenticateUserRequest request{};
        request.set_username(username);
        request.set_password(password);

        return ExecuteRpcCall<cppforge::starter::auth::AuthenticateUserRequest, cppforge::starter::auth::AuthResponse>("AuthenticateUser", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::AuthenticateUserRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->AuthenticateUser(context, req, response);
        });
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::UserExists(const std::string& username) const
    {
        cppforge::starter::auth::UserExistsRequest request{};
        request.set_username(username);

        return ExecuteRpcCall<cppforge::starter::auth::UserExistsRequest, cppforge::starter::auth::AuthResponse>("UserExists", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::UserExistsRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->UserExists(context, req, response);
        });
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::ChangePassword(const std::string& username, const std::string& current_password, const std::string& new_password) const
    {
        cppforge::starter::auth::ChangePasswordRequest request{};
        request.set_username(username);
        request.set_current_password(current_password);
        request.set_new_password(new_password);

        return ExecuteRpcCall<cppforge::starter::auth::ChangePasswordRequest, cppforge::starter::auth::AuthResponse>("ChangePassword", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::ChangePasswordRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->ChangePassword(context, req, response);
        });
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::ResetPassword(const std::string& username, const std::string& new_password) const
    {
        cppforge::starter::auth::ResetPasswordRequest request{};
        request.set_username(username);
        request.set_new_password(new_password);

        return ExecuteRpcCall<cppforge::starter::auth::ResetPasswordRequest, cppforge::starter::auth::AuthResponse>("ResetPassword", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::ResetPasswordRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->ResetPassword(context, req, response);
        });
    }

    [[nodiscard]] cppforge::starter::auth::AuthResponse AuthRpcClient::DeleteUser(const std::string& username) const
    {
        cppforge::starter::auth::DeleteUserRequest request{};
        request.set_username(username);

        return ExecuteRpcCall<cppforge::starter::auth::DeleteUserRequest, cppforge::starter::auth::AuthResponse>("DeleteUser", request, [this](grpc::ClientContext* context, const cppforge::starter::auth::DeleteUserRequest& req, cppforge::starter::auth::AuthResponse* response) -> grpc::Status
        {
            return this->stub_->DeleteUser(context, req, response);
        });
    }

    cppforge::rpc::GrpcConnectivityState AuthRpcClient::getConnectivityState() const
    {
        const grpc_connectivity_state raw_state = channel_->GetState(false);
        return cppforge::rpc::RpcMetadata::grpcStateToEnum(raw_state);
    }

    bool AuthRpcClient::isReady() const
    {
        return getConnectivityState() == cppforge::rpc::GrpcConnectivityState::READY;
    }

    template <typename RequestType, typename ResponseType>
    [[nodiscard]] ResponseType AuthRpcClient::ExecuteRpcCall(const std::string& operation_name, const RequestType& request, const std::function<grpc::Status(grpc::ClientContext*, const RequestType&, ResponseType*)>& rpc_call) const
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
