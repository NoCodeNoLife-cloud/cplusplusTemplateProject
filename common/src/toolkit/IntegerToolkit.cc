#include "IntegerToolkit.hpp"

#include <fmt/format.h>
#include <string>
#include <utility>
#include <stdexcept>
#include <cctype>

namespace common::toolkit {
    auto IntegerToolkit::intToRoman(const int32_t num) -> std::string {
        if (num < 1 || num > 3999) {
throw std::out_of_range("IntegerToolkit::intToRoman: Number must be between 1 and 3999");
        }
std::string roman;
        int32_t remaining = num;
        for (const auto &[value, symbol]: valueSymbols) {
            while (remaining >= value) {
                remaining -= value;
                roman += symbol;
            }
            if (remaining == 0) {
                break;
            }
        }
return roman;
    }

    auto IntegerToolkit::tryIntToRoman(const int32_t num) noexcept -> std::optional<std::string> {
        if (num < 1 || num > 3999) {
return std::nullopt;
        }

        try {
            const auto result = intToRoman(num);
return result;
        } catch (...) {
return std::nullopt;
        }
    }

    auto IntegerToolkit::romanToInt(const std::string &roman) -> int32_t {
        if (roman.empty()) {
throw std::invalid_argument("IntegerToolkit::romanToInt: Empty Roman numeral string");
        }
// Convert the Roman numeral string to uppercase for consistency
        std::string upperRoman = roman;
        for (char &c: upperRoman) {
            c = std::toupper(c);
        }

        int32_t result = 0;
        size_t i = 0;

        for (const auto &[value, symbol]: valueSymbols) {
            while (i + symbol.length() <= upperRoman.length() && upperRoman.substr(i, symbol.length()) == symbol) {
                result += value;
                i += symbol.length();
            }
        }

        // Verify that the entire string was processed correctly
        if (i != upperRoman.length()) {
throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }

        // Verify by converting back to Roman and comparing
        const std::string verification = intToRoman(result);
        if (verification != upperRoman) {
throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }
return result;
    }

    auto IntegerToolkit::isPowerOfTwo(const int32_t num) noexcept -> bool {
        return num > 0 && (num & (num - 1)) == 0;
    }

    auto IntegerToolkit::countSetBits(int32_t num) noexcept -> int32_t {
int32_t count = 0;
        while (num) {
            count += num & 1;
            num >>= 1;
        }
return count;
    }
}
