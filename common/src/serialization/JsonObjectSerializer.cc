/**
 * @file JsonObjectSerializer.cc
 * @brief JsonObjectSerializer implementation — toJson/fromJson type mapping
 * @details Implements JSON serialisation: traverses object member fields via
 *          a registered mapping and converts to/from JSON tree structures.
 */

#include "serialization/JsonObjectSerializer.hpp"

namespace common::serialization
{
    std::string JsonObjectSerializer::getStringOrDefault(const rapidjson::Value& json, const std::string& key, const std::string& defaultValue)
    {
        return getValueOrDefault<std::string>(json, key, defaultValue);
    }

    int32_t JsonObjectSerializer::getIntOrDefault(const rapidjson::Value& json, const std::string& key, const int32_t defaultValue)
    {
        return getValueOrDefault<int32_t>(json, key, defaultValue);
    }

    double JsonObjectSerializer::getDoubleOrDefault(const rapidjson::Value& json, const std::string& key, const double defaultValue)
    {
        return getValueOrDefault<double>(json, key, defaultValue);
    }

    bool JsonObjectSerializer::getBoolOrDefault(const rapidjson::Value& json, const std::string& key, const bool defaultValue)
    {
        return getValueOrDefault<bool>(json, key, defaultValue);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const std::string& value)
    {
        writer.Key(key.c_str());
        writer.String(value.c_str());
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const char* value)
    {
        writer.Key(key.c_str());
        writer.String(value);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const int32_t value)
    {
        writer.Key(key.c_str());
        writer.Int(value);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const double value)
    {
        writer.Key(key.c_str());
        writer.Double(value);
    }

    void JsonObjectSerializer::serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const bool value)
    {
        writer.Key(key.c_str());
        writer.Bool(value);
    }
}
