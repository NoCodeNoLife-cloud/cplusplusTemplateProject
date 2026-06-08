/**
 * @file StackTraceExceptionHandler.hpp
 * @brief Utility class for logging exceptions with stack traces
 * @details Provides static methods to log exceptions along with C++23 std::stacktrace
 *          information, reducing boilerplate in catch blocks.
 */

#pragma once
#include <exception>
#include <stacktrace>
#include <string_view>

namespace common::toolkit
{
    /// @brief Utility class for logging exceptions with stack trace information
    class StackTraceExceptionHandler final
    {
    public:
        StackTraceExceptionHandler() = delete;

        /// @brief Log a caught std::exception with its stack trace
        /// @param ex The caught exception
        /// @param prefix Optional prefix message
        static void logException(const std::exception& ex, std::string_view prefix = {});

        /// @brief Log a caught unknown exception with stack trace
        /// @param prefix Optional prefix message
        static void logUnknownException(std::string_view prefix = {});

    private:
        /// @brief Internal helper to output a message with stack trace
        static void logMessage(std::string_view message, std::string_view prefix);
    };
}
