#pragma once

#include <yaml-cpp/node/convert.h>

namespace common::interfaces {
    /// @brief Interface for YAML serializable objects.
    /// This interface provides methods to encode and decode objects to and from YAML nodes.
    /// Classes implementing this interface can be seamlessly serialized and deserialized
    /// using the YAML-CPP library.
    class IYamlSerializable {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~IYamlSerializable() = default;

        /// @brief Encode the object to YAML node.
        /// This method serializes the object's data into a YAML node representation.
        /// The implementing class should populate the returned node with all relevant
        /// data that needs to be serialized.
        /// @return YAML node containing the serialized object data
        [[nodiscard]] virtual auto encode() const -> YAML::Node = 0;

        /// @brief Decode the object from YAML node.
        /// This method deserializes data from a YAML node into the object.
        /// The implementing class should parse the node and populate its member
        /// variables with the corresponding values.
        /// @param node YAML node containing the data to deserialize
        /// @return True if decoding is successful, false otherwise
        [[nodiscard]] virtual auto decode(const YAML::Node &node) -> bool = 0;
    };
}

/// @brief YAML convert template specialization for IYamlSerializable objects.
/// This template specialization enables automatic conversion of IYamlSerializable
/// objects to and from YAML nodes using the YAML-CPP library's conversion mechanism.
/// It delegates the actual encoding/decoding to the object's encode() and decode() methods.
/// @tparam T Type of the object that implements IYamlSerializable
template<typename T>
struct YAML::convert {
    /// @brief Encode an object to a YAML node.
    /// Delegates the encoding to the object's encode() method.
    /// @param obj The object to encode
    /// @return YAML node containing the encoded object
    [[nodiscard]] static auto encode(const T &obj) -> Node {
        return obj.encode();
    }

    /// @brief Decode an object from a YAML node.
    /// Delegates the decoding to the object's decode() method.
    /// @param node YAML node containing the data to decode
    /// @param obj Reference to the object to populate with decoded data
    /// @return True if decoding is successful, false otherwise
    static auto decode(const Node &node, T &obj) -> bool {
        return obj.decode(node);
    }
};
