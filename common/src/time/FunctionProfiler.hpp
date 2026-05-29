/**
 * @file FunctionProfiler.hpp
 * @brief FunctionProfiler class declaration
 * @details This header defines the FunctionProfiler class that provides functionality for Time and date utilities including profilers and formatters.
 */

#pragma once

#include <chrono>
#include <string>

namespace common::time
{
    /// @brief A utility class for profiling function execution time.
    /// @details This class provides functionality to measure and print the execution time
    /// of a function. It records the start and end times, and calculates the
    /// duration between them. The profiler can be used to measure performance
    /// of specific code sections.
    class FunctionProfiler
    {
    public:
        /// @brief Construct a FunctionProfiler with the specified function name.
        /// @param function_name The name of the function or code section being profiled.
        /// @param autoStart If true, automatically start timing upon construction.
        explicit FunctionProfiler(std::string function_name, bool autoStart = false);

        /// @brief Records the start time of the function execution.
        /// @details Captures the current time as the start time for profiling.
        auto recordStart() -> void;

        /// @brief Records the end time of the function execution.
        /// @details Captures the current time as the end time for profiling.
        auto recordEnd() -> void;

        /// @brief Gets the runtime as a formatted string.
        /// @details Calculates the duration between start and end times and returns it as a human-readable string.
        /// @return Formatted string with execution time (e.g., "functionName finished in 0.123 s (123.456 ms)").
        /// @throws std::runtime_error If timing data is incomplete.
        [[nodiscard]] auto getRunTime() const -> std::string;

        /// @brief Gets the runtime duration in milliseconds.
        /// @details Calculates the duration between start and end times.
        /// @return The duration in milliseconds as a double.
        /// @throws std::runtime_error If timing data is incomplete.
        [[nodiscard]] auto getRunTimeMs() const -> double;

        /// @brief Gets the runtime duration in seconds.
        /// @return The duration in seconds as a double.
        /// @throws std::runtime_error If timing data is incomplete.
        [[nodiscard]] auto getRunTimeSec() const -> double;

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_{};
        std::chrono::time_point<std::chrono::high_resolution_clock> end_{};
        std::string function_name_{};
        bool started_{false};
        bool ended_{false};
    };
}
