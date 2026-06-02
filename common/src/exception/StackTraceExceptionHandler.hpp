/**
 * @file StackTraceExceptionHandler.hpp
 * @brief Utility class for logging exceptions with stack traces
 * @details Provides static methods to log exceptions along with C++23 std::stacktrace
 *          information, reducing boilerplate in catch blocks.
 */

#pragma once
#include <stacktrace>
#include <string>

namespace common::exception
{
    /// @brief Utility class for logging exceptions with stack trace information
    class StackTraceExceptionHandler final
    {
    public:
        StackTraceExceptionHandler() = delete;

        /// @brief Log a caught std::exception with its stack trace
        /// @param ex The caught exception
        /// @param prefix Optional prefix message
        static void logException(const std::exception& ex, const std::string& prefix = "");

        /// @brief Log a caught unknown exception with stack trace
        /// @param prefix Optional prefix message
        static void logUnknownException(const std::string& prefix = "");
    };
}
