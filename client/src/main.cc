/**
 * @file main.cc
 * @brief Client application entry point
 * @details Main function that initializes and runs the client task,
 *          serving as the starting point for the client application.
 */

#include <glog/logging.h>

#include "task/ClientTask.hpp"

int32_t main(const int32_t, char*[])
{
    try
    {
        client_app::task::ClientTask main_task("client");
        main_task.run();
    }
    catch (const std::exception& ex)
    {
        DLOG(ERROR) << ex.what();
    }
    catch (...)
    {
        DLOG(ERROR) << "Program terminated due to an unrecognized exception.";
    }
}
