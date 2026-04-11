#pragma once

namespace common::service::interfaces {
    /// @brief Interface for startup tasks that need to be executed when the service starts.
    /// Startup tasks are used to perform initialization operations that are required
    /// for the service to function properly. These tasks are executed in the order
    /// they are registered.
    class IStartupTask {
    public:
        virtual ~IStartupTask() = default;

        /// @brief Executes the startup task.
        /// @return True if the task was executed successfully, false otherwise.
        /// @throws std::runtime_error if execution fails with details about the failure
        [[nodiscard]] virtual auto execute() -> bool = 0;
    };
}
