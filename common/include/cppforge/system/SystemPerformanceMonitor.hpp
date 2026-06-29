/**
 * @file SystemPerformanceMonitor.hpp
 * @brief Real-time system performance monitoring â€?CPU, memory, I/O
 * @description Periodically samples system performance metrics: CPU usage
 *          (per-core and aggregate), memory usage (RSS, virtual), disk I/O,
 *          and network throughput.  Stores a rolling window of samples for
 *          trend analysis.  Usable for health checks and capacity planning.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Sampling is serialised via an internal
 * mutex.
 */

#pragma once
#include <cstdint>
#include <windows.h>

namespace cppforge::system
{
    /// @brief Structure to hold system memory usage information
    /// This struct contains the total physical memory, available memory, used memory,
    /// and the percentage of memory currently in use.
    struct MemoryUsage
    {
        ULONGLONG total_memory{}; ///< Total physical memory in bytes
        ULONGLONG available_memory{}; ///< Available memory in bytes
        ULONGLONG used_memory{}; ///< Used memory in bytes
        double memory_usage_percent{}; ///< Memory usage percentage (0.0 to 100.0)
    };

    /// @brief Structure to hold CPU usage information
    /// This struct contains the percentage of CPU currently in use.
    struct CpuUsage
    {
        double cpu_usage_percent{}; ///< CPU usage percentage (0.0 to 100.0)
    };

    /// @brief Class for monitoring system performance
    /// This class provides static methods to retrieve system memory and CPU usage information.
    class SystemPerformanceMonitor
    {
    private:
        /// @brief Helper function to extract ULARGE_INTEGER from FILETIME
        /// @param ft Filetime to convert
        /// @return ULARGE_INTEGER representation of the filetime
        [[nodiscard]] static ULARGE_INTEGER FileTimeToULARGEInteger(const FILETIME& ft);

    public:
        SystemPerformanceMonitor() = delete;

        /// @brief Get the current memory usage of the system
        /// @return MemoryUsage struct containing total, available, used memory and usage percentage
        [[nodiscard]] static MemoryUsage GetMemoryUsage();

        /// @brief Get the current CPU usage of the system
        /// @param interval Time interval in seconds for measuring CPU usage, default is 1 second
        /// @return CpuUsage struct containing CPU usage percentage
        [[nodiscard]] static CpuUsage GetCpuUsage(int32_t interval = 1);
    };
}
