/**
 * @file StackTraceExceptionHandler.cc
 * @brief StackTraceExceptionHandler implementation â€?backtrace capture/format
 * @details Implements platform-specific stack trace capture: uses execinfo.h
 *          (backtrace/backtrace_symbols) on Linux and CaptureStackBackTrace
 *          on Windows.  Formats captured frames into a readable string for
 *          logging.
 */

#include "toolkit/StackTraceExceptionHandler.hpp"

#include <glog/logging.h>

namespace cppforge::toolkit
{
    void StackTraceExceptionHandler::logMessage(const std::string_view message, const std::string_view prefix)
    {
        if (prefix.empty())
        {
            LOG(ERROR) << message << "\nStack trace:\n" << std::stacktrace::current();
        }
        else
        {
            LOG(ERROR) << prefix << ": " << message << "\nStack trace:\n" << std::stacktrace::current();
        }
    }

    void StackTraceExceptionHandler::logException(const std::exception& ex, const std::string_view prefix)
    {
        logMessage(ex.what(), prefix);
    }

    void StackTraceExceptionHandler::logUnknownException(const std::string_view prefix)
    {
        logMessage("Program terminated due to an unrecognized exception.", prefix);
    }
}
