#pragma once
#include <string>
#include <vector>
#include <windows.h>

namespace common::system {
/// @brief Structure to hold motherboard information
struct MotherboardInfo {
    std::string manufacturer{};
    std::string model{};
    std::string biosVersion{};
    std::string systemSerial{};
};

/// @brief RAII wrapper for Windows Registry keys
class RegistryKey {
public:
    explicit RegistryKey(HKEY__* const hKey = nullptr) noexcept : hKey_(hKey) {
    }

    ~RegistryKey() noexcept {
        if (hKey_) {
            RegCloseKey(hKey_);
        }
    }

    RegistryKey(const RegistryKey&) = delete;

    RegistryKey& operator=(const RegistryKey&) = delete;

    RegistryKey(RegistryKey&& other) noexcept : hKey_(other.hKey_) {
        other.hKey_ = nullptr;
    }

    RegistryKey& operator=(RegistryKey&& other) noexcept {
        if (this != &other) {
            if (hKey_) {
                RegCloseKey(hKey_);
            }
            hKey_ = other.hKey_;
            other.hKey_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] HKEY get() const noexcept {
        return hKey_;
    }

    explicit operator bool() const noexcept {
        return hKey_ != nullptr;
    }

private:
    HKEY hKey_{};
};

/// @brief A utility class for retrieving system hardware and OS information.
/// The SystemInfo class provides static methods to fetch various system details
/// such as CPU model, memory information, OS version, motherboard details,
/// graphics card info, disk drives, and BIOS information.
class SystemInfo {
public:
    SystemInfo() = delete;

    /// @brief Get CPU model from registry
    /// @return CPU model as string
    [[nodiscard]] static auto GetCpuModelFromRegistry() noexcept -> std::string;

    /// @brief Get memory details
    /// @return Memory details as string
    [[nodiscard]] static auto GetMemoryDetails() noexcept -> std::string;

    /// @brief Get OS version
    /// @return OS version as string
    [[nodiscard]] static auto GetOSVersion() noexcept -> std::string;

    /// @brief Get motherboard information
    /// @return MotherboardInfo struct
    [[nodiscard]] static auto GetMotherboardInfo() noexcept -> MotherboardInfo;

    /// @brief Get graphics card information
    /// @return Graphics card info as string
    [[nodiscard]] static auto GetGraphicsCardInfo() noexcept -> std::string;

    /// @brief Get disk drive information
    /// @return Vector of disk drive info strings
    [[nodiscard]] static auto GetDiskDriveInfo() noexcept -> std::vector<std::string>;

    /// @brief Get BIOS information
    /// @return Vector of BIOS info strings
    [[nodiscard]] static auto GetBIOSInfo() noexcept -> std::vector<std::string>;

private:
    /// @brief Helper function to read string value from registry
    /// @param hKeyRoot Root key to open
    /// @param subKey Subkey path
    /// @param valueName Value name to read
    /// @return String value from registry or empty string if failed
    [[nodiscard]] static auto ReadRegistryStringValue(HKEY hKeyRoot, const wchar_t* subKey, const wchar_t* valueName) noexcept -> std::string;

    /// @brief Helper function to enumerate registry values
    /// @param hKeyRoot Root key to open
    /// @param subKey Subkey path to enumerate
    /// @return Vector of string values from registry
    [[nodiscard]] static auto EnumerateRegistryValues(HKEY hKeyRoot, const wchar_t* subKey) noexcept -> std::vector<std::string>;
};
}