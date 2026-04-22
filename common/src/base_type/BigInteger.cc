#include "src/base_type/BigInteger.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include <compare>
#include <stdexcept>

namespace common::base_type {
    BigInteger::BigInteger() noexcept : value_(0) {
    }

    BigInteger::BigInteger(const std::string &str) : value_(str) {
        DLOG(INFO) << fmt::format("BigInteger constructed from string: {}", str);
    }

    BigInteger::BigInteger(const int64_t num) noexcept : value_(num) {
        DLOG(INFO) << fmt::format("BigInteger constructed from int64: {}", num);
    }

    auto BigInteger::operator+(const BigInteger &other) const noexcept -> BigInteger {
        return BigInteger((value_ + other.value_).convert_to<std::string>());
    }

    auto BigInteger::operator-(const BigInteger &other) const noexcept -> BigInteger {
        return BigInteger((value_ - other.value_).convert_to<std::string>());
    }

    auto BigInteger::operator*(const BigInteger &other) const noexcept -> BigInteger {
        return BigInteger((value_ * other.value_).convert_to<std::string>());
    }

    auto BigInteger::operator/(const BigInteger &other) const -> BigInteger {
        if (other.value_ == 0) {
            DLOG(ERROR) << "Division by zero attempted";
            throw std::invalid_argument("Division by zero");
        }
        const auto result = BigInteger((value_ / other.value_).convert_to<std::string>());
        DLOG(INFO) << fmt::format("BigInteger division: {} / {} = {}", 
            value_.convert_to<std::string>(), other.value_.convert_to<std::string>(), 
            (value_ / other.value_).convert_to<std::string>());
        return result;
    }

    auto BigInteger::operator%(const BigInteger &other) const -> BigInteger {
        if (other.value_ == 0) {
            DLOG(ERROR) << "Modulo by zero attempted";
            throw std::invalid_argument("Modulo by zero");
        }
        const auto result = BigInteger((value_ % other.value_).convert_to<std::string>());
        DLOG(INFO) << fmt::format("BigInteger modulo: {} % {} = {}", 
            value_.convert_to<std::string>(), other.value_.convert_to<std::string>(), 
            (value_ % other.value_).convert_to<std::string>());
        return result;
    }

    auto BigInteger::operator<=>(const BigInteger &other) const noexcept -> std::strong_ordering {
        if (value_ < other.value_) {
            return std::strong_ordering::less;
        }
        if (value_ > other.value_) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    auto BigInteger::operator==(const BigInteger &other) const noexcept -> bool {
        return value_ == other.value_;
    }

    auto BigInteger::fromString(const std::string &str) -> BigInteger {
        return BigInteger(str);
    }

    auto BigInteger::fromInt(const int64_t num) noexcept -> BigInteger {
        return BigInteger(num);
    }
}
