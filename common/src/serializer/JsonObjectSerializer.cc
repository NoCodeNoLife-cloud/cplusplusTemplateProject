/**
 * @file JsonObjectSerializer.cc
 * @brief JsonObjectSerializer class implementation
 * @details This file contains the implementation of the JsonObjectSerializer class methods for Serialization utilities for JSON and YAML.
 */

#include "src/serializer/JsonObjectSerializer.hpp"

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace common::serializer
{
    template <typename T>
    T JsonObjectSerializer::getValueOrDefault(const rapidjson::Value& json, const std::string& key, T defaultValue) noexcept
    {
        // Convert std::string to c_str for rapidjson compatibility
        if (json.HasMember(key.c_str()))
        {
            const auto& member = json[key.c_str()];
            if constexpr (std::is_same_v<T, std::string>)
            {
                if (member.IsString())
                {
                    return std::string(member.GetString());
                }
            }
            else if constexpr (std::is_same_v<T, int32_t>)
            {
                if (member.IsInt())
                {
                    return member.GetInt();
                }
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                if (member.IsDouble())
                {
                    return member.GetDouble();
                }
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (member.IsBool())
                {
                    return member.GetBool();
                }
            }
        }
        return defaultValue;
    }

    std::string JsonObjectSerializer::getStringOrDefault(const rapidjson::Value& json, const std::string& key, const std::string& defaultValue) noexcept
    {
        return getValueOrDefault<std::string>(json, key, defaultValue);
    }

    int32_t JsonObjectSerializer::getIntOrDefault(const rapidjson::Value& json, const std::string& key, const int32_t defaultValue) noexcept
    {
        return getValueOrDefault<int32_t>(json, key, defaultValue);
    }

    double JsonObjectSerializer::getDoubleOrDefault(const rapidjson::Value& json, const std::string& key, const double defaultValue) noexcept
    {
        return getValueOrDefault<double>(json, key, defaultValue);
    }

    bool JsonObjectSerializer::getBoolOrDefault(const rapidjson::Value& json, const std::string& key, const bool defaultValue) noexcept
    {
        return getValueOrDefault<bool>(json, key, defaultValue);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const std::string& value) noexcept
    {
        writer.Key(key.c_str());
        writer.String(value.c_str());
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const int32_t value) noexcept
    {
        writer.Key(key.c_str());
        writer.Int(value);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const double value) noexcept
    {
        writer.Key(key.c_str());
        writer.Double(value);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const bool value) noexcept
    {
        writer.Key(key.c_str());
        writer.Bool(value);
    }
}

// Explicitly instantiate templates to avoid linker errors
template auto common::serializer::JsonObjectSerializer::getValueOrDefault<std::string>(const rapidjson::Value&, const std::string&, std::string) noexcept -> std::string;

template auto common::serializer::JsonObjectSerializer::getValueOrDefault<int32_t>(const rapidjson::Value&, const std::string&, int32_t) noexcept -> int32_t;

template auto common::serializer::JsonObjectSerializer::getValueOrDefault<double>(const rapidjson::Value&, const std::string&, double) noexcept -> double;

template auto common::serializer::JsonObjectSerializer::getValueOrDefault<bool>(const rapidjson::Value&, const std::string&, bool) noexcept -> bool;