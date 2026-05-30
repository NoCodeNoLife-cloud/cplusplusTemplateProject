/**
 * @file ITask.hpp
 * @brief ITask class declaration
 * @details This header defines the ITask class that provides functionality for Common interface definitions for the framework.
 */

#pragma once

namespace common::interfaces
{
    /**
     * @brief Abstract interface class for tasks
     */
    class ITask
    {
    public:
        /// @brief Virtual destructor for proper cleanup in derived classes
        virtual ~ITask() = default;

        /// @brief Pure virtual function to run the task
        /// @details This function must be implemented by derived classes
        virtual void run() = 0;
    };
}
