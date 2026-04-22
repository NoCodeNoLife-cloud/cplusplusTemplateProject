#include "IntegerToolkit.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <string>
#include <utility>
#include <stdexcept>
#include <cctype>

namespace common::toolkit {
    auto IntegerToolkit::intToRoman(const int32_t num) -> std::string {
        if (num < 1 || num > 3999) {
            DLOG(ERROR) << fmt::format("IntegerToolkit intToRoman failed - number out of range: {}", num);
            throw std::out_of_range("IntegerToolkit::intToRoman: Number must be between 1 and 3999");
        }

        DLOG(INFO) << fmt::format("IntegerToolkit intToRoman - converting: {}", num);
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
        DLOG(INFO) << fmt::format("IntegerToolkit intToRoman succeeded - result: {}", roman);
        return roman;
    }

    auto IntegerToolkit::tryIntToRoman(const int32_t num) noexcept -> std::optional<std::string> {
        if (num < 1 || num > 3999) {
            DLOG(WARNING) << fmt::format("IntegerToolkit tryIntToRoman - number out of range: {}, returning nullopt", num);
            return std::nullopt;
        }

        try {
            const auto result = intToRoman(num);
            DLOG(INFO) << fmt::format("IntegerToolkit tryIntToRoman succeeded - result: {}", result);
            return result;
        } catch (...) {
            DLOG(ERROR) << fmt::format("IntegerToolkit tryIntToRoman exception - number: {}", num);
            return std::nullopt;
        }
    }

    auto IntegerToolkit::romanToInt(const std::string &roman) -> int32_t {
        if (roman.empty()) {
            DLOG(ERROR) << "IntegerToolkit romanToInt failed - empty Roman numeral string";
            throw std::invalid_argument("IntegerToolkit::romanToInt: Empty Roman numeral string");
        }

        DLOG(INFO) << fmt::format("IntegerToolkit romanToInt - parsing: {}", roman);
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
            DLOG(ERROR) << fmt::format("IntegerToolkit romanToInt failed - invalid Roman numeral: {}", roman);
            throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }

        // Verify by converting back to Roman and comparing
        const std::string verification = intToRoman(result);
        if (verification != upperRoman) {
            DLOG(ERROR) << fmt::format("IntegerToolkit romanToInt verification failed - input: {}, result: {}, verification: {}", roman, result, verification);
            throw std::invalid_argument("IntegerToolkit::romanToInt: Invalid Roman numeral string");
        }

        DLOG(INFO) << fmt::format("IntegerToolkit romanToInt succeeded - result: {}", result);
        return result;
    }

    auto IntegerToolkit::isPowerOfTwo(const int32_t num) noexcept -> bool {
        return num > 0 && (num & (num - 1)) == 0;
    }

    auto IntegerToolkit::countSetBits(int32_t num) noexcept -> int32_t {
        DLOG(INFO) << fmt::format("IntegerToolkit countSetBits - counting bits for: {}", num);
        int32_t count = 0;
        while (num) {
            count += num & 1;
            num >>= 1;
        }
        DLOG(INFO) << fmt::format("IntegerToolkit countSetBits result - count: {}", count);
        return count;
    }
}
