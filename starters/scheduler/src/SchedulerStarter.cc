/**
 * @file SchedulerStarter.cc
 * @brief DI registration implementation for the scheduler starter
 */

#include <cppforge/starter/scheduler/SchedulerStarter.hpp>
#include <cppforge/starter/scheduler/TaskScheduler.hpp>
#include <glog/logging.h>

namespace cppforge::starter::scheduler
{
    void SchedulerStarter::registerServices(di::IServiceCollection& collection, const SchedulerConfig& config)
    {
        collection.addSingleton<TaskScheduler>([config]() -> std::shared_ptr<TaskScheduler> {
            LOG(INFO) << "Registering TaskScheduler singleton (pool_size=" << config.thread_pool_size
                      << ", tick=" << config.tick_interval.count() << "ms)";
            return std::make_shared<TaskScheduler>(config.thread_pool_size);
        });
    }
}
