/**
 * @file IYamlConfigurable.hpp
 * @brief Interface for objects configurable via YAML input
 * @description Defines the contract for objects that can read their
 *          configuration from a YAML node/source.  Implementations extract
 *          settings from a YAML::Node and apply them to internal state.
 */

#pragma once
#include <filesystem>

namespace common::interface::serialization
{
    /// @brief Interface for classes that can be configured from YAML files
    /// @details This interface defines a contract for objects that can deserialize
    /// their configuration from YAML files. Classes implementing this interface
    /// should provide implementation for loading their settings from YAML format.
    class IYamlConfigurable
    {
    public:
        /// @brief Virtual destructor for proper cleanup of derived classes
        virtual ~IYamlConfigurable() = default;

        /// @brief Deserialize object configuration from a YAML file
        /// @param path The file path to the YAML configuration file
        /// @throws std::runtime_error If the file cannot be read or parsed
        virtual void deserializeFromYamlFile(const std::filesystem::path& path) = 0;
    };
}
