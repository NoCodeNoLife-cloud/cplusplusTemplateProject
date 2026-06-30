/**
 * @file SchedulerConfig.hpp
 * @brief Configuration options for the scheduler starter
 * @details Defines SchedulerConfig struct with thread pool size and tick interval.
 */

#pragma once
#include <chrono>
#include <cstddef>

namespace cppforge::starter::scheduler
{
    /// @brief Configuration for the task scheduler
    struct SchedulerConfig
    {
        /// @brief Number of worker threads in the scheduler thread pool
        std::size_t thread_pool_size = 4;

        /// @brief Tick interval for checking scheduled tasks
        std::chrono::milliseconds tick_interval = std::chrono::milliseconds(100);
    };
}
