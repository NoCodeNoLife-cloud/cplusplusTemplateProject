/**
 * @file FunctionProfilerAspect.hpp
 * @brief AOP aspect for automatic function profiling via RAII
 * @description Implements a cross-cutting profiling aspect using RAII.
 *          When applied to a function scope, automatically records execution
 *          time and logs slow invocations.  Can be selectively enabled/disabled
 *          at runtime without modifying the target function code.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  Each thread requires its own aspect
 * instance for correct timing.
 */

#pragma once
#include <exception>
#include <string>

#include <cppforge/interface/IAopAspect.hpp>
#include <cppforge/system/FunctionProfiler.hpp>

namespace cppforge::aop
{
    /// @brief This class is an aspect for profiling function execution time.
    /// @details It implements the IAopAspect interface to provide entry and exit points
    /// for measuring the duration of function calls. This aspect can be used
    /// with the AOP framework to automatically profile function execution.
    /// Subclasses can override onProfileComplete() to customize how timing info is handled.
    class FunctionProfilerAspect : public interface::aop::IAopAspect<FunctionProfilerAspect>
    {
    public:
        /// @brief Construct a FunctionProfilerAspect with the given function name
        /// @param function_name The name of the function to profile
        explicit FunctionProfilerAspect(std::string function_name);

        /// @brief Entry point - records the start time of the function
        /// @details Called when entering the function to be profiled
        void onEntry() override;

        /// @brief Exit point - records the end time of the function
        /// @details Called when exiting the function to be profiled
        void onExit() override;

        /// @brief Exception point - records the end time when exception occurs
        /// @details Called when function exits with exception
        void onException(std::exception_ptr e) override;

    protected:
        /// @brief Called when profiling is complete, subclasses can override to handle timing info
        /// @param time_info Formatted string with execution time information
        /// @details Default implementation does nothing. Override to log, send to metrics, etc.
        virtual void onProfileComplete(const std::string& time_info);

    private:
        static constexpr double kSlowThresholdMs = 100.0;

        system::FunctionProfiler profiler_;
        std::string function_name_;
    };
}
