/**
 * @file JsonObjectSerializer.hpp
 * @brief JsonObjectSerializer class declaration
 * @details This header defines the JsonObjectSerializer class that provides functionality for Serialization utilities for JSON and YAML.
 */

#pragma once
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace common::serializer
{
    /// @class JsonObjectSerializer
    /// @brief A utility class for serializing and deserializing JSON objects using RapidJSON.
    /// This class provides static methods to extract values from JSON objects with default fallbacks
    /// and to serialize fields to JSON format.
    class JsonObjectSerializer
    {
    private:
        /// @brief Template helper to get value from JSON with default fallback
        /// @tparam T Type of the value to extract
        /// @param json The JSON object to extract the value from
        /// @param key The key of the value to extract
        /// @param defaultValue The default value to return if key is not found or not of expected type
        /// @return The extracted value or the default value
        template <typename T>
        [[nodiscard]] static auto getValueOrDefault(const rapidjson::Value& json, const std::string& key, T defaultValue)  -> T;

    public:
        /// @brief Gets a string value from JSON or returns a default value.
        /// @param json The JSON object to extract the value from.
        /// @param key The key of the value to extract.
        /// @param defaultValue The default value to return if key is not found or not a string.
        /// @return The extracted string value or the default value.
        [[nodiscard]] static std::string getStringOrDefault(const rapidjson::Value& json, const std::string& key, const std::string& defaultValue) ;

        /// @brief Gets an integer value from JSON or returns a default value.
        /// @param json The JSON object to extract the value from.
        /// @param key The key of the value to extract.
        /// @param defaultValue The default value to return if key is not found or not an integer.
        /// @return The extracted integer value or the default value.
        [[nodiscard]] static int32_t getIntOrDefault(const rapidjson::Value& json, const std::string& key, int32_t defaultValue) ;

        /// @brief Gets a double value from JSON or returns a default value.
        /// @param json The JSON object to extract the value from.
        /// @param key The key of the value to extract.
        /// @param defaultValue The default value to return if key is not found or not a double.
        /// @return The extracted double value or the default value.
        [[nodiscard]] static double getDoubleOrDefault(const rapidjson::Value& json, const std::string& key, double defaultValue) ;

        /// @brief Gets a boolean value from JSON or returns a default value.
        /// @param json The JSON object to extract the value from.
        /// @param key The key of the value to extract.
        /// @param defaultValue The default value to return if key is not found or not a boolean.
        /// @return The extracted boolean value or the default value.
        [[nodiscard]] static bool getBoolOrDefault(const rapidjson::Value& json, const std::string& key, bool defaultValue) ;

        /// @brief Serializes a string field to JSON.
        /// @param writer The JSON writer to use for serialization.
        /// @param key The key for the field.
        /// @param value The string value to serialize.
        static void serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, const std::string& value) ;

        /// @brief Serializes an integer field to JSON.
        /// @param writer The JSON writer to use for serialization.
        /// @param key The key for the field.
        /// @param value The integer value to serialize.
        static void serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, int32_t value) ;

        /// @brief Serializes a double field to JSON.
        /// @param writer The JSON writer to use for serialization.
        /// @param key The key for the field.
        /// @param value The double value to serialize.
        static void serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, double value) ;

        /// @brief Serializes a boolean field to JSON.
        /// @param writer The JSON writer to use for serialization.
        /// @param key The key for the field.
        /// @param value The boolean value to serialize.
        static void serializeField(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::string& key, bool value) ;
    };
}
