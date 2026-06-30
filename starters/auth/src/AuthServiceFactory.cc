#include <cppforge/starter/auth/AuthServiceFactory.hpp>
#include <cppforge/starter/auth/AuthRpcServiceImpl.hpp>
#include <cppforge/starter/auth/AuthRpcClient.hpp>
#include <cppforge/di/IServiceCollection.hpp>

namespace cppforge::starter::auth
{
    std::unique_ptr<AuthRpcServiceImpl> AuthServiceFactory::createServer(const std::string& db_path)
    {
        return std::make_unique<AuthRpcServiceImpl>(db_path);
    }

    std::unique_ptr<AuthRpcClient> AuthServiceFactory::createClient(const std::shared_ptr<grpc::Channel>& channel)
    {
        return std::make_unique<AuthRpcClient>(channel);
    }

    void AuthServiceFactory::registerServices(di::IServiceCollection& collection, const std::string& db_path)
    {
        collection.addSingleton<AuthRpcServiceImpl>([db_path]() {
            return std::make_shared<AuthRpcServiceImpl>(db_path);
        });
    }
}