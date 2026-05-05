#include "src/aop/FunctionProfilerAspect.hpp"
#include <utility>

namespace common::aop {
    FunctionProfilerAspect::FunctionProfilerAspect(std::string function_name) : profiler_(function_name, true), function_name_(std::move(function_name)) {
    }

    auto FunctionProfilerAspect::onEntry() -> void {
    }

    auto FunctionProfilerAspect::onExit() -> void {
        profiler_.recordEnd();
        const auto time_info = profiler_.getRunTime();
        onProfileComplete(time_info);
    }

    auto FunctionProfilerAspect::onException(std::exception_ptr e) -> void {
        profiler_.recordEnd();
        const auto time_info = profiler_.getRunTime();
        onProfileComplete(time_info);
    }

    auto FunctionProfilerAspect::onProfileComplete(const std::string &time_info) -> void {
        // Default implementation does nothing
        // Subclasses can override to log, send to metrics, etc.
    }
}
