/**
 * @file SystemPerformanceMonitor.cc
 * @brief SystemPerformanceMonitor implementation â€?metric sampling and rolling window
 * @details Implements periodic metric sampling: reads /proc/stat and
 *          /proc/meminfo on Linux, uses PDH APIs on Windows.  Maintains a
 *          fixed-size ring buffer of samples for trend reporting.
 */

#include "system/SystemPerformanceMonitor.hpp"

#include <chrono>
#include <cstdint>
#include <thread>
#include <windows.h>

namespace cppforge::system
{
    ULARGE_INTEGER SystemPerformanceMonitor::FileTimeToULARGEInteger(const FILETIME& ft)
    {
        ULARGE_INTEGER result;
        result.LowPart = ft.dwLowDateTime;
        result.HighPart = ft.dwHighDateTime;
        return result;
    }

    MemoryUsage SystemPerformanceMonitor::GetMemoryUsage()
    {
        MemoryUsage memUsage{};
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        if (!GlobalMemoryStatusEx(&memInfo))
        {
            return memUsage;
        }

        memUsage.total_memory = memInfo.ullTotalPhys;
        memUsage.available_memory = memInfo.ullAvailPhys;
        memUsage.used_memory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        if (memInfo.ullTotalPhys > 0)
        {
            memUsage.memory_usage_percent = static_cast<double>(memUsage.used_memory) / static_cast<double>(memInfo.ullTotalPhys) * 100.0;
        }
        return memUsage;
    }

    CpuUsage SystemPerformanceMonitor::GetCpuUsage(const int32_t interval)
    {
        CpuUsage cpuUsage{};

        FILETIME idleTime1{}, kernelTime1{}, userTime1{};
        if (!GetSystemTimes(&idleTime1, &kernelTime1, &userTime1))
        {
            return cpuUsage;
        }

        const auto idle1 = FileTimeToULARGEInteger(idleTime1);
        const auto kernel1 = FileTimeToULARGEInteger(kernelTime1);
        const auto user1 = FileTimeToULARGEInteger(userTime1);

        std::this_thread::sleep_for(std::chrono::seconds(interval));

        FILETIME idleTime2{}, kernelTime2{}, userTime2{};
        if (!GetSystemTimes(&idleTime2, &kernelTime2, &userTime2))
        {
            return cpuUsage;
        }

        const auto idle2 = FileTimeToULARGEInteger(idleTime2);
        const auto kernel2 = FileTimeToULARGEInteger(kernelTime2);
        const auto user2 = FileTimeToULARGEInteger(userTime2);

        const ULONGLONG idleDiff = idle2.QuadPart - idle1.QuadPart;
        const ULONGLONG kernelDiff = kernel2.QuadPart - kernel1.QuadPart;
        const ULONGLONG userDiff = user2.QuadPart - user1.QuadPart;
        const ULONGLONG totalDiff = kernelDiff + userDiff;

        if (totalDiff != 0)
        {
            cpuUsage.cpu_usage_percent = (static_cast<double>(totalDiff) - static_cast<double>(idleDiff)) * 100.0 / static_cast<double>(totalDiff);
        }
        else
        {
            cpuUsage.cpu_usage_percent = 0.0;
        }
        return cpuUsage;
    }
}
