/**
 * @file IntegerToolkit.hpp
 * @brief IntegerToolkit class declaration
 * @details This header defines the IntegerToolkit class that provides functionality for General utility toolkits for strings, arrays, and other operations.
 */

#pragma once
#include <string>
#include <utility>
#include <optional>
#include <stdexcept>

namespace common::toolkit
{
    /// @brief A utility class for integer operations
    /// @details Provides various integer conversion and manipulation functions
    class IntegerToolkit
    {
        /// @brief Roman numeral conversion range constants
        static constexpr int32_t ROMAN_MIN_VALUE = 1;
        static constexpr int32_t ROMAN_MAX_VALUE = 3999;

        static const inline std::pair<int32_t, std::string> valueSymbols[] = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"},};

    public:
        IntegerToolkit() = delete;

        /// @brief Converts an integer to its Roman numeral representation
        /// @param num The integer to convert (must be between 1 and 3999)
        /// @return The Roman numeral representation as a string
        /// @throws std::out_of_range If the number is outside the valid range [1, 3999]
        [[nodiscard]] static std::string intToRoman(int32_t num);

        /// @brief Safely converts an integer to its Roman numeral representation
        /// @param num The integer to convert (must be between 1 and 3999)
        /// @return Optional containing the Roman numeral if valid, otherwise empty
        [[nodiscard]] static std::optional<std::string> tryIntToRoman(int32_t num) ;

        /// @brief Converts a Roman numeral string back to an integer
        /// @param roman The Roman numeral string to convert
        /// @return The integer value
        /// @throws std::invalid_argument If the Roman numeral is invalid
        [[nodiscard]] static int32_t romanToInt(const std::string& roman);

        /// @brief Checks if an integer is a power of two
        /// @param num The integer to check
        /// @return true if the number is a power of two, false otherwise
        [[nodiscard]] static bool isPowerOfTwo(int32_t num) ;

        /// @brief Counts the number of set bits in an integer
        /// @param num The integer to count bits for
        /// @return The number of set bits
        [[nodiscard]] static int32_t countSetBits(int32_t num) ;
    };
}