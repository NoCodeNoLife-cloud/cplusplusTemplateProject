/**
 * @file CacheStarter.hpp
 * @brief DI registration for the cache starter
 * @details Provides registerServices() to register MultiLevelCache and related
 *          services with the DI container.
 */

#pragma once
#include <cppforge/di/IServiceCollection.hpp>
#include <cppforge/starter/cache/CacheConfig.hpp>

namespace cppforge::starter::cache
{
    /// @brief Registers cache services with the DI container
    /// @details Registers MultiLevelCache<std::string, std::string> as a singleton.
    class CacheStarter
    {
    public:
        /// @brief Register cache services with the DI container
        /// @param collection The service collection to register with
        /// @param config Cache configuration parameters
        static void registerServices(di::IServiceCollection& collection, const CacheConfig& config);

        CacheStarter() = delete;
    };
}
