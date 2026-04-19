#include "src/task/ServerTask.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

#include "config/GLogConfigurator.hpp"
#include "src/auth/AuthRpcService.hpp"

namespace app_server::task {
    ServerTask::ServerTask(std::string name) noexcept : timer_(std::move(name)) {
    }

    ServerTask::ServerTask(ServerTask &&) noexcept = default;

    auto ServerTask::init() -> void {
        const glog::config::GLogConfigurator log_configurator{glog_config_path_};
        log_configurator.execute();
        LOG(INFO) << fmt::format("Initializing ServerTask with glog config path: {}, loading gRPC configuration from: {}", glog_config_path_, application_dev_config_path_);

        grpc_options_.deserializedFromYamlFile(application_dev_config_path_);

        LOG(INFO) << fmt::format("gRPC configuration loaded successfully - Max Connection Idle: {}ms, Max Connection Age: {}ms, Keepalive Time: {}ms, Keepalive Timeout: {}ms, Permit Without Calls: {}, Server Address: {}", grpc_options_.maxConnectionIdleMs(), grpc_options_.maxConnectionAgeMs(), grpc_options_.keepaliveTimeMs(), grpc_options_.keepaliveTimeoutMs(), grpc_options_.keepalivePermitWithoutCalls(), grpc_options_.serverAddress());
    }

    auto ServerTask::run() -> void {
        try {
            init();
        } catch (const std::exception &e) {
            LOG(ERROR) << fmt::format("Failed to initialize ServerTask: {}", e.what());
            exit();
            return;
        } catch (...) {
            LOG(ERROR) << "Failed to initialize ServerTask: Unknown error";
            exit();
            return;
        }

        if (!establishGrpcConnection()) {
            LOG(ERROR) << "Failed to establish gRPC connection";
            exit();
            return;
        }

        exit();
    }

    [[nodiscard]] auto ServerTask::establishGrpcConnection() -> bool {
        // Build the server.
        const std::string server_address = grpc_options_.serverAddress();
        LOG(INFO) << fmt::format("Configuring server to listen on: {}", server_address);
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

        // Set the keepalive parameters.
        LOG(INFO) << "Setting gRPC server channel arguments";
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_IDLE_MS, grpc_options_.maxConnectionIdleMs());
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_MS, grpc_options_.maxConnectionAgeMs());
        builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_GRACE_MS, grpc_options_.maxConnectionAgeGraceMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, grpc_options_.keepaliveTimeMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, grpc_options_.keepaliveTimeoutMs());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, grpc_options_.keepalivePermitWithoutCalls());

        LOG(INFO) << fmt::format("Channel arguments set - Max Connection Idle: {}ms, Max Connection Age: {}ms, Max Connection Age Grace: {}ms, Keepalive Time: {}ms, Keepalive Timeout: {}ms, Keepalive Permit Without Calls: {}", grpc_options_.maxConnectionIdleMs(), grpc_options_.maxConnectionAgeMs(), grpc_options_.maxConnectionAgeGraceMs(), grpc_options_.keepaliveTimeMs(), grpc_options_.keepaliveTimeoutMs(), grpc_options_.keepalivePermitWithoutCalls());

        LOG(INFO) << "Registering RPC service implementation";
        server_app::auth::AuthRpcService service("./users.db");
        builder.RegisterService(&service);
        LOG(INFO) << "Service registered successfully";

        LOG(INFO) << "Building and starting gRPC server";
        server_ = builder.BuildAndStart();
        if (!server_) {
            LOG(ERROR) << "Failed to build and start gRPC server. Server object is null.";
            LOG(ERROR) << "Check server configuration and port availability.";
            return false;
        }

        LOG(INFO) << fmt::format("Server listening on {}, gRPC server started and waiting for connections...", server_address);
        server_->Wait();

        LOG(INFO) << "gRPC connection established.";
        return true;
    }

    auto ServerTask::exit() const -> void {
        LOG(INFO) << "Shutting down service task...";
        if (server_) {
            LOG(INFO) << "Initiating gRPC server shutdown";
            server_->Shutdown();
            LOG(INFO) << "gRPC server shutdown complete.";
        } else {
            LOG(WARNING) << "Server object is null during shutdown. Nothing to shutdown.";
        }
        LOG(INFO) << "Service task shutdown complete.";
    }
}
