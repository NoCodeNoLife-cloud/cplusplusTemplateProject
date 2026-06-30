#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <cppforge/di/IServiceCollection.hpp>

namespace grpc { class Channel; }

namespace cppforge::starter::auth
{
    class AuthRpcServiceImpl;
    class AuthRpcClient;

    /// @brief Factory for creating authentication service instances
    class AuthServiceFactory
    {
    public:
        /// @brief Create a server-side authentication RPC service
        /// @param db_path Path to SQLite database file
        /// @return Unique pointer to the created AuthRpcServiceImpl
        static std::unique_ptr<AuthRpcServiceImpl> createServer(const std::string& db_path);

        /// @brief Create a client-side authentication RPC client
        /// @param channel The gRPC channel to use for communication
        /// @return Unique pointer to the created AuthRpcClient
        static std::unique_ptr<AuthRpcClient> createClient(const std::shared_ptr<grpc::Channel>& channel);

        /// @brief Register auth services with DI container
        /// @param collection The service collection to register with
        /// @param db_path Path to SQLite database file (for server)
        static void registerServices(di::IServiceCollection& collection, const std::string& db_path);

        AuthServiceFactory() = delete;
    };
}