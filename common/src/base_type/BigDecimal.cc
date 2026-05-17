/**
 * @file BigDecimal.cc
 * @brief BigDecimal class implementation
 * @details This file contains the implementation of the BigDecimal class methods for Base type classes providing fundamental object functionality.
 */

#include "src/base_type/BigDecimal.hpp"

#include <fmt/format.h>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <stdexcept>
#include <string>
#include <compare>
#include <glog/logging.h>

namespace common::base_type {
BigDecimal::BigDecimal(const std::string& str) : value_(0) {
    if (str.empty()) {
        DLOG(WARNING) << "Cannot construct BigDecimal from empty string";
        throw std::invalid_argument("Cannot construct BigDecimal from empty string");
    }
    value_ = boost::multiprecision::cpp_dec_float_100(str);
}

BigDecimal::BigDecimal(const double num) : value_(num) {
}

auto BigDecimal::operator+(const BigDecimal& other) const noexcept -> BigDecimal {
    return BigDecimal(value_ + other.value_);
}

auto BigDecimal::operator-(const BigDecimal& other) const noexcept -> BigDecimal {
    return BigDecimal(value_ - other.value_);
}

auto BigDecimal::operator*(const BigDecimal& other) const noexcept -> BigDecimal {
    return BigDecimal(value_ * other.value_);
}

auto BigDecimal::operator/(const BigDecimal& other) const -> BigDecimal {
    if (other.value_ == 0) {
        DLOG(WARNING) << "Division by zero attempted in BigDecimal";
        throw std::invalid_argument("Division by zero is not allowed.");
    }
    return BigDecimal(value_ / other.value_);
}

auto BigDecimal::operator<=>(const BigDecimal& other) const noexcept -> std::strong_ordering {
    if (value_ < other.value_) {
        return std::strong_ordering::less;
    }
    if (value_ > other.value_) {
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}

auto BigDecimal::operator==(const BigDecimal& other) const noexcept -> bool {
    return value_ == other.value_;
}
}