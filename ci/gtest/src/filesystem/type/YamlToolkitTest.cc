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

#include <cppforge/filesystem/type/YamlToolkit.hpp>

using namespace cppforge::filesystem::type;
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

/**
 * @brief Test creating a YAML file from a YAML::Node
 * @details Verifies that create() writes a YAML::Node to disk and that the resulting file exists
 */
TEST_F(YamlToolkitTest, CreateYamlFile)
{
    const auto filepath = path("create_test.yml");
    YAML::Node data;
    data["name"] = "test";
    data["value"] = 42;

    EXPECT_TRUE(YamlToolkit::create(filepath, data));
    EXPECT_TRUE(fs::exists(filepath));
}

/**
 * @brief Test creating a YAML file in a nested directory
 * @details Verifies that create() creates intermediate directories when the target path contains subdirectories
 */
TEST_F(YamlToolkitTest, CreateYamlFileWithNestedDir)
{
    const auto filepath = path("nested/subdir/create_test.yml");
    YAML::Node data;
    data["key"] = "value";

    EXPECT_TRUE(YamlToolkit::create(filepath, data));
    EXPECT_TRUE(fs::exists(filepath));
}

/**
 * @brief Test create returns false for an invalid (empty) path
 * @details Verifies that create() returns false when the file path is empty
 */
TEST_F(YamlToolkitTest, CreateYamlFileReturnsFalseOnInvalidPath)
{
    YAML::Node data;
    data["x"] = 1;
    EXPECT_FALSE(YamlToolkit::create("", data));
}

/**
 * @brief Test reading an existing YAML file
 * @details Verifies that read() returns a YAML::Node that matches the original data written by create()
 */
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

/**
 * @brief Test updating an existing YAML file
 * @details Verifies that update() replaces the content of an existing file and can be verified by reading it back
 */
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

/**
 * @brief Test removing an existing YAML file
 * @details Verifies that remove() deletes the file from disk and returns true
 */
TEST_F(YamlToolkitTest, RemoveExistingFile)
{
    const auto filepath = path("remove_test.yml");
    YAML::Node data;
    data["x"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_TRUE(YamlToolkit::remove(filepath));
    EXPECT_FALSE(fs::exists(filepath));
}

/**
 * @brief Test remove returns false for a non-existent file
 * @details Verifies that remove() returns false when the file path does not exist on disk
 */
TEST_F(YamlToolkitTest, RemoveNonExistentFile)
{
    EXPECT_FALSE(YamlToolkit::remove(path("ghost.yml")));
}

/**
 * @brief Test getValue returns the correct value for an existing key
 * @details Verifies that getValue() retrieves a defined YAML::Node for a key that exists in the file
 */
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

/**
 * @brief Test getValue returns undefined for a non-existent key
 * @details Verifies that getValue() returns an undefined YAML::Node when the key is not present
 */
TEST_F(YamlToolkitTest, GetValueNonExistentKey)
{
    const auto filepath = path("getvalue_missing.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getValue(filepath, "nonexistent");
    EXPECT_FALSE(result.IsDefined());
}

/**
 * @brief Test getValue returns undefined for a non-existent file
 * @details Verifies that getValue() returns an undefined YAML::Node when the file does not exist
 */
TEST_F(YamlToolkitTest, GetValueNonExistentFile)
{
    const auto result = YamlToolkit::getValue(path("no_file.yml"), "key");
    EXPECT_FALSE(result.IsDefined());
}

/**
 * @brief Test setValue adds a new key to an existing file
 * @details Verifies that setValue() adds a new key-value pair while preserving existing keys
 */
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

/**
 * @brief Test setValue creates a new file and writes a key
 * @details Verifies that setValue() creates a new YAML file when the file does not already exist
 */
TEST_F(YamlToolkitTest, SetValueNewFile)
{
    const auto filepath = path("setvalue_new.yml");
    EXPECT_TRUE(YamlToolkit::setValue(filepath, "key", YAML::Node(123)));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["key"].as<int>(), 123);
}

/**
 * @brief Test getNestedValue retrieves a value using dot-separated path
 * @details Verifies that getNestedValue("parent.child.value") returns 99 from a three-level nested YAML structure
 */
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

/**
 * @brief Test getNestedValue returns undefined for a non-existent nested path
 * @details Verifies that getNestedValue() returns an undefined node when the dotted path does not exist
 */
TEST_F(YamlToolkitTest, GetNestedValueNonExistent)
{
    const auto filepath = path("nested_get_missing.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    const auto result = YamlToolkit::getNestedValue(filepath, "a.b.c");
    EXPECT_FALSE(result.IsDefined());
}

/**
 * @brief Test getNestedValue with an empty path returns the root node
 * @details Verifies that getNestedValue("") returns the entire YAML document as a map
 */
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

/**
 * @brief Test setNestedValue with an empty path replaces the entire document
 * @details Verifies that setNestedValue("", replacement) overwrites the entire YAML content
 */
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

/**
 * @brief Test hasKey returns true for an existing key
 * @details Verifies that hasKey() correctly identifies a key that exists in the YAML file
 */
TEST_F(YamlToolkitTest, HasKeyReturnsTrue)
{
    const auto filepath = path("haskey_true.yml");
    YAML::Node data;
    data["present"] = "here";
    YamlToolkit::create(filepath, data);

    EXPECT_TRUE(YamlToolkit::hasKey(filepath, "present"));
}

/**
 * @brief Test hasKey returns false for a missing key
 * @details Verifies that hasKey() returns false when the specified key is not present in the YAML file
 */
TEST_F(YamlToolkitTest, HasKeyReturnsFalse)
{
    const auto filepath = path("haskey_false.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_FALSE(YamlToolkit::hasKey(filepath, "missing"));
}

/**
 * @brief Test hasKey returns false for a non-existent file
 * @details Verifies that hasKey() returns false when the file itself does not exist on disk
 */
TEST_F(YamlToolkitTest, HasKeyNonExistentFile)
{
    EXPECT_FALSE(YamlToolkit::hasKey(path("ghost.yml"), "key"));
}

/**
 * @brief Test merging data into an existing YAML file
 * @details Verifies that merge() updates existing keys and adds new keys while preserving unmodified keys
 */
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

/**
 * @brief Test merging data into a non-existent file creates it
 * @details Verifies that merge() creates a new YAML file when the target file does not exist
 */
TEST_F(YamlToolkitTest, MergeIntoNewFile)
{
    const auto filepath = path("merge_new.yml");
    YAML::Node data;
    data["x"] = 100;
    EXPECT_TRUE(YamlToolkit::merge(filepath, data));

    const auto result = YamlToolkit::read(filepath);
    EXPECT_EQ(result["x"].as<int>(), 100);
}

/**
 * @brief Test merge returns false when data is not a map
 * @details Verifies that merge() returns false when the provided YAML data is a scalar rather than a map
 */
TEST_F(YamlToolkitTest, MergeNonMapDataReturnsFalse)
{
    const auto filepath = path("merge_bad.yml");
    YAML::Node data;
    data["a"] = 1;
    YamlToolkit::create(filepath, data);

    EXPECT_FALSE(YamlToolkit::merge(filepath, YAML::Node("scalar")));
}

/**
 * @brief Test toString serializes a YAML::Node to a string
 * @details Verifies that toString() produces a non-empty string containing the expected keys and values
 */
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

/**
 * @brief Test fromString deserializes a YAML string to a node
 * @details Verifies that fromString() correctly parses a YAML-formatted string into a YAML::Node
 */
TEST_F(YamlToolkitTest, FromString)
{
    const std::string yamlStr = "name: bob\nage: 25\n";
    const auto node = YamlToolkit::fromString(yamlStr);

    EXPECT_TRUE(node.IsMap());
    EXPECT_EQ(node["name"].as<std::string>(), "bob");
    EXPECT_EQ(node["age"].as<int>(), 25);
}

/**
 * @brief Test round-trip serialization via toString/fromString
 * @details Verifies that a YAML::Node survives a toString -> fromString cycle without data loss
 */
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

/**
 * @brief Test getNodeOrRoot returns the node when the key exists
 * @details Verifies that getNodeOrRoot() returns the sub-node for an existing top-level key
 */
TEST_F(YamlToolkitTest, GetNodeOrRootReturnsNodeWhenKeyExists)
{
    YAML::Node root;
    root["grpc"]["server"] = "localhost";

    const auto result = YamlToolkit::getNodeOrRoot(root, "grpc");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["server"].as<std::string>(), "localhost");
}

/**
 * @brief Test getNodeOrRoot returns the root when the key is missing
 * @details Verifies that getNodeOrRoot() falls back to the root node when the specified key does not exist
 */
TEST_F(YamlToolkitTest, GetNodeOrRootReturnsRootWhenKeyMissing)
{
    YAML::Node root;
    root["something"] = "else";

    const auto result = YamlToolkit::getNodeOrRoot(root, "missing");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["something"].as<std::string>(), "else");
}

/**
 * @brief Test getNodeOrRoot with a dot-separated nested path
 * @details Verifies that getNodeOrRoot("a.b") traverses nested nodes and returns the correct sub-node
 */
TEST_F(YamlToolkitTest, GetNodeOrRootWithNestedPath)
{
    YAML::Node root;
    root["a"]["b"]["c"] = 42;

    const auto result = YamlToolkit::getNodeOrRoot(root, "a.b");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["c"].as<int>(), 42);
}

/**
 * @brief Test getNodeOrRoot with an empty path returns the root
 * @details Verifies that getNodeOrRoot("") returns the entire root node unchanged
 */
TEST_F(YamlToolkitTest, GetNodeOrRootEmptyPathReturnsRoot)
{
    YAML::Node root;
    root["a"] = 1;

    const auto result = YamlToolkit::getNodeOrRoot(root, "");
    EXPECT_TRUE(result.IsMap());
    EXPECT_EQ(result["a"].as<int>(), 1);
}
