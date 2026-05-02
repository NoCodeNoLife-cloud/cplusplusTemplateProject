#pragma once

#include <string>

namespace common::interfaces {
    /// @brief Abstract base class defining the interface for object serialization/deserialization
    /// @tparam T The type of object to be serialized/deserialized
    template<typename T>
    class ISerializer {
    public:
        virtual ~ISerializer() = default;

        /// @brief Serializes an object to a file
        /// @param obj The object to serialize
        /// @param filename The path to the file where the data will be saved
        virtual void serialize(const T &obj, const std::string &filename) = 0;

        /// @brief Deserializes an object from a file
        /// @param filename The path to the file to deserialize from
        /// @return The deserialized object
        virtual T deserialize(const std::string &filename) = 0;
    };
}
