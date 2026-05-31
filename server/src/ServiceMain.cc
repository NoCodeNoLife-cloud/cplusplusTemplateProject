/**
 * @file ServiceMain.cc
 * @brief Server application entry point
 * @details Main function that initializes and runs the server task,
 *          serving as the starting point for the server application.
 */

#include <glog/logging.h>

#include "src/task/ServerTask.hpp"

int32_t main(const int32_t, char*[])
{
    server_app::task::ServerTask service_task("server");
    service_task.run();
}