/**
 * @file ClientTask.hpp
 * @brief Client task implementation for the application
 * @details This header defines the ClientTask class that manages the client application
 *          lifecycle including initialization, authentication, task execution,
 *          and cleanup with performance profiling support.
 */

#pragma once
#include <memory>
#include <string>

#include "auth/AuthRpcService.hpp"
#include <cppforge/interface/task/ITask.hpp>
#include <cppforge/system/FunctionProfiler.hpp>

namespace client_app::task
{
    class ClientTask final : public cppforge::interface::task::ITask
    {
    public:
        /// @brief Default constructor explicitly deleted to enforce parameterized construction
        ClientTask() = delete;

        /// @brief Construct a ClientTask with the specified project name
        /// @param project_name_ The name of the project for profiling purposes
        explicit ClientTask(const std::string& project_name_) ;

        /// @brief Copy constructor deleted to prevent unintended resource duplication
        ClientTask(const ClientTask&) = delete;

        /// @brief Copy assignment operator deleted to prevent unintended resource duplication
        void operator=(const ClientTask&) = delete;

        /// @brief Initialize the client task
        /// @details Sets up logging, loads configuration, and logs system information
        static void init();

        /// @brief Run the main task
        /// @details Initializes the client, creates a gRPC channel, sends a message to the server,
        /// and exits cleanly
        void run() override;

        /// @brief Exit the client task
        /// @details Records the end time and logs completion
        void exit() ;

    private:
        /// @brief Logs a message indicating that the client is logging in
        /// @param auth_rpc_client Reference to the RPC client for authentication
        /// @return Username of the authenticated user
        [[nodiscard]] static std::string logIn(const auth::AuthRpcService& auth_rpc_client);

        /// @brief Check if a new account should be created
        /// @return True if user wants to create a new account
        [[nodiscard]] static bool shouldCreateNewAccount();

        /// @brief Register a new user
        /// @param auth_rpc_client Reference to the RPC client for registration
        /// @param username Username for the new account
        /// @param password Password for the new account
        /// @throws std::runtime_error if registration fails
        static void registerNewUser(const auth::AuthRpcService& auth_rpc_client, const std::string& username, const std::string& password);

        // Changed return type to void since it throws on failure

        /// @brief Logs a message indicating that the client is logging out
        /// @param auth_rpc_client Reference to the RPC client for logout operations
        /// @param username Username of the user to log out
        static void logOut(const auth::AuthRpcService& auth_rpc_client, const std::string& username) ;

        /// @brief Main task
        /// @param auth_rpc_client Reference to the RPC client for executing tasks
        static void task(const auth::AuthRpcService& auth_rpc_client) ;

        /// @brief Logs client system information
        /// @details Logs OS version and CPU model to the application log
        static void logClientInfo() ;

        cppforge::system::FunctionProfiler timer_;
    };
}
