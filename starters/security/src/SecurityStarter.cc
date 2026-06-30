/**
 * @file SecurityStarter.cc
 * @brief DI registration implementation for the security starter
 */

#include <cppforge/starter/security/SecurityStarter.hpp>
#include <cppforge/starter/security/JwtToken.hpp>
#include <cppforge/starter/security/RbacManager.hpp>
#include <glog/logging.h>

namespace cppforge::starter::security
{
    void SecurityStarter::registerServices(di::IServiceCollection& collection, const SecurityConfig& config)
    {
        if (config.jwt_secret == "default_secret_change_me")
        {
            LOG(WARNING) << "Using default JWT secret. This is insecure for production use.";
        }

        collection.addSingleton<JwtToken>([config]() -> std::shared_ptr<JwtToken> {
            LOG(INFO) << "Registering JwtToken singleton (expiry=" << config.jwt_expiry.count() << "h)";
            return std::make_shared<JwtToken>(config.jwt_secret);
        });

        collection.addSingleton<RbacManager>([]() -> std::shared_ptr<RbacManager> {
            LOG(INFO) << "Registering RbacManager singleton";
            return std::make_shared<RbacManager>();
        });
    }
}