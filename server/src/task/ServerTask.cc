/**
 * @file ServerTask.cc
 * @brief Implementation of server task functionality
 * @details This file contains the implementation of ServerTask class methods,
 *          including gRPC server initialization, configuration loading,
 *          channel setup, and application lifecycle management.
 */

#include "task/ServerTask.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

#include "auth/AuthRpcService.hpp"
#include "config/ConfigParam.hpp"
#include <cppforge/glog/config/GLogConfigurator.hpp>

namespace server_app::task
{
    ServerTask::ServerTask()
    {
    }

    ServerTask::ServerTask(ServerTask&&) noexcept = default;

    void ServerTask::init()
    {
        const cppforge::glog::config::GLogConfigurator log_configurator{config::ConfigParam::getInstance().applicationDevConfigPath()};
        log_configurator.execute();
        DLOG(INFO) << fmt::format("Initializing ServerTask with config path: {}", config::ConfigParam::getInstance().applicationDevConfigPath());
    }

    void ServerTask::run()
    {
        try
        {
            init();

            auth::AuthRpcService::init(config::ConfigParam::getInstance().applicationDevConfigPath());
            auth::AuthRpcService::getInstance().start();
        }
        catch (const std::exception& e)
        {
            LOG(ERROR) << fmt::format("Failed to initialize ServerTask: {}", e.what());
        }
        catch (...)
        {
            LOG(ERROR) << "Failed to initialize ServerTask: Unknown error";
        }

        exit();
    }

    void ServerTask::exit() const
    {
        DLOG(INFO) << "Shutting down service task...";
        DLOG(INFO) << "Service task shutdown complete.";
    }
}
