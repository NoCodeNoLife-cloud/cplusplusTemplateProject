/**
 * @file StackTraceExceptionHandler.cc
 * @brief Implementation of the StackTraceExceptionHandler utility class
 */

#include "toolkit/StackTraceExceptionHandler.hpp"

#include <glog/logging.h>

namespace common::toolkit
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
