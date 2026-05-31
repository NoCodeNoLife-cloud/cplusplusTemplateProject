/**
 * @file ITimerTask.hpp
 * @brief ITimerTask class declaration
 * @details This header defines the ITimerTask class that provides functionality for Common interface definitions for the framework.
 */

#pragma once

namespace common::interfaces
{
    /// @brief Interface for timer tasks that can be executed by a timer.
    /// This interface defines the contract for tasks that can be scheduled
    /// and executed by a timer. Implementations of this interface should
    /// provide the logic for what should happen when the timer triggers
    /// the task execution.
    class ITimerTask
    {
    public:
        virtual ~ITimerTask() = default;

        /// @brief Executes the timer task.
        virtual void execute() = 0;
    };
}