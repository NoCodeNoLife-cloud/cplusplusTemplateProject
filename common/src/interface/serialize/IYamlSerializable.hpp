/**
 * @file serialize/IYamlSerializable.hpp
 * @brief IYamlSerializable class declaration
 * @details This header defines the IYamlSerializable class that provides functionality for Common interface definitions for the framework.
 */

#pragma once
#include <type_traits>
#include <yaml-cpp/node/convert.h>

namespace common::interfaces::serialize
{
    /// @brief Interface for YAML serializable objects.
    /// This interface provides methods to encode and decode objects to and from YAML nodes.
    /// Classes implementing this interface can be seamlessly serialized and deserialized
    /// using the YAML-CPP library.
    class IYamlSerializable
    {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~IYamlSerializable() = default;

        /// @brief Encode the object to YAML node.
        /// This method serializes the object's data into a YAML node representation.
        /// The implementing class should populate the returned node with all relevant
        /// data that needs to be serialized.
        /// @return YAML node containing the serialized object data
        [[nodiscard]] virtual YAML::Node encode() const = 0;

        /// @brief Decode the object from YAML node.
        /// This method deserializes data from a YAML node into the object.
        /// The implementing class should parse the node and populate its member
        /// variables with the corresponding values.
        /// @param node YAML node containing the data to deserialize
        /// @return True if decoding is successful, false otherwise
        [[nodiscard]] virtual bool decode(const YAML::Node& node) = 0;
    };
}

/// @brief Constrained partial specialization of YAML::convert for IYamlSerializable subtypes.
/// This specialization enables automatic conversion of any IYamlSerializable object
/// to and from YAML nodes by delegating to the object's encode() and decode() methods.
/// @tparam T Type that derives from IYamlSerializable
template <typename T>
    requires std::is_base_of_v<common::interfaces::serialize::IYamlSerializable, T>
struct YAML::convert<T>
{
    /// @brief Encode an object to a YAML node.
    /// Delegates the encoding to the object's encode() method.
    /// @param obj The object to encode
    /// @return YAML node containing the encoded object
    [[nodiscard]] static Node encode(const T& obj)
    {
        return obj.encode();
    }

    /// @brief Decode an object from a YAML node.
    /// Delegates the decoding to the object's decode() method.
    /// @param node YAML node containing the data to decode
    /// @param obj Reference to the object to populate with decoded data
    /// @return True if decoding is successful, false otherwise
    static bool decode(const Node& node, T& obj)
    {
        return obj.decode(node);
    }
};
