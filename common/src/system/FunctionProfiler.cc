/**
 * @file FunctionProfiler.cc
 * @brief FunctionProfiler class implementation
 * @details This file contains the implementation of the FunctionProfiler class methods for Time and date utilities including profilers and formatters.
 */

#include "system/FunctionProfiler.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::system
{
    FunctionProfiler::FunctionProfiler(std::string function_name, const bool autoStart) : function_name_(std::move(function_name))
    {
        if (autoStart)
        {
            recordStart();
        }
    }

    void FunctionProfiler::recordStart()
    {
        start_ = std::chrono::high_resolution_clock::now();
        started_ = true;
        ended_ = false; // Reset ended flag when starting again
    }

    void FunctionProfiler::recordEnd()
    {
        end_ = std::chrono::high_resolution_clock::now();
        ended_ = true;
    }

    void FunctionProfiler::assertTimingComplete() const
    {
        if (started_ && ended_)
        {
            return;
        }

        DLOG(WARNING) << fmt::format("FunctionProfiler: {} timing incomplete. started={}, ended={}", function_name_, started_, ended_);
        throw std::runtime_error(
            fmt::format("FunctionProfiler:: timing data is incomplete for '{}'. started={}, ended={}",
                        function_name_, started_, ended_));
    }

    std::string FunctionProfiler::getRunTime() const
    {
        assertTimingComplete();

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        const auto duration_sec = duration_ms.count() / 1000.0;

        return fmt::format("{} finished in {:.3f} s ({:.3f} ms)", function_name_, duration_sec, duration_ms.count());
    }

    double FunctionProfiler::getRunTimeMs() const
    {
        assertTimingComplete();

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        return duration_ms.count();
    }

    double FunctionProfiler::getRunTimeSec() const
    {
        assertTimingComplete();

        const auto duration_sec = std::chrono::duration<double>(end_ - start_);
        return duration_sec.count();
    }
}
