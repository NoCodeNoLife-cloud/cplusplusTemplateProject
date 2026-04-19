#pragma once
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include "src/auth/AuthRpcClient.hpp"
#include "src/auth/AuthRpcClientOptions.hpp"
#include "src/time/FunctionProfiler.hpp"
#include "task/interface/ITask.h"

namespace app_client::task {
    class ClientTask final : public common::interfaces::ITask {
    public:
        /// @brief Default constructor explicitly deleted to enforce parameterized construction
        ClientTask() = delete;

        /// @brief Construct a ClientTask with the specified project name
        /// @param project_name_ The name of the project for profiling purposes
        explicit ClientTask(const std::string &project_name_) noexcept;

        /// @brief Copy constructor deleted to prevent unintended resource duplication
        ClientTask(const ClientTask &) = delete;

        /// @brief Copy assignment operator deleted to prevent unintended resource duplication
        auto operator=(const ClientTask &) -> ClientTask & = delete;

        /// @brief Initialize the client task
        /// @details Sets up logging, loads configuration, and logs system information
        auto init() const noexcept -> void;

        /// @brief Run the main task
        /// @details Initializes the client, creates a gRPC channel, sends a message to the server,
        /// and exits cleanly
        auto run() -> void override;

        /// @brief Exit the client task
        /// @details Records the end time and logs completion
        auto exit() const noexcept -> void;

    private:
        /// @brief Logs a message indicating that the client is logging in
        /// @param auth_rpc_client Reference to the RPC client for authentication
        /// @return Username of the authenticated user
        [[nodiscard]] static auto logIn(const client_app::auth::AuthRpcClient &auth_rpc_client) -> std::string;

        /// @brief Check if a new account should be created
        /// @return True if user wants to create a new account
        [[nodiscard]] static auto shouldCreateNewAccount() -> bool;

        /// @brief Register a new user
        /// @param auth_rpc_client Reference to the RPC client for registration
        /// @param username Username for the new account
        /// @param password Password for the new account
        /// @throws std::runtime_error if registration fails
        static auto registerNewUser(const client_app::auth::AuthRpcClient &auth_rpc_client, const std::string &username, const std::string &password) -> void;

        // Changed return type to void since it throws on failure

        /// @brief Logs a message indicating that the client is logging out
        /// @param auth_rpc_client Reference to the RPC client for logout operations
        /// @param username Username of the user to log out
        static auto logOut(const client_app::auth::AuthRpcClient &auth_rpc_client, const std::string &username) noexcept -> void;

        /// @brief Main task
        /// @param auth_rpc_client Reference to the RPC client for executing tasks
        auto task(const client_app::auth::AuthRpcClient &auth_rpc_client) noexcept -> void;

        /// @brief Create a gRPC channel with custom arguments
        /// @details This function sets up a gRPC channel with keepalive parameters and connects to the server
        /// @return A shared pointer to the created gRPC channel
        [[nodiscard]] auto createChannel() const -> std::shared_ptr<grpc::Channel>;

        /// @brief Create RPC client with gRPC channel
        /// @details This function creates an RPC client using a gRPC channel
        /// @return An RPC client instance
        [[nodiscard]] auto createRpcClient() const -> client_app::auth::AuthRpcClient;

        /// @brief Logs client system information
        /// @details Logs OS version and CPU model to the application log
        static auto logClientInfo() noexcept -> void;

        const std::string glog_config_path_{"../../log/src/config/glog-dev.yml"};
        const std::string application_dev_config_path_{"../../client/src/application-dev.yml"};
        mutable auth::AuthRpcClientOptions rpc_options_;
        mutable common::time::FunctionProfiler timer_;
    };
}
