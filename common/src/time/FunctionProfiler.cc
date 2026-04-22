#include "src/time/FunctionProfiler.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <chrono>
#include <string>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace common::time {
    FunctionProfiler::FunctionProfiler(std::string function_name, const bool autoStart) : function_name_(std::move(function_name)) {
        DLOG(INFO) << fmt::format("FunctionProfiler created - function: {}, autoStart={}", function_name_, autoStart);
        if (autoStart) {
            recordStart();
        }
    }

    auto FunctionProfiler::recordStart() -> void {
        start_ = std::chrono::high_resolution_clock::now();
        started_ = true;
        ended_ = false; // Reset ended flag when starting again
        DLOG(INFO) << fmt::format("FunctionProfiler recordStart - function: {}", function_name_);
    }

    auto FunctionProfiler::recordEnd(const bool autoPrint) -> void {
        end_ = std::chrono::high_resolution_clock::now();
        ended_ = true;
        DLOG(INFO) << fmt::format("FunctionProfiler recordEnd - function: {}, autoPrint={}", function_name_, autoPrint);
        if (autoPrint) {
            getRunTime();
        }
    }

    auto FunctionProfiler::getRunTime() const -> void {
        if (!started_ || !ended_) {
            DLOG(ERROR) << fmt::format("FunctionProfiler getRunTime failed - incomplete timing data. Started: {}, Ended: {}", started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTime: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        const auto duration_sec = duration_ms.count() / 1000.0;

        std::cout << function_name_ << " finished in " << std::fixed << std::setprecision(3) << duration_sec << " s (" << duration_ms.count() << " ms)" << std::endl;
        DLOG(INFO) << fmt::format("FunctionProfiler getRunTime - function: {} took {:.3f}s ({:.3f}ms)", function_name_, duration_sec, duration_ms.count());
    }

    auto FunctionProfiler::getRunTimeMs() const -> double {
        if (!started_ || !ended_) {
            DLOG(ERROR) << fmt::format("FunctionProfiler getRunTimeMs failed - incomplete timing data. Started: {}, Ended: {}", started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTimeMs: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_ms = std::chrono::duration<double, std::milli>(end_ - start_);
        DLOG(INFO) << fmt::format("FunctionProfiler getRunTimeMs - function: {} took {:.3f}ms", function_name_, duration_ms.count());
        return duration_ms.count();
    }

    auto FunctionProfiler::getRunTimeSec() const -> double {
        if (!started_ || !ended_) {
            DLOG(ERROR) << fmt::format("FunctionProfiler getRunTimeSec failed - incomplete timing data. Started: {}, Ended: {}", started_, ended_);
            throw std::runtime_error("FunctionProfiler::getRunTimeSec: " + function_name_ + " timing data is incomplete. Started: " + (started_ ? "true" : "false") + ", Ended: " + (ended_ ? "true" : "false"));
        }

        const auto duration_sec = std::chrono::duration<double>(end_ - start_);
        DLOG(INFO) << fmt::format("FunctionProfiler getRunTimeSec - function: {} took {:.6f}s", function_name_, duration_sec.count());
        return duration_sec.count();
    }
}
