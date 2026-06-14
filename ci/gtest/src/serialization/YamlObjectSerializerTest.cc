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

#include "serialization/YamlObjectSerializer.hpp"

using namespace common::serialization;

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

/// @brief Test fixture for YamlObjectSerializer tests.
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

/**
 * @brief Serialize and deserialize preserves all data
 * @details Verifies round-trip of a complete TestConfig object.
 */
TEST_F(YamlObjectSerializerTest, SerializeAndDeserializePreservesData)
{
    const TestConfig original{"localhost", 8080, true};

    serializer_->serialize(original, test_file_.string());
    const TestConfig result = serializer_->deserialize(test_file_.string());

    EXPECT_EQ(result, original);
}

/**
 * @brief Deserialize throws for nonexistent file
 * @details Verifies runtime_error when the YAML file does not exist.
 */
TEST_F(YamlObjectSerializerTest, DeserializeThrowsForMissingFile)
{
    EXPECT_THROW(
        static_cast<void>(serializer_->deserialize((test_file_.string() + "_nonexistent"))),
        std::runtime_error);
}

/**
 * @brief Serialize throws for empty filename
 * @details Verifies invalid_argument when filename is empty.
 */
TEST_F(YamlObjectSerializerTest, SerializeThrowsForEmptyFilename)
{
    const TestConfig cfg{"localhost", 8080, false};
    EXPECT_THROW(serializer_->serialize(cfg, ""), std::invalid_argument);
}

/**
 * @brief Deserialize throws for empty filename
 * @details Verifies runtime_error when filename is empty.
 */
TEST_F(YamlObjectSerializerTest, DeserializeThrowsForEmptyFilename)
{
    EXPECT_THROW(static_cast<void>(serializer_->deserialize("")), std::runtime_error);
}

/**
 * @brief Overwrite existing file with new data
 * @details Verifies that serializing again overwrites the old content.
 */
TEST_F(YamlObjectSerializerTest, OverwriteExistingFile)
{
    const TestConfig original{"original", 111, false};
    serializer_->serialize(original, test_file_.string());

    const TestConfig updated{"updated", 222, true};
    serializer_->serialize(updated, test_file_.string());

    const TestConfig result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result, updated);
}

/**
 * @brief Test serialization of empty/default values
 * @details Verifies empty string and default values survive round-trip
 */
TEST_F(YamlObjectSerializerTest, EmptyValues)
{
    const TestConfig original{"", 0, false};
    serializer_->serialize(original, test_file_.string());
    const TestConfig result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result, original);
}


/**
 * @brief Test multiple serialize/deserialize cycles
 * @details Verifies that repeated round-trips preserve data
 */
TEST_F(YamlObjectSerializerTest, MultipleCycles)
{
    TestConfig config{"cycle", 999, true};
    for (int i = 0; i < 10; ++i)
    {
        serializer_->serialize(config, test_file_.string());
        config = serializer_->deserialize(test_file_.string());
    }
    EXPECT_EQ(config.host, "cycle");
    EXPECT_EQ(config.port, 999);
    EXPECT_TRUE(config.debug);
}

/**
 * @brief Test deserialize from malformed YAML file
 * @details Verifies that malformed YAML throws an exception
 */
TEST_F(YamlObjectSerializerTest, MalformedYaml)
{
    {
        std::ofstream f(test_file_);
        f << "host: localhost\nport: not_an_int\ndebug: maybe\n";
    }
    EXPECT_THROW(static_cast<void>(serializer_->deserialize(test_file_.string())), std::exception);
}

/**
 * @brief Test deserialize from empty YAML file
 * @details Verifies that an empty file throws
 */
TEST_F(YamlObjectSerializerTest, EmptyFile)
{
    {
        std::ofstream f(test_file_);
        f << "";
    }
    EXPECT_THROW(static_cast<void>(serializer_->deserialize(test_file_.string())), std::exception);
}

/**
 * @brief Test deserialize from YAML with extra fields
 * @details Extra fields should not prevent deserialization
 */
TEST_F(YamlObjectSerializerTest, ExtraFields)
{
    {
        std::ofstream f(test_file_);
        f << "host: localhost\nport: 8080\ndebug: true\nextra_field: ignored\n";
    }
    const TestConfig result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result.host, "localhost");
    EXPECT_EQ(result.port, 8080);
    EXPECT_TRUE(result.debug);
}

/**
 * @brief Test deserialize with partial data
 * @details Missing required fields should throw
 */
TEST_F(YamlObjectSerializerTest, PartialData)
{
    {
        std::ofstream f(test_file_);
        f << "host: localhost\n";
    }
    EXPECT_THROW(static_cast<void>(serializer_->deserialize(test_file_.string())), std::exception);
}

/**
 * @brief Test serialization preserves YAML structure
 * @details Check that serialized YAML can be loaded by yaml-cpp directly
 */
TEST_F(YamlObjectSerializerTest, VerifyYamlStructure)
{
    const TestConfig original{"verify_host", 443, true};
    serializer_->serialize(original, test_file_.string());

    YAML::Node root = YAML::LoadFile(test_file_.string());
    EXPECT_EQ(root["host"].as<std::string>(), "verify_host");
    EXPECT_EQ(root["port"].as<int>(), 443);
    EXPECT_EQ(root["debug"].as<bool>(), true);
}

/**
 * @brief Test serialization with extreme port values
 * @details Boundary values for numeric fields
 */
TEST_F(YamlObjectSerializerTest, ExtremeValues)
{
    TestConfig original{"extreme", 0, true};
    serializer_->serialize(original, test_file_.string());
    TestConfig result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result.port, 0);

    original.port = 65535;
    serializer_->serialize(original, test_file_.string());
    result = serializer_->deserialize(test_file_.string());
    EXPECT_EQ(result.port, 65535);
}

/**
 * @brief Test concurrent serialization to different files
 * @details Multiple serializer instances working independently
 */
TEST_F(YamlObjectSerializerTest, MultipleFiles)
{
    const auto path2 = std::filesystem::temp_directory_path() / "yaml_test_config_2.yaml";
    YamlObjectSerializer<TestConfig> ser2;

    const TestConfig cfg1{"first", 100, false};
    const TestConfig cfg2{"second", 200, true};

    serializer_->serialize(cfg1, test_file_.string());
    ser2.serialize(cfg2, path2.string());

    const auto result1 = serializer_->deserialize(test_file_.string());
    const auto result2 = ser2.deserialize(path2.string());

    EXPECT_EQ(result1.host, "first");
    EXPECT_EQ(result2.host, "second");

    std::error_code ec;
    std::filesystem::remove(path2, ec);
}
