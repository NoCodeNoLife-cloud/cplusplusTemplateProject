#pragma once
#include <string>
#include <string_view>
#include <initializer_list>

namespace common::interfaces {
    /// @brief Interface for appendable objects.
    /// This interface provides methods for appending characters and strings to objects.
    /// Implementing classes should provide efficient appending operations while maintaining
    /// the object's internal state consistency.
    /// @tparam T The type of the object that implements this interface.
    template<typename T>
    class IAppendable {
    public:
        /// @brief Virtual destructor for proper cleanup of derived classes
        virtual ~IAppendable() = default;

        /// @brief Append a character to the object
        /// @param c The character to append
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(char c) -> T & = 0;

        /// @brief Append a string to the object
        /// @param str The string to append
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(const std::string &str) -> T & = 0;

        /// @brief Append a substring to the object
        /// @param str The string to append
        /// @param start The start index of the substring (inclusive)
        /// @param end The end index of the substring (exclusive)
        /// @pre start <= end <= str.size()
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(const std::string &str, size_t start, size_t end) -> T & = 0;

        /// @brief Append a string view to the object
        /// @param str The string view to append
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(std::string_view str) -> T & = 0;

        /// @brief Append a C-style string to the object
        /// @param str The C-style string to append
        /// @pre str != nullptr
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(const char *str) -> T & = 0;

        /// @brief Append characters from an initializer list to the object
        /// @param chars The characters to append
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(std::initializer_list<char> chars) -> T & = 0;

        /// @brief Append a character array to the object
        /// @param chars Pointer to the character array
        /// @param count Number of characters to append
        /// @pre chars != nullptr || count == 0
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(const char *chars, size_t count) -> T & = 0;

        /// @brief Append a character multiple times to the object
        /// @param c The character to append
        /// @param count Number of times to append the character
        /// @return A reference to the object for method chaining
        [[nodiscard]] virtual auto append(char c, size_t count) -> T & = 0;

        /// @brief Append a range of characters to the object
        /// @tparam InputIt Iterator type
        /// @param first Iterator to the first character
        /// @param last Iterator to one past the last character
        /// @return A reference to the object for method chaining
        template<typename InputIt>
        [[nodiscard]] auto append(InputIt first, InputIt last) -> T & {
            while (first != last) {
                static_cast<T *>(this)->append(*first);
                ++first;
            }
            return *static_cast<T *>(this);
        }
    };
}
