#include "src/time/Clock.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace common::time {
    // Helper function to format time using strftime in a thread-safe way
    static auto formatTime(std::time_t time, const std::string &format) -> std::string {
        std::tm tm_time{};
#ifdef _WIN32
        localtime_s(&tm_time, &time); // Windows-specific thread-safe function
#else
        localtime_r(&time, &tm_time); // POSIX-specific thread-safe function
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm_time, format.c_str());
        return oss.str();
    }

    auto Clock::getCurrentDateTimeString() noexcept -> std::string {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, "%Y-%m-%d %H:%M:%S");
        DLOG(INFO) << fmt::format("Clock getCurrentDateTimeString - formatted: {}", result);
        return result;
    }

    auto Clock::getCompressedCurrentDateTimeString() noexcept -> std::string {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, "%Y%m%d_%H%M%S");
        DLOG(INFO) << fmt::format("Clock getCompressedCurrentDateTimeString - formatted: {}", result);
        return result;
    }

    auto Clock::getCurrentTimestampMs() noexcept -> std::chrono::milliseconds::rep {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        DLOG(INFO) << fmt::format("Clock getCurrentTimestampMs - timestamp: {}ms", timestamp);
        return timestamp;
    }

    auto Clock::getCurrentTimestamp() noexcept -> std::chrono::seconds::rep {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        DLOG(INFO) << fmt::format("Clock getCurrentTimestamp - timestamp: {}s", timestamp);
        return timestamp;
    }

    auto Clock::formatCurrentTime(const std::string &format) -> std::string {
        DLOG(INFO) << fmt::format("Clock formatCurrentTime - using format: {}", format);
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, format);
        DLOG(INFO) << fmt::format("Clock formatCurrentTime - formatted: {}", result);
        return result;
    }
}
