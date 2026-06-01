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
#include "config/GLogConfigurator.hpp"

namespace server_app::task
{
    ServerTask::ServerTask()
    {
        grpc_options_.deserializedFromYamlFile(config::ConfigParam::getInstance().applicationDevConfigPath());
        DLOG(INFO) << fmt::format("gRPC configuration loaded successfully - Max Connection Idle: {}ms, Max Connection Age: {}ms, Keepalive Time: {}ms, Keepalive Timeout: {}ms, Permit Without Calls: {}, Server Address: {}", grpc_options_.maxConnectionIdleMs(), grpc_options_.maxConnectionAgeMs(), grpc_options_.keepaliveTimeMs(), grpc_options_.keepaliveTimeoutMs(), grpc_options_.keepalivePermitWithoutCalls(), grpc_options_.serverAddress());
    }

    ServerTask::ServerTask(ServerTask&&) noexcept = default;

    void ServerTask::init()
    {
        const glog::config::GLogConfigurator log_configurator{config::ConfigParam::getInstance().applicationDevConfigPath()};
        log_configurator.execute();
        DLOG(INFO) << fmt::format("Initializing ServerTask with config path: {}, loading gRPC configuration from: {}", config::ConfigParam::getInstance().applicationDevConfigPath(), config::ConfigParam::getInstance().applicationDevConfigPath());
    }

    void ServerTask::run()
    {
        try
        {
            init();
        }
        catch (const std::exception& e)
        {
            LOG(ERROR) << fmt::format("Failed to initialize ServerTask: {}", e.what());
            exit();
            return;
        }
        catch (...)
        {
            LOG(ERROR) << "Failed to initialize ServerTask: Unknown error";
            exit();
            return;
        }

        if (!establishGrpcConnection())
        {
            LOG(ERROR) << "Failed to establish gRPC connection";
            exit();
            return;
        }

        exit();
    }

    void ServerTask::exit() const
    {
        DLOG(INFO) << "Shutting down service task...";
        if (server_)
        {
            DLOG(INFO) << "Initiating gRPC server shutdown";
            server_->Shutdown();
            DLOG(INFO) << "gRPC server shutdown complete.";
        }
        else
        {
            LOG(WARNING) << "Server object is null during shutdown. Nothing to shutdown.";
        }
        DLOG(INFO) << "Service task shutdown complete.";
    }

    [[nodiscard]] bool ServerTask::establishGrpcConnection()
    {
        // Build the server.
        const std::string server_address = grpc_options_.serverAddress();
        DLOG(INFO) << fmt::format("Configuring server to listen on: {}", server_address);
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

        // Set the keepalive parameters.
        DLOG(INFO) << "Setting gRPC server channel arguments";
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_IDLE_MS, grpc_options_.maxConnectionIdleMs());
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_MS, grpc_options_.maxConnectionAgeMs());
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_GRACE_MS, grpc_options_.maxConnectionAgeGraceMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, grpc_options_.keepaliveTimeMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, grpc_options_.keepaliveTimeoutMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, grpc_options_.keepalivePermitWithoutCalls());

        DLOG(INFO) << fmt::format("Channel arguments set - Max Connection Idle: {}ms, Max Connection Age: {}ms, Max Connection Age Grace: {}ms, Keepalive Time: {}ms, Keepalive Timeout: {}ms, Keepalive Permit Without Calls: {}", grpc_options_.maxConnectionIdleMs(), grpc_options_.maxConnectionAgeMs(), grpc_options_.maxConnectionAgeGraceMs(), grpc_options_.keepaliveTimeMs(), grpc_options_.keepaliveTimeoutMs(), grpc_options_.keepalivePermitWithoutCalls());

        DLOG(INFO) << "Registering RPC service implementation";
        auth::AuthRpcService service("./users.db");
        builder.RegisterService(&service);
        DLOG(INFO) << "Service registered successfully";

        DLOG(INFO) << "Building and starting gRPC server";
        server_ = builder.BuildAndStart();
        if (!server_)
        {
            LOG(ERROR) << "Failed to build and start gRPC server. Server object is null.";
            LOG(ERROR) << "Check server configuration and port availability.";
            return false;
        }

        DLOG(INFO) << fmt::format("Server listening on {}, gRPC server started and waiting for connections...", server_address);
        server_->Wait();

        DLOG(INFO) << "gRPC connection established.";
        return true;
    }
}
