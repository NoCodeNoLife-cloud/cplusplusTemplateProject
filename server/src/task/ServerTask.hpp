#pragma once
#include <memory>
#include <string>
#include <grpcpp/server_builder.h>

#include "src/auth/AuthRpcServiceOptions.hpp"
#include "src/time/FunctionProfiler.hpp"
#include "task/interface/ITask.h"

namespace server_app::task {
    /// @brief ServerTask is responsible for managing the main service loop
    /// @details This class coordinates various subsystems within the application server,
    /// initializes the gRPC server, loads configurations, and manages the server lifecycle.
    class ServerTask final : public common::interfaces::ITask {
    public:
        /// @brief Construct a ServerTask with the specified name
        /// @param name The name of the server task for profiling purposes
        explicit ServerTask(std::string name) noexcept;

        /// @brief Copy constructor deleted to prevent copying
        ServerTask(const ServerTask &) = delete;

        /// @brief Move constructor
        ServerTask(ServerTask &&) noexcept;

        /// @brief Copy assignment operator deleted to prevent copying
        auto operator=(const ServerTask &) -> ServerTask & = delete;

        /// @brief Move assignment operator
        auto operator=(ServerTask &&) -> ServerTask & = delete;

        /// @brief Initialize the service task and its associated resources
        /// @details Sets up logging, loads configuration, and validates gRPC parameters
        auto init() -> void;

        /// @brief Run the main task
        /// @details Initializes the server, establishes gRPC connection, and starts listening
        auto run() -> void override;

        /// @brief Exit the service task and clean up resources
        /// @details Shuts down the gRPC server and performs cleanup operations
        auto exit() const -> void;

    private:
        const std::string glog_config_path_{"../../log/src/config/glog-dev.yml"};
        const std::string application_dev_config_path_{"../../server/src/application-dev.yml"};
        app_server::auth::AuthRpcServiceOptions grpc_options_;
        common::time::FunctionProfiler timer_;
        std::unique_ptr<grpc::Server> server_;

        /// @brief Establish a gRPC connection to the specified service
        /// @details Configures and starts the gRPC server with specified options
        [[nodiscard]] auto establishGrpcConnection() -> bool;
    };
}
