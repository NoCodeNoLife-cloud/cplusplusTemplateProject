/**
 * @file SystemInfo.cc
 * @brief SystemInfo class implementation
 * @details This file contains the implementation of the SystemInfo class methods for System information and performance monitoring.
 */

#include "system/SystemInfo.hpp"

#include <cstdint>
#include <cwctype>
#include <iterator>
#include <string>
#include <vector>
#include <windows.h>
#include <fmt/format.h>

namespace common::system
{
    namespace
    {
        // Helper function to convert wide string to UTF-8 string
        std::string WideToUtf8(const wchar_t* wideStr)
        {
            if (!wideStr) return {};
            const int32_t len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0) return {};
            std::string result(len - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &result[0], len, nullptr, nullptr);
            return result;
        }

        // Helper function to read a REG_SZ value from an already-open registry key
        std::string ReadStringFromOpenKey(HKEY hKey, const wchar_t* valueName)
        {
            DWORD type = 0;
            DWORD size = 0;
            if (RegQueryValueExW(hKey, valueName, nullptr, &type, nullptr, &size) != ERROR_SUCCESS)
                return {};
            if (type != REG_SZ && type != REG_EXPAND_SZ)
                return {};

            std::wstring buffer(size / sizeof(wchar_t), L'\0');
            if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.data()), &size) != ERROR_SUCCESS)
                return {};

            return WideToUtf8(buffer.c_str());
        }

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

        /// @brief Read a string registry value from a given key path
        std::string ReadRegistryStringValue(HKEY hKeyRoot, const wchar_t* subKey, const wchar_t* valueName)
        {
            HKEY hKey;
            if (const LONG result = RegOpenKeyExW(hKeyRoot, subKey, 0, KEY_READ, &hKey); result == ERROR_SUCCESS)
            {
                RegistryKey keyGuard(hKey);
                return ReadStringFromOpenKey(hKey, valueName);
            }
            return {};
        }

        /// @brief Enumerate all values under a registry key path
        std::vector<std::string> EnumerateRegistryValues(HKEY hKeyRoot, const wchar_t* subKey)
        {
            std::vector<std::string> values;
            HKEY hKey;

            if (const LONG result = RegOpenKeyExW(hKeyRoot, subKey, 0, KEY_READ, &hKey); result == ERROR_SUCCESS)
            {
                RegistryKey keyGuard(hKey);

                DWORD index = 0;
                wchar_t valueName[256];
                wchar_t valueData[512];
                DWORD valueNameSize = static_cast<DWORD>(std::size(valueName));
                DWORD valueDataSize = sizeof(valueData);

                while (RegEnumValueW(hKey, index++, valueName, &valueNameSize, nullptr, nullptr, reinterpret_cast<LPBYTE>(valueData), &valueDataSize) == ERROR_SUCCESS)
                {
                    const std::string value = WideToUtf8(valueData);
                    if (!value.empty())
                    {
                        values.emplace_back(value);
                    }

                    valueNameSize = static_cast<DWORD>(std::size(valueName));
                    valueDataSize = sizeof(valueData);
                }
            }

            return values;
        }
    }

    std::string SystemInfo::GetCpuModelFromRegistry()
    {
        const std::string cpuModel = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", L"ProcessorNameString");
        return cpuModel.empty() ? "Unknown CPU Model" : cpuModel;
    }

    std::string SystemInfo::GetMemoryDetails()
    {
        const std::string result = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E965-E325-11CE-BFC1-08002BE10318}", L"DeviceDesc");
        return result.empty() ? "Memory details not available" : result;
    }

    std::string SystemInfo::GetOSVersion()
    {
        std::string result = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");

        if (!result.empty())
        {
            const std::string buildNum = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuildNumber");
            if (!buildNum.empty())
            {
                result += " (Build ";
                result += buildNum;
                result += ")";
            }
        }

        return result.empty() ? "Windows OS Information Not Available" : result;
    }

    MotherboardInfo SystemInfo::GetMotherboardInfo()
    {
        MotherboardInfo info{};

        info.manufacturer = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardManufacturer");
        info.model = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardProduct");
        info.biosVersion = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BiosVersion");

        std::string serial = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"SystemSerialNumber");
        if (serial.empty())
        {
            serial = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductId");
        }
        info.systemSerial = serial;
        return info;
    }

    std::string SystemInfo::GetGraphicsCardInfo()
    {
        // Read from the Display adapters device class GUID
        const std::string cardName = ReadRegistryStringValue(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000",
            L"DriverDesc");
        if (!cardName.empty())
            return cardName;

        // Fallback: try legacy GraphicsDrivers path
        HKEY hKey;
        if (const LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Devices", 0, KEY_READ, &hKey); result == ERROR_SUCCESS)
        {
            RegistryKey keyGuard(hKey);

            wchar_t subKeyName[256];
            DWORD subKeySize = static_cast<DWORD>(std::size(subKeyName));
            FILETIME lastWriteTime{};

            if (RegEnumKeyExW(hKey, 0, subKeyName, &subKeySize, nullptr, nullptr, nullptr, &lastWriteTime) == ERROR_SUCCESS)
            {
                HKEY hSubKey;
                if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
                {
                    RegistryKey subKeyGuard(hSubKey);
                    const std::string desc = ReadStringFromOpenKey(hSubKey, L"DeviceDesc");
                    if (!desc.empty())
                        return desc;
                }
            }
        }

        return "Graphics card information not available";
    }

    std::vector<std::string> SystemInfo::GetDiskDriveInfo()
    {
        return EnumerateRegistryValues(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum");
    }

    std::vector<std::string> SystemInfo::GetBIOSInfo()
    {
        std::vector<std::string> entries;

        const auto addValue = [&entries](const wchar_t* subKey, const wchar_t* valueName, const std::string& label)
        {
            const std::string value = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, subKey, valueName);
            if (!value.empty())
            {
                entries.push_back(label + value);
            }
        };

        addValue(L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BIOSVendor", "BIOS Vendor: ");
        addValue(L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BIOSVersion", "BIOS Version: ");
        addValue(L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BIOSReleaseDate", "BIOS Release Date: ");
        addValue(L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"SystemManufacturer", "System Manufacturer: ");
        addValue(L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"SystemProductName", "System Product: ");

        return entries;
    }

}
