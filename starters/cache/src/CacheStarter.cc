/**
 * @file CacheStarter.cc
 * @brief DI registration implementation for the cache starter
 */

#include <cppforge/starter/cache/CacheStarter.hpp>
#include <cppforge/starter/cache/MultiLevelCache.hpp>
#include <glog/logging.h>

namespace cppforge::starter::cache
{
    void CacheStarter::registerServices(di::IServiceCollection& collection, const CacheConfig& config)
    {
        if (config.enable_l2)
        {
            LOG(WARNING) << "L2 cache (Redis) is not yet implemented. Only L1 cache will be used.";
        }

        collection.addSingleton<MultiLevelCache<std::string, std::string>>([config]() -> std::shared_ptr<MultiLevelCache<std::string, std::string>> {
            LOG(INFO) << "Registering MultiLevelCache<string,string> singleton (L1 capacity=" << config.l1_capacity
                      << ", L2 enabled=" << (config.enable_l2 ? "true" : "false") << ")";
            return std::make_shared<MultiLevelCache<std::string, std::string>>(config.l1_capacity);
        });
    }
}