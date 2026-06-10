/**
 * @file GLogConfiguratorTest.cc
 * @brief Unit tests for the GLogConfigurator class
 * @details Tests cover construction, configuration loading, getConfig/updateConfig,
 *          and initialization state management.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "config/GLogConfigurator.hpp"
#include "param/GLogParam.hpp"

using namespace glog::config;

class GLogConfiguratorTest : public testing::Test
{
protected:
    std::string tmp_yaml_;

    void SetUp() override
    {
        const auto tmpDir = std::filesystem::temp_directory_path() / "GLogConfiguratorTest";
        std::filesystem::create_directories(tmpDir);
        tmp_yaml_ = (tmpDir / "test_glog.yaml").string();
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(tmp_yaml_, ec);
    }

    void writeYaml(const std::string& content)
    {
        std::ofstream f(tmp_yaml_);
        f << content;
    }
};

TEST_F(GLogConfiguratorTest, Constructor_ValidYaml)
{
    writeYaml(R"(
glog:
  logName: test_app
  minLogLevel: 0
  logToStderr: true
  customLogFormat: false
)");
    EXPECT_NO_THROW(GLogConfigurator config(tmp_yaml_));
}

TEST_F(GLogConfiguratorTest, Constructor_InvalidYaml)
{
    writeYaml("invalid: yaml: [[");
    EXPECT_THROW(GLogConfigurator config(tmp_yaml_), std::exception);
}

TEST_F(GLogConfiguratorTest, Constructor_NonExistentPath)
{
    EXPECT_THROW(GLogConfigurator config("nonexistent_file_xyz.yaml"), std::exception);
}

TEST_F(GLogConfiguratorTest, Constructor_EmptyFile)
{
    writeYaml("");
    EXPECT_NO_THROW(GLogConfigurator config(tmp_yaml_));
}

TEST_F(GLogConfiguratorTest, GetConfig_AfterConstruction)
{
    writeYaml(R"(
glog:
  logName: test_app
  minLogLevel: 0
  logToStderr: true
  customLogFormat: false
)");
    GLogConfigurator config(tmp_yaml_);
    const auto& cfg = config.getConfig();
    EXPECT_EQ(cfg.logName(), "test_app");
    EXPECT_EQ(cfg.minLogLevel(), 0);
    EXPECT_TRUE(cfg.logToStderr());
    EXPECT_FALSE(cfg.customLogFormat());
}

TEST_F(GLogConfiguratorTest, GetConfig_MinLogLevel)
{
    writeYaml(R"(
glog:
  logName: test_app
  minLogLevel: 2
  logToStderr: false
  customLogFormat: true
)");
    GLogConfigurator config(tmp_yaml_);
    EXPECT_EQ(config.getConfig().minLogLevel(), 2);
    EXPECT_FALSE(config.getConfig().logToStderr());
    EXPECT_TRUE(config.getConfig().customLogFormat());
}

TEST_F(GLogConfiguratorTest, GetConfig_WarningLogLevel)
{
    writeYaml(R"(
glog:
  logName: warn_test
  minLogLevel: 1
  logToStderr: true
  customLogFormat: false
)");
    GLogConfigurator config(tmp_yaml_);
    EXPECT_EQ(config.getConfig().logName(), "warn_test");
    EXPECT_EQ(config.getConfig().minLogLevel(), 1);
}

TEST_F(GLogConfiguratorTest, IsInitialized_InitiallyFalse)
{
    EXPECT_FALSE(GLogConfigurator::isInitialized());
}

TEST_F(GLogConfiguratorTest, UpdateConfig)
{
    writeYaml(R"(
glog:
  logName: original
  minLogLevel: 0
  logToStderr: false
  customLogFormat: false
)");
    GLogConfigurator config(tmp_yaml_);
    EXPECT_EQ(config.getConfig().logName(), "original");

    glog::param::GLogParam new_config;
    new_config.logName("updated");
    new_config.minLogLevel(3);
    config.updateConfig(new_config);
    EXPECT_EQ(config.getConfig().logName(), "updated");
    EXPECT_EQ(config.getConfig().minLogLevel(), 3);
}

TEST_F(GLogConfiguratorTest, MultipleConfigurators_Independent)
{
    writeYaml(R"(
glog:
  logName: first
  minLogLevel: 0
  logToStderr: false
  customLogFormat: false
)");
    const std::string yaml2 = tmp_yaml_ + ".2.yaml";
    {
        std::ofstream f(yaml2);
        f << R"(
glog:
  logName: second
  minLogLevel: 2
  logToStderr: true
  customLogFormat: true
)";
    }

    GLogConfigurator config1(tmp_yaml_);
    GLogConfigurator config2(yaml2);

    EXPECT_EQ(config1.getConfig().logName(), "first");
    EXPECT_EQ(config2.getConfig().logName(), "second");
    EXPECT_EQ(config2.getConfig().minLogLevel(), 2);

    std::error_code ec;
    std::filesystem::remove(yaml2, ec);
}
