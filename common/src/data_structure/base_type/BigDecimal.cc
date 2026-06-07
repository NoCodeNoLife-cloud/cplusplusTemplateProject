/**
 * @file BigDecimal.cc
 * @brief BigDecimal class implementation
 * @details This file contains the implementation of the BigDecimal class methods for Base type classes providing fundamental object functionality.
 */

#include "BigDecimal.hpp"

#include <compare>
#include <stdexcept>
#include <string>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::data_structure::base_type
{
    BigDecimal::BigDecimal(const std::string& str) : value_(0)
    {
        if (str.empty())
        {
            DLOG(WARNING) << "Cannot construct BigDecimal from empty string";
            throw std::invalid_argument("Cannot construct BigDecimal from empty string");
        }
        value_ = boost::multiprecision::cpp_dec_float_100(str);
    }

    BigDecimal::BigDecimal(double num) : value_(num)
    {
        DLOG(WARNING) << "BigDecimal constructed from double; precision may be lost, use string constructor instead";
    }

    BigDecimal BigDecimal::operator+(const BigDecimal& other) const
    {
        return BigDecimal(value_ + other.value_);
    }

    BigDecimal BigDecimal::operator-(const BigDecimal& other) const
    {
        return BigDecimal(value_ - other.value_);
    }

    BigDecimal BigDecimal::operator*(const BigDecimal& other) const
    {
        return BigDecimal(value_ * other.value_);
    }

    BigDecimal BigDecimal::operator/(const BigDecimal& other) const
    {
        if (other.value_ == 0)
        {
            DLOG(ERROR) << "Cannot divide BigDecimal by zero";
            throw std::invalid_argument("Cannot divide BigDecimal by zero");
        }
        return BigDecimal(value_ / other.value_);
    }

    std::strong_ordering BigDecimal::operator<=>(const BigDecimal& other) const
    {
        if (value_ < other.value_)
        {
            return std::strong_ordering::less;
        }
        if (value_ > other.value_)
        {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    bool BigDecimal::operator==(const BigDecimal& other) const
    {
        return value_ == other.value_;
    }
}
