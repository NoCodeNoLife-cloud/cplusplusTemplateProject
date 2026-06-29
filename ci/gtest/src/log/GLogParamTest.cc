/**
 * @file GLogParamTest.cc
 * @brief Unit tests for the GLogParam class
 * @details Tests cover construction, setters, YAML deserialization,
 *          comparison operators, encode/decode round-trip, and edge cases.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "param/GLogParam.hpp"

namespace
{
    /// @brief Test fixture for GLogParam tests.
    class GLogParamTest : public testing::Test
    {
    protected:
        void SetUp() override
        {
            testFile_ = std::filesystem::temp_directory_path() / "glog_param_test.yaml";
            nestedFile_ = std::filesystem::temp_directory_path() / "glog_param_nested.yaml";
            {
                std::ofstream ofs(testFile_);
                ofs << "minLogLevel: 1\n"
                       "logName: standalone_log\n"
                       "logToStderr: true\n"
                       "customLogFormat: true\n";
            }
            {
                std::ofstream ofs(nestedFile_);
                ofs << "grpc:\n"
                       "  serverAddress: localhost:50051\n"
                       "glog:\n"
                       "  minLogLevel: 2\n"
                       "  logName: nested_log\n"
                       "  logToStderr: false\n"
                       "  customLogFormat: false\n";
            }
        }

        void TearDown() override
        {
            if (std::filesystem::exists(testFile_))
                std::filesystem::remove(testFile_);
            if (std::filesystem::exists(nestedFile_))
                std::filesystem::remove(nestedFile_);
        }

        std::filesystem::path testFile_;
        std::filesystem::path nestedFile_;
    };
}

/**
 * @brief Default construction uses zero values
 * @details Verifies default-constructed GLogParam has expected defaults.
 */
TEST_F(GLogParamTest, DefaultConstruction)
{
    const cppforge::glog::param::GLogParam param;
    EXPECT_EQ(param.minLogLevel(), 0);
    EXPECT_EQ(param.logName(), "");
    EXPECT_FALSE(param.logToStderr());
    EXPECT_FALSE(param.customLogFormat());
}

/**
 * @brief Parameterized construction sets all fields
 * @details Verifies 4-argument constructor assigns all values correctly.
 */
TEST_F(GLogParamTest, ParameterizedConstruction)
{
    const cppforge::glog::param::GLogParam param(3, "test_log", true, true);
    EXPECT_EQ(param.minLogLevel(), 3);
    EXPECT_EQ(param.logName(), "test_log");
    EXPECT_TRUE(param.logToStderr());
    EXPECT_TRUE(param.customLogFormat());
}

/**
 * @brief Parameterized construction with default custom format
 * @details Verifies 3-argument constructor leaves customLogFormat as false.
 */
TEST_F(GLogParamTest, ParameterizedConstructionDefaultCustomFormat)
{
    const cppforge::glog::param::GLogParam param(0, "default_fmt", false);
    EXPECT_FALSE(param.customLogFormat());
}

/**
 * @brief Setter for minLogLevel
 * @details Verifies minLogLevel setter returns the expected value.
 */
TEST_F(GLogParamTest, MinLogLevelSetter)
{
    cppforge::glog::param::GLogParam param;
    param.minLogLevel(5);
    EXPECT_EQ(param.minLogLevel(), 5);
}

/**
 * @brief Setter for logName
 * @details Verifies logName setter returns the expected value.
 */
TEST_F(GLogParamTest, LogNameSetter)
{
    cppforge::glog::param::GLogParam param;
    param.logName("custom_name");
    EXPECT_EQ(param.logName(), "custom_name");
}

/**
 * @brief Setter for logToStderr
 * @details Verifies logToStderr setter returns the expected value.
 */
TEST_F(GLogParamTest, LogToStderrSetter)
{
    cppforge::glog::param::GLogParam param;
    param.logToStderr(true);
    EXPECT_TRUE(param.logToStderr());
}

/**
 * @brief Setter for customLogFormat
 * @details Verifies customLogFormat setter returns the expected value.
 */
TEST_F(GLogParamTest, CustomLogFormatSetter)
{
    cppforge::glog::param::GLogParam param;
    param.customLogFormat(true);
    EXPECT_TRUE(param.customLogFormat());
}

/**
 * @brief Deserialize from flat YAML file
 * @details Verifies deserialization of top-level YAML fields.
 */
TEST_F(GLogParamTest, DeserializeFromFlatYaml)
{
    cppforge::glog::param::GLogParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(testFile_));
    EXPECT_EQ(param.minLogLevel(), 1);
    EXPECT_EQ(param.logName(), "standalone_log");
    EXPECT_TRUE(param.logToStderr());
    EXPECT_TRUE(param.customLogFormat());
}

/**
 * @brief Deserialize from nested YAML under glog: key
 * @details Verifies deserialization from nested YAML structure.
 */
TEST_F(GLogParamTest, DeserializeFromNestedYaml)
{
    cppforge::glog::param::GLogParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(nestedFile_));
    EXPECT_EQ(param.minLogLevel(), 2);
    EXPECT_EQ(param.logName(), "nested_log");
    EXPECT_FALSE(param.logToStderr());
    EXPECT_FALSE(param.customLogFormat());
}

/**
 * @brief Deserialize from nonexistent file throws
 * @details Verifies runtime_error when YAML file is missing.
 */
TEST_F(GLogParamTest, DeserializeFromNonExistentFileThrows)
{
    cppforge::glog::param::GLogParam param;
    auto nonexistent = std::filesystem::temp_directory_path() / "nonexistent_glog_param.yaml";
    EXPECT_THROW(param.deserializeFromYamlFile(nonexistent), std::runtime_error);
}

/**
 * @brief Equality operator for identical params
 * @details Verifies operator== returns true for equal objects.
 */
TEST_F(GLogParamTest, EqualityOperator)
{
    const cppforge::glog::param::GLogParam a(1, "eq_log", true, false);
    const cppforge::glog::param::GLogParam b(1, "eq_log", true, false);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Inequality operator for different params
 * @details Verifies operator!= returns true for different objects.
 */
TEST_F(GLogParamTest, InequalityOperator)
{
    const cppforge::glog::param::GLogParam a(1, "log_a", true, false);
    const cppforge::glog::param::GLogParam b(2, "log_b", false, true);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

/**
 * @brief YAML encode/decode round-trip
 * @details Verifies encode and decode preserve all field values.
 */
TEST_F(GLogParamTest, YamlEncodeDecodeRoundTrip)
{
    const cppforge::glog::param::GLogParam original(3, "roundtrip", true, true);
    YAML::Node node;
    node = YAML::convert<cppforge::glog::param::GLogParam>::encode(original);

    cppforge::glog::param::GLogParam decoded;
    ASSERT_TRUE(YAML::convert<cppforge::glog::param::GLogParam>::decode(node, decoded));
    EXPECT_EQ(original, decoded);
}

/**
 * @brief Decode non-map node returns false
 * @details Verifies decode returns false for scalar YAML nodes.
 */
TEST_F(GLogParamTest, YamlDecodeNonMapNodeReturnsFalse)
{
    const YAML::Node scalar_node = YAML::Node("hello");
    cppforge::glog::param::GLogParam param;
    EXPECT_FALSE(YAML::convert<cppforge::glog::param::GLogParam>::decode(scalar_node, param));
}

/**
 * @brief logName returns a reference, not a copy
 * @details Verifies that logName() getter returns a const reference.
 */
TEST_F(GLogParamTest, LogNameReturnsReferenceNotCopy)
{
    cppforge::glog::param::GLogParam param;
    param.logName("ref_check");
    const auto& ref = param.logName();
    EXPECT_EQ(ref, "ref_check");
    const void* addr1 = static_cast<const void*>(&ref);
    const void* addr2 = static_cast<const void*>(&param.logName());
    EXPECT_EQ(addr1, addr2);
}
