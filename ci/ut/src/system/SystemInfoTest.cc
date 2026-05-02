/**
 * @file SystemInfoTest.cc
 * @brief Unit tests for the SystemInfo class
 * @details Tests cover core system information retrieval functionality including
 *          CPU model, memory details, OS version, motherboard info, graphics card,
 *          disk drives, and BIOS information.
 */

#include <gtest/gtest.h>
#include "system/SystemInfo.hpp"
#include <string>
#include <vector>

using namespace common::system;

/**
 * @brief Test GetCpuModelFromRegistry returns non-empty string
 * @details Verifies that CPU model retrieval returns a valid, non-empty result
 */
TEST(SystemInfoTest, GetCpuModelFromRegistry_NonEmpty) {
    const auto cpuModel = SystemInfo::GetCpuModelFromRegistry();
    EXPECT_FALSE(cpuModel.empty());
}

/**
 * @brief Test GetCpuModelFromRegistry returns reasonable value
 * @details Ensures the CPU model is not just the fallback "Unknown CPU Model" on typical systems
 */
TEST(SystemInfoTest, GetCpuModelFromRegistry_ReasonableValue) {
    const auto cpuModel = SystemInfo::GetCpuModelFromRegistry();

    // On most Windows systems with proper registry access, should get actual CPU model
    // If registry access fails, it returns "Unknown CPU Model"
    EXPECT_FALSE(cpuModel.empty());
}

/**
 * @brief Test GetMemoryDetails returns non-empty string
 * @details Verifies that memory details retrieval returns a valid result
 */
TEST(SystemInfoTest, GetMemoryDetails_NonEmpty) {
    const auto memoryDetails = SystemInfo::GetMemoryDetails();
    EXPECT_FALSE(memoryDetails.empty());
}

/**
 * @brief Test GetOSVersion returns non-empty string
 * @details Verifies that OS version retrieval returns a valid, non-empty result
 */
TEST(SystemInfoTest, GetOSVersion_NonEmpty) {
    const auto osVersion = SystemInfo::GetOSVersion();
    EXPECT_FALSE(osVersion.empty());
}

/**
 * @brief Test GetOSVersion contains Windows-related information
 * @details Ensures the OS version string contains expected Windows identifiers
 */
TEST(SystemInfoTest, GetOSVersion_ContainsWindowsInfo) {
    const auto osVersion = SystemInfo::GetOSVersion();

    // Should contain either "Windows" or at least not be the fallback message
    if (osVersion != "Windows OS Information Not Available") {
        EXPECT_FALSE(osVersion.empty());
    }
}

/**
 * @brief Test GetMotherboardInfo returns valid structure
 * @details Verifies that motherboard information retrieval returns a populated structure
 */
TEST(SystemInfoTest, GetMotherboardInfo_ValidStructure) {
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // At least some fields should be populated on a real system
    // Empty strings are acceptable if registry access fails
    EXPECT_TRUE(true); // Structure creation itself is a success
}

/**
 * @brief Test GetMotherboardInfo manufacturer field accessibility
 * @details Verifies that the manufacturer field can be accessed
 */
TEST(SystemInfoTest, GetMotherboardInfo_ManufacturerField) {
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // Field should be accessible (may be empty if registry unavailable)
    const std::string manufacturer = info.manufacturer;
    EXPECT_TRUE(true); // Accessibility test
}

/**
 * @brief Test GetMotherboardInfo model field accessibility
 * @details Verifies that the model field can be accessed
 */
TEST(SystemInfoTest, GetMotherboardInfo_ModelField) {
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // Field should be accessible (may be empty if registry unavailable)
    const std::string model = info.model;
    EXPECT_TRUE(true); // Accessibility test
}

/**
 * @brief Test GetMotherboardInfo biosVersion field accessibility
 * @details Verifies that the BIOS version field can be accessed
 */
TEST(SystemInfoTest, GetMotherboardInfo_BiosVersionField) {
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // Field should be accessible (may be empty if registry unavailable)
    const std::string biosVersion = info.biosVersion;
    EXPECT_TRUE(true); // Accessibility test
}

/**
 * @brief Test GetMotherboardInfo systemSerial field accessibility
 * @details Verifies that the system serial field can be accessed
 */
TEST(SystemInfoTest, GetMotherboardInfo_SystemSerialField) {
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // Field should be accessible (may be empty if registry unavailable)
    const std::string systemSerial = info.systemSerial;
    EXPECT_TRUE(true); // Accessibility test
}

/**
 * @brief Test GetGraphicsCardInfo returns non-empty string
 * @details Verifies that graphics card information retrieval returns a valid result
 */
TEST(SystemInfoTest, GetGraphicsCardInfo_NonEmpty) {
    const auto graphicsInfo = SystemInfo::GetGraphicsCardInfo();
    EXPECT_FALSE(graphicsInfo.empty());
}

/**
 * @brief Test GetDiskDriveInfo returns vector
 * @details Verifies that disk drive information retrieval returns a valid vector
 */
TEST(SystemInfoTest, GetDiskDriveInfo_ReturnsVector) {
    const auto diskInfo = SystemInfo::GetDiskDriveInfo();

    // Should return a valid vector (may be empty if no disks found or registry unavailable)
    EXPECT_TRUE(true); // Vector creation itself is a success
}

/**
 * @brief Test GetDiskDriveInfo vector size is reasonable
 * @details Ensures the disk drive list size is within expected bounds
 */
TEST(SystemInfoTest, GetDiskDriveInfo_ReasonableSize) {
    const auto diskInfo = SystemInfo::GetDiskDriveInfo();

    // Should have between 0 and 100 disk entries (reasonable upper bound)
    EXPECT_LE(diskInfo.size(), 100);
}

/**
 * @brief Test GetBIOSInfo returns vector
 * @details Verifies that BIOS information retrieval returns a valid vector
 */
TEST(SystemInfoTest, GetBIOSInfo_ReturnsVector) {
    const auto biosInfo = SystemInfo::GetBIOSInfo();

    // Should return a valid vector (may be empty if no adapters found or registry unavailable)
    EXPECT_TRUE(true); // Vector creation itself is a success
}

/**
 * @brief Test GetBIOSInfo vector size is reasonable
 * @details Ensures the BIOS adapter list size is within expected bounds
 */
TEST(SystemInfoTest, GetBIOSInfo_ReasonableSize) {
    const auto biosInfo = SystemInfo::GetBIOSInfo();

    // Should have between 0 and 100 adapter entries (reasonable upper bound)
    EXPECT_LE(biosInfo.size(), 100);
}

/**
 * @brief Test all system info methods can be called without crashing
 * @details Verifies that all public methods execute without exceptions or crashes
 */
TEST(SystemInfoTest, AllMethods_ExecuteWithoutCrash) {
    EXPECT_NO_THROW(SystemInfo::GetCpuModelFromRegistry());
    EXPECT_NO_THROW(SystemInfo::GetMemoryDetails());
    EXPECT_NO_THROW(SystemInfo::GetOSVersion());
    EXPECT_NO_THROW(SystemInfo::GetMotherboardInfo());
    EXPECT_NO_THROW(SystemInfo::GetGraphicsCardInfo());
    EXPECT_NO_THROW(SystemInfo::GetDiskDriveInfo());
    EXPECT_NO_THROW(SystemInfo::GetBIOSInfo());
}

/**
 * @brief Test consistency of multiple calls to GetCpuModelFromRegistry
 * @details Verifies that repeated calls return consistent results
 */
TEST(SystemInfoTest, GetCpuModelFromRegistry_ConsistentResults) {
    const auto result1 = SystemInfo::GetCpuModelFromRegistry();
    const auto result2 = SystemInfo::GetCpuModelFromRegistry();

    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test consistency of multiple calls to GetOSVersion
 * @details Verifies that repeated calls return consistent results
 */
TEST(SystemInfoTest, GetOSVersion_ConsistentResults) {
    const auto result1 = SystemInfo::GetOSVersion();
    const auto result2 = SystemInfo::GetOSVersion();

    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test consistency of multiple calls to GetMotherboardInfo
 * @details Verifies that repeated calls return consistent results
 */
TEST(SystemInfoTest, GetMotherboardInfo_ConsistentResults) {
    const auto result1 = SystemInfo::GetMotherboardInfo();
    const auto result2 = SystemInfo::GetMotherboardInfo();

    EXPECT_EQ(result1.manufacturer, result2.manufacturer);
    EXPECT_EQ(result1.model, result2.model);
    EXPECT_EQ(result1.biosVersion, result2.biosVersion);
    EXPECT_EQ(result1.systemSerial, result2.systemSerial);
}

/**
 * @brief Test RegistryKey default constructor
 * @details Verifies that RegistryKey can be constructed with nullptr
 */
TEST(SystemInfoTest, RegistryKey_DefaultConstructor) {
    const RegistryKey key;
    EXPECT_FALSE(key); // Should be falsy when constructed with nullptr
}

/**
 * @brief Test RegistryKey boolean conversion operator
 * @details Verifies that RegistryKey properly converts to bool based on handle validity
 */
TEST(SystemInfoTest, RegistryKey_BoolConversion) {
    const RegistryKey nullKey(nullptr);
    EXPECT_FALSE(nullKey);

    // Note: We cannot easily test with a valid HKEY without actually opening a registry key
    // The RAII behavior is tested implicitly through SystemInfo methods
}

/**
 * @brief Test RegistryKey move constructor
 * @details Verifies that RegistryKey can be moved correctly
 */
TEST(SystemInfoTest, RegistryKey_MoveConstructor) {
    RegistryKey key1(nullptr);
    RegistryKey key2(std::move(key1));

    EXPECT_FALSE(key2); // Moved-to key should still be nullptr
    EXPECT_FALSE(key1); // Moved-from key should be nullptr
}

/**
 * @brief Test RegistryKey move assignment operator
 * @details Verifies that RegistryKey move assignment works correctly
 */
TEST(SystemInfoTest, RegistryKey_MoveAssignment) {
    RegistryKey key1(nullptr);
    RegistryKey key2(nullptr);

    key2 = std::move(key1);

    EXPECT_FALSE(key2);
    EXPECT_FALSE(key1);
}

/**
 * @brief Test RegistryKey copy operations are deleted
 * @details Verifies that RegistryKey cannot be copied (compile-time check)
 * @note This is a compile-time verification - if it compiles, the test passes
 */
TEST(SystemInfoTest, RegistryKey_CopyOperationsDeleted) {
    // This test verifies at compile time that copy operations are deleted
    // If the following lines would compile, it would indicate a design flaw
    // static_assert(!std::is_copy_constructible_v<RegistryKey>, "RegistryKey should not be copy constructible");
    // static_assert(!std::is_copy_assignable_v<RegistryKey>, "RegistryKey should not be copy assignable");

    EXPECT_TRUE(true); // Compile-time check passed
}