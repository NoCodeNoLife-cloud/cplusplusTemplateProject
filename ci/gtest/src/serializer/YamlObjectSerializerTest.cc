/**
 * @file YamlObjectSerializerTest.cc
 * @brief Unit tests for YamlObjectSerializer
 * @details Tests cover file-based YAML serialization and deserialization with cleanup.
 */

#include <cstdint>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "serializer/YamlObjectSerializer.hpp"

using namespace common::serializer;

namespace
{
    /// @brief Simple config struct for YAML serialization testing
    struct TestConfig
    {
        std::string host;
        int port = 0;
        bool debug = false;

        bool operator==(const TestConfig& other) const
        {
            return host == other.host && port == other.port && debug == other.debug;
        }
    };
}

template <>
struct YAML::convert<TestConfig>
{
    static Node encode(const TestConfig& cfg)
    {
        Node node;
        node["host"] = cfg.host;
        node["port"] = cfg.port;
        node["debug"] = cfg.debug;
        return node;
    }

    static bool decode(const Node& node, TestConfig& cfg)
    {
        cfg.host = node["host"].as<std::string>();
        cfg.port = node["port"].as<int>();
        cfg.debug = node["debug"].as<bool>();
        return true;
    }
};

class YamlObjectSerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        serializer_ = std::make_unique<YamlObjectSerializer<TestConfig>>();
        test_file_ = std::filesystem::temp_directory_path() / "yaml_test_config.yaml";
    }

    void TearDown() override
    {
        serializer_.reset();
        if (std::filesystem::exists(test_file_))
        {
            std::filesystem::remove(test_file_);
        }
    }

    std::unique_ptr<YamlObjectSerializer<TestConfig>> serializer_;
    std::filesystem::path test_file_;
};

TEST_F(YamlObjectSerializerTest, SerializeAndDeserializePreservesData)
{
    TestConfig original{"localhost", 8080, true};

    serializer_->serialize(original, test_file_.string());
    TestConfig result = serializer_->deserialize(test_file_.string());

    EXPECT_EQ(result, original);
}

TEST_F(YamlObjectSerializerTest, DeserializeThrowsForMissingFile)
{
    EXPECT_THROW(
        serializer_->deserialize((test_file_.string() + "_nonexistent")),
        std::runtime_error);
}

TEST_F(YamlObjectSerializerTest, SerializeThrowsForEmptyFilename)
{
    TestConfig cfg{"localhost", 8080, false};
    EXPECT_THROW(serializer_->serialize(cfg, ""), std::invalid_argument);
}

TEST_F(YamlObjectSerializerTest, DeserializeThrowsForEmptyFilename)
{
    EXPECT_THROW(serializer_->deserialize(""), std::runtime_error);
}

TEST_F(YamlObjectSerializerTest, OverwriteExistingFile)
{
    TestConfig original{"original", 111, false};
    serializer_->serialize(original, test_file_.string());

    TestConfig updated{"updated", 222, true};
    serializer_->serialize(updated, test_file_.string());

    TestConfig result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result, updated);
}
