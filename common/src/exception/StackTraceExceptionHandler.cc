/**
 * @file StackTraceExceptionHandler.cc
 * @brief Implementation of the StackTraceExceptionHandler utility class
 */

#include "exception/StackTraceExceptionHandler.hpp"

#include <glog/logging.h>

namespace common::exception
{
    void StackTraceExceptionHandler::logException(const std::exception& ex, const std::string& prefix)
    {
        if (prefix.empty())
        {
            LOG(ERROR) << ex.what() << "\nStack trace:\n" << std::to_string(std::stacktrace::current());
        }
        else
        {
            LOG(ERROR) << prefix << ": " << ex.what() << "\nStack trace:\n" << std::to_string(std::stacktrace::current());
        }
    }

    void StackTraceExceptionHandler::logUnknownException(const std::string& prefix)
    {
        if (prefix.empty())
        {
            LOG(ERROR) << "Program terminated due to an unrecognized exception.\nStack trace:\n" << std::to_string(std::stacktrace::current());
        }
        else
        {
            LOG(ERROR) << prefix << ": Program terminated due to an unrecognized exception.\nStack trace:\n" << std::to_string(std::stacktrace::current());
        }
    }
}
