/**
 * @file main.cc
 * @brief Server application entry point
 * @details Main function that initializes and runs the server task,
 *          serving as the starting point for the server application.
 */

#include <glog/logging.h>

#include "toolkit/StackTraceExceptionHandler.hpp"
#include "task/ServerTask.hpp"

int32_t main(const int32_t argc, char* argv[])
{
    try
    {
        server_app::task::ServerTask service_task;
        service_task.run();
        return 0;
    }
    catch (const std::exception& ex)
    {
        cppforge::toolkit::StackTraceExceptionHandler::logException(ex);
    }
    catch (...)
    {
        cppforge::toolkit::StackTraceExceptionHandler::logUnknownException();
    }
    return 1;
}
