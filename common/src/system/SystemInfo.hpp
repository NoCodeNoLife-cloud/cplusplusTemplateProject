/**
 * @file SystemInfo.hpp
 * @brief SystemInfo class declaration
 * @details This header defines the SystemInfo class that provides functionality for System information and performance monitoring.
 */

#pragma once
#include <string>
#include <vector>
#include <windows.h>

namespace common::system
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