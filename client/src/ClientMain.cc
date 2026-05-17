/**
 * @file ClientMain.cc
 * @brief Client application entry point
 * @details Main function that initializes and runs the client task,
 *          serving as the starting point for the client application.
 */

#include <glog/logging.h>

#include "src/task/ClientTask.hpp"

auto main(const int32_t, char*[]) -> int32_t {
    client_app::task::ClientTask main_task("client");
    main_task.run();
}