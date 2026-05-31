/**
 * @file Clock.hpp
 * @brief Clock class declaration
 * @details This header defines the Clock class that provides functionality for Time and date utilities including profilers and formatters.
 */

#pragma once
#include <chrono>
#include <string>

namespace common::time
{
    /// @brief Utility class for time-related operations
    class Clock
    {
    public:
        Clock() = delete;

        /// @brief Get current date and time as a string in format "YYYY-MM-DD HH:MM:SS"
        /// @return Formatted date and time string
        [[nodiscard]] static std::string getCurrentDateTimeString() noexcept;

        /// @brief Get current date and time as a compressed string in format "YYYYMMDD_HHMMSS"
        /// @return Compressed date and time string
        [[nodiscard]] static std::string getCompressedCurrentDateTimeString() noexcept;

        /// @brief Get current timestamp as a count of milliseconds since epoch
        /// @return Milliseconds since epoch
        [[nodiscard]] static std::chrono::milliseconds::rep getCurrentTimestampMs() noexcept;

        /// @brief Get current timestamp as a count of seconds since epoch
        /// @return Seconds since epoch
        [[nodiscard]] static std::chrono::seconds::rep getCurrentTimestamp() noexcept;

        /// @brief Format current time with a custom format string
        /// @param format Format string using strftime-compatible format specifiers
        /// @return Formatted date and time string
        [[nodiscard]] static std::string formatCurrentTime(const std::string& format);
    };
}