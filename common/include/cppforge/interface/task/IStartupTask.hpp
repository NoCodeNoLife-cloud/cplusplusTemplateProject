/**
 * @file IStartupTask.hpp
 * @brief Interface for tasks executed during application startup
 * @description Defines the contract for initialisation tasks that run during
 *          application startup.  Supports ordered execution via a priority
 *          value.  Implementations perform setup actions like loading config,
 *          connecting to databases, or registering services.
 */

#pragma once

namespace cppforge::interface::task
{
    /// @brief Interface for startup tasks that need to be executed when the service starts.
    /// Startup tasks are used to perform initialization operations that are required
    /// for the service to function properly. These tasks are executed in the order
    /// they are registered.
    class IStartupTask
    {
    public:
        virtual ~IStartupTask() = default;

        /// @brief Executes the startup task.
        /// @return True if the task was executed successfully, false otherwise.
        /// @throws std::runtime_error if execution fails with details about the failure
        [[nodiscard]] virtual bool execute() = 0;
    };
}
