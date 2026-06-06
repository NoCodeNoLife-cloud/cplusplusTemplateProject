/**
 * @file YamlToolkit.hpp
 * @brief YamlToolkit class declaration
 * @details This header defines the YamlToolkit class that provides functionality for Common library utilities.
 */

#pragma once
#include <string>
#include <yaml-cpp/yaml.h>

namespace common::filesystem
{
    /// @brief A utility class for performing CRUD operations on YAML files.
    /// @details This class provides static methods to create, read, update, and delete YAML files,
    /// as well as get and set specific values within YAML files.
    class YamlToolkit
    {
    public:
        /// @brief Create a new YAML file with the specified data.
        /// @param filepath The path to the YAML file to create.
        /// @param data The YAML node data to write to the file.
        /// @return true if the file was created successfully, false otherwise.
        static bool create(const std::string& filepath, const YAML::Node& data);

        /// @brief Read YAML data from a file.
        /// @param filepath The path to the YAML file to read.
        /// @return A YAML node containing the file's data, or an empty node if the file doesn't exist or an error occurred.
        [[nodiscard]] static YAML::Node read(const std::string& filepath);

        /// @brief Update a YAML file with new data.
        /// @param filepath The path to the YAML file to update.
        /// @param data The new YAML node data to write to the file.
        /// @return true if the file was updated successfully, false otherwise.
        static bool update(const std::string& filepath, const YAML::Node& data);

        /// @brief Remove (delete) a YAML file.
        /// @param filepath The path to the YAML file to delete.
        /// @return true if the file was deleted successfully, false otherwise.
        static bool remove(const std::string& filepath);

        /// @brief Get a value from a YAML file by key.
        /// @param filepath The path to the YAML file.
        /// @param key The key of the value to retrieve.
        /// @return A YAML node containing the value associated with the key, or an empty node if the key doesn't exist or an error occurred.
        [[nodiscard]] static YAML::Node getValue(const std::string& filepath, const std::string& key);

        /// @brief Set a value in a YAML file by key.
        /// @param filepath The path to the YAML file.
        /// @param key The key of the value to set.
        /// @param value The YAML node value to set.
        /// @return true if the value was set successfully, false otherwise.
        static bool setValue(const std::string& filepath, const std::string& key, const YAML::Node& value);

        /// @brief Get a nested value from a YAML file using a path.
        /// @param filepath The path to the YAML file.
        /// @param path The dot-separated path to the value (e.g., "parent.child.grandchild").
        ///        An empty path returns the entire root node.
        /// @return A YAML node containing the value at the specified path.
        [[nodiscard]] static YAML::Node getNestedValue(const std::string& filepath, const std::string& path);

        /// @brief Set a nested value in a YAML file using a path.
        /// @param filepath The path to the YAML file.
        /// @param path The dot-separated path to the value (e.g., "parent.child.grandchild").
        ///        An empty path sets the value at the root level.
        /// @param value The YAML node value to set.
        /// @return true if the value was set successfully, false otherwise.
        static bool setNestedValue(const std::string& filepath, const std::string& path, const YAML::Node& value);

        /// @brief Check if a key exists in the YAML file.
        /// @param filepath The path to the YAML file.
        /// @param key The key to check for.
        /// @return true if the key exists, false otherwise.
        [[nodiscard]] static bool hasKey(const std::string& filepath, const std::string& key);

        /// @brief Merge another YAML node into the existing file.
        /// @param filepath The path to the YAML file.
        /// @param data The YAML node to merge.
        /// @return true if the merge was successful, false otherwise.
        static bool merge(const std::string& filepath, const YAML::Node& data);

        /// @brief Convert YAML node to string representation.
        /// @param node The YAML node to convert.
        /// @return String representation of the YAML node.
        [[nodiscard]] static std::string toString(const YAML::Node& node);

        /// @brief Parse YAML from string.
        /// @param str The string containing YAML data.
        /// @return YAML node parsed from the string.
        [[nodiscard]] static YAML::Node fromString(const std::string& str);

        /// @brief Traverse a dot-separated path within a YAML node and return the sub-node,
        ///        or fall back to the entire root node if the path does not exist.
        /// @param root The root YAML node.
        /// @param path The dot-separated path to traverse (e.g., "grpc.server").
        /// @return The sub-node at the path if it exists and is defined,
        ///         otherwise the original root node.
        [[nodiscard]] static YAML::Node getNodeOrRoot(const YAML::Node& root, const std::string& path);
    };
}