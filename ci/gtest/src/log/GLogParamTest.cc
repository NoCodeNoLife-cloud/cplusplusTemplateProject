#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "param/GLogParam.hpp"

namespace
{
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

TEST_F(GLogParamTest, DefaultConstruction)
{
    const glog::param::GLogParam param;
    EXPECT_EQ(param.minLogLevel(), 0);
    EXPECT_EQ(param.logName(), "");
    EXPECT_FALSE(param.logToStderr());
    EXPECT_FALSE(param.customLogFormat());
}

TEST_F(GLogParamTest, ParameterizedConstruction)
{
    const glog::param::GLogParam param(3, "test_log", true, true);
    EXPECT_EQ(param.minLogLevel(), 3);
    EXPECT_EQ(param.logName(), "test_log");
    EXPECT_TRUE(param.logToStderr());
    EXPECT_TRUE(param.customLogFormat());
}

TEST_F(GLogParamTest, ParameterizedConstructionDefaultCustomFormat)
{
    const glog::param::GLogParam param(0, "default_fmt", false);
    EXPECT_FALSE(param.customLogFormat());
}

TEST_F(GLogParamTest, MinLogLevelSetter)
{
    glog::param::GLogParam param;
    param.minLogLevel(5);
    EXPECT_EQ(param.minLogLevel(), 5);
}

TEST_F(GLogParamTest, LogNameSetter)
{
    glog::param::GLogParam param;
    param.logName("custom_name");
    EXPECT_EQ(param.logName(), "custom_name");
}

TEST_F(GLogParamTest, LogToStderrSetter)
{
    glog::param::GLogParam param;
    param.logToStderr(true);
    EXPECT_TRUE(param.logToStderr());
}

TEST_F(GLogParamTest, CustomLogFormatSetter)
{
    glog::param::GLogParam param;
    param.customLogFormat(true);
    EXPECT_TRUE(param.customLogFormat());
}

TEST_F(GLogParamTest, DeserializeFromFlatYaml)
{
    glog::param::GLogParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(testFile_));
    EXPECT_EQ(param.minLogLevel(), 1);
    EXPECT_EQ(param.logName(), "standalone_log");
    EXPECT_TRUE(param.logToStderr());
    EXPECT_TRUE(param.customLogFormat());
}

TEST_F(GLogParamTest, DeserializeFromNestedYaml)
{
    glog::param::GLogParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(nestedFile_));
    EXPECT_EQ(param.minLogLevel(), 2);
    EXPECT_EQ(param.logName(), "nested_log");
    EXPECT_FALSE(param.logToStderr());
    EXPECT_FALSE(param.customLogFormat());
}

TEST_F(GLogParamTest, DeserializeFromNonExistentFileThrows)
{
    glog::param::GLogParam param;
    auto nonexistent = std::filesystem::temp_directory_path() / "nonexistent_glog_param.yaml";
    EXPECT_THROW(param.deserializeFromYamlFile(nonexistent), std::runtime_error);
}

TEST_F(GLogParamTest, EqualityOperator)
{
    const glog::param::GLogParam a(1, "eq_log", true, false);
    const glog::param::GLogParam b(1, "eq_log", true, false);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST_F(GLogParamTest, InequalityOperator)
{
    const glog::param::GLogParam a(1, "log_a", true, false);
    const glog::param::GLogParam b(2, "log_b", false, true);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST_F(GLogParamTest, YamlEncodeDecodeRoundTrip)
{
    const glog::param::GLogParam original(3, "roundtrip", true, true);
    YAML::Node node;
    node = YAML::convert<glog::param::GLogParam>::encode(original);

    glog::param::GLogParam decoded;
    ASSERT_TRUE(YAML::convert<glog::param::GLogParam>::decode(node, decoded));
    EXPECT_EQ(original, decoded);
}

TEST_F(GLogParamTest, YamlDecodeNonMapNodeReturnsFalse)
{
    const YAML::Node scalar_node = YAML::Node("hello");
    glog::param::GLogParam param;
    EXPECT_FALSE(YAML::convert<glog::param::GLogParam>::decode(scalar_node, param));
}

TEST_F(GLogParamTest, LogNameReturnsReferenceNotCopy)
{
    glog::param::GLogParam param;
    param.logName("ref_check");
    const auto& ref = param.logName();
    EXPECT_EQ(ref, "ref_check");
    const void* addr1 = static_cast<const void*>(&ref);
    const void* addr2 = static_cast<const void*>(&param.logName());
    EXPECT_EQ(addr1, addr2);
}
