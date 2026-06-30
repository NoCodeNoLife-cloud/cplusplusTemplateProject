/**
 * @file TracingStarter.hpp
 * @brief DI registration for the tracing starter
 * @details Provides registerServices() to register Tracer with the DI container.
 */

#pragma once
#include <cppforge/di/IServiceCollection.hpp>
#include <cppforge/starter/tracing/TracingConfig.hpp>

namespace cppforge::starter::tracing
{
    /// @brief Registers tracing services with the DI container
    /// @details Registers Tracer as a singleton.
    class TracingStarter
    {
    public:
        /// @brief Register tracing services with the DI container
        /// @param collection The service collection to register with
        /// @param config Tracing configuration parameters
        static void registerServices(di::IServiceCollection& collection, const TracingConfig& config);

        TracingStarter() = delete;
    };
}
