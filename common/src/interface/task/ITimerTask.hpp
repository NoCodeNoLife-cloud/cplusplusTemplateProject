/**
 * @file ITimerTask.hpp
 * @brief Interface for tasks scheduled by a timer (one-shot or periodic)
 * @description Defines the contract for timer-scheduled tasks.  Implementations
 *          provide a run() method that is invoked when the timer fires.
 *          Supports both one-shot execution and periodic execution with
 *          configurable interval.
 */

#pragma once

namespace common::interface::task
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
