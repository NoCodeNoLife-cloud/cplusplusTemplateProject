/**
 * @file IntegerToolkit.cc
 * @brief IntegerToolkit class implementation
 * @details This file contains the implementation of the IntegerToolkit class methods for General utility toolkits for strings, arrays, and other operations.
 */

#include "IntegerToolkit.hpp"

#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::toolkit
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
            c = std::toupper(c);
        }

        int32_t result = 0;
        size_t i = 0;

        for (const auto& [value, symbol] : valueSymbols)
        {
            while (i + symbol.length() <= upperRoman.length() && upperRoman.substr(i, symbol.length()) == symbol)
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

    int32_t IntegerToolkit::countSetBits(int32_t num)
    {
        int32_t count = 0;
        while (num)
        {
            count += num & 1;
            num >>= 1;
        }
        return count;
    }
}
