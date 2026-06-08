/**
 * @file main.cc
 * @brief Client application entry point
 * @details Main function that initializes and runs the client task,
 *          serving as the starting point for the client application.
 */

#include <glog/logging.h>

#include "toolkit/StackTraceExceptionHandler.hpp"
#include "task/ClientTask.hpp"

int32_t main(const int32_t argc, char* argv[])
{
    try
    {
        client_app::task::ClientTask main_task("client");
        main_task.run();
        return 0;
    }
    catch (const std::exception& ex)
    {
        common::toolkit::StackTraceExceptionHandler::logException(ex);
    }
    catch (...)
    {
        common::toolkit::StackTraceExceptionHandler::logUnknownException();
    }
    return 1;
}
