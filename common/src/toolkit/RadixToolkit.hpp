/**
 * @file RadixToolkit.hpp
 * @brief RadixToolkit class declaration
 * @details This header defines the RadixToolkit class that provides functionality for General utility toolkits for strings, arrays, and other operations.
 */

#pragma once

#include <algorithm>
#include <charconv>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace common::toolkit
{
    /**
     * @class RadixToolkit
     * @brief Comprehensive toolkit for integer base conversion operations
     * @details Supports configurable character sets and default bases for instance-based operations,
     *          alongside static methods for stateless conversions
     */
    class RadixToolkit
    {
    public:
        /**
         * @brief Constructs toolkit with specified configuration
         * @param default_base Default numeric base for instance methods (2-36)
         * @param charset Character set mapping for digit representation (minimum size >= max base)
         * @throws std::invalid_argument If charset too small for default_base
         */
        constexpr explicit RadixToolkit(
            int default_base = 10,
            std::string_view charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        );

        /**
         * @brief Convert integer to string using instance default base/charset
         * @tparam T Integral type (deduced)
         * @param value Value to convert
         * @param override_base Optional base override (0 means use default)
         * @return String representation
         */
        template <std::integral T>
        [[nodiscard]] constexpr std::string to_string(T value, int override_base = 0) const;

        /**
         * @brief Parse string using instance default base
         * @tparam T Target integral type
         * @param str String view to parse
         * @param override_base Optional base override (0 means use default)
         * @return Converted integral value
         */
        template <std::integral T>
        [[nodiscard]] constexpr auto from_string(std::string_view str, int override_base = 0) const -> T;

        /**
         * @brief Stateless conversion to specified base with custom charset
         * @tparam T Integral type
         * @param value Value to convert
         * @param base Target base (2-charset.size())
         * @param charset Character mapping array/string
         * @return String representation in specified base
         * @throws std::invalid_argument For invalid base/charset combination
         */
        template <std::integral T, typename CharSet = std::string_view>
        [[nodiscard]] static constexpr std::string convert_to_string(
            T value,
            int base,
            const CharSet& charset
        );

        /**
         * @brief Stateless parsing from specified base
         * @tparam T Target integral type
         * @param str String view to parse
         * @param base Source base (2-36)
         * @return Parsed value
         * @throws std::invalid_argument For invalid characters/format
         * @throws std::out_of_range For overflow beyond type limits
         */
        template <std::integral T>
        [[nodiscard]] static constexpr T convert_from_string(
            std::string_view str,
            int base
        );

        /**
         * @brief Binary conversion shorthand (base-2)
         * @tparam T Integral type
         * @param value Value to convert
         * @return Binary string
         */
        template <std::integral T>
        [[nodiscard]] static constexpr std::string to_binary(T value);

        /**
         * @brief Octal conversion shorthand (base-8)
         * @tparam T Integral type
         * @param value Value to convert
         * @return Octal string
         */
        template <std::integral T>
        [[nodiscard]] static constexpr std::string to_octal(T value);

        /**
         * @brief Hexadecimal conversion (uppercase, base-16)
         * @tparam T Integral type
         * @param value Value to convert
         * @return Hexadecimal string
         */
        template <std::integral T>
        [[nodiscard]] static constexpr std::string to_hex(T value);

        /**
         * @brief Hexadecimal conversion (lowercase, base-16)
         * @tparam T Integral type
         * @param value Value to convert
         * @return Lowercase hexadecimal string
         */
        template <std::integral T>
        [[nodiscard]] static constexpr std::string to_hex_lower(T value);

        /**
         * @brief Non-throwing string to value conversion using std::from_chars
         * @tparam T Integral type
         * @param str Input string
         * @param base Numeric base
         * @param ec Error code output
         * @return Converted value or 0 on error
         */
        template <std::integral T>
        [[nodiscard]] static constexpr T from_string_nothrow(
            std::string_view str,
            int base,
            std::errc& ec
        ) ;

        /**
         * @brief Get current default base configuration
         * @return Default base value
         */
        [[nodiscard]] constexpr auto get_default_base() const  -> int;

        /**
         * @brief Get current charset configuration
         * @return Character set string view
         */
        [[nodiscard]] constexpr auto get_charset() const  -> std::string_view;

    private:
        int default_base_;
        std::string_view charset_;

        [[nodiscard]] static constexpr auto char_to_digit(char c)  -> int;
    };

    constexpr RadixToolkit::RadixToolkit(const int default_base, const std::string_view charset) : default_base_(default_base), charset_(charset)
    {
        if (default_base_ < 2 || static_cast<size_t>(default_base_) > charset_.size())
        {
            throw std::invalid_argument("Default base incompatible with charset size");
        }
    }

    template <std::integral T>
    constexpr std::string RadixToolkit::to_string(T value, const int override_base) const
    {
        const int base = override_base == 0 ? default_base_ : override_base;
        return convert_to_string(value, base, charset_);
    }

    template <std::integral T>
    constexpr auto RadixToolkit::from_string(const std::string_view str, const int override_base) const -> T
    {
        const int base = override_base == 0 ? default_base_ : override_base;
        return convert_from_string<T>(str, base);
    }

    template <std::integral T, typename CharSet>
    constexpr std::string RadixToolkit::convert_to_string(T value, int base, const CharSet& charset)
    {
        // Calculate charset size: handle both string_view and C-style arrays
        constexpr bool is_array = std::is_array_v<std::remove_reference_t<CharSet>>;
        const size_t charset_size = [&]()
        {
            if constexpr (is_array)
            {
                return std::extent_v<std::remove_reference_t<CharSet>>;
            }
            else
            {
                return charset.size();
            }
        }();

        if (base < 2 || static_cast<size_t>(base) > charset_size)
        {
            throw std::invalid_argument("Base must be between 2 and charset size");
        }

        if (value == 0)
        {
            return std::string{charset[0]};
        }

        using UnsignedT = std::make_unsigned_t<T>;
        UnsignedT abs_value;
        bool negative = false;

        if constexpr (std::is_signed_v < T >)
        {
            if (value < 0)
            {
                negative = true;
                // Safe negation avoiding INT_MIN overflow
                abs_value = static_cast<UnsignedT>(-(value + 1)) + 1;
            }
            else
            {
                abs_value = static_cast<UnsignedT>(value);
            }
        }
        else
        {
            abs_value = value;
        }

        std::string result;
        result.reserve(sizeof(T) * 8 + 1); // Max binary digits + sign

        const auto ubase = static_cast<UnsignedT>(base);
        while (abs_value > 0)
        {
            result.push_back(charset[static_cast<size_t>(abs_value % ubase)]);
            abs_value /= ubase;
        }

        if (negative)
        {
            result.push_back('-');
        }

        std::ranges::reverse(result);
        return result;
    }

    template <std::integral T>
    constexpr auto RadixToolkit::convert_from_string(const std::string_view str, int base) -> T
    {
        if (base < 2 || base > 36)
        {
            throw std::invalid_argument("Base must be between 2 and 36");
        }
        if (str.empty())
        {
            throw std::invalid_argument("Empty input string");
        }

        size_t pos = 0;
        bool negative = false;

        if constexpr (std::is_signed_v < T >)
        {
            if (str[0] == '-')
            {
                negative = true;
                pos = 1;
                if (pos >= str.size())
                {
                    throw std::invalid_argument("Invalid format: lone minus sign");
                }
            }
            else if (str[0] == '+')
            {
                pos = 1;
            }
        }

        using Limits = std::numeric_limits<T>;
        using UnsignedT = std::make_unsigned_t<T>;

        constexpr auto max_unsigned = std::numeric_limits<UnsignedT>::max();
        UnsignedT result = 0;
        const auto ubase = static_cast<UnsignedT>(base);

        for (; pos < str.size(); ++pos)
        {
            const char c = str[pos];
            const int digit = char_to_digit(c);

            if (digit < 0 || digit >= base)
            {
                throw std::invalid_argument(
                    std::string("Invalid character '") + c + "' for base " + std::to_string(base)
                );
            }

            const auto digit_val = static_cast<UnsignedT>(digit);

            // Overflow check: result * base + digit > max
            if (result > (max_unsigned - digit_val) / ubase)
            {
                throw std::out_of_range("Value exceeds type limits");
            }

            result = result * ubase + digit_val;
        }

        if constexpr (std::is_signed_v < T >)
        {
            if (negative)
            {
                const auto min_mag = static_cast<UnsignedT>(Limits::min());
                if (result > min_mag)
                {
                    throw std::out_of_range("Value below type minimum");
                }
                return -static_cast<T>(result);
            }
            if (result > static_cast<UnsignedT>(Limits::max()))
            {
                throw std::out_of_range("Value exceeds type maximum");
            }
        }

        return static_cast<T>(result);
    }

    template <std::integral T>
    constexpr std::string RadixToolkit::to_binary(T value)
    {
        return convert_to_string(value, 2, "01");
    }

    template <std::integral T>
    constexpr std::string RadixToolkit::to_octal(T value)
    {
        return convert_to_string(value, 8, "01234567");
    }

    template <std::integral T>
    constexpr std::string RadixToolkit::to_hex(T value)
    {
        return convert_to_string(value, 16, "0123456789ABCDEF");
    }

    template <std::integral T>
    constexpr std::string RadixToolkit::to_hex_lower(T value)
    {
        return convert_to_string(value, 16, "0123456789abcdef");
    }

    template <std::integral T>
    constexpr auto RadixToolkit::from_string_nothrow(std::string_view str, int base, std::errc& ec)  -> T
    {
        T result{};
        const auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), result, base);

        if (err != std::errc{} || ptr != str.data() + str.size())
        {
            ec = std::errc::invalid_argument;
            return T{};
        }

        ec = std::errc{};
        return result;
    }

    constexpr auto RadixToolkit::get_default_base() const  -> int
    {
        return default_base_;
    }

    constexpr auto RadixToolkit::get_charset() const  -> std::string_view
    {
        return charset_;
    }

    constexpr auto RadixToolkit::char_to_digit(const char c)  -> int
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
        if (c >= 'a' && c <= 'z') return c - 'a' + 10;
        return -1;
    }
}