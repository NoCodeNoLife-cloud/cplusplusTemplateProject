/**
 * @file IBoostSerializable.hpp
 * @brief IBoostSerializable class declaration
 * @details This header defines the IBoostSerializable class that provides functionality for Common library utilities.
 */

#pragma once
#include <cstdint>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/access.hpp>

namespace fox::interfaces
{
    /// @brief Interface for Boost serialization
    /// This interface provides a common base for objects that can be serialized using Boost.Serialization.
    /// Classes implementing this interface need to provide a serializeImpl method for the actual serialization logic.
    /// @tparam T The type to be serialized (the derived class)
    // ReSharper disable once CppTemplateParameterNeverUsed
    template <typename T>
    // ReSharper disable once CppClassCanBeFinal
    class IBoostSerializable
    {
    public:
        /// @brief Virtual destructor for proper cleanup of derived classes
        virtual ~IBoostSerializable() = default;

        /// @brief Serialize object to output stream
        /// This method serializes the object to the provided output stream using Boost text archive.
        /// @param stream Output stream to serialize to
        /// @return true if serialization was successful, false otherwise
        [[nodiscard]] bool serializeTo(std::ostream& stream) const;

        /// @brief Deserialize object from input stream
        /// This method deserializes the object from the provided input stream using Boost text archive.
        /// @param stream Input stream to deserialize from
        /// @return true if deserialization was successful, false otherwise
        [[nodiscard]] bool deserializeFrom(std::istream& stream);

    private:
        /// @brief Friend declaration to allow Boost serialization access
        friend class boost::serialization::access;

        /// @brief Serialization implementation
        /// This method is called by the Boost serialization library to serialize the object.
        /// It forwards the call to the derived class's serializeImpl method.
        /// @tparam Archive The type of archive used for serialization
        /// @param archive Archive object used for serialization
        /// @param version Version number for serialization
        template <class Archive>
        void serialize(Archive& archive, uint32_t version);
    };

    template <typename T>
    bool IBoostSerializable<T>::serializeTo(std::ostream& stream) const
    {
        boost::archive::text_oarchive oa(stream);
        oa << static_cast<const T&>(*this);
        return true;
    }

    template <typename T>
    bool IBoostSerializable<T>::deserializeFrom(std::istream& stream)
    {
        boost::archive::text_iarchive ia(stream);
        ia >> static_cast<T&>(*this);
        return true;
    }

    template <typename T>
    template <class Archive>
    void IBoostSerializable<T>::serialize(Archive& archive, const uint32_t version)
    {
        static_cast<T*>(this)->serializeImpl(archive, version);
    }
}