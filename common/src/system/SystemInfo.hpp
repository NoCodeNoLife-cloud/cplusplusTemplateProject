/**
 * @file SystemInfo.hpp
 * @brief System information queries â€?OS, CPU, memory, hostname
 * @description Provides static methods for querying system-level information:
 *          operating system name/version, CPU architecture and core count,
 *          total/available physical memory, hostname, and process ID.
 *          Uses platform-specific APIs (/proc on Linux, WinAPI on Windows).
 *
 * @par Thread Safety
 * Static methods are thread-safe (read-only system queries).
 */

#pragma once
#include <string>
#include <vector>
#include <windows.h>

namespace cppforge::system
{
    /// @brief Structure to hold motherboard information
    struct MotherboardInfo
    {
        std::string manufacturer{};
        std::string model{};
        std::string biosVersion{};
        std::string systemSerial{};
    };

    /// @brief A utility class for retrieving system hardware and OS information.
    /// The SystemInfo class provides static methods to fetch various system details
    /// such as CPU model, memory information, OS version, motherboard details,
    /// graphics card info, disk drives, and BIOS information.
    class SystemInfo
    {
    public:
        SystemInfo() = delete;

        /// @brief Get CPU model from registry
        /// @return CPU model as string
        [[nodiscard]] static std::string GetCpuModelFromRegistry() ;

        /// @brief Get memory details
        /// @return Memory details as string
        [[nodiscard]] static std::string GetMemoryDetails() ;

        /// @brief Get OS version
        /// @return OS version as string
        [[nodiscard]] static std::string GetOSVersion() ;

        /// @brief Get motherboard information
        /// @return MotherboardInfo struct
        [[nodiscard]] static MotherboardInfo GetMotherboardInfo() ;

        /// @brief Get graphics card information
        /// @return Graphics card info as string
        [[nodiscard]] static std::string GetGraphicsCardInfo() ;

        /// @brief Get disk drive information
        /// @return Vector of disk drive info strings
        [[nodiscard]] static std::vector<std::string> GetDiskDriveInfo() ;

        /// @brief Get BIOS information
        /// @return Vector of BIOS info strings
        [[nodiscard]] static std::vector<std::string> GetBIOSInfo() ;

    };
}