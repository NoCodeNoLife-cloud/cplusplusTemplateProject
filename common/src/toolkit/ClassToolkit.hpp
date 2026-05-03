#pragma once
#include <string>
#include <unordered_map>
#include <format>
#include <vector>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <boost/type_index.hpp>
#include <tuple>

namespace common::toolkit {
    template<typename T>
    struct ReflectTraits;

    /// @brief Utility class for type identification at runtime.
    /// This class provides a set of static methods to retrieve the type information
    /// of objects or classes using boost::typeindex. It is designed as a
    /// class with deleted constructor to prevent instantiation.
    class ClassToolkit {
    public:
        ClassToolkit() = delete;

        // ReSharper disable once CppDoxygenUnresolvedReference
        /// @brief Get the type id of the given object.
        /// @tparam T Type of the object.
        /// @param t The object to get type id from.
        /// @return std::string The pretty name of the type.
        template<typename T>
        [[nodiscard]] static auto getTypeId(const T & /*t*/) noexcept -> std::string {
            return boost::typeindex::type_id<T>().pretty_name();
        }

        // ReSharper disable once CppDoxygenUnresolvedReference
        /// @brief Get the type id with CVR (const, volatile, reference) of the given object.
        /// @tparam T Type of the object.
        /// @param t The object to get type id with CVR from.
        /// @return std::string The pretty name of the type with CVR.
        template<typename T>
        [[nodiscard]] static auto getTypeIdWithCvr(const T & /*t*/) noexcept -> std::string {
            return boost::typeindex::type_id_with_cvr<T>().pretty_name();
        }

        /// @brief Get the type id by class type.
        /// @tparam T Type of the class.
        /// @return std::string The pretty name of the type.
        template<typename T>
        [[nodiscard]] static auto getTypeIdByClass() noexcept -> std::string {
            return boost::typeindex::type_id<T>().pretty_name();
        }

        /// @brief Get the type id with CVR by class type.
        /// @tparam T Type of the class.
        /// @return std::string The pretty name of the type with CVR.
        template<typename T>
        [[nodiscard]] static auto getTypeIdWithCvrByClass() noexcept -> std::string {
            return boost::typeindex::type_id_with_cvr<T>().pretty_name();
        }

        /// @brief Get the fields of the given object.
        /// @tparam T Type of the object.
        /// @param obj The object to get fields from.
        /// @return std::unordered_map<std::string, std::string> A map of field names to their string representations.
        template<typename T>
        [[nodiscard]] static auto getFields(const T &obj) -> std::unordered_map<std::string, std::string> {
            std::unordered_map<std::string, std::string> field_map;
            constexpr auto fields = ReflectTraits<T>::fields;

            // Process each field using index-based access
            [&obj, &field_map, fields]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((field_map.insert(std::make_pair(std::get<Is>(fields).first, std::format("{}", invokeHelper(obj, std::get<Is>(fields).second))))), ...);
            }(std::make_index_sequence<ReflectTraits<T>::field_count>{});

            return field_map;
        }

        /// @brief Checks if two types are the same.
        /// @tparam T1 First type.
        /// @tparam T2 Second type.
        /// @return True if types are identical, false otherwise.
        template<typename T1, typename T2>
        [[nodiscard]] static auto isSameType() noexcept -> bool {
            return std::is_same_v<T1, T2>;
        }

        /// @brief Checks if a type is a base class of another.
        /// @tparam Base Base class type.
        /// @tparam Derived Derived class type.
        /// @return True if Base is a base of Derived, false otherwise.
        template<typename Base, typename Derived>
        [[nodiscard]] static auto isBaseOf() noexcept -> bool {
            return std::is_base_of_v<Base, Derived>;
        }

        /// @brief Checks if a type is polymorphic.
        /// @tparam T Type to check.
        /// @return True if the type is polymorphic, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isPolymorphic() noexcept -> bool {
            return std::is_polymorphic_v<T>;
        }

        /// @brief Checks if a type is abstract.
        /// @tparam T Type to check.
        /// @return True if the type is abstract, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isAbstract() noexcept -> bool {
            return std::is_abstract_v<T>;
        }

        /// @brief Gets the size of a type in bytes.
        /// @tparam T Type to get size for.
        /// @return Size in bytes.
        template<typename T>
        [[nodiscard]] static auto getTypeSize() noexcept -> std::size_t {
            return sizeof(T);
        }

        /// @brief Gets the alignment requirement of a type.
        /// @tparam T Type to get alignment for.
        /// @return Alignment in bytes.
        template<typename T>
        [[nodiscard]] static auto getTypeAlignment() noexcept -> std::size_t {
            return alignof(T);
        }

        /// @brief Checks if a type is default constructible.
        /// @tparam T Type to check.
        /// @return True if default constructible, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isDefaultConstructible() noexcept -> bool {
            return std::is_default_constructible_v<T>;
        }

        /// @brief Checks if a type is copy constructible.
        /// @tparam T Type to check.
        /// @return True if copy constructible, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isCopyConstructible() noexcept -> bool {
            return std::is_copy_constructible_v<T>;
        }

        /// @brief Checks if a type is move constructible.
        /// @tparam T Type to check.
        /// @return True if move constructible, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isMoveConstructible() noexcept -> bool {
            return std::is_move_constructible_v<T>;
        }

        /// @brief Checks if a type is trivially copyable.
        /// @tparam T Type to check.
        /// @return True if trivially copyable, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isTriviallyCopyable() noexcept -> bool {
            return std::is_trivially_copyable_v<T>;
        }

        /// @brief Checks if a type is trivially destructible.
        /// @tparam T Type to check.
        /// @return True if trivially destructible, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isTriviallyDestructible() noexcept -> bool {
            return std::is_trivially_destructible_v<T>;
        }

        /// @brief Gets the hash code of a type.
        /// @tparam T Type to hash.
        /// @return Hash value.
        template<typename T>
        [[nodiscard]] static auto getTypeHash() noexcept -> std::size_t {
            return boost::typeindex::type_id<T>().hash_code();
        }

        /// @brief Gets field names of a type (without values).
        /// @tparam T Type to get field names for.
        /// @return Vector of field names.
        template<typename T>
        [[nodiscard]] static auto getFieldNames() -> std::vector<std::string> {
            std::vector<std::string> field_names;
            constexpr auto fields = ReflectTraits<T>::fields;

            [&field_names, fields]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((field_names.push_back(std::get<Is>(fields).first)), ...);
            }(std::make_index_sequence<ReflectTraits<T>::field_count>{});

            return field_names;
        }

        /// @brief Gets field count of a type.
        /// @tparam T Type to get field count for.
        /// @return Number of fields.
        template<typename T>
        [[nodiscard]] static auto getFieldCount() noexcept -> std::size_t {
            return ReflectTraits<T>::field_count;
        }

        /// @brief Gets a specific field value by name.
        /// @tparam T Type of the object.
        /// @param obj Object to get field from.
        /// @param fieldName Name of the field.
        /// @return String representation of the field value.
        /// @throws std::invalid_argument If field name not found
        template<typename T>
        [[nodiscard]] static auto getFieldByName(const T &obj, const std::string &fieldName) -> std::string {
            constexpr auto fields = ReflectTraits<T>::fields;

            // Search for the field by name
            auto result = [&obj, &fieldName, fields]<std::size_t... Is>(std::index_sequence<Is...>) -> std::string {
                std::string not_found = "Field '" + fieldName + "' not found";
                std::string result_str = not_found;
                
                (([&]() {
                    if (std::get<Is>(fields).first == fieldName) {
                        result_str = std::format("{}", invokeHelper(obj, std::get<Is>(fields).second));
                    }
                })(), ...);
                
                return result_str;
            }(std::make_index_sequence<ReflectTraits<T>::field_count>{});

            if (result == "Field '" + fieldName + "' not found") {
                throw std::invalid_argument(result);
            }
            return result;
        }

        /// @brief Compares two objects field by field.
        /// @tparam T Type of the objects.
        /// @param obj1 First object.
        /// @param obj2 Second object.
        /// @return True if all fields are equal, false otherwise.
        template<typename T>
        [[nodiscard]] static auto compareObjects(const T &obj1, const T &obj2) -> bool {
            constexpr auto fields = ReflectTraits<T>::fields;

            return [&obj1, &obj2, fields]<std::size_t... Is>(std::index_sequence<Is...>) -> bool {
                bool all_equal = true;
                (([&]() {
                    if (invokeHelper(obj1, std::get<Is>(fields).second) != invokeHelper(obj2, std::get<Is>(fields).second)) {
                        all_equal = false;
                    }
                })(), ...);
                return all_equal;
            }(std::make_index_sequence<ReflectTraits<T>::field_count>{});
        }

        /// @brief Gets the difference between two objects.
        /// @tparam T Type of the objects.
        /// @param obj1 First object.
        /// @param obj2 Second object.
        /// @return Map of field names to pair of values (different fields only).
        template<typename T>
        [[nodiscard]] static auto getObjectDiff(const T &obj1, const T &obj2) 
            -> std::unordered_map<std::string, std::pair<std::string, std::string>> {
            std::unordered_map<std::string, std::pair<std::string, std::string>> diff_map;
            constexpr auto fields = ReflectTraits<T>::fields;

            [&obj1, &obj2, &diff_map, fields]<std::size_t... Is>(std::index_sequence<Is...>) {
                (([&]() {
                    auto val1 = invokeHelper(obj1, std::get<Is>(fields).second);
                    auto val2 = invokeHelper(obj2, std::get<Is>(fields).second);
                    if (val1 != val2) {
                        diff_map[std::get<Is>(fields).first] = std::make_pair(
                            std::format("{}", val1), 
                            std::format("{}", val2)
                        );
                    }
                })(), ...);
            }(std::make_index_sequence<ReflectTraits<T>::field_count>{});

            return diff_map;
        }

        /// @brief Gets comprehensive type information as a structured string.
        /// @tparam T Type to describe.
        /// @return Detailed type description including size, alignment, traits.
        template<typename T>
        [[nodiscard]] static auto getTypeInfo() noexcept -> std::string {
            return std::format(
                "Type: {}\n"
                "Size: {} bytes\n"
                "Alignment: {} bytes\n"
                "Is Polymorphic: {}\n"
                "Is Abstract: {}\n"
                "Is Default Constructible: {}\n"
                "Is Copy Constructible: {}\n"
                "Is Move Constructible: {}\n"
                "Is Trivially Copyable: {}\n"
                "Is Trivially Destructible: {}",
                boost::typeindex::type_id<T>().pretty_name(),
                sizeof(T),
                alignof(T),
                std::is_polymorphic_v<T> ? "true" : "false",
                std::is_abstract_v<T> ? "true" : "false",
                std::is_default_constructible_v<T> ? "true" : "false",
                std::is_copy_constructible_v<T> ? "true" : "false",
                std::is_move_constructible_v<T> ? "true" : "false",
                std::is_trivially_copyable_v<T> ? "true" : "false",
                std::is_trivially_destructible_v<T> ? "true" : "false"
            );
        }

    private:
        /// @brief Helper function to invoke member pointers.
        /// @tparam T Type of the object.
        /// @tparam M Type of the member pointer.
        /// @param obj The object to invoke the member pointer on.
        /// @param member The member pointer.
        /// @return The result of invoking the member pointer.
        template<typename T, typename M>
        [[nodiscard]] static auto invokeHelper(const T &obj, M member) -> decltype(obj.*member) {
            return obj.*member;
        }

        /// @brief Helper function to invoke member functions.
        /// @tparam T Type of the object.
        /// @tparam M Type of the member function pointer.
        /// @param obj The object to invoke the member function on.
        /// @param member The member function pointer.
        /// @return The result of invoking the member function.
        template<typename T, typename M>
        [[nodiscard]] static auto invokeHelper(const T &obj, M member) -> decltype((obj.*member)()) {
            return (obj.*member)();
        }
    };
}
