/**
 * @file Clock.cc
 * @brief Clock class implementation
 * @details This file contains the implementation of the Clock class methods for Time and date utilities including profilers and formatters.
 */

#include "time/Clock.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
    std::string formatTime(std::time_t time, const std::string& format)
    {
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
}

namespace common::time
{
    std::string Clock::getCurrentDateTimeString()
    {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, "%Y-%m-%d %H:%M:%S");
        return result;
    }

    std::string Clock::getCompressedCurrentDateTimeString()
    {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, "%Y%m%d_%H%M%S");
        return result;
    }

    std::chrono::milliseconds::rep Clock::getCurrentTimestampMs()
    {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return timestamp;
    }

    std::chrono::seconds::rep Clock::getCurrentTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return timestamp;
    }

    std::string Clock::formatCurrentTime(const std::string& format)
    {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto result = formatTime(time_t, format);
        return result;
    }
}
