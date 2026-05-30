/**
 * @file YamlObjectSerializer.hpp
 * @brief YamlObjectSerializer class declaration
 * @details This header defines the YamlObjectSerializer class that provides functionality for Serialization utilities for JSON and YAML.
 */

#pragma once

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>

namespace common::serializer
{
    /// @brief Concrete implementation of ISerializer for serializing and deserializing objects to and from YAML format.
    /// @tparam T The type of object to be serialized/deserialized.
    template <typename T>
    class YamlObjectSerializer : public ISerializer<T>
    {
    public:
        /// @brief Serializes an object to a YAML file.
        /// @param obj The object to serialize.
        /// @param filename The path to the file where the YAML data will be saved.
        /// @throws std::invalid_argument If filename is empty.
        /// @throws std::runtime_error If file cannot be opened or written to.
        void serialize(const T& obj, const std::string& filename) override;

        /// @brief Deserializes an object from a YAML file.
        /// @param filename The path to the YAML file to deserialize from.
        /// @return The deserialized object.
        /// @throws std::runtime_error If file cannot be opened or decoded.
        [[nodiscard]] T deserialize(const std::string& filename) override;
    };

    template <typename T>
    void YamlObjectSerializer<T>::serialize(const T& obj, const std::string& filename)
    {
        if (filename.empty())
        {
            throw std::invalid_argument("YamlObjectSerializer::serialize: filename is empty");
        }

        const YAML::Node node = YAML::convert<T>::encode(obj);
        YAML::Emitter emitter;
        emitter << node;
        const std::string yaml_str = emitter.c_str();

        std::ofstream file_out(filename);
        if (!file_out.is_open())
        {
            throw std::runtime_error("YamlObjectSerializer::serialize: Could not open file " + filename);
        }

        file_out << yaml_str;

        // Check for write errors before closing
        if (file_out.fail())
        {
            throw std::runtime_error("YamlObjectSerializer::serialize: Could not write to file " + filename);
        }
    }

    template <typename T>
    [[nodiscard]] T YamlObjectSerializer<T>::deserialize(const std::string& filename)
    {
        if (filename.empty())
        {
            throw std::runtime_error("YamlObjectSerializer::deserialize: filename is empty");
        }

        if (!std::filesystem::exists(filename))
        {
            throw std::runtime_error("YamlObjectSerializer::deserialize: File does not exist: " + filename);
        }

        const YAML::Node node = YAML::LoadFile(filename);
        T obj{};

        if (!YAML::convert<T>::decode(node, obj))
        {
            throw std::runtime_error("YamlObjectSerializer::deserialize: Failed to decode YAML file " + filename);
        }

        return obj;
    }
}
