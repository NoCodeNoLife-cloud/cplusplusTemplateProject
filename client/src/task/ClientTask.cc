/**
 * @file ClientTask.cc
 * @brief Implementation of the client task functionality
 * @details This file contains the implementation of ClientTask class methods,
 *          including user authentication flow, gRPC channel management,
 *          system information logging, and application lifecycle control.
 */

#include "task/ClientTask.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

#include "auth/AuthRpcService.hpp"
#include "config/ConfigParam.h"
#include "config/GLogConfigurator.hpp"
#include "filesystem/io/Console.hpp"
#include "rpc/RpcMetadata.hpp"
#include "system/SystemInfo.hpp"

namespace client_app::task
{
    ClientTask::ClientTask(const std::string& project_name_) : timer_{project_name_}
    {
        timer_.recordStart();
    }

    void ClientTask::init()
    {
        const glog::config::GLogConfigurator log_configurator{config::ConfigParam::getInstance().applicationDevConfigPath()};
        log_configurator.execute();
        DLOG(INFO) << fmt::format("Initializing GLog configuration from: {}, configuration initialized successfully", config::ConfigParam::getInstance().applicationDevConfigPath());

        DLOG(INFO) << "Application starting...";
        logClientInfo();
        DLOG(INFO) << "Initialization completed successfully";
    }

    void ClientTask::run()
    {
        init();

        auth::AuthRpcService::init(config::ConfigParam::getInstance().applicationDevConfigPath());
        const auto& client = auth::AuthRpcService::getInstance();

        // Log initial connection state
        DLOG(INFO) << "Initial connection state: " << common::rpc::RpcMetadata::grpcStateToString(client.getConnectivityState());

        const std::string username = logIn(client);

        task(client);
        DLOG(INFO) << "Client task execution completed";

        logOut(client, username);
        exit();
    }

    void ClientTask::exit()
    {
        timer_.recordEnd();
        const auto time_info = timer_.getRunTime();
        LOG(INFO) << time_info;
        DLOG(INFO) << "Application finished successfully.";
    }

    std::string ClientTask::logIn(const auth::AuthRpcService& auth_rpc_client)
    {
        DLOG(INFO) << "Starting authentication process";

        // Check if the RPC client is ready before proceeding
        if (!auth_rpc_client.isReady())
        {
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
        if (authenticateUserResponse.success())
        {
            DLOG(INFO) << "User authenticated successfully, authentication process completed";
            return username;
        }

        LOG(ERROR) << "Authentication failed: " << authenticateUserResponse.message();

        // Check if user exists.
        if (const auto userExistsResponse = auth_rpc_client.UserExists(username); userExistsResponse.success())
        {
            DLOG(INFO) << fmt::format("User already exists, authentication failed\nAuthentication failed, please check your username and password.");
        }
        else
        {
            // User doesn't exist, ask if they want to create a new account.
            if (shouldCreateNewAccount())
            {
                registerNewUser(auth_rpc_client, username, password);
                DLOG(INFO) << "Registered user successfully";
            }
            else
            {
                DLOG(INFO) << "Authentication failed, please check your username and password.";
            }
        }

        DLOG(INFO) << "Authentication process completed";
        return username;
    }

    bool ClientTask::shouldCreateNewAccount()
    {
        DLOG(INFO) << "User does not exist, do you want to create a new account? [y/n] ";
        const std::string createNewAccount = common::filesystem::Console::readLine();
        return createNewAccount == "y" || createNewAccount == "Y";
    }

    void ClientTask::registerNewUser(const auth::AuthRpcService& auth_rpc_client, const std::string& username, const std::string& password)
    {
        DLOG(INFO) << "Registering user...";
        const auto registerUserResponse = auth_rpc_client.RegisterUser(username, password);
        if (!registerUserResponse.success())
        {
            const auto error_msg = fmt::format("Failed to register user: {} Error code: {}", registerUserResponse.message(), registerUserResponse.error_code());
            LOG(ERROR) << error_msg;
            throw std::runtime_error(error_msg);
        }

        DLOG(INFO) << fmt::format("Registered user successfully, return value: {}", registerUserResponse.message());
    }

    void ClientTask::logOut(const auth::AuthRpcService& auth_rpc_client, const std::string& username)
    {
        if (const auto deleteUserResponse = auth_rpc_client.DeleteUser(username); !deleteUserResponse.success())
        {
            LOG(ERROR) << fmt::format("Failed to delete user: {}, Error code: {}", deleteUserResponse.message(), deleteUserResponse.error_code());
        }
        else
        {
            DLOG(INFO) << fmt::format("Deleted user successfully, return value: {}", deleteUserResponse.message());
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void ClientTask::task(const auth::AuthRpcService& auth_rpc_client)
    {
        DLOG(INFO) << "Current connection state: " << common::rpc::RpcMetadata::grpcStateToString(auth_rpc_client.getConnectivityState());
        // Implement actual task logic here
    }

    void ClientTask::logClientInfo()
    {
        DLOG(INFO) << fmt::format("OS Version: {}, CPU Model: {}, Memory Details: {}, Graphics Card Info: {}", common::system::SystemInfo::GetOSVersion(), common::system::SystemInfo::GetCpuModelFromRegistry(), common::system::SystemInfo::GetMemoryDetails(), common::system::SystemInfo::GetGraphicsCardInfo());
    }
}
