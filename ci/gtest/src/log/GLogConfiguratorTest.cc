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

using namespace cppforge::glog::config;

/// @brief Test fixture for GLogConfigurator tests.
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

/**
 * @brief Construct with valid YAML config file
 * @details Verifies no exception is thrown when constructing from valid YAML.
 */
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

/**
 * @brief Construct with malformed YAML
 * @details Verifies exception thrown for invalid YAML syntax.
 */
TEST_F(GLogConfiguratorTest, Constructor_InvalidYaml)
{
    writeYaml("invalid: yaml: [[");
    EXPECT_THROW(GLogConfigurator config(tmp_yaml_), std::exception);
}

/**
 * @brief Construct with nonexistent file path
 * @details Verifies exception thrown when config file does not exist.
 */
TEST_F(GLogConfiguratorTest, Constructor_NonExistentPath)
{
    EXPECT_THROW(GLogConfigurator config("nonexistent_file_xyz.yaml"), std::exception);
}

/**
 * @brief Construct with empty YAML file
 * @details Verifies no exception on empty file; defaults are used.
 */
TEST_F(GLogConfiguratorTest, Constructor_EmptyFile)
{
    writeYaml("");
    EXPECT_NO_THROW(GLogConfigurator config(tmp_yaml_));
}

/**
 * @brief GetConfig returns values matching input YAML
 * @details Verifies all config fields are correctly parsed after construction.
 */
TEST_F(GLogConfiguratorTest, GetConfig_AfterConstruction)
{
    writeYaml(R"(
glog:
  logName: test_app
  minLogLevel: 0
  logToStderr: true
  customLogFormat: false
)");
    const GLogConfigurator config(tmp_yaml_);
    const auto& cfg = config.getConfig();
    EXPECT_EQ(cfg.logName(), "test_app");
    EXPECT_EQ(cfg.minLogLevel(), 0);
    EXPECT_TRUE(cfg.logToStderr());
    EXPECT_FALSE(cfg.customLogFormat());
}

/**
 * @brief GetConfig extracts minLogLevel field
 * @details Verifies correct parsing of minLogLevel from YAML.
 */
TEST_F(GLogConfiguratorTest, GetConfig_MinLogLevel)
{
    writeYaml(R"(
glog:
  logName: test_app
  minLogLevel: 2
  logToStderr: false
  customLogFormat: true
)");
    const GLogConfigurator config(tmp_yaml_);
    EXPECT_EQ(config.getConfig().minLogLevel(), 2);
}

/**
 * @brief GetConfig with warning-level settings
 * @details Verifies logName and minLogLevel for warning-level config.
 */
TEST_F(GLogConfiguratorTest, GetConfig_WarningLogLevel)
{
    writeYaml(R"(
glog:
  logName: warn_test
  minLogLevel: 1
  logToStderr: true
  customLogFormat: false
)");
    const GLogConfigurator config(tmp_yaml_);
    EXPECT_EQ(config.getConfig().logName(), "warn_test");
    EXPECT_EQ(config.getConfig().minLogLevel(), 1);
}

/**
 * @brief IsInitialized returns false before any config
 * @details Verifies the static init flag is false initially.
 */
TEST_F(GLogConfiguratorTest, IsInitialized_InitiallyFalse)
{
    EXPECT_FALSE(GLogConfigurator::isInitialized());
}

/**
 * @brief Update config at runtime
 * @details Verifies updateConfig changes values and persists them.
 */
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

    cppforge::glog::param::GLogParam new_config;
    new_config.logName("updated");
    new_config.minLogLevel(3);
    config.updateConfig(new_config);
    EXPECT_EQ(config.getConfig().logName(), "updated");
    EXPECT_EQ(config.getConfig().minLogLevel(), 3);
}

/**
 * @brief Multiple configurators are independent
 * @details Verifies separate instances hold independent config state.
 */
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
