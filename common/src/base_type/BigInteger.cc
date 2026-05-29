/**
 * @file BigInteger.cc
 * @brief BigInteger class implementation
 * @details This file contains the implementation of the BigInteger class methods for Base type classes providing fundamental object functionality.
 */

#include "src/base_type/BigInteger.hpp"

#include <fmt/format.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include <compare>
#include <stdexcept>
#include <glog/logging.h>

namespace common::base_type
{
    BigInteger::BigInteger() noexcept : value_(0)
    {
    }

    BigInteger::BigInteger(const std::string& str) : value_(0)
    {
        if (str.empty())
        {
            DLOG(WARNING) << "Cannot construct BigInteger from empty string";
            throw std::invalid_argument("Cannot construct BigInteger from empty string");
        }
        value_ = boost::multiprecision::cpp_int(str);
    }

    BigInteger::BigInteger(const int64_t num) noexcept : value_(num)
    {
    }

    BigInteger BigInteger::operator+(const BigInteger& other) const noexcept
    {
        return BigInteger(value_ + other.value_);
    }

    BigInteger BigInteger::operator-(const BigInteger& other) const noexcept
    {
        return BigInteger(value_ - other.value_);
    }

    BigInteger BigInteger::operator*(const BigInteger& other) const noexcept
    {
        return BigInteger(value_ * other.value_);
    }

    BigInteger BigInteger::operator/(const BigInteger& other) const
    {
        if (other.value_ == 0)
        {
            DLOG(WARNING) << "Division by zero attempted in BigInteger";
            throw std::invalid_argument("Division by zero");
        }
        return BigInteger(value_ / other.value_);
    }

    BigInteger BigInteger::operator%(const BigInteger& other) const
    {
        if (other.value_ == 0)
        {
            DLOG(WARNING) << "Modulo by zero attempted in BigInteger";
            throw std::invalid_argument("Modulo by zero");
        }
        return BigInteger(value_ % other.value_);
    }

    std::strong_ordering BigInteger::operator<=>(const BigInteger& other) const noexcept
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

    bool BigInteger::operator==(const BigInteger& other) const noexcept
    {
        return value_ == other.value_;
    }

    BigInteger BigInteger::fromString(const std::string& str)
    {
        return BigInteger(str);
    }

    BigInteger BigInteger::fromInt(const int64_t num) noexcept
    {
        return BigInteger(num);
    }
}
