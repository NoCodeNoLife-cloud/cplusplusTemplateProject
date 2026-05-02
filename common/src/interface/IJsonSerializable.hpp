#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace common::interfaces {
    /// @brief Interface for JSON serializable objects.
    /// This interface defines the contract for objects that can be serialized to
    /// and deserialized from JSON format using the RapidJSON library. Any class
    /// that implements this interface must provide implementations for the
    /// serialize and deserialize methods.
    class IJsonSerializable {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~IJsonSerializable() = default;

        /// @brief Serializes the object to JSON format.
        /// This method converts the object's data into a JSON representation
        /// using the provided RapidJSON writer. The implementation should handle
        /// all necessary object properties and nested objects.
        /// @param writer Reference to a RapidJSON writer that will be used to build the JSON output
        [[nodiscard]] virtual auto serialize(rapidjson::Writer<rapidjson::StringBuffer> &writer) const -> void = 0;

        /// @brief Deserializes the object from JSON format.
        /// This method populates the object's data from a JSON representation
        /// provided as a RapidJSON value. The implementation should validate
        /// the input data and handle any missing or invalid fields appropriately.
        /// @param json Const reference to a RapidJSON value containing the JSON data to parse
        virtual auto deserialize(const rapidjson::Value &json) -> void = 0;
    };
}
