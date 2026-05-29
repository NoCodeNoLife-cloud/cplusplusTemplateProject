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
        virtual auto run() -> void = 0;
    };
}
