/**
 * @file SchedulerStarter.hpp
 * @brief DI registration for the scheduler starter
 * @details Provides registerServices() to register TaskScheduler with the DI container.
 */

#pragma once
#include <cppforge/di/IServiceCollection.hpp>
#include <cppforge/starter/scheduler/SchedulerConfig.hpp>

namespace cppforge::starter::scheduler
{
    /// @brief Registers scheduler services with the DI container
    /// @details Registers TaskScheduler as a singleton.
    class SchedulerStarter
    {
    public:
        /// @brief Register scheduler services with the DI container
        /// @param collection The service collection to register with
        /// @param config Scheduler configuration parameters
        static void registerServices(di::IServiceCollection& collection, const SchedulerConfig& config);

        SchedulerStarter() = delete;
    };
}
