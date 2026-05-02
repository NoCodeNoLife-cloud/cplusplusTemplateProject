#pragma once

namespace common::interfaces {
    /// @brief Interface for a runnable task that can be executed with arguments and return a result.
    /// @tparam ReturnType The type of the value returned by the task.
    /// @tparam Args The types of the arguments that the task accepts.
    template<typename ReturnType, typename... Args>
    class IRunnable {
    public:
        virtual ~IRunnable() = default;

        /// @brief Runs the task with given arguments and returns a value of ReturnType.
        /// @param args Arguments to pass to the task.
        /// @return ReturnType The result of the task execution.
        [[nodiscard]] virtual auto run(Args... args) -> ReturnType = 0;
    };

    /// @brief Specialization for void return type
    template<typename... Args>
    class IRunnable<void, Args...> {
    public:
        virtual ~IRunnable() = default;

        /// @brief Runs the task with given arguments (void return type specialization)
        /// @param args Arguments to pass to the task.
        virtual auto run(Args... args) -> void = 0;
    };
}
