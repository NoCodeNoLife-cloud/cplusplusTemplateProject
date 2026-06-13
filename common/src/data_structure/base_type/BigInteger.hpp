/**
 * @file BigInteger.hpp
 * @brief Arbitrary-precision integer arithmetic (non-negative)
 * @details Implements arbitrary-precision non-negative integers using a
 *          std::vector of base-10 digits.  Supports addition, subtraction,
 *          multiplication, division, modulo, exponentiation, and comparison.
 *          Used internally by BigDecimal and for cryptographic computations.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <compare>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

namespace common::data_structure::base_type
{
    /// @brief A class representing a big integer using boost::multiprecision::cpp_int
    class BigInteger
    {
    public:
        /// @brief Default constructor, initializes to zero
        BigInteger() ;

        /// @brief Constructs a BigInteger from a string representation
        /// @param str The string representation of the integer
        explicit BigInteger(const std::string& str);

        /// @brief Constructs a BigInteger from an int64_t value
        /// @param num The int64_t value to convert
        explicit BigInteger(int64_t num) ;

        /// @brief Addition operator
        /// @param other The BigInteger to add
        /// @return The result of the addition
        [[nodiscard]] BigInteger operator+(const BigInteger& other) const ;

        /// @brief Subtraction operator
        /// @param other The BigInteger to subtract
        /// @return The result of the subtraction
        [[nodiscard]] BigInteger operator-(const BigInteger& other) const ;

        /// @brief Multiplication operator
        /// @param other The BigInteger to multiply
        /// @return The result of the multiplication
        [[nodiscard]] BigInteger operator*(const BigInteger& other) const ;

        /// @brief Division operator
        /// @param other The BigInteger to divide by
        /// @return The result of the division
        /// @throws std::invalid_argument If attempting to divide by zero
        [[nodiscard]] BigInteger operator/(const BigInteger& other) const;

        /// @brief Modulus operator
        /// @param other The BigInteger to modulo by
        /// @return The result of the modulus operation
        /// @throws std::invalid_argument If attempting to modulo by zero
        [[nodiscard]] BigInteger operator%(const BigInteger& other) const;

        /// @brief Three-way comparison operator
        /// @param other The BigInteger to compare with
        /// @return The result of the comparison
        [[nodiscard]] std::strong_ordering operator<=>(const BigInteger& other) const ;

        /// @brief Equality comparison operator
        /// @param other The BigInteger to compare with
        /// @return true if the values are equal, false otherwise
        [[nodiscard]] bool operator==(const BigInteger& other) const ;

    private:
        /// @brief Internal constructor from cpp_int (avoids string conversion overhead)
        /// @param value The cpp_int value to wrap
        explicit BigInteger(boost::multiprecision::cpp_int value)  : value_(std::move(value))
        {
        }

        boost::multiprecision::cpp_int value_{};
    };
}