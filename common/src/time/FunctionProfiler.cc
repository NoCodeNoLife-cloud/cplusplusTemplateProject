/**
 * @file FunctionProfiler.cc
 * @brief FunctionProfiler class implementation
 * @details This file contains the implementation of the FunctionProfiler class methods for Time and date utilities including profilers and formatters.
 */

#include "src/time/FunctionProfiler.hpp"

#include <fmt/format.h>
#include <chrono>
#include <string>
#include <utility>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <glog/logging.h>

namespace common::time
{
    FunctionProfiler::FunctionProfiler(std::string function_name, const bool autoStart) : function_name_(std::move(function_name))
    {
        if (autoStart)
        {
            recordStart();
        }
    }

    auto FunctionProfiler::recordStart() -> void
    {
        start_ = std::chrono::high_resolution_clock::now();
        started_ = true;
        ended_ = false; // Reset ended flag when starting again
    }

    auto FunctionProfiler::recordEnd() -> void
    {
        end_ = std::chrono::high_resolution_clock::now();
        ended_ = true;
    }

    auto FunctionProfiler::getRunTime() const -> std::string
    {
        if (!started_ || !ended_)
        {
            DLOG(WARNING) << fmt::format("FunctionProfiler getRunTime: {} timing incomplete. Started={}, Ended={}", function_name_, started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTime: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        const auto duration_sec = duration_ms.count() / 1000.0;

        std::ostringstream oss;
        oss << function_name_ << " finished in " << std::fixed << std::setprecision(3) << duration_sec << " s (" << duration_ms.count() << " ms)";
        return oss.str();
    }

    auto FunctionProfiler::getRunTimeMs() const -> double
    {
        if (!started_ || !ended_)
        {
            DLOG(WARNING) << fmt::format("FunctionProfiler getRunTimeMs: {} timing incomplete. Started={}, Ended={}", function_name_, started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTimeMs: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        return duration_ms.count();
    }

    auto FunctionProfiler::getRunTimeSec() const -> double
    {
        if (!started_ || !ended_)
        {
            DLOG(WARNING) << fmt::format("FunctionProfiler getRunTimeSec: {} timing incomplete. Started={}, Ended={}", function_name_, started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTimeSec: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_sec = std::chrono::duration<double>(end_ - start_);
        return duration_sec.count();
    }
}
