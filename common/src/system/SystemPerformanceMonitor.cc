#include "src/system/SystemPerformanceMonitor.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <windows.h>

#include <chrono>
#include <cstdint>
#include <thread>

namespace common::system {
    auto SystemPerformanceMonitor::FileTimeToULARGEInteger(const FILETIME &ft) noexcept -> ULARGE_INTEGER {
        ULARGE_INTEGER result;
        result.LowPart = ft.dwLowDateTime;
        result.HighPart = ft.dwHighDateTime;
        return result;
    }

    auto SystemPerformanceMonitor::GetMemoryUsage() noexcept -> MemoryUsage {
        DLOG(INFO) << "SystemPerformanceMonitor GetMemoryUsage - retrieving memory statistics";
        MemoryUsage memUsage{};
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        // Retrieve memory status information
        GlobalMemoryStatusEx(&memInfo);

        memUsage.total_memory = memInfo.ullTotalPhys;
        memUsage.available_memory = memInfo.ullAvailPhys;
        memUsage.used_memory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        memUsage.memory_usage_percent = static_cast<double>(memUsage.used_memory) / static_cast<double>(memInfo.ullTotalPhys) * 100.0;

        DLOG(INFO) << fmt::format("SystemPerformanceMonitor GetMemoryUsage - total={}MB, available={}MB, used={}MB, usage={:.2f}%",
                                  memUsage.total_memory / (1024 * 1024),
                                  memUsage.available_memory / (1024 * 1024),
                                  memUsage.used_memory / (1024 * 1024),
                                  memUsage.memory_usage_percent);

        return memUsage;
    }

    auto SystemPerformanceMonitor::GetCpuUsage(const int32_t interval) noexcept -> CpuUsage {
        DLOG(INFO) << fmt::format("SystemPerformanceMonitor GetCpuUsage - measuring CPU usage with {}s interval", interval);
        CpuUsage cpuUsage{};

        // Initial system times measurement
        FILETIME idleTime1{}, kernelTime1{}, userTime1{};
        GetSystemTimes(&idleTime1, &kernelTime1, &userTime1);

        const auto idle1 = FileTimeToULARGEInteger(idleTime1);
        const auto kernel1 = FileTimeToULARGEInteger(kernelTime1);
        const auto user1 = FileTimeToULARGEInteger(userTime1);

        // Wait for the specified interval
        std::this_thread::sleep_for(std::chrono::seconds(interval));

        // Second system times measurement
        FILETIME idleTime2{}, kernelTime2{}, userTime2{};
        GetSystemTimes(&idleTime2, &kernelTime2, &userTime2);

        const auto idle2 = FileTimeToULARGEInteger(idleTime2);
        const auto kernel2 = FileTimeToULARGEInteger(kernelTime2);
        const auto user2 = FileTimeToULARGEInteger(userTime2);

        // Calculate differences
        const ULONGLONG idleDiff = idle2.QuadPart - idle1.QuadPart;
        const ULONGLONG kernelDiff = kernel2.QuadPart - kernel1.QuadPart;
        const ULONGLONG userDiff = user2.QuadPart - user1.QuadPart;
        const ULONGLONG totalDiff = kernelDiff + userDiff;

        // Calculate CPU usage percentage
        if (totalDiff != 0) {
            cpuUsage.cpu_usage_percent = (static_cast<double>(totalDiff) - static_cast<double>(idleDiff)) * 100.0 / static_cast<double>(totalDiff);
        } else {
            cpuUsage.cpu_usage_percent = 0.0;
        }

        DLOG(INFO) << fmt::format("SystemPerformanceMonitor GetCpuUsage - CPU usage: {:.2f}%", cpuUsage.cpu_usage_percent);

        return cpuUsage;
    }
}
