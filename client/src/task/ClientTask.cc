#include "src/task/ClientTask.hpp"

#include <fmt/format.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>

#include "config/GLogConfigurator.hpp"
#include "rpc/RpcMetadata.hpp"
#include "src/auth/AuthRpcClient.hpp"
#include "src/filesystem/io/Console.hpp"
#include "src/system/SystemInfo.hpp"

namespace client_app::task {
    ClientTask::ClientTask(const std::string &project_name_) noexcept : rpc_options_{auth::AuthRpcClientOptions::builder().build()}, timer_{project_name_} {
        timer_.recordStart();
    }

    auto ClientTask::init() const noexcept -> void {
        const glog::config::GLogConfigurator log_configurator{glog_config_path_};
        log_configurator.execute();
        DLOG(INFO) << fmt::format("Initializing GLog configuration from: {}, RPC Options - Keepalive Time: {}ms, Timeout: {}ms, Permit Without Calls: {}, configuration initialized successfully", glog_config_path_, rpc_options_.keepaliveTimeMs(), rpc_options_.keepaliveTimeoutMs(), rpc_options_.keepalivePermitWithoutCalls());

        DLOG(INFO) << "Application starting...";
        logClientInfo();
        DLOG(INFO) << "Initialization completed successfully";
    }

    auto ClientTask::logIn(const client_app::auth::AuthRpcClient &auth_rpc_client) -> std::string {
        DLOG(INFO) << "Starting authentication process";

        // Check if the RPC client is ready before proceeding
        if (!auth_rpc_client.isReady()) {
            LOG(WARNING) << "RPC client is not ready. Current state: " << common::rpc::RpcMetadata::grpcStateToString(auth_rpc_client.getConnectivityState());
        }

        // Authenticate user
        DLOG(INFO) << "Please enter your username: ";
        const std::string username = common::filesystem::Console::readLine();
        DLOG(INFO) << "Please enter your password: ";
        const std::string password = common::filesystem::Console::readLine();
        DLOG(INFO) << fmt::format("Login attempt for user: {}", username);

        // Try to authenticate user
        const auto authenticateUserResponse = auth_rpc_client.AuthenticateUser(username, password);
        if (authenticateUserResponse.success()) {
            DLOG(INFO) << "User authenticated successfully, authentication process completed";
            return username;
        }

        LOG(ERROR) << "Authentication failed: " << authenticateUserResponse.message();

        // Check if user exists.
        if (const auto userExistsResponse = auth_rpc_client.UserExists(username); userExistsResponse.success()) {
            DLOG(INFO) << fmt::format("User already exists, authentication failed\nAuthentication failed, please check your username and password.");
        } else {
            // User doesn't exist, ask if they want to create a new account.
            if (shouldCreateNewAccount()) {
                registerNewUser(auth_rpc_client, username, password);
                DLOG(INFO) << "Registered user successfully";
            } else {
                DLOG(INFO) << "Authentication failed, please check your username and password.";
            }
        }

        DLOG(INFO) << "Authentication process completed";
        return username;
    }

    auto ClientTask::shouldCreateNewAccount() -> bool {
        DLOG(INFO) << "User does not exist, do you want to create a new account? [y/n] ";
        const std::string createNewAccount = common::filesystem::Console::readLine();
        return createNewAccount == "y" || createNewAccount == "Y";
    }

    auto ClientTask::registerNewUser(const client_app::auth::AuthRpcClient &auth_rpc_client, const std::string &username, const std::string &password) -> void {
        DLOG(INFO) << "Registering user...";
        const auto registerUserResponse = auth_rpc_client.RegisterUser(username, password);
        if (!registerUserResponse.success()) {
            const auto error_msg = fmt::format("Failed to register user: {} Error code: {}", registerUserResponse.message(), registerUserResponse.error_code());
            LOG(ERROR) << error_msg;
            throw std::runtime_error(error_msg);
        }

        DLOG(INFO) << fmt::format("Registered user successfully, return value: {}", registerUserResponse.message());
    }

    auto ClientTask::logOut(const client_app::auth::AuthRpcClient &auth_rpc_client, const std::string &username) noexcept -> void {
        if (const auto deleteUserResponse = auth_rpc_client.DeleteUser(username); !deleteUserResponse.success()) {
            LOG(ERROR) << fmt::format("Failed to delete user: {}, Error code: {}", deleteUserResponse.message(), deleteUserResponse.error_code());
        } else {
            DLOG(INFO) << fmt::format("Deleted user successfully, return value: {}", deleteUserResponse.message());
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    auto ClientTask::task(const client_app::auth::AuthRpcClient &auth_rpc_client) noexcept -> void {
        // Implement actual task logic here
        DLOG(INFO) << "Current connection state: " << common::rpc::RpcMetadata::grpcStateToString(auth_rpc_client.getConnectivityState());
    }

    auto ClientTask::run() -> void {
        init();
        const auto client = createRpcClient();

        // Log initial connection state
        DLOG(INFO) << "Initial connection state: " << common::rpc::RpcMetadata::grpcStateToString(client.getConnectivityState());

        const std::string username = logIn(client);

        task(client);
        DLOG(INFO) << "Client task execution completed";

        logOut(client, username);
        exit();
    }

    auto ClientTask::createRpcClient() const -> client_app::auth::AuthRpcClient {
        DLOG(INFO) << "Creating gRPC channel";
        // Create channel using the existing createChannel method with custom arguments
        const auto channel = createChannel();

        // Get state using the new GrpcConnectivityState enum
        const auto state_enum = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(true));
        const std::string state_str = common::rpc::RpcMetadata::grpcStateToString(state_enum);

        DLOG(INFO) << fmt::format("gRPC channel created with state: {}", state_str);
        DLOG(INFO) << "Creating RPC client";
        // Create client using the channel with custom arguments
        client_app::auth::AuthRpcClient client{channel};
        DLOG(INFO) << "RPC client created successfully";

        return client;
    }

    auto ClientTask::exit() const noexcept -> void {
        timer_.recordEnd(true);
        DLOG(INFO) << "Application finished successfully.";
    }

    auto ClientTask::logClientInfo() noexcept -> void {
        DLOG(INFO) << fmt::format("OS Version: {}, CPU Model: {}, Memory Details: {}, Graphics Card Info: {}", common::system::SystemInfo::GetOSVersion(), common::system::SystemInfo::GetCpuModelFromRegistry(), common::system::SystemInfo::GetMemoryDetails(), common::system::SystemInfo::GetGraphicsCardInfo());
    }

    auto ClientTask::createChannel() const -> std::shared_ptr<grpc::Channel> {
        DLOG(INFO) << "Setting up gRPC channel with custom arguments";

        // Setup channel
        grpc::ChannelArguments channel_args;
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, rpc_options_.keepaliveTimeMs());
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, rpc_options_.keepaliveTimeoutMs());
        channel_args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, rpc_options_.keepalivePermitWithoutCalls());

        DLOG(INFO) << fmt::format("Channel arguments set - Time: {}ms, Timeout: {}ms, Permit without calls: {}", rpc_options_.keepaliveTimeMs(), rpc_options_.keepaliveTimeoutMs(), rpc_options_.keepalivePermitWithoutCalls());

        // Create client
        const std::string server_address = rpc_options_.serverAddress();
        DLOG(INFO) << fmt::format("Creating channel to server at: {}", server_address);
        const auto channel = grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), channel_args);

        // Get state using the new GrpcConnectivityState enum
        const auto state_enum = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(true));
        const std::string state_str = common::rpc::RpcMetadata::grpcStateToString(state_enum);

        DLOG(INFO) << fmt::format("Channel state after creation: {}", state_str);

        // Give channel some time to connect
        if (!channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5))) {
            const auto error_msg = fmt::format("Failed to connect to gRPC server at {} within timeout period", server_address);
            LOG(ERROR) << error_msg;

            // Get final state using the new enum
            const auto final_state_enum = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
            const std::string final_state_str = common::rpc::RpcMetadata::grpcStateToString(final_state_enum);

            DLOG(INFO) << fmt::format("Connection attempt finished with state: {}", final_state_str);
            throw std::runtime_error(error_msg);
        }
        DLOG(INFO) << fmt::format("Successfully connected to gRPC server at {}", server_address);

        // Get final state using the new enum
        const auto final_state_enum = common::rpc::RpcMetadata::grpcStateToEnum(channel->GetState(false));
        const std::string final_state_str = common::rpc::RpcMetadata::grpcStateToString(final_state_enum);

        DLOG(INFO) << fmt::format("Final connection state: {}", final_state_str);

        return channel;
    }
}
