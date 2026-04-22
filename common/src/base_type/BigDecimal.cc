#include "src/base_type/BigDecimal.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <stdexcept>
#include <string>
#include <compare>

namespace common::base_type {
    BigDecimal::BigDecimal(const std::string &str) : value_(str) {
        DLOG(INFO) << fmt::format("BigDecimal constructed from string: {}", str);
    }

    BigDecimal::BigDecimal(const double num) : value_(num) {
        DLOG(INFO) << fmt::format("BigDecimal constructed from double: {}", num);
    }

    auto BigDecimal::operator+(const BigDecimal &other) const noexcept -> BigDecimal {
        return BigDecimal((value_ + other.value_).convert_to<std::string>());
    }

    auto BigDecimal::operator-(const BigDecimal &other) const noexcept -> BigDecimal {
        return BigDecimal((value_ - other.value_).convert_to<std::string>());
    }

    auto BigDecimal::operator*(const BigDecimal &other) const noexcept -> BigDecimal {
        return BigDecimal((value_ * other.value_).convert_to<std::string>());
    }

    auto BigDecimal::operator/(const BigDecimal &other) const -> BigDecimal {
        if (other.value_ == 0) {
            DLOG(ERROR) << "Division by zero attempted in BigDecimal";
            throw std::invalid_argument("Division by zero is not allowed.");
        }
        const auto result = BigDecimal((value_ / other.value_).convert_to<std::string>());
        DLOG(INFO) << fmt::format("BigDecimal division: {} / {} = {}", 
            value_.convert_to<std::string>(), other.value_.convert_to<std::string>(), 
            (value_ / other.value_).convert_to<std::string>());
        return result;
    }

    auto BigDecimal::operator<=>(const BigDecimal &other) const noexcept -> std::strong_ordering {
        if (value_ < other.value_) {
            return std::strong_ordering::less;
        }
        if (value_ > other.value_) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    auto BigDecimal::operator==(const BigDecimal &other) const noexcept -> bool {
        return value_ == other.value_;
    }
}
