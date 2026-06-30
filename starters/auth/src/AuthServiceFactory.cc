/**
 * @file AuthServiceFactory.cc
 * @brief Factory for creating auth service components
 */
#include "cppforge/starter/auth/AuthServiceFactory.hpp"
#include "cppforge/starter/auth/AuthRpcServiceImpl.hpp"
#include "cppforge/starter/auth/AuthRpcClient.hpp"
#include <memory>

namespace cppforge::starter::auth {

std::unique_ptr<AuthRpcServiceImpl> AuthServiceFactory::createServer(const std::string& db_path) {
    return std::make_unique<AuthRpcServiceImpl>(db_path);
}

std::unique_ptr<AuthRpcClient> AuthServiceFactory::createClient(const std::shared_ptr<grpc::Channel>& channel) {
    return std::make_unique<AuthRpcClient>(channel);
}

} // namespace cppforge::starter::auth