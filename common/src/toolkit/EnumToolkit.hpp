/**
 * @file EnumToolkit.hpp
 * @brief EnumToolkit class declaration
 * @details This header defines the EnumToolkit class that provides functionality for General utility toolkits for strings, arrays, and other operations.
 */

#pragma once
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <sstream>
#include <magic_enum/magic_enum.hpp>

namespace common::toolkit
{
    /// @brief Utility class for enum operations.
    /// This class provides static methods to work with enums,
    /// such as converting enum values to their string representations.
    class EnumToolkit
    {
    public:
        EnumToolkit() = delete;

        /// @brief Get the name of an enum value as a string.
        /// @tparam T The enum type.
        /// @param value The enum value.
        /// @return std::string The name of the enum value as a string.
        template <typename T>
        [[nodiscard]] static std::string getEnumName(T value) noexcept;

        /// @brief Get the class name of an enum type as a string.
        /// @tparam T The enum type.
        /// @return std::string The name of the enum type as a string.
        template <typename T>
        [[nodiscard]] static std::string getEnumTypeName() noexcept;

        /// @brief Convert a string to an enum value.
        /// @tparam T The enum type.
        /// @param name The string name of the enum value.
        /// @return Optional containing the enum value if found, otherwise empty optional.
        template <typename T>
        [[nodiscard]] static std::optional<T> stringToEnum(const std::string& name) noexcept;

        /// @brief Get all enum values as a vector.
        /// @tparam T The enum type.
        /// @return std::vector<T> A vector containing all enum values.
        template <typename T>
        [[nodiscard]] static std::vector<T> getAllEnumValues() noexcept;

        /// @brief Get all enum names as a vector of strings.
        /// @tparam T The enum type.
        /// @return std::vector<std::string> A vector containing all enum names.
        template <typename T>
        [[nodiscard]] static std::vector<std::string> getAllEnumNames() noexcept;

        /// @brief Check if a string is a valid enum name.
        /// @tparam T The enum type.
        /// @param name The string to check.
        /// @return bool True if the string is a valid enum name, false otherwise.
        template <typename T>
        [[nodiscard]] static bool isValidEnumName(const std::string& name) noexcept;

        /// @brief Get the count of enum values.
        /// @tparam T The enum type.
        /// @return size_t The number of enum values.
        template <typename T>
        [[nodiscard]] static size_t getEnumCount() noexcept;

        /// @brief Convert an enum value to its underlying integer value.
        /// @tparam T The enum type.
        /// @param value The enum value.
        /// @return The underlying integer value.
        template <typename T>
        [[nodiscard]] static std::underlying_type_t<T> enumToInteger(T value) noexcept;

        /// @brief Convert an integer to an enum value safely.
        /// @tparam T The enum type.
        /// @param value The integer value.
        /// @return Optional containing the enum value if valid, otherwise empty optional.
        template <typename T>
        [[nodiscard]] static std::optional<T> integerToEnum(std::underlying_type_t<T> value) noexcept;

        /// @brief Get formatted information about an enum type.
        /// @tparam T The enum type.
        /// @return std::string Formatted string with enum type info and all values.
        template <typename T>
        [[nodiscard]] static std::string formatEnumInfo() noexcept;
    };

    template <typename T>
    std::string EnumToolkit::getEnumName(T value) noexcept
    {
        return static_cast<std::string>(magic_enum::enum_name(value));
    }

    template <typename T>
    std::string EnumToolkit::getEnumTypeName() noexcept
    {
        return static_cast<std::string>(magic_enum::enum_type_name<T>());
    }

    template <typename T>
    std::optional<T> EnumToolkit::stringToEnum(const std::string& name) noexcept
    {
        return magic_enum::enum_cast<T>(name);
    }

    template <typename T>
    std::vector<T> EnumToolkit::getAllEnumValues() noexcept
    {
        constexpr auto enum_values = magic_enum::enum_values<T>();
        return std::vector<T>(enum_values.begin(), enum_values.end());
    }

    template <typename T>
    std::vector<std::string> EnumToolkit::getAllEnumNames() noexcept
    {
        constexpr auto enum_names = magic_enum::enum_names<T>();
        return std::vector<std::string>(enum_names.begin(), enum_names.end());
    }

    template <typename T>
    bool EnumToolkit::isValidEnumName(const std::string& name) noexcept
    {
        return magic_enum::enum_cast<T>(name).has_value();
    }

    template <typename T>
    size_t EnumToolkit::getEnumCount() noexcept
    {
        return magic_enum::enum_count<T>();
    }

    template <typename T>
    std::underlying_type_t<T> EnumToolkit::enumToInteger(T value) noexcept
    {
        return magic_enum::enum_integer(value);
    }

    template <typename T>
    std::optional<T> EnumToolkit::integerToEnum(std::underlying_type_t<T> value) noexcept
    {
        return magic_enum::enum_cast<T>(value);
    }

    template <typename T>
    std::string EnumToolkit::formatEnumInfo() noexcept
    {
        std::ostringstream oss;
        oss << "Enum Type: " << getEnumTypeName<T>() << "\n";
        oss << "Count: " << getEnumCount<T>() << "\n";
        oss << "Values:\n";

        const auto values = getAllEnumValues<T>();
        const auto names = getAllEnumNames<T>();

        for (size_t i = 0; i < values.size(); ++i)
        {
            oss << "  " << names[i] << " = " << enumToInteger(values[i]) << "\n";
        }

        return oss.str();
    }
}