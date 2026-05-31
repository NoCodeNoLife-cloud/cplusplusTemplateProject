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

    /// @brief RAII wrapper for Windows Registry keys
    class RegistryKey
    {
    public:
        explicit RegistryKey(HKEY__* const hKey = nullptr)  : hKey_(hKey)
        {
        }

        ~RegistryKey()
        {
            if (hKey_)
            {
                RegCloseKey(hKey_);
            }
        }

        RegistryKey(const RegistryKey&) = delete;

        RegistryKey& operator=(const RegistryKey&) = delete;

        RegistryKey(RegistryKey&& other)  : hKey_(other.hKey_)
        {
            other.hKey_ = nullptr;
        }

        RegistryKey& operator=(RegistryKey&& other)
        {
            if (this != &other)
            {
                if (hKey_)
                {
                    RegCloseKey(hKey_);
                }
                hKey_ = other.hKey_;
                other.hKey_ = nullptr;
            }
            return *this;
        }

        [[nodiscard]] HKEY get() const
        {
            return hKey_;
        }

        explicit operator bool() const
        {
            return hKey_ != nullptr;
        }

    private:
        HKEY hKey_{};
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

    private:
        /// @brief Helper function to read string value from registry
        /// @param hKeyRoot Root key to open
        /// @param subKey Subkey path
        /// @param valueName Value name to read
        /// @return String value from registry or empty string if failed
        [[nodiscard]] static std::string ReadRegistryStringValue(HKEY hKeyRoot, const wchar_t* subKey, const wchar_t* valueName) ;

        /// @brief Helper function to enumerate registry values
        /// @param hKeyRoot Root key to open
        /// @param subKey Subkey path to enumerate
        /// @return Vector of string values from registry
        [[nodiscard]] static std::vector<std::string> EnumerateRegistryValues(HKEY hKeyRoot, const wchar_t* subKey) ;
    };
}