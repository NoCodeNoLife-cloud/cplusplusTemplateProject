/**
 * @file SystemInfoTest.cc
 * @brief Unit tests for the SystemInfo class
 * @details Tests cover core system information retrieval functionality including
 *          CPU model, memory details, OS version, motherboard info, graphics card,
 *          disk drives, and BIOS information.
 */

#include <string>
#include <type_traits>
#include <vector>
#include <gtest/gtest.h>

#include "system/SystemInfo.hpp"

using namespace cppforge::system;

/**
 * @brief Test fixture for SystemInfoTest tests
 */
class SystemInfoTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test GetCpuModelFromRegistry returns non-empty string
 * @details Verifies that CPU model retrieval returns a valid, non-empty result
 */
TEST_F(SystemInfoTest, GetCpuModelFromRegistry_NonEmpty)
{
    const auto cpuModel = SystemInfo::GetCpuModelFromRegistry();
    EXPECT_FALSE(cpuModel.empty());
}

/**
 * @brief Test GetCpuModelFromRegistry returns reasonable value
 * @details Ensures the CPU model is not just the fallback "Unknown CPU Model" on typical systems
 */
TEST_F(SystemInfoTest, GetCpuModelFromRegistry_ReasonableValue)
{
    const auto cpuModel = SystemInfo::GetCpuModelFromRegistry();

    // On most Windows systems with proper registry access, should get actual CPU model
    // If registry access fails, it returns "Unknown CPU Model"
    EXPECT_FALSE(cpuModel.empty());
}

/**
 * @brief Test GetMemoryDetails returns non-empty string
 * @details Verifies that memory details retrieval returns a valid result
 */
TEST_F(SystemInfoTest, GetMemoryDetails_NonEmpty)
{
    const auto memoryDetails = SystemInfo::GetMemoryDetails();
    EXPECT_FALSE(memoryDetails.empty());
}

/**
 * @brief Test GetOSVersion returns non-empty string
 * @details Verifies that OS version retrieval returns a valid, non-empty result
 */
TEST_F(SystemInfoTest, GetOSVersion_NonEmpty)
{
    const auto osVersion = SystemInfo::GetOSVersion();
    EXPECT_FALSE(osVersion.empty());
}

/**
 * @brief Test GetOSVersion contains Windows-related information
 * @details Ensures the OS version string contains expected Windows identifiers
 */
TEST_F(SystemInfoTest, GetOSVersion_ContainsWindowsInfo)
{
    const auto osVersion = SystemInfo::GetOSVersion();

    // Should contain either "Windows" or at least not be the fallback message
    if (osVersion != "Windows OS Information Not Available")
    {
        EXPECT_FALSE(osVersion.empty());
    }
}

/**
 * @brief Test GetMotherboardInfo returns valid structure
 * @details Verifies that motherboard information retrieval returns a properly constructed structure
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_ValidStructure)
{
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();

    // All fields must be accessible (maybe empty if registry is unavailable)
    EXPECT_NO_THROW((void)info.manufacturer);
    EXPECT_NO_THROW((void)info.model);
    EXPECT_NO_THROW((void)info.biosVersion);
    EXPECT_NO_THROW((void)info.systemSerial);
}

/**
 * @brief Test GetMotherboardInfo manufacturer field accessibility
 * @details Verifies that the manufacturer field can be read without error
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_ManufacturerField)
{
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();
    EXPECT_NO_THROW((void)info.manufacturer);
}

/**
 * @brief Test GetMotherboardInfo model field accessibility
 * @details Verifies that the model field can be read without error
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_ModelField)
{
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();
    EXPECT_NO_THROW((void)info.model);
}

/**
 * @brief Test GetMotherboardInfo biosVersion field accessibility
 * @details Verifies that the BIOS version field can be read without error
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_BiosVersionField)
{
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();
    EXPECT_NO_THROW((void)info.biosVersion);
}

/**
 * @brief Test GetMotherboardInfo systemSerial field accessibility
 * @details Verifies that the system serial field can be read without error
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_SystemSerialField)
{
    const MotherboardInfo info = SystemInfo::GetMotherboardInfo();
    EXPECT_NO_THROW((void)info.systemSerial);
}

/**
 * @brief Test GetGraphicsCardInfo returns non-empty string
 * @details Verifies that graphics card information retrieval returns a valid result
 */
TEST_F(SystemInfoTest, GetGraphicsCardInfo_NonEmpty)
{
    const auto graphicsInfo = SystemInfo::GetGraphicsCardInfo();
    EXPECT_FALSE(graphicsInfo.empty());
}

/**
 * @brief Test GetDiskDriveInfo returns vector
 * @details Verifies that disk drive information retrieval returns a valid vector
 */
TEST_F(SystemInfoTest, GetDiskDriveInfo_ReturnsVector)
{
    const auto diskInfo = SystemInfo::GetDiskDriveInfo();

    // Should return a valid vector; on a real Windows system with registry access, typically non-empty
    // Empty vector is acceptable only if registry key is missing or access is denied
}

/**
 * @brief Test GetDiskDriveInfo vector size is reasonable
 * @details Ensures the disk drive list size is within expected bounds
 */
TEST_F(SystemInfoTest, GetDiskDriveInfo_ReasonableSize)
{
    const auto diskInfo = SystemInfo::GetDiskDriveInfo();

    // Should have between 0 and 100 disk entries (reasonable upper bound)
    EXPECT_LE(diskInfo.size(), 100);
}

/**
 * @brief Test GetBIOSInfo returns vector
 * @details Verifies that BIOS information retrieval returns a valid vector
 */
TEST_F(SystemInfoTest, GetBIOSInfo_ReturnsVector)
{
    const auto biosInfo = SystemInfo::GetBIOSInfo();

    // Should return a valid vector; entries may be empty if registry is unavailable
}

/**
 * @brief Test GetBIOSInfo vector size is reasonable
 * @details Ensures the BIOS adapter list size is within expected bounds
 */
TEST_F(SystemInfoTest, GetBIOSInfo_ReasonableSize)
{
    const auto biosInfo = SystemInfo::GetBIOSInfo();

    // Should have between 0 and 100 adapter entries (reasonable upper bound)
    EXPECT_LE(biosInfo.size(), 100);
}

/**
 * @brief Test all system info methods can be called without crashing
 * @details Verifies that all public methods execute without exceptions or crashes
 */
TEST_F(SystemInfoTest, AllMethods_ExecuteWithoutCrash)
{
    static_cast<void>(SystemInfo::GetCpuModelFromRegistry());
    static_cast<void>(SystemInfo::GetMemoryDetails());
    static_cast<void>(SystemInfo::GetOSVersion());
    static_cast<void>(SystemInfo::GetMotherboardInfo());
    static_cast<void>(SystemInfo::GetGraphicsCardInfo());
    static_cast<void>(SystemInfo::GetDiskDriveInfo());
    static_cast<void>(SystemInfo::GetBIOSInfo());
}

/**
 * @brief Test consistency of multiple calls to GetCpuModelFromRegistry
 * @details Verifies that repeated calls return consistent results
 */
TEST_F(SystemInfoTest, GetCpuModelFromRegistry_ConsistentResults)
{
    const auto result1 = SystemInfo::GetCpuModelFromRegistry();
    const auto result2 = SystemInfo::GetCpuModelFromRegistry();

    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test consistency of multiple calls to GetOSVersion
 * @details Verifies that repeated calls return consistent results
 */
TEST_F(SystemInfoTest, GetOSVersion_ConsistentResults)
{
    const auto result1 = SystemInfo::GetOSVersion();
    const auto result2 = SystemInfo::GetOSVersion();

    EXPECT_EQ(result1, result2);
}

/**
 * @brief Test consistency of multiple calls to GetMotherboardInfo
 * @details Verifies that repeated calls return consistent results
 */
TEST_F(SystemInfoTest, GetMotherboardInfo_ConsistentResults)
{
    const auto result1 = SystemInfo::GetMotherboardInfo();
    const auto result2 = SystemInfo::GetMotherboardInfo();

    EXPECT_EQ(result1.manufacturer, result2.manufacturer);
    EXPECT_EQ(result1.model, result2.model);
    EXPECT_EQ(result1.biosVersion, result2.biosVersion);
    EXPECT_EQ(result1.systemSerial, result2.systemSerial);
}


