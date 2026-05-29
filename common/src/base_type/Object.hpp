/**
 * @file Object.hpp
 * @brief Base object class declaration
 * @details This header defines the Object class that provides functionality for Base type classes providing fundamental object functionality.
 */

#pragma once
#include <format>
#include <typeinfo>
#include <memory>
#include <string>

namespace common::base_type
{
    /// @brief Base class for all objects in the system.
    /// @details This class provides basic functionality for type information, hashing, string representation,
    /// and object utilities similar to Java's Object class.
    /// It is designed to be inherited by other classes that need these features.
    // ReSharper disable once CppClassCanBeFinal
    class Object
    {
    public:
        virtual ~Object() = default;

        /// @brief Get the type information of the object.
        /// @return const std::type_info& The type information.
        [[nodiscard]] virtual const std::type_info& getClass() const noexcept;

        /// @brief Compares this object with another object for equality.
        /// @param[in] other The object to compare with.
        /// @return true if the objects are equal, false otherwise.
        /// @details By default, this implementation compares the memory addresses of the objects.
        /// Derived classes should override this method to provide meaningful equality comparison.
        [[nodiscard]] virtual bool equals(const Object& other) const;

        /// @brief Get the hash code of the object.
        /// @return size_t The hash code.
        /// @details This method should be overridden by derived classes to provide a meaningful hash
        /// based on the object's content. The default implementation returns the memory address hash.
        [[nodiscard]] virtual size_t hashCode() const noexcept;

        /// @brief Returns a string representation of the object.
        /// @return std::string representation of the object.
        /// @details This method should be overridden by derived classes to provide a meaningful string
        /// representation of the object's content. The default implementation returns the type name.
        [[nodiscard]] virtual std::string toString() const;

        /// @brief Creates a copy of this object.
        /// @return std::unique_ptr<Object> A new instance that is a copy of this object.
        /// @details This is a virtual copy constructor. Derived classes should override this method
        /// to provide proper copying behavior. The default implementation throws an exception
        /// to indicate that derived classes should implement this.
        /// @throws std::logic_error If called on base Object class (must be overridden by derived classes)
        [[nodiscard]] virtual std::unique_ptr<Object> clone() const;

        /// @brief Checks if this object is an instance of the specified type.
        /// @tparam T The type to check against.
        /// @return true if the object is an instance of type T, false otherwise.
        template <typename T>
        [[nodiscard]] bool instanceOf() const noexcept
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        /// @brief Checks if this object is an instance of the specified type at runtime.
        /// @param[in] target_type The type_info of the target type to check against.
        /// @return true if the object is an instance of the specified type, false otherwise.
        [[nodiscard]] bool isInstance(const std::type_info& target_type) const noexcept;

        /// @brief Gets the class name as a string.
        /// @return std::string The name of the class.
        [[nodiscard]] std::string getClassName() const;

        /// @brief Checks if this object is the same as another object (reference equality).
        /// @param[in] other The object to compare references with.
        /// @return true if both objects are the same reference, false otherwise.
        [[nodiscard]] bool is(const Object& other) const noexcept;

    private:
        friend std::formatter<Object>;
    };
}
