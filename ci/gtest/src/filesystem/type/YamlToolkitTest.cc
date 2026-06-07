/**
 * @file YamlToolkitTest.cc
 * @brief Unit tests for the YamlToolkit class
 * @details Tests cover CRUD operations, value get/set, nested path access,
 *          key existence checks, merge, and string conversions.
 */

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "filesystem/type/YamlToolkit.hpp"

using namespace common::filesystem::type;
namespace fs = std::filesystem;

class YamlToolkitTest : public testing::Test
{
protected:
    fs::path tmpDir_;

    void SetUp() override
    {
        tmpDir_ = fs::temp_directory_path() / "YamlToolkitTest";
        fs::remove_all(tmpDir_);
        fs::create_directories(tmpDir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tmpDir_, ec);
    }

    [[nodiscard]] std::string path(const std::string& name) const
    {
        return (tmpDir_ / name).string();
    }
};

TEST_F(YamlToolkitTest, CreateYamlFile)
{
    const auto filepath = path("create_test.yml");
    YAML::Node data;
    data["name"] = "test";
    data["value"] = 42;

    EXPECT_TRUE(YamlToolkit::create(filepath, data));
    EXPECT_TRUE(fs::exists(filepath));
}

TEST_F(YamlToolkitTest, CreateYamlFileWithNestedDir)
{
    const auto filepath = path("nested/subdir/create_test.yml");
    YAML::Node data;
    data["key"] = "value";

    EXPECT_TRUE(YamlToolkit::create(filepath, data));
    EXPECT_TRUE(fs::exists(filepath));
}

TEST_F(YamlToolkitTest, CreateYamlFileReturnsFalseOnInvalidPath)
{
    YAML::Node data;
    data["x"] = 1;
    EXPECT_FALSE(YamlToolkit::create("", data));
}

TEST_F(YamlToolkitTest, ReadExistingFile)
{
    const auto filepath = path("read_test.yml");
    YAML::Node data;
    data["hello"] = "world";

    YamlToolkit::create(filepath, data);
    const auto result = YamlToolkit::read(filepath);

    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["hello"].as<std::string>(), "world");
}

TEST_F(YamlToolkitTest, UpdateExistingFile)
{
    const auto filepath = path("update_test.yml");
    YAML::Node data;
    data["version"] = 1;
    YamlToolkit::create(filepath, data);

    YAML::Node newData;
    newData["version"] = 2;
    EXPECT_TRUE(YamlToolkit::update(filepath, newData));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["version"].as<int>(), 2);
}

TEST_F(YamlToolkitTest, RemoveExistingFile)
{
    const auto filepath = path("remove_test.yml");
    YAML::Node data;
    data["x"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_TRUE(YamlToolkit::remove(filepath));
    EXPECT_FALSE(fs::exists(filepath));
}

TEST_F(YamlToolkitTest, RemoveNonExistentFile)
{
    EXPECT_FALSE(YamlToolkit::remove(path("ghost.yml")));
}

TEST_F(YamlToolkitTest, GetValueExistingKey)
{
    const auto filepath = path("getvalue_test.yml");
    YAML::Node data;
    data["name"] = "alice";
    data["age"] = 30;
    YamlToolkit::create(filepath, data);

    const auto name = YamlToolkit::getValue(filepath, "name");
    EXPECT_TRUE(name.IsDefined());
    EXPECT_EQ(name.as<std::string>(), "alice");
}

TEST_F(YamlToolkitTest, GetValueNonExistentKey)
{
    const auto filepath = path("getvalue_missing.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getValue(filepath, "nonexistent");
    EXPECT_FALSE(result.IsDefined());
}

TEST_F(YamlToolkitTest, GetValueNonExistentFile)
{
    const auto result = YamlToolkit::getValue(path("no_file.yml"), "key");
    EXPECT_FALSE(result.IsDefined());
}

TEST_F(YamlToolkitTest, SetValueExistingFile)
{
    const auto filepath = path("setvalue_test.yml");
    YAML::Node data;
    data["initial"] = "old";
    YamlToolkit::create(filepath, data);

    EXPECT_TRUE(YamlToolkit::setValue(filepath, "new_key", YAML::Node("new_value")));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["initial"].as<std::string>(), "old");
    EXPECT_EQ(result["new_key"].as<std::string>(), "new_value");
}

TEST_F(YamlToolkitTest, SetValueNewFile)
{
    const auto filepath = path("setvalue_new.yml");
    EXPECT_TRUE(YamlToolkit::setValue(filepath, "key", YAML::Node(123)));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["key"].as<int>(), 123);
}

TEST_F(YamlToolkitTest, GetNestedValue)
{
    const auto filepath = path("nested_get.yml");
    YAML::Node data;
    data["parent"]["child"]["value"] = 99;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getNestedValue(filepath, "parent.child.value");
    EXPECT_TRUE(result.IsDefined());
    EXPECT_EQ(result.as<int>(), 99);
}

TEST_F(YamlToolkitTest, GetNestedValueNonExistent)
{
    const auto filepath = path("nested_get_missing.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getNestedValue(filepath, "a.b.c");
    EXPECT_FALSE(result.IsDefined());
}

TEST_F(YamlToolkitTest, GetNestedValueEmptyPath)
{
    const auto filepath = path("nested_get_empty.yml");
    YAML::Node data;
    data["x"] = 10;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getNestedValue(filepath, "");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["x"].as<int>(), 10);
}

TEST_F(YamlToolkitTest, SetNestedValueEmptyPath)
{
    const auto filepath = path("nested_set_empty.yml");
    YAML::Node data;
    data["original"] = "keep";
    YamlToolkit::create(filepath, data);

    YAML::Node replacement;
    replacement["completely"] = "new";
    EXPECT_TRUE(YamlToolkit::setNestedValue(filepath, "", replacement));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["completely"].as<std::string>(), "new");
    EXPECT_FALSE(result["original"].IsDefined());
}

TEST_F(YamlToolkitTest, HasKeyReturnsTrue)
{
    const auto filepath = path("haskey_true.yml");
    YAML::Node data;
    data["present"] = "here";
    YamlToolkit::create(filepath, data);

    EXPECT_TRUE(YamlToolkit::hasKey(filepath, "present"));
}

TEST_F(YamlToolkitTest, HasKeyReturnsFalse)
{
    const auto filepath = path("haskey_false.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_FALSE(YamlToolkit::hasKey(filepath, "missing"));
}

TEST_F(YamlToolkitTest, HasKeyNonExistentFile)
{
    EXPECT_FALSE(YamlToolkit::hasKey(path("ghost.yml"), "key"));
}

TEST_F(YamlToolkitTest, MergeIntoExisting)
{
    const auto filepath = path("merge_existing.yml");
    YAML::Node data;
    data["a"] = "original_a";
    data["b"] = "original_b";
    YamlToolkit::create(filepath, data);

    YAML::Node mergeData;
    mergeData["b"] = "updated_b";
    mergeData["c"] = "new_c";
    EXPECT_TRUE(YamlToolkit::merge(filepath, mergeData));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["a"].as<std::string>(), "original_a");
    EXPECT_EQ(result["b"].as<std::string>(), "updated_b");
    EXPECT_EQ(result["c"].as<std::string>(), "new_c");
}

TEST_F(YamlToolkitTest, MergeIntoNewFile)
{
    const auto filepath = path("merge_new.yml");
    YAML::Node data;
    data["x"] = 100;
    EXPECT_TRUE(YamlToolkit::merge(filepath, data));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["x"].as<int>(), 100);
}

TEST_F(YamlToolkitTest, MergeNonMapDataReturnsFalse)
{
    const auto filepath = path("merge_bad.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_FALSE(YamlToolkit::merge(filepath, YAML::Node("scalar")));
}

TEST_F(YamlToolkitTest, ToString)
{
    YAML::Node data;
    data["name"] = "test";
    data["value"] = 42;

    const auto str = YamlToolkit::toString(data);
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("name"), std::string::npos);
    EXPECT_NE(str.find("test"), std::string::npos);
    EXPECT_NE(str.find("value"), std::string::npos);
    EXPECT_NE(str.find("42"), std::string::npos);
}

TEST_F(YamlToolkitTest, FromString)
{
    const std::string yamlStr = "name: bob\nage: 25\n";
    const auto node = YamlToolkit::fromString(yamlStr);

    EXPECT_TRUE(node.IsMap());
    EXPECT_EQ(node["name"].as<std::string>(), "bob");
    EXPECT_EQ(node["age"].as<int>(), 25);
}

TEST_F(YamlToolkitTest, ToStringAndFromStringRoundTrip)
{
    YAML::Node original;
    original["key"] = "value";
    original["nested"]["inner"] = 7;

    const auto str = YamlToolkit::toString(original);
    const auto restored = YamlToolkit::fromString(str);

    EXPECT_EQ(restored["key"].as<std::string>(), "value");
    EXPECT_EQ(restored["nested"]["inner"].as<int>(), 7);
}

TEST_F(YamlToolkitTest, GetNodeOrRootReturnsNodeWhenKeyExists)
{
    YAML::Node root;
    root["grpc"]["server"] = "localhost";

    const auto result = YamlToolkit::getNodeOrRoot(root, "grpc");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["server"].as<std::string>(), "localhost");
}

TEST_F(YamlToolkitTest, GetNodeOrRootReturnsRootWhenKeyMissing)
{
    YAML::Node root;
    root["something"] = "else";

    const auto result = YamlToolkit::getNodeOrRoot(root, "missing");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["something"].as<std::string>(), "else");
}

TEST_F(YamlToolkitTest, GetNodeOrRootWithNestedPath)
{
    YAML::Node root;
    root["a"]["b"]["c"] = 42;

    const auto result = YamlToolkit::getNodeOrRoot(root, "a.b");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["c"].as<int>(), 42);
}

TEST_F(YamlToolkitTest, GetNodeOrRootEmptyPathReturnsRoot)
{
    YAML::Node root;
    root["a"] = 1;

    const auto result = YamlToolkit::getNodeOrRoot(root, "");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["a"].as<int>(), 1);
}
