#include "src/system/SystemInfo.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <cwctype>
#include <string>
#include <vector>
#include <windows.h>

namespace common::system {
    namespace {
        // Helper function to convert wide string to UTF-8 string
        auto WideToUtf8(const wchar_t *wideStr) -> std::string {
            if (!wideStr) return "";
            const int32_t len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0) return "";
            std::string result(len - 1, 0); // len includes null terminator
            WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &result[0], len, nullptr, nullptr);
            return result;
        }
    }

    auto SystemInfo::ReadRegistryStringValue(HKEY__ *const hKeyRoot, const wchar_t *subKey, const wchar_t *valueName) noexcept -> std::string {
        DLOG(INFO) << fmt::format("SystemInfo ReadRegistryStringValue - reading registry value: {}\\{}", 
                                  WideToUtf8(subKey),
                                  WideToUtf8(valueName));
        HKEY hKey;
        if (const LONG result = RegOpenKeyExW(hKeyRoot, subKey, 0, KEY_READ, &hKey); result == ERROR_SUCCESS) {
            RegistryKey keyGuard(hKey);

            wchar_t buffer[512]; // Increased buffer size for safety
            DWORD size = sizeof(buffer);

            if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS) {
                if (const int32_t len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr); len > 0) {
                    std::string basic_string(len - 1, 0); // len includes null terminator, so subtract 1
                    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &basic_string[0], len, nullptr, nullptr);
                    DLOG(INFO) << fmt::format("SystemInfo ReadRegistryStringValue - successfully read value: {}", basic_string);
                    return basic_string;
                }
            }
        }
        DLOG(WARNING) << fmt::format("SystemInfo ReadRegistryStringValue - failed to read registry value");
        return {}; // Return empty string if failed
    }

    auto SystemInfo::EnumerateRegistryValues(HKEY__ *const hKeyRoot, const wchar_t *subKey) noexcept -> std::vector<std::string> {
        std::vector<std::string> values;
        HKEY hKey;

        if (const LONG result = RegOpenKeyExW(hKeyRoot, subKey, 0, KEY_READ, &hKey); result == ERROR_SUCCESS) {
            RegistryKey keyGuard(hKey);

            DWORD index = 0;
            wchar_t valueName[256];
            wchar_t valueData[512];
            DWORD valueNameSize = sizeof(valueName) / sizeof(wchar_t);
            DWORD valueDataSize = sizeof(valueData);

            while (RegEnumValueW(hKey, index++, valueName, &valueNameSize, nullptr, nullptr, reinterpret_cast<LPBYTE>(valueData), &valueDataSize) == ERROR_SUCCESS) {
                if (const int32_t len = WideCharToMultiByte(CP_UTF8, 0, valueData, -1, nullptr, 0, nullptr, nullptr); len > 0) {
                    std::string value(len - 1, 0); // len includes null terminator, so subtract 1
                    WideCharToMultiByte(CP_UTF8, 0, valueData, -1, &value[0], len, nullptr, nullptr);
                    values.emplace_back(std::move(value));
                }

                valueNameSize = sizeof(valueName) / sizeof(wchar_t);
                valueDataSize = sizeof(valueData);
            }
        }

        return values;
    }

    auto SystemInfo::GetCpuModelFromRegistry() noexcept -> std::string {
        DLOG(INFO) << "SystemInfo GetCpuModelFromRegistry - retrieving CPU model";
        const std::string cpuModel = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", L"ProcessorNameString");
        const std::string result = cpuModel.empty() ? "Unknown CPU Model" : cpuModel;
        DLOG(INFO) << fmt::format("SystemInfo GetCpuModelFromRegistry - CPU model: {}", result);
        return result;
    }

    auto SystemInfo::GetMemoryDetails() noexcept -> std::string {
        const std::string result = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E965-E325-11CE-BFC1-08002BE10318}", L"DeviceDesc");
        return result.empty() ? "Memory details not available" : result;
    }

    auto SystemInfo::GetOSVersion() noexcept -> std::string {
        DLOG(INFO) << "SystemInfo GetOSVersion - retrieving OS version";
        std::string result = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");

        if (!result.empty()) {
            const std::string buildNum = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuildNumber");
            if (!buildNum.empty()) {
                result += " (Build ";
                result += buildNum;
                result += ")";
            }
        }

        const std::string finalResult = result.empty() ? "Windows OS Information Not Available" : result;
        DLOG(INFO) << fmt::format("SystemInfo GetOSVersion - OS version: {}", finalResult);
        return finalResult;
    }

    auto SystemInfo::GetMotherboardInfo() noexcept -> MotherboardInfo {
        DLOG(INFO) << "SystemInfo GetMotherboardInfo - retrieving motherboard information";
        MotherboardInfo info{};

        // Read motherboard information from BIOS key
        info.manufacturer = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardManufacturer");
        info.model = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardProduct");
        info.biosVersion = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BiosVersion");

        // Try to read system serial from either SystemSerialNumber or ProductId
        std::string serial = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"SystemSerialNumber");
        if (serial.empty()) {
            serial = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductId");
        }
        info.systemSerial = serial;

        DLOG(INFO) << fmt::format("SystemInfo GetMotherboardInfo - manufacturer={}, model={}", info.manufacturer, info.model);
        return info;
    }

    auto SystemInfo::GetGraphicsCardInfo() noexcept -> std::string {
        DLOG(INFO) << "SystemInfo GetGraphicsCardInfo - retrieving graphics card information";
        // Open the graphics drivers devices key
        HKEY hKey;
        if (const LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Devices", 0, KEY_READ, &hKey); result == ERROR_SUCCESS) {
            RegistryKey keyGuard(hKey);

            wchar_t subKeyName[256];
            DWORD subKeySize = sizeof(subKeyName) / sizeof(wchar_t);
            FILETIME lastWriteTime;

            if (RegEnumKeyExW(hKey, 0, subKeyName, &subKeySize, nullptr, nullptr, nullptr, &lastWriteTime) == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    RegistryKey subKeyGuard(hSubKey);

                    wchar_t deviceDesc[256];
                    DWORD size = sizeof(deviceDesc);

                    if (RegQueryValueExW(hSubKey, L"DeviceDesc", nullptr, nullptr, reinterpret_cast<LPBYTE>(deviceDesc), &size) == ERROR_SUCCESS) {
                        if (const int32_t len = WideCharToMultiByte(CP_UTF8, 0, deviceDesc, -1, nullptr, 0, nullptr, nullptr); len > 0) {
                            std::string basic_string(len - 1, 0); // len includes null terminator, so subtract 1
                            WideCharToMultiByte(CP_UTF8, 0, deviceDesc, -1, &basic_string[0], len, nullptr, nullptr);
                            DLOG(INFO) << fmt::format("SystemInfo GetGraphicsCardInfo - GPU: {}", basic_string);
                            return basic_string;
                        }
                    }
                }
            }
        }

        DLOG(WARNING) << "SystemInfo GetGraphicsCardInfo - graphics card information not available";
        return "Graphics card information not available";
    }

    auto SystemInfo::GetDiskDriveInfo() noexcept -> std::vector<std::string> {
        return EnumerateRegistryValues(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum");
    }

    auto SystemInfo::GetBIOSInfo() noexcept -> std::vector<std::string> {
        std::vector<std::string> adapters;
        HKEY hKey;

        if (const LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &hKey); result == ERROR_SUCCESS) {
            RegistryKey keyGuard(hKey);

            DWORD index = 0;
            wchar_t subKeyName[256];
            DWORD subKeySize = sizeof(subKeyName) / sizeof(wchar_t);

            while (RegEnumKeyExW(hKey, index++, subKeyName, &subKeySize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                if (iswdigit(subKeyName[0])) {
                    HKEY hSubKey;
                    if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                        RegistryKey subKeyGuard(hSubKey);

                        wchar_t adapterName[256];
                        DWORD size = sizeof(adapterName);

                        if (RegQueryValueExW(hSubKey, L"DriverDesc", nullptr, nullptr, reinterpret_cast<LPBYTE>(adapterName), &size) == ERROR_SUCCESS) {
                            if (const int32_t len = WideCharToMultiByte(CP_UTF8, 0, adapterName, -1, nullptr, 0, nullptr, nullptr); len > 0) {
                                std::string name(len - 1, 0); // len includes null terminator, so subtract 1
                                WideCharToMultiByte(CP_UTF8, 0, adapterName, -1, &name[0], len, nullptr, nullptr);
                                adapters.emplace_back(std::move(name));
                            }
                        }
                    }
                }

                subKeySize = sizeof(subKeyName) / sizeof(wchar_t);
            }
        }

        return adapters;
    }
}
