/**
 * @file BigDecimal.hpp
 * @brief Arbitrary-precision decimal arithmetic with rounding control
 * @details A decimal number implementation for precise financial and
 *          scientific calculations where floating-point rounding errors
 *          are unacceptable.  Stores the value as an unscaled BigInteger
 *          with a scale (number of fractional digits).  Supports arithmetic
 *          operations, comparison, rounding modes, and string conversion.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * BigDecimal a("123.45");
 * BigDecimal b("67.89");
 * BigDecimal sum = a + b;
 * @endcode
 */

#pragma once
#include <compare>
#include <string>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace common::data_structure::base_type
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
        /// @param num The double value to convert (precision may be lost; prefer string construction)
        [[deprecated("Use string constructor to avoid double precision loss")]]
        explicit BigDecimal(double num);

        /// @brief Addition operator
        /// @param other The BigDecimal to add
        /// @return The sum of this BigDecimal and other
        [[nodiscard]] BigDecimal operator+(const BigDecimal& other) const;

        /// @brief Subtraction operator
        /// @param other The BigDecimal to subtract
        /// @return The difference of this BigDecimal and other
        [[nodiscard]] BigDecimal operator-(const BigDecimal& other) const;

        /// @brief Multiplication operator
        /// @param other The BigDecimal to multiply
        /// @return The product of this BigDecimal and other
        [[nodiscard]] BigDecimal operator*(const BigDecimal& other) const;

        /// @brief Division operator
        /// @param other The BigDecimal to divide by
        /// @return The quotient of this BigDecimal and other
        /// @throws std::invalid_argument If attempting to divide by zero
        [[nodiscard]] BigDecimal operator/(const BigDecimal& other) const;

        /// @brief Three-way comparison operator
        /// @param other The BigDecimal to compare with
        /// @return std::strong_ordering result of the comparison
        [[nodiscard]] std::strong_ordering operator<=>(const BigDecimal& other) const;

        /// @brief Equality comparison operator
        /// @param other The BigDecimal to compare with
        /// @return true if the values are equal, false otherwise
        [[nodiscard]] bool operator==(const BigDecimal& other) const;

    private:
        /// @brief Internal constructor from cpp_dec_float_100 (avoids string conversion overhead)
        /// @param value The cpp_dec_float_100 value to wrap
        explicit BigDecimal(boost::multiprecision::cpp_dec_float_100 value) : value_(std::move(value))
        {
        }

        boost::multiprecision::cpp_dec_float_100 value_{};
    };
}
