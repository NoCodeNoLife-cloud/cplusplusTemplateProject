/**
 * @file IntegerToolkit.cc
 * @brief IntegerToolkit implementation â€?parsing, formatting, bit ops
 * @details Implements integer utility methods: string-to-int parsing with
 *          radix validation, int-to-hex/octal/binary formatting, and
 *          bit-manipulation helpers (popcount, clz, ctz, rotl, rotr).
 */

#include <cppforge/toolkit/IntegerToolkit.hpp>

#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>
#include <fmt/format.h>
#include <glog/logging.h>

namespace cppforge::toolkit
{
    std::string IntegerToolkit::intToRoman(const int32_t num)
    {
        if (num < ROMAN_MIN_VALUE || num > ROMAN_MAX_VALUE)
        {
            DLOG(WARNING) << fmt::format("IntegerToolkit intToRoman: Number {} out of range [{}, {}]", num, ROMAN_MIN_VALUE, ROMAN_MAX_VALUE);
            throw std::out_of_range("IntegerToolkit::intToRoman: Number must be between " +
                std::to_string(ROMAN_MIN_VALUE) + " and " +
                std::to_string(ROMAN_MAX_VALUE));
        }
        std::string roman;
        int32_t remaining = num;
        for (const auto& [value, symbol] : valueSymbols)
        {
            while (remaining >= value)
            {
                remaining -= value;
                roman += symbol;
            }
            if (remaining == 0)
            {
                break;
            }
        }
        return roman;
    }

    std::optional<std::string> IntegerToolkit::tryIntToRoman(const int32_t num)
    {
        if (num < ROMAN_MIN_VALUE || num > ROMAN_MAX_VALUE)
        {
            return std::nullopt;
        }

        try
        {
            const auto result = intToRoman(num);
            return result;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    int32_t IntegerToolkit::romanToInt(const std::string& roman)
    {
        if (roman.empty())
        {
            DLOG(WARNING) << "IntegerToolkit romanToInt: Empty Roman numeral string";
            throw std::invalid_argument("IntegerToolkit::romanToInt: Empty Roman numeral string");
        }
        // Convert the Roman numeral string to uppercase for consistency
        std::string upperRoman = roman;
        for (char& c : upperRoman)
        {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        int32_t result = 0;
        size_t i = 0;

        for (const auto& [value, symbol] : valueSymbols)
        {
            while (i + symbol.length() <= upperRoman.length() &&
                   upperRoman.compare(i, symbol.length(), symbol) == 0)
            {
                result += value;
                i += symbol.length();
            }
        }

        // Verify that the entire string was processed correctly
        if (i != upperRoman.length())
        {
            throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }

        // Verify by converting back to Roman and comparing
        const std::string verification = intToRoman(result);
        if (verification != upperRoman)
        {
            throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }
        return result;
    }

    bool IntegerToolkit::isPowerOfTwo(const int32_t num)
    {
        return num > 0 && (num & num - 1) == 0;
    }

    int32_t IntegerToolkit::countSetBits(const int32_t num)
    {
        return static_cast<int32_t>(std::popcount(static_cast<uint32_t>(num)));
    }
}
