#pragma once
#include <string>
#include <exception>

#include "src/time/FunctionProfiler.hpp"
#include "src/interface/IAopAspect.hpp"

namespace common::aop {
    /// @brief This class is an aspect for profiling function execution time.
    /// @details It implements the IAopAspect interface to provide entry and exit points
    /// for measuring the duration of function calls. This aspect can be used
    /// with the AOP framework to automatically profile function execution.
    class FunctionProfilerAspect : public interfaces::IAopAspect<FunctionProfilerAspect> {
    public:
        /// @brief Construct a FunctionProfilerAspect with the given function name
        /// @param function_name The name of the function to profile
        explicit FunctionProfilerAspect(std::string function_name);

        /// @brief Entry point - records the start time of the function
        /// @details Called when entering the function to be profiled
        auto onEntry() -> void override;

        /// @brief Exit point - records the end time of the function and prints the result
        /// @details Called when exiting the function to be profiled
        auto onExit() -> void override;

        /// @brief Exception point - records the end time when exception occurs
        /// @details Called when function exits with exception
        auto onException(std::exception_ptr e) -> void override;

    private:
        time::FunctionProfiler profiler_;
        std::string function_name_;
    };
}
