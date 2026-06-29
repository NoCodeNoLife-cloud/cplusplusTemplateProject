/**
 * @file ServerTask.hpp
 * @brief Server task implementation for the application
 * @details This header defines the ServerTask class that manages the server application
 *          lifecycle including initialization, gRPC server setup, configuration loading,
 *          and cleanup with performance profiling support.
 */

#pragma once
#include <memory>

#include "interface/task/ITask.hpp"

namespace server_app::task
{
    /// @brief ServerTask is responsible for managing the main service loop
    /// @details This class coordinates various subsystems within the application server,
    /// initializes the gRPC server, loads configurations, and manages the server lifecycle.
    class ServerTask final : public cppforge::interface::task::ITask
    {
    public:
        /// @brief Construct a ServerTask with the specified name
        explicit ServerTask();

        /// @brief Copy constructor deleted to prevent copying
        ServerTask(const ServerTask&) = delete;

        /// @brief Move constructor
        ServerTask(ServerTask&&) noexcept;

        /// @brief Copy assignment operator deleted to prevent copying
        ServerTask& operator=(const ServerTask&) = delete;

        /// @brief Move assignment operator
        ServerTask& operator=(ServerTask&&) = delete;

        /// @brief Initialize the service task and its associated resources
        /// @details Sets up logging, loads configuration, and validates gRPC parameters
        static void init();

        /// @brief Run the main task
        /// @details Initializes the server, establishes gRPC connection, and starts listening
        void run() override;

        /// @brief Exit the service task and clean up resources
        /// @details Shuts down the gRPC server and performs cleanup operations
        void exit() const;
    };
}
