#pragma once

#include <yaml-cpp/yaml.h>
#include <string>

namespace common::filesystem {
/// @brief A utility class for performing CRUD operations on YAML files.
/// @details This class provides static methods to create, read, update, and delete YAML files,
/// as well as get and set specific values within YAML files.
class YamlToolkit {
public:
    /// @brief Create a new YAML file with the specified data.
    /// @param filepath The path to the YAML file to create.
    /// @param data The YAML node data to write to the file.
    /// @return true if the file was created successfully, false otherwise.
    static auto create(const std::string& filepath, const YAML::Node& data) -> bool;

    /// @brief Read YAML data from a file.
    /// @param filepath The path to the YAML file to read.
    /// @return A YAML node containing the file's data, or an empty node if the file doesn't exist or an error occurred.
    [[nodiscard]] static auto read(const std::string& filepath) -> YAML::Node;

    /// @brief Update a YAML file with new data.
    /// @param filepath The path to the YAML file to update.
    /// @param data The new YAML node data to write to the file.
    /// @return true if the file was updated successfully, false otherwise.
    static auto update(const std::string& filepath, const YAML::Node& data) -> bool;

    /// @brief Remove (delete) a YAML file.
    /// @param filepath The path to the YAML file to delete.
    /// @return true if the file was deleted successfully, false otherwise.
    static auto remove(const std::string& filepath) -> bool;

    /// @brief Get a value from a YAML file by key.
    /// @param filepath The path to the YAML file.
    /// @param key The key of the value to retrieve.
    /// @return A YAML node containing the value associated with the key, or an empty node if the key doesn't exist or an error occurred.
    [[nodiscard]] static auto getValue(const std::string& filepath, const std::string& key) -> YAML::Node;

    /// @brief Set a value in a YAML file by key.
    /// @param filepath The path to the YAML file.
    /// @param key The key of the value to set.
    /// @param value The YAML node value to set.
    /// @return true if the value was set successfully, false otherwise.
    static auto setValue(const std::string& filepath, const std::string& key, const YAML::Node& value) -> bool;

    /// @brief Get a nested value from a YAML file using a path.
    /// @param filepath The path to the YAML file.
    /// @param path The path to the value (e.g., "parent.child.grandchild").
    /// @return A YAML node containing the value at the specified path.
    [[nodiscard]] static auto getNestedValue(const std::string& filepath, const std::string& path) -> YAML::Node;

    /// @brief Set a nested value in a YAML file using a path.
    /// @param filepath The path to the YAML file.
    /// @param path The path to the value (e.g., "parent.child.grandchild").
    /// @param value The YAML node value to set.
    /// @return true if the value was set successfully, false otherwise.
    static auto setNestedValue(const std::string& filepath, const std::string& path, const YAML::Node& value) -> bool;

    /// @brief Check if a key exists in the YAML file.
    /// @param filepath The path to the YAML file.
    /// @param key The key to check for.
    /// @return true if the key exists, false otherwise.
    [[nodiscard]] static auto hasKey(const std::string& filepath, const std::string& key) -> bool;

    /// @brief Merge another YAML node into the existing file.
    /// @param filepath The path to the YAML file.
    /// @param data The YAML node to merge.
    /// @return true if the merge was successful, false otherwise.
    static auto merge(const std::string& filepath, const YAML::Node& data) -> bool;

    /// @brief Convert YAML node to string representation.
    /// @param node The YAML node to convert.
    /// @return String representation of the YAML node.
    [[nodiscard]] static auto toString(const YAML::Node& node) -> std::string;

    /// @brief Parse YAML from string.
    /// @param str The string containing YAML data.
    /// @return YAML node parsed from the string.
    [[nodiscard]] static auto fromString(const std::string& str) -> YAML::Node;

    /// @brief Get a node from a YAML node by path, with fallback to root if path doesn't exist
    /// @param root The root YAML node
    /// @param path The path to try (e.g., "grpc")
    /// @return The node at path if it exists, otherwise the root node
    [[nodiscard]] static auto getNodeOrRoot(const YAML::Node& root, const std::string& path) -> YAML::Node;
};
}