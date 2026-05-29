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
        [[nodiscard]] static auto getCurrentDateTimeString() noexcept -> std::string;

        /// @brief Get current date and time as a compressed string in format "YYYYMMDD_HHMMSS"
        /// @return Compressed date and time string
        [[nodiscard]] static auto getCompressedCurrentDateTimeString() noexcept -> std::string;

        /// @brief Get current timestamp as a count of milliseconds since epoch
        /// @return Milliseconds since epoch
        [[nodiscard]] static auto getCurrentTimestampMs() noexcept -> std::chrono::milliseconds::rep;

        /// @brief Get current timestamp as a count of seconds since epoch
        /// @return Seconds since epoch
        [[nodiscard]] static auto getCurrentTimestamp() noexcept -> std::chrono::seconds::rep;

        /// @brief Format current time with a custom format string
        /// @param format Format string using strftime-compatible format specifiers
        /// @return Formatted date and time string
        [[nodiscard]] static auto formatCurrentTime(const std::string& format) -> std::string;
    };
}
