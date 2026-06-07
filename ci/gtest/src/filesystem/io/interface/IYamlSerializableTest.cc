/**
 * @file IYamlSerializableTest.cc
 * @brief Unit tests for IYamlSerializable interface
 * @details Tests cover encode, decode, and YAML::convert specialization via a mock implementation.
 */

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "interface/serialize/IYamlSerializable.hpp"

using namespace common::interfaces::serialize;

namespace
{
    /// @brief Mock implementation of IYamlSerializable for testing
    class MockYamlSerializable : public IYamlSerializable
    {
    public:
        std::string name_;
        int value_ = 0;

        MockYamlSerializable() = default;

        MockYamlSerializable(std::string name, int value)
            : name_(std::move(name)), value_(value)
        {
        }

        [[nodiscard]] YAML::Node encode() const override
        {
            YAML::Node node;
            node["name"] = name_;
            node["value"] = value_;
            return node;
        }

        bool decode(const YAML::Node& node) override
        {
            if (!node["name"] || !node["value"])
                return false;
            try
            {
                name_ = node["name"].as<std::string>();
                value_ = node["value"].as<int>();
                return true;
            }
            catch (const YAML::BadConversion&)
            {
                return false;
            }
        }
    };
}

class IYamlSerializableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        obj_.reset(new MockYamlSerializable("test", 42));
    }

    void TearDown() override
    {
        obj_.reset();
    }

    std::unique_ptr<MockYamlSerializable> obj_;
};

TEST_F(IYamlSerializableTest, EncodeProducesValidYaml)
{
    YAML::Node node = obj_->encode();

    ASSERT_TRUE(node["name"].IsDefined());
    ASSERT_TRUE(node["value"].IsDefined());
    EXPECT_EQ(node["name"].as<std::string>(), "test");
    EXPECT_EQ(node["value"].as<int>(), 42);
}

TEST_F(IYamlSerializableTest, DecodePopulatesFields)
{
    YAML::Node node;
    node["name"] = "updated";
    node["value"] = 99;

    bool result = obj_->decode(node);
    ASSERT_TRUE(result);
    EXPECT_EQ(obj_->name_, "updated");
    EXPECT_EQ(obj_->value_, 99);
}

TEST_F(IYamlSerializableTest, RoundTripPreservesData)
{
    YAML::Node node = obj_->encode();

    MockYamlSerializable deserialized;
    bool result = deserialized.decode(node);

    ASSERT_TRUE(result);
    EXPECT_EQ(deserialized.name_, "test");
    EXPECT_EQ(deserialized.value_, 42);
}

TEST_F(IYamlSerializableTest, DecodeMissingFieldReturnsFalse)
{
    YAML::Node node;
    node["name"] = "partial";

    bool result = obj_->decode(node);
    EXPECT_FALSE(result);
}

TEST_F(IYamlSerializableTest, DecodeWrongTypeReturnsFalse)
{
    YAML::Node node;
    node["name"] = 123;
    node["value"] = "abc";

    bool result = obj_->decode(node);
    EXPECT_FALSE(result);
}

TEST_F(IYamlSerializableTest, YamlConvertEncode)
{
    MockYamlSerializable ms("convert", 77);
    YAML::Node node = YAML::convert<MockYamlSerializable>::encode(ms);

    EXPECT_EQ(node["name"].as<std::string>(), "convert");
    EXPECT_EQ(node["value"].as<int>(), 77);
}

TEST_F(IYamlSerializableTest, YamlConvertDecode)
{
    YAML::Node node;
    node["name"] = "converted";
    node["value"] = 88;

    MockYamlSerializable ms;
    bool result = YAML::convert<MockYamlSerializable>::decode(node, ms);

    ASSERT_TRUE(result);
    EXPECT_EQ(ms.name_, "converted");
    EXPECT_EQ(ms.value_, 88);
}

TEST_F(IYamlSerializableTest, EmptyEncode)
{
    MockYamlSerializable empty("", 0);
    YAML::Node node = empty.encode();

    EXPECT_EQ(node["name"].as<std::string>(), "");
    EXPECT_EQ(node["value"].as<int>(), 0);
}
