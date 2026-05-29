/**
 * @file BigDecimal.hpp
 * @brief BigDecimal class declaration
 * @details This header defines the BigDecimal class that provides functionality for Base type classes providing fundamental object functionality.
 */

#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>
#include <compare>

namespace common::base_type
{
    /// @brief A class for high-precision decimal arithmetic
    /// @details This class uses boost::multiprecision::cpp_dec_float_100 for underlying storage
    ///          to provide high precision decimal operations.
    class BigDecimal
    {
    public:
        /// @brief Constructs a BigDecimal from a string representation
        /// @param str The string representation of the decimal number
        explicit BigDecimal(const std::string& str);

        /// @brief Constructs a BigDecimal from a double value
        /// @param num The double value to convert
        explicit BigDecimal(double num);

        /// @brief Addition operator
        /// @param other The BigDecimal to add
        /// @return The sum of this BigDecimal and other
        [[nodiscard]] BigDecimal operator+(const BigDecimal& other) const noexcept;

        /// @brief Subtraction operator
        /// @param other The BigDecimal to subtract
        /// @return The difference of this BigDecimal and other
        [[nodiscard]] BigDecimal operator-(const BigDecimal& other) const noexcept;

        /// @brief Multiplication operator
        /// @param other The BigDecimal to multiply
        /// @return The product of this BigDecimal and other
        [[nodiscard]] BigDecimal operator*(const BigDecimal& other) const noexcept;

        /// @brief Division operator
        /// @param other The BigDecimal to divide by
        /// @return The quotient of this BigDecimal and other
        /// @throws std::invalid_argument If attempting to divide by zero
        [[nodiscard]] BigDecimal operator/(const BigDecimal& other) const;

        /// @brief Three-way comparison operator
        /// @param other The BigDecimal to compare with
        /// @return std::strong_ordering result of the comparison
        [[nodiscard]] std::strong_ordering operator<=>(const BigDecimal& other) const noexcept;

        /// @brief Equality comparison operator
        /// @param other The BigDecimal to compare with
        /// @return true if the values are equal, false otherwise
        [[nodiscard]] bool operator==(const BigDecimal& other) const noexcept;

    private:
        /// @brief Internal constructor from cpp_dec_float_100 (avoids string conversion overhead)
        /// @param value The cpp_dec_float_100 value to wrap
        explicit BigDecimal(boost::multiprecision::cpp_dec_float_100 value) noexcept : value_(std::move(value))
        {
        }

        boost::multiprecision::cpp_dec_float_100 value_{};
    };
}
