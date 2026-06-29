/**
 * @file FunctionProfilerAspect.cc
 * @brief FunctionProfilerAspect implementation â€?timer RAII logic
 * @details Implements the AOP profiler aspect: constructor starts a timer,
 *          destructor stops it and logs the elapsed duration if it exceeds
 *          the configurable threshold.
 */

#include "aop/FunctionProfilerAspect.hpp"

#include <utility>
#include <glog/logging.h>

namespace cppforge::aop
{
    FunctionProfilerAspect::FunctionProfilerAspect(std::string function_name) : profiler_(function_name), function_name_(std::move(function_name))
    {
        DLOG(INFO) << "FunctionProfilerAspect created for: " << function_name_;
    }

    void FunctionProfilerAspect::onEntry()
    {
        profiler_.recordStart();
    }

    void FunctionProfilerAspect::onExit()
    {
        profiler_.recordEnd();
        const auto& time_info = profiler_.getRunTime();
        if (const double elapsed_ms = profiler_.getRunTimeMs(); elapsed_ms > kSlowThresholdMs)
        {
            DLOG(INFO) << "Exiting function: " << function_name_ << ", " << time_info << " (" << elapsed_ms << " ms)";
        }
        onProfileComplete(time_info);
    }

    void FunctionProfilerAspect::onException(std::exception_ptr /*e*/)
    {
        profiler_.recordEnd();
        const auto& time_info = profiler_.getRunTime();
        DLOG(WARNING) << "Exception in function: " << function_name_ << ", " << time_info;
        onProfileComplete(time_info);
    }

    void FunctionProfilerAspect::onProfileComplete(const std::string& time_info)
    {
        // Default implementation does nothing
        // Subclass can override to log, send to metrics, etc.
    }
}
