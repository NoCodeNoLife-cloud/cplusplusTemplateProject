/**
 * @file ITask.hpp
 * @brief Generic task interface with name, status, and cancel support
 * @description Defines the contract for managed tasks: getTaskName(),
 *          getStatus() (PENDING, RUNNING, COMPLETED, FAILED, CANCELLED),
 *          cancel(), and getResult().  Used by task schedulers and executors
 *          for tracking and controlling task execution.
 */

#pragma once

namespace cppforge::interface::task
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
