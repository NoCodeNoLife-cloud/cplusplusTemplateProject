/**
 * @file IYamlConfigurableTest.cc
 * @brief Unit tests for IYamlConfigurable interface
 * @details Tests cover deserialization from YAML files via a mock implementation.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <cppforge/interface/serialization/IYamlConfigurable.hpp>

using namespace cppforge::interface::serialization;

namespace
{
    /// @brief Mock configurable that reads YAML files
    class MockYamlConfigurable : public IYamlConfigurable
    {
    public:
        std::string loadedPath_;
        bool throwOnLoad_ = false;

        void deserializeFromYamlFile(const std::filesystem::path& path) override
        {
            if (throwOnLoad_)
                throw std::runtime_error("load failed");
            loadedPath_ = path.string();
            if (!std::filesystem::exists(path))
                throw std::runtime_error("file not found: " + path.string());
        }
    };
}

class IYamlConfigurableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        configurable_ = std::make_unique<MockYamlConfigurable>();
        testFile_ = std::filesystem::temp_directory_path() / "test_config.yaml";
        {
            std::ofstream ofs(testFile_);
            ofs << "key: value\n";
        }
    }

    void TearDown() override
    {
        configurable_.reset();
        if (std::filesystem::exists(testFile_))
            std::filesystem::remove(testFile_);
    }

    std::unique_ptr<MockYamlConfigurable> configurable_;
    std::filesystem::path testFile_;
};

TEST_F(IYamlConfigurableTest, DeserializeFromExistingFile)
{
    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(testFile_));
    EXPECT_EQ(configurable_->loadedPath_, testFile_.string());
}

TEST_F(IYamlConfigurableTest, DeserializeFromNonExistentFileThrows)
{
    const auto nonexistent = std::filesystem::temp_directory_path() / "nonexistent_config.yaml";

    EXPECT_THROW(configurable_->deserializeFromYamlFile(nonexistent), std::runtime_error);
}

TEST_F(IYamlConfigurableTest, DeserializeFromEmptyPathThrows)
{
    EXPECT_THROW(configurable_->deserializeFromYamlFile(""), std::runtime_error);
}

TEST_F(IYamlConfigurableTest, DeserializeFromMultipleFiles)
{
    auto file2 = std::filesystem::temp_directory_path() / "test_config2.yaml";
    {
        std::ofstream ofs(file2);
        ofs << "other: data\n";
    }

    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(testFile_));
    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(file2));

    EXPECT_EQ(configurable_->loadedPath_, file2.string());

    std::filesystem::remove(file2);
}

TEST_F(IYamlConfigurableTest, DeserializeDoesNotThrowWhenFileExists)
{
    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(testFile_));
}

TEST_F(IYamlConfigurableTest, DeserializeFromYamlWithSubdirectory)
{
    const auto subdir = std::filesystem::temp_directory_path() / "subdir";
    std::filesystem::create_directories(subdir);
    const auto subdirFile = subdir / "config.yaml";
    {
        std::ofstream ofs(subdirFile);
        ofs << "nested: true\n";
    }

    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(subdirFile));

    std::filesystem::remove(subdirFile);
    std::filesystem::remove(subdir);
}

TEST_F(IYamlConfigurableTest, VerifyMethodNameIsDeserializeNotDeserialized)
{
    // Compile-time check: the method is named deserializeFromYamlFile (not deserializedFromYamlFile)
    // This test verifies the method exists with the correct spelling
    EXPECT_NO_THROW(configurable_->deserializeFromYamlFile(testFile_));
}
