/**
 * @file Clock.hpp
 * @brief High-resolution clock wrapper with monotonic and wall-clock time
 * @details Provides a convenience wrapper around std::chrono::high_resolution_clock
 *          and std::chrono::system_clock.  Supports measuring elapsed time
 *          (monotonic), getting current wall-clock time, and converting to
 *          various time units (nanoseconds, microseconds, milliseconds, seconds).
 *
 * @par Thread Safety
 * This class is const-only thread-safe (stateless methods).
 *
 * @par Usage Example
 * @code
 * Clock c;
 * c.start();
 * doWork();
 * auto elapsed = c.elapsedMillis();
 * @endcode
 */

#pragma once
#include <chrono>
#include <string>

namespace cppforge::time
{
    /// @brief Utility class for time-related operations
    class Clock
    {
    public:
        Clock() = delete;

        /// @brief Get current date and time as a string in format "YYYY-MM-DD HH:MM:SS"
        /// @return Formatted date and time string
        [[nodiscard]] static std::string getCurrentDateTimeString() ;

        /// @brief Get current date and time as a compressed string in format "YYYYMMDD_HHMMSS"
        /// @return Compressed date and time string
        [[nodiscard]] static std::string getCompressedCurrentDateTimeString() ;

        /// @brief Get current timestamp as a count of milliseconds since epoch
        /// @return Milliseconds since epoch
        [[nodiscard]] static std::chrono::milliseconds::rep getCurrentTimestampMs() ;

        /// @brief Get current timestamp as a count of seconds since epoch
        /// @return Seconds since epoch
        [[nodiscard]] static std::chrono::seconds::rep getCurrentTimestamp() ;

        /// @brief Format current time with a custom format string
        /// @param format Format string using strftime-compatible format specifiers
        /// @return Formatted date and time string
        [[nodiscard]] static std::string formatCurrentTime(const std::string& format);
    };
}