/**
 * @file FunctionProfilerAspect.cc
 * @brief FunctionProfilerAspect class implementation
 * @details This file contains the implementation of the FunctionProfilerAspect class methods for Aspect-Oriented Programming support.
 */

#include "aop/FunctionProfilerAspect.hpp"

#include <utility>
#include <glog/logging.h>

namespace common::aop
{
    FunctionProfilerAspect::FunctionProfilerAspect(std::string function_name) : profiler_(function_name, true), function_name_(std::move(function_name))
    {
        DLOG(INFO) << "FunctionProfilerAspect created for: " << function_name_;
    }

    void FunctionProfilerAspect::onEntry()
    {
        // Removed frequent entry logging to avoid spam
        // DLOG(INFO) << "Entering function: " << function_name_;
    }

    void FunctionProfilerAspect::onExit()
    {
        profiler_.recordEnd();
        const auto time_info = profiler_.getRunTime();
        // Only log if execution time is significant (> 100ms) to reduce noise
        const double elapsed_ms = profiler_.getRunTimeMs();
        if (elapsed_ms > 100.0)
        {
            DLOG(INFO) << "Exiting function: " << function_name_ << ", " << time_info;
        }
        onProfileComplete(time_info);
    }

    void FunctionProfilerAspect::onException(std::exception_ptr e)
    {
        profiler_.recordEnd();
        const auto time_info = profiler_.getRunTime();
        DLOG(WARNING) << "Exception in function: " << function_name_ << ", " << time_info;
        onProfileComplete(time_info);
    }

    void FunctionProfilerAspect::onProfileComplete(const std::string& time_info)
    {
        // Default implementation does nothing
        // Subclasses can override to log, send to metrics, etc.
    }
}
