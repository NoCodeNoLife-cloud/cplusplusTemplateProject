/**
 * @file IRunnable.hpp
 * @brief Interface for runnable tasks (analogous to java.lang.Runnable)
 * @description Defines the contract for objects that can be executed as tasks.
 *          Implementations provide a run() method containing the task logic.
 *          Usable with thread pools and executors.
 */

#pragma once

namespace common::interface::task
{
    /// @brief Interface for a runnable task that can be executed with arguments and return a result.
    /// @tparam ReturnType The type of the value returned by the task.
    /// @tparam Args The types of the arguments that the task accepts.
    template <typename ReturnType, typename... Args>
    class IRunnable
    {
    public:
        virtual ~IRunnable() = default;

        /// @brief Runs the task with given arguments and returns a value of ReturnType.
        /// @param args Arguments to pass to the task.
        /// @return ReturnType The result of the task execution.
        [[nodiscard]] virtual ReturnType run(Args... args) = 0;
    };

    /// @brief Specialization for void return type
    template <typename... Args>
    class IRunnable<void, Args...>
    {
    public:
        virtual ~IRunnable() = default;

        /// @brief Runs the task with given arguments (void return type specialization)
        /// @param args Arguments to pass to the task.
        virtual void run(Args... args) = 0;
    };
}
